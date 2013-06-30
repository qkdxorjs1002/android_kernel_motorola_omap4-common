/*
 * drivers/cpufreq/cpufreq_interactive.c
 *
 * Copyright (C) 2010 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: Mike Chan (mike@android.com)
 *
 */

#include &lt;linux/cpu.h&gt;
#include &lt;linux/cpumask.h&gt;
#include &lt;linux/cpufreq.h&gt;
#include &lt;linux/module.h&gt;
#include &lt;linux/moduleparam.h&gt;
#include &lt;linux/rwsem.h&gt;
#include &lt;linux/sched.h&gt;
#include &lt;linux/tick.h&gt;
#include &lt;linux/time.h&gt;
#include &lt;linux/timer.h&gt;
#include &lt;linux/workqueue.h&gt;
#include &lt;linux/kthread.h&gt;
#include &lt;linux/slab.h&gt;
#include &lt;asm/cputime.h&gt;

#define CREATE_TRACE_POINTS
#include &lt;trace/events/cpufreq_interactive.h&gt;

static int active_count;

struct cpufreq_interactive_cpuinfo {
	struct timer_list cpu_timer;
	struct timer_list cpu_slack_timer;
	spinlock_t load_lock; /* protects the next 4 fields */
	u64 time_in_idle;
	u64 time_in_idle_timestamp;
	u64 cputime_speedadj;
	u64 cputime_speedadj_timestamp;
	struct cpufreq_policy *policy;
	struct cpufreq_frequency_table *freq_table;
	unsigned int target_freq;
	unsigned int floor_freq;
	u64 floor_validate_time;
	u64 hispeed_validate_time;
	struct rw_semaphore enable_sem;
	int governor_enabled;
};

static DEFINE_PER_CPU(struct cpufreq_interactive_cpuinfo, cpuinfo);

/* realtime thread handles frequency scaling */
static struct task_struct *speedchange_task;
static cpumask_t speedchange_cpumask;
static spinlock_t speedchange_cpumask_lock;
static struct mutex gov_lock;

/* Hi speed to bump to from lo speed when load burst (default max) */
static unsigned int hispeed_freq;

/* Go to hi speed when CPU load at or above this value. */
#define DEFAULT_GO_HISPEED_LOAD 99
static unsigned long go_hispeed_load = DEFAULT_GO_HISPEED_LOAD;

/* Target load.  Lower values result in higher CPU speeds. */
#define DEFAULT_TARGET_LOAD 90
static unsigned int default_target_loads[] = {DEFAULT_TARGET_LOAD};
static spinlock_t target_loads_lock;
static unsigned int *target_loads = default_target_loads;
static int ntarget_loads = ARRAY_SIZE(default_target_loads);

/*
 * The minimum amount of time to spend at a frequency before we can ramp down.
 */
#define DEFAULT_MIN_SAMPLE_TIME (80 * USEC_PER_MSEC)
static unsigned long min_sample_time = DEFAULT_MIN_SAMPLE_TIME;

/*
 * The sample rate of the timer used to increase frequency
 */
#define DEFAULT_TIMER_RATE (20 * USEC_PER_MSEC)
static unsigned long timer_rate = DEFAULT_TIMER_RATE;

/*
 * Wait this long before raising speed above hispeed, by default a single
 * timer interval.
 */
#define DEFAULT_ABOVE_HISPEED_DELAY DEFAULT_TIMER_RATE
static unsigned long above_hispeed_delay_val = DEFAULT_ABOVE_HISPEED_DELAY;

/* Non-zero means indefinite speed boost active */
static int boost_val;
/* Duration of a boot pulse in usecs */
static int boostpulse_duration_val = DEFAULT_MIN_SAMPLE_TIME;
/* End time of boost pulse in ktime converted to usecs */
static u64 boostpulse_endtime;

/*
 * Max additional time to wait in idle, beyond timer_rate, at speeds above
 * minimum before wakeup to reduce speed, or -1 if unnecessary.
 */
#define DEFAULT_TIMER_SLACK (4 * DEFAULT_TIMER_RATE)
static int timer_slack_val = DEFAULT_TIMER_SLACK;

static int cpufreq_governor_interactive(struct cpufreq_policy *policy,
		unsigned int event);

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_INTERACTIVE
static
#endif
struct cpufreq_governor cpufreq_gov_interactive = {
	.name = &quot;interactive&quot;,
	.governor = cpufreq_governor_interactive,
	.max_transition_latency = 10000000,
	.owner = THIS_MODULE,
};

static void cpufreq_interactive_timer_resched(
	struct cpufreq_interactive_cpuinfo *pcpu)
{
	unsigned long expires;
	unsigned long flags;

	spin_lock_irqsave(&amp;pcpu-&gt;load_lock, flags);
	pcpu-&gt;time_in_idle =
		get_cpu_idle_time_us(smp_processor_id(),
				     &amp;pcpu-&gt;time_in_idle_timestamp);
	pcpu-&gt;cputime_speedadj = 0;
	pcpu-&gt;cputime_speedadj_timestamp = pcpu-&gt;time_in_idle_timestamp;
	expires = jiffies + usecs_to_jiffies(timer_rate);
	mod_timer_pinned(&amp;pcpu-&gt;cpu_timer, expires);

	if (timer_slack_val &gt;= 0 &amp;&amp; pcpu-&gt;target_freq &gt; pcpu-&gt;policy-&gt;min) {
		expires += usecs_to_jiffies(timer_slack_val);
		mod_timer_pinned(&amp;pcpu-&gt;cpu_slack_timer, expires);
	}

	spin_unlock_irqrestore(&amp;pcpu-&gt;load_lock, flags);
}

static unsigned int freq_to_targetload(unsigned int freq)
{
	int i;
	unsigned int ret;
	unsigned long flags;

	spin_lock_irqsave(&amp;target_loads_lock, flags);

	for (i = 0; i &lt; ntarget_loads - 1 &amp;&amp; freq &gt;= target_loads[i+1]; i += 2)
		;

	ret = target_loads[i];
	spin_unlock_irqrestore(&amp;target_loads_lock, flags);
	return ret;
}

/*
 * If increasing frequencies never map to a lower target load then
 * choose_freq() will find the minimum frequency that does not exceed its
 * target load given the current load.
 */

static unsigned int choose_freq(
	struct cpufreq_interactive_cpuinfo *pcpu, unsigned int loadadjfreq)
{
	unsigned int freq = pcpu-&gt;policy-&gt;cur;
	unsigned int prevfreq, freqmin, freqmax;
	unsigned int tl;
	int index;

	freqmin = 0;
	freqmax = UINT_MAX;

	do {
		prevfreq = freq;
		tl = freq_to_targetload(freq);

		/*
		 * Find the lowest frequency where the computed load is less
		 * than or equal to the target load.
		 */

		if (cpufreq_frequency_table_target(
			    pcpu-&gt;policy, pcpu-&gt;freq_table, loadadjfreq / tl,
			    CPUFREQ_RELATION_L, &amp;index))
			break;
		freq = pcpu-&gt;freq_table[index].frequency;

		if (freq &gt; prevfreq) {
			/* The previous frequency is too low. */
			freqmin = prevfreq;

			if (freq &gt;= freqmax) {
				/*
				 * Find the highest frequency that is less
				 * than freqmax.
				 */
				if (cpufreq_frequency_table_target(
					    pcpu-&gt;policy, pcpu-&gt;freq_table,
					    freqmax - 1, CPUFREQ_RELATION_H,
					    &amp;index))
					break;
				freq = pcpu-&gt;freq_table[index].frequency;

				if (freq == freqmin) {
					/*
					 * The first frequency below freqmax
					 * has already been found to be too
					 * low.  freqmax is the lowest speed
					 * we found that is fast enough.
					 */
					freq = freqmax;
					break;
				}
			}
		} else if (freq &lt; prevfreq) {
			/* The previous frequency is high enough. */
			freqmax = prevfreq;

			if (freq &lt;= freqmin) {
				/*
				 * Find the lowest frequency that is higher
				 * than freqmin.
				 */
				if (cpufreq_frequency_table_target(
					    pcpu-&gt;policy, pcpu-&gt;freq_table,
					    freqmin + 1, CPUFREQ_RELATION_L,
					    &amp;index))
					break;
				freq = pcpu-&gt;freq_table[index].frequency;

				/*
				 * If freqmax is the first frequency above
				 * freqmin then we have already found that
				 * this speed is fast enough.
				 */
				if (freq == freqmax)
					break;
			}
		}

		/* If same frequency chosen as previous then done. */
	} while (freq != prevfreq);

	return freq;
}

static u64 update_load(int cpu)
{
	struct cpufreq_interactive_cpuinfo *pcpu = &amp;per_cpu(cpuinfo, cpu);
	u64 now;
	u64 now_idle;
	unsigned int delta_idle;
	unsigned int delta_time;
	u64 active_time;

	now_idle = get_cpu_idle_time_us(cpu, &amp;now);
	delta_idle = (unsigned int)(now_idle - pcpu-&gt;time_in_idle);
	delta_time = (unsigned int)(now - pcpu-&gt;time_in_idle_timestamp);

	if (delta_time &lt;= delta_idle)
		active_time = 0;
	else
		active_time = delta_time - delta_idle;

	pcpu-&gt;cputime_speedadj += active_time * pcpu-&gt;policy-&gt;cur;

	pcpu-&gt;time_in_idle = now_idle;
	pcpu-&gt;time_in_idle_timestamp = now;
	return now;
}

static void cpufreq_interactive_timer(unsigned long data)
{
	u64 now;
	unsigned int delta_time;
	u64 cputime_speedadj;
	int cpu_load;
	struct cpufreq_interactive_cpuinfo *pcpu =
		&amp;per_cpu(cpuinfo, data);
	unsigned int new_freq;
	unsigned int loadadjfreq;
	unsigned int index;
	unsigned long flags;
	bool boosted;

	if (!down_read_trylock(&amp;pcpu-&gt;enable_sem))
		return;
	if (!pcpu-&gt;governor_enabled)
		goto exit;

	spin_lock_irqsave(&amp;pcpu-&gt;load_lock, flags);
	now = update_load(data);
	delta_time = (unsigned int)(now - pcpu-&gt;cputime_speedadj_timestamp);
	cputime_speedadj = pcpu-&gt;cputime_speedadj;
	spin_unlock_irqrestore(&amp;pcpu-&gt;load_lock, flags);

	if (WARN_ON_ONCE(!delta_time))
		goto rearm;

	do_div(cputime_speedadj, delta_time);
	loadadjfreq = (unsigned int)cputime_speedadj * 100;
	cpu_load = loadadjfreq / pcpu-&gt;target_freq;
	boosted = boost_val || now &lt; boostpulse_endtime;

	if (cpu_load &gt;= go_hispeed_load || boosted) {
		if (pcpu-&gt;target_freq &lt; hispeed_freq) {
			new_freq = hispeed_freq;
		} else {
			new_freq = choose_freq(pcpu, loadadjfreq);

			if (new_freq &lt; hispeed_freq)
				new_freq = hispeed_freq;
		}
	} else {
		new_freq = choose_freq(pcpu, loadadjfreq);
	}

	if (pcpu-&gt;target_freq &gt;= hispeed_freq &amp;&amp;
	    new_freq &gt; pcpu-&gt;target_freq &amp;&amp;
	    now - pcpu-&gt;hispeed_validate_time &lt; above_hispeed_delay_val) {
		trace_cpufreq_interactive_notyet(
			data, cpu_load, pcpu-&gt;target_freq,
			pcpu-&gt;policy-&gt;cur, new_freq);
		goto rearm;
	}

	pcpu-&gt;hispeed_validate_time = now;

	if (cpufreq_frequency_table_target(pcpu-&gt;policy, pcpu-&gt;freq_table,
					   new_freq, CPUFREQ_RELATION_L,
					   &amp;index))
		goto rearm;

	new_freq = pcpu-&gt;freq_table[index].frequency;

	/*
	 * Do not scale below floor_freq unless we have been at or above the
	 * floor frequency for the minimum sample time since last validated.
	 */
	if (new_freq &lt; pcpu-&gt;floor_freq) {
		if (now - pcpu-&gt;floor_validate_time &lt; min_sample_time) {
			trace_cpufreq_interactive_notyet(
				data, cpu_load, pcpu-&gt;target_freq,
				pcpu-&gt;policy-&gt;cur, new_freq);
			goto rearm;
		}
	}

	/*
	 * Update the timestamp for checking whether speed has been held at
	 * or above the selected frequency for a minimum of min_sample_time,
	 * if not boosted to hispeed_freq.  If boosted to hispeed_freq then we
	 * allow the speed to drop as soon as the boostpulse duration expires
	 * (or the indefinite boost is turned off).
	 */

	if (!boosted || new_freq &gt; hispeed_freq) {
		pcpu-&gt;floor_freq = new_freq;
		pcpu-&gt;floor_validate_time = now;
	}

	if (pcpu-&gt;target_freq == new_freq) {
		trace_cpufreq_interactive_already(
			data, cpu_load, pcpu-&gt;target_freq,
			pcpu-&gt;policy-&gt;cur, new_freq);
		goto rearm_if_notmax;
	}

	trace_cpufreq_interactive_target(data, cpu_load, pcpu-&gt;target_freq,
					 pcpu-&gt;policy-&gt;cur, new_freq);

	pcpu-&gt;target_freq = new_freq;
	spin_lock_irqsave(&amp;speedchange_cpumask_lock, flags);
	cpumask_set_cpu(data, &amp;speedchange_cpumask);
	spin_unlock_irqrestore(&amp;speedchange_cpumask_lock, flags);
	wake_up_process(speedchange_task);

rearm_if_notmax:
	/*
	 * Already set max speed and don&#39;t see a need to change that,
	 * wait until next idle to re-evaluate, don&#39;t need timer.
	 */
	if (pcpu-&gt;target_freq == pcpu-&gt;policy-&gt;max)
		goto exit;

rearm:
	if (!timer_pending(&amp;pcpu-&gt;cpu_timer))
		cpufreq_interactive_timer_resched(pcpu);

exit:
	up_read(&amp;pcpu-&gt;enable_sem);
	return;
}

static void cpufreq_interactive_idle_start(void)
{
	struct cpufreq_interactive_cpuinfo *pcpu =
		&amp;per_cpu(cpuinfo, smp_processor_id());
	int pending;

	if (!down_read_trylock(&amp;pcpu-&gt;enable_sem))
		return;
	if (!pcpu-&gt;governor_enabled) {
		up_read(&amp;pcpu-&gt;enable_sem);
		return;
	}

	pending = timer_pending(&amp;pcpu-&gt;cpu_timer);

	if (pcpu-&gt;target_freq != pcpu-&gt;policy-&gt;min) {
		/*
		 * Entering idle while not at lowest speed.  On some
		 * platforms this can hold the other CPU(s) at that speed
		 * even though the CPU is idle. Set a timer to re-evaluate
		 * speed so this idle CPU doesn&#39;t hold the other CPUs above
		 * min indefinitely.  This should probably be a quirk of
		 * the CPUFreq driver.
		 */
		if (!pending)
			cpufreq_interactive_timer_resched(pcpu);
	}

	up_read(&amp;pcpu-&gt;enable_sem);
}

static void cpufreq_interactive_idle_end(void)
{
	struct cpufreq_interactive_cpuinfo *pcpu =
		&amp;per_cpu(cpuinfo, smp_processor_id());

	if (!down_read_trylock(&amp;pcpu-&gt;enable_sem))
		return;
	if (!pcpu-&gt;governor_enabled) {
		up_read(&amp;pcpu-&gt;enable_sem);
		return;
	}

	/* Arm the timer for 1-2 ticks later if not already. */
	if (!timer_pending(&amp;pcpu-&gt;cpu_timer)) {
		cpufreq_interactive_timer_resched(pcpu);
	} else if (time_after_eq(jiffies, pcpu-&gt;cpu_timer.expires)) {
		del_timer(&amp;pcpu-&gt;cpu_timer);
		del_timer(&amp;pcpu-&gt;cpu_slack_timer);
		cpufreq_interactive_timer(smp_processor_id());
	}

	up_read(&amp;pcpu-&gt;enable_sem);
}

static int cpufreq_interactive_speedchange_task(void *data)
{
	unsigned int cpu;
	cpumask_t tmp_mask;
	unsigned long flags;
	struct cpufreq_interactive_cpuinfo *pcpu;

	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);
		spin_lock_irqsave(&amp;speedchange_cpumask_lock, flags);

		if (cpumask_empty(&amp;speedchange_cpumask)) {
			spin_unlock_irqrestore(&amp;speedchange_cpumask_lock,
					       flags);
			schedule();

			if (kthread_should_stop())
				break;

			spin_lock_irqsave(&amp;speedchange_cpumask_lock, flags);
		}

		set_current_state(TASK_RUNNING);
		tmp_mask = speedchange_cpumask;
		cpumask_clear(&amp;speedchange_cpumask);
		spin_unlock_irqrestore(&amp;speedchange_cpumask_lock, flags);

		for_each_cpu(cpu, &amp;tmp_mask) {
			unsigned int j;
			unsigned int max_freq = 0;

			pcpu = &amp;per_cpu(cpuinfo, cpu);
			if (!down_read_trylock(&amp;pcpu-&gt;enable_sem))
				continue;
			if (!pcpu-&gt;governor_enabled) {
				up_read(&amp;pcpu-&gt;enable_sem);
				continue;
			}

			for_each_cpu(j, pcpu-&gt;policy-&gt;cpus) {
				struct cpufreq_interactive_cpuinfo *pjcpu =
					&amp;per_cpu(cpuinfo, j);

				if (pjcpu-&gt;target_freq &gt; max_freq)
					max_freq = pjcpu-&gt;target_freq;
			}

			if (max_freq != pcpu-&gt;policy-&gt;cur)
				__cpufreq_driver_target(pcpu-&gt;policy,
							max_freq,
							CPUFREQ_RELATION_H);
			trace_cpufreq_interactive_setspeed(cpu,
						     pcpu-&gt;target_freq,
						     pcpu-&gt;policy-&gt;cur);

			up_read(&amp;pcpu-&gt;enable_sem);
		}
	}

	return 0;
}

static void cpufreq_interactive_boost(void)
{
	int i;
	int anyboost = 0;
	unsigned long flags;
	struct cpufreq_interactive_cpuinfo *pcpu;

	spin_lock_irqsave(&amp;speedchange_cpumask_lock, flags);

	for_each_online_cpu(i) {
		pcpu = &amp;per_cpu(cpuinfo, i);

		if (pcpu-&gt;target_freq &lt; hispeed_freq) {
			pcpu-&gt;target_freq = hispeed_freq;
			cpumask_set_cpu(i, &amp;speedchange_cpumask);
			pcpu-&gt;hispeed_validate_time =
				ktime_to_us(ktime_get());
			anyboost = 1;
		}

		/*
		 * Set floor freq and (re)start timer for when last
		 * validated.
		 */

		pcpu-&gt;floor_freq = hispeed_freq;
		pcpu-&gt;floor_validate_time = ktime_to_us(ktime_get());
	}

	spin_unlock_irqrestore(&amp;speedchange_cpumask_lock, flags);

	if (anyboost)
		wake_up_process(speedchange_task);
}

static int cpufreq_interactive_notifier(
	struct notifier_block *nb, unsigned long val, void *data)
{
	struct cpufreq_freqs *freq = data;
	struct cpufreq_interactive_cpuinfo *pcpu;
	int cpu;
	unsigned long flags;

	if (val == CPUFREQ_POSTCHANGE) {
		pcpu = &amp;per_cpu(cpuinfo, freq-&gt;cpu);
		if (!down_read_trylock(&amp;pcpu-&gt;enable_sem))
			return 0;
		if (!pcpu-&gt;governor_enabled) {
			up_read(&amp;pcpu-&gt;enable_sem);
			return 0;
		}

		for_each_cpu(cpu, pcpu-&gt;policy-&gt;cpus) {
			struct cpufreq_interactive_cpuinfo *pjcpu =
				&amp;per_cpu(cpuinfo, cpu);
			spin_lock_irqsave(&amp;pjcpu-&gt;load_lock, flags);
			update_load(cpu);
			spin_unlock_irqrestore(&amp;pjcpu-&gt;load_lock, flags);
		}

		up_read(&amp;pcpu-&gt;enable_sem);
	}
	return 0;
}

static struct notifier_block cpufreq_notifier_block = {
	.notifier_call = cpufreq_interactive_notifier,
};

static ssize_t show_target_loads(
	struct kobject *kobj, struct attribute *attr, char *buf)
{
	int i;
	ssize_t ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&amp;target_loads_lock, flags);

	for (i = 0; i &lt; ntarget_loads; i++)
		ret += sprintf(buf + ret, &quot;%u%s&quot;, target_loads[i],
			       i &amp; 0x1 ? &quot;:&quot; : &quot; &quot;);

	ret += sprintf(buf + ret, &quot;\n&quot;);
	spin_unlock_irqrestore(&amp;target_loads_lock, flags);
	return ret;
}

static ssize_t store_target_loads(
	struct kobject *kobj, struct attribute *attr, const char *buf,
	size_t count)
{
	int ret;
	const char *cp;
	unsigned int *new_target_loads = NULL;
	int ntokens = 1;
	int i;
	unsigned long flags;

	cp = buf;
	while ((cp = strpbrk(cp + 1, &quot; :&quot;)))
		ntokens++;

	if (!(ntokens &amp; 0x1))
		goto err_inval;

	new_target_loads = kmalloc(ntokens * sizeof(unsigned int), GFP_KERNEL);
	if (!new_target_loads) {
		ret = -ENOMEM;
		goto err;
	}

	cp = buf;
	i = 0;
	while (i &lt; ntokens) {
		if (sscanf(cp, &quot;%u&quot;, &amp;new_target_loads[i++]) != 1)
			goto err_inval;

		cp = strpbrk(cp, &quot; :&quot;);
		if (!cp)
			break;
		cp++;
	}

	if (i != ntokens)
		goto err_inval;

	spin_lock_irqsave(&amp;target_loads_lock, flags);
	if (target_loads != default_target_loads)
		kfree(target_loads);
	target_loads = new_target_loads;
	ntarget_loads = ntokens;
	spin_unlock_irqrestore(&amp;target_loads_lock, flags);
	return count;

err_inval:
	ret = -EINVAL;
err:
	kfree(new_target_loads);
	return ret;
}

static struct global_attr target_loads_attr =
	__ATTR(target_loads, S_IRUGO | S_IWUSR,
		show_target_loads, store_target_loads);

static ssize_t show_hispeed_freq(struct kobject *kobj,
				 struct attribute *attr, char *buf)
{
	return sprintf(buf, &quot;%u\n&quot;, hispeed_freq);
}

static ssize_t store_hispeed_freq(struct kobject *kobj,
				  struct attribute *attr, const char *buf,
				  size_t count)
{
	int ret;
	long unsigned int val;

	ret = strict_strtoul(buf, 0, &amp;val);
	if (ret &lt; 0)
		return ret;
	hispeed_freq = val;
	return count;
}

static struct global_attr hispeed_freq_attr = __ATTR(hispeed_freq, 0644,
		show_hispeed_freq, store_hispeed_freq);


static ssize_t show_go_hispeed_load(struct kobject *kobj,
				     struct attribute *attr, char *buf)
{
	return sprintf(buf, &quot;%lu\n&quot;, go_hispeed_load);
}

static ssize_t store_go_hispeed_load(struct kobject *kobj,
			struct attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = strict_strtoul(buf, 0, &amp;val);
	if (ret &lt; 0)
		return ret;
	go_hispeed_load = val;
	return count;
}

static struct global_attr go_hispeed_load_attr = __ATTR(go_hispeed_load, 0644,
		show_go_hispeed_load, store_go_hispeed_load);

static ssize_t show_min_sample_time(struct kobject *kobj,
				struct attribute *attr, char *buf)
{
	return sprintf(buf, &quot;%lu\n&quot;, min_sample_time);
}

static ssize_t store_min_sample_time(struct kobject *kobj,
			struct attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = strict_strtoul(buf, 0, &amp;val);
	if (ret &lt; 0)
		return ret;
	min_sample_time = val;
	return count;
}

static struct global_attr min_sample_time_attr = __ATTR(min_sample_time, 0644,
		show_min_sample_time, store_min_sample_time);

static ssize_t show_above_hispeed_delay(struct kobject *kobj,
					struct attribute *attr, char *buf)
{
	return sprintf(buf, &quot;%lu\n&quot;, above_hispeed_delay_val);
}

static ssize_t store_above_hispeed_delay(struct kobject *kobj,
					 struct attribute *attr,
					 const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = strict_strtoul(buf, 0, &amp;val);
	if (ret &lt; 0)
		return ret;
	above_hispeed_delay_val = val;
	return count;
}

define_one_global_rw(above_hispeed_delay);

static ssize_t show_timer_rate(struct kobject *kobj,
			struct attribute *attr, char *buf)
{
	return sprintf(buf, &quot;%lu\n&quot;, timer_rate);
}

static ssize_t store_timer_rate(struct kobject *kobj,
			struct attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = strict_strtoul(buf, 0, &amp;val);
	if (ret &lt; 0)
		return ret;
	timer_rate = val;
	return count;
}

static struct global_attr timer_rate_attr = __ATTR(timer_rate, 0644,
		show_timer_rate, store_timer_rate);

static ssize_t show_timer_slack(
	struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, &quot;%d\n&quot;, timer_slack_val);
}

static ssize_t store_timer_slack(
	struct kobject *kobj, struct attribute *attr, const char *buf,
	size_t count)
{
	int ret;
	unsigned long val;

	ret = kstrtol(buf, 10, &amp;val);
	if (ret &lt; 0)
		return ret;

	timer_slack_val = val;
	return count;
}

define_one_global_rw(timer_slack);

static ssize_t show_boost(struct kobject *kobj, struct attribute *attr,
			  char *buf)
{
	return sprintf(buf, &quot;%d\n&quot;, boost_val);
}

static ssize_t store_boost(struct kobject *kobj, struct attribute *attr,
			   const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = kstrtoul(buf, 0, &amp;val);
	if (ret &lt; 0)
		return ret;

	boost_val = val;

	if (boost_val) {
		trace_cpufreq_interactive_boost(&quot;on&quot;);
		cpufreq_interactive_boost();
	} else {
		trace_cpufreq_interactive_unboost(&quot;off&quot;);
	}

	return count;
}

define_one_global_rw(boost);

static ssize_t store_boostpulse(struct kobject *kobj, struct attribute *attr,
				const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = kstrtoul(buf, 0, &amp;val);
	if (ret &lt; 0)
		return ret;

	boostpulse_endtime = ktime_to_us(ktime_get()) + boostpulse_duration_val;
	trace_cpufreq_interactive_boost(&quot;pulse&quot;);
	cpufreq_interactive_boost();
	return count;
}

static struct global_attr boostpulse =
	__ATTR(boostpulse, 0200, NULL, store_boostpulse);

static ssize_t show_boostpulse_duration(
	struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, &quot;%d\n&quot;, boostpulse_duration_val);
}

static ssize_t store_boostpulse_duration(
	struct kobject *kobj, struct attribute *attr, const char *buf,
	size_t count)
{
	int ret;
	unsigned long val;

	ret = kstrtoul(buf, 0, &amp;val);
	if (ret &lt; 0)
		return ret;

	boostpulse_duration_val = val;
	return count;
}

define_one_global_rw(boostpulse_duration);

static struct attribute *interactive_attributes[] = {
	&amp;target_loads_attr.attr,
	&amp;hispeed_freq_attr.attr,
	&amp;go_hispeed_load_attr.attr,
	&amp;above_hispeed_delay.attr,
	&amp;min_sample_time_attr.attr,
	&amp;timer_rate_attr.attr,
	&amp;timer_slack.attr,
	&amp;boost.attr,
	&amp;boostpulse.attr,
	&amp;boostpulse_duration.attr,
	NULL,
};

static struct attribute_group interactive_attr_group = {
	.attrs = interactive_attributes,
	.name = &quot;interactive&quot;,
};

static int cpufreq_interactive_idle_notifier(struct notifier_block *nb,
					     unsigned long val,
					     void *data)
{
	switch (val) {
	case IDLE_START:
		cpufreq_interactive_idle_start();
		break;
	case IDLE_END:
		cpufreq_interactive_idle_end();
		break;
	}

	return 0;
}

static struct notifier_block cpufreq_interactive_idle_nb = {
	.notifier_call = cpufreq_interactive_idle_notifier,
};

static int cpufreq_governor_interactive(struct cpufreq_policy *policy,
		unsigned int event)
{
	int rc;
	unsigned int j;
	struct cpufreq_interactive_cpuinfo *pcpu;
	struct cpufreq_frequency_table *freq_table;

	switch (event) {
	case CPUFREQ_GOV_START:
		if (!cpu_online(policy-&gt;cpu))
			return -EINVAL;

		mutex_lock(&amp;gov_lock);

		freq_table =
			cpufreq_frequency_get_table(policy-&gt;cpu);
		if (!hispeed_freq)
			hispeed_freq = policy-&gt;max;

		for_each_cpu(j, policy-&gt;cpus) {
			unsigned long expires;

			pcpu = &amp;per_cpu(cpuinfo, j);
			pcpu-&gt;policy = policy;
			pcpu-&gt;target_freq = policy-&gt;cur;
			pcpu-&gt;freq_table = freq_table;
			pcpu-&gt;floor_freq = pcpu-&gt;target_freq;
			pcpu-&gt;floor_validate_time =
				ktime_to_us(ktime_get());
			pcpu-&gt;hispeed_validate_time =
				pcpu-&gt;floor_validate_time;
			down_write(&amp;pcpu-&gt;enable_sem);
			expires = jiffies + usecs_to_jiffies(timer_rate);
			pcpu-&gt;cpu_timer.expires = expires;
			add_timer_on(&amp;pcpu-&gt;cpu_timer, j);
			if (timer_slack_val &gt;= 0) {
				expires += usecs_to_jiffies(timer_slack_val);
				pcpu-&gt;cpu_slack_timer.expires = expires;
				add_timer_on(&amp;pcpu-&gt;cpu_slack_timer, j);
			}
			pcpu-&gt;governor_enabled = 1;
			up_write(&amp;pcpu-&gt;enable_sem);
		}

		/*
		 * Do not register the idle hook and create sysfs
		 * entries if we have already done so.
		 */
		if (++active_count &gt; 1) {
			mutex_unlock(&amp;gov_lock);
			return 0;
		}

		rc = sysfs_create_group(cpufreq_global_kobject,
				&amp;interactive_attr_group);
		if (rc) {
			mutex_unlock(&amp;gov_lock);
			return rc;
		}

		idle_notifier_register(&amp;cpufreq_interactive_idle_nb);
		cpufreq_register_notifier(
			&amp;cpufreq_notifier_block, CPUFREQ_TRANSITION_NOTIFIER);
		mutex_unlock(&amp;gov_lock);
		break;

	case CPUFREQ_GOV_STOP:
		mutex_lock(&amp;gov_lock);
		for_each_cpu(j, policy-&gt;cpus) {
			pcpu = &amp;per_cpu(cpuinfo, j);
			down_write(&amp;pcpu-&gt;enable_sem);
			pcpu-&gt;governor_enabled = 0;
			del_timer_sync(&amp;pcpu-&gt;cpu_timer);
			del_timer_sync(&amp;pcpu-&gt;cpu_slack_timer);
			up_write(&amp;pcpu-&gt;enable_sem);
		}

		if (--active_count &gt; 0) {
			mutex_unlock(&amp;gov_lock);
			return 0;
		}

		cpufreq_unregister_notifier(
			&amp;cpufreq_notifier_block, CPUFREQ_TRANSITION_NOTIFIER);
		idle_notifier_unregister(&amp;cpufreq_interactive_idle_nb);
		sysfs_remove_group(cpufreq_global_kobject,
				&amp;interactive_attr_group);
		mutex_unlock(&amp;gov_lock);

		break;

	case CPUFREQ_GOV_LIMITS:
		if (policy-&gt;max &lt; policy-&gt;cur)
			__cpufreq_driver_target(policy,
					policy-&gt;max, CPUFREQ_RELATION_H);
		else if (policy-&gt;min &gt; policy-&gt;cur)
			__cpufreq_driver_target(policy,
					policy-&gt;min, CPUFREQ_RELATION_L);
		break;
	}
	return 0;
}

static void cpufreq_interactive_nop_timer(unsigned long data)
{
}

static int __init cpufreq_interactive_init(void)
{
	unsigned int i;
	struct cpufreq_interactive_cpuinfo *pcpu;
	struct sched_param param = { .sched_priority = MAX_RT_PRIO-1 };

	/* Initalize per-cpu timers */
	for_each_possible_cpu(i) {
		pcpu = &amp;per_cpu(cpuinfo, i);
		init_timer_deferrable(&amp;pcpu-&gt;cpu_timer);
		pcpu-&gt;cpu_timer.function = cpufreq_interactive_timer;
		pcpu-&gt;cpu_timer.data = i;
		init_timer(&amp;pcpu-&gt;cpu_slack_timer);
		pcpu-&gt;cpu_slack_timer.function = cpufreq_interactive_nop_timer;
		spin_lock_init(&amp;pcpu-&gt;load_lock);
		init_rwsem(&amp;pcpu-&gt;enable_sem);
	}

	spin_lock_init(&amp;target_loads_lock);
	spin_lock_init(&amp;speedchange_cpumask_lock);
	mutex_init(&amp;gov_lock);
	speedchange_task =
		kthread_create(cpufreq_interactive_speedchange_task, NULL,
			       &quot;cfinteractive&quot;);
	if (IS_ERR(speedchange_task))
		return PTR_ERR(speedchange_task);

	sched_setscheduler_nocheck(speedchange_task, SCHED_FIFO, &amp;param);
	get_task_struct(speedchange_task);

	/* NB: wake up so the thread does not look hung to the freezer */
	wake_up_process(speedchange_task);

	return cpufreq_register_governor(&amp;cpufreq_gov_interactive);
}

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_INTERACTIVE
fs_initcall(cpufreq_interactive_init);
#else
module_init(cpufreq_interactive_init);
#endif

static void __exit cpufreq_interactive_exit(void)
{
	cpufreq_unregister_governor(&amp;cpufreq_gov_interactive);
	kthread_stop(speedchange_task);
	put_task_struct(speedchange_task);
}

module_exit(cpufreq_interactive_exit);

MODULE_AUTHOR(&quot;Mike Chan &lt;mike@android.com&gt;&quot;);
MODULE_DESCRIPTION(&quot;&#39;cpufreq_interactive&#39; - A cpufreq governor for &quot;
	&quot;Latency sensitive workloads&quot;);
MODULE_LICENSE(&quot;GPL&quot;);
</pre><div class="footer">Powered by <a href="https://code.google.com/p/gitiles/">Gitiles</a></div></body></html>
