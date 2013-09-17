/*
 * Author: Manuel Manz alias dtrail <mnl.manz@gmail.com>
 *
 * Copyright 2013 Manuel Manz
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
 *
 * Suspend Governor Control Version 1.0
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>
#include <linux/suspend_gov.h>
#include <linux/earlysuspend.h>

// cpufreq.c
//SYMSEARCH_DECLARE_FUNCTION_STATIC(struct cpufreq_governor *, __find_governor_s, const char *str_governor);
//SYMSEARCH_DECLARE_FUNCTION_STATIC(int, __cpufreq_set_policy_s, struct cpufreq_policy *data, struct cpufreq_policy *policy);

#define SUSPEND_GOV_VERSION_MAJOR 1
#define SUSPEND_GOV_VERSION_MINOR 1

#define MAX_GOV_NAME_LEN 16

static struct cpufreq_driver *cpufreq_driver;
static DEFINE_PER_CPU(struct cpufreq_policy *, cpufreq_cpu_data);

static DEFINE_MUTEX(suspend_mutex);
static LIST_HEAD(cpufreq_governor_list);
static DEFINE_MUTEX(cpufreq_governor_mutex);
static struct cpufreq_driver *cpufreq_driver;
static DEFINE_SPINLOCK(cpufreq_driver_lock);
static BLOCKING_NOTIFIER_HEAD(cpufreq_policy_notifier_list);

bool suspend_gov_early_suspend_active __read_mostly = true;
static int suspend_gov;

char def_governor[16];
char good_governor[16] = "conservative";

bool change_g;

// Prototypes

static int __cpufreq_governor(struct cpufreq_policy *policy,
		unsigned int event);
static unsigned int __cpufreq_get(unsigned int cpu);
static void handle_update(struct work_struct *work);


static struct srcu_notifier_head cpufreq_transition_notifier_list;

static bool init_cpufreq_transition_notifier_list_called;
static int __init init_cpufreq_transition_notifier_list(void)
{
	srcu_init_notifier_head(&cpufreq_transition_notifier_list);
	init_cpufreq_transition_notifier_list_called = true;
	return 0;
}
pure_initcall(init_cpufreq_transition_notifier_list);



// cpufreq.c

static struct cpufreq_governor *__find_governor_s(const char *str_governor)
{
	struct cpufreq_governor *t;

	list_for_each_entry(t, &cpufreq_governor_list, governor_list)
		if (!strnicmp(str_governor, t->name, CPUFREQ_NAME_LEN))
			return t;

	return NULL;
}

static int __cpufreq_set_policy_s(struct cpufreq_policy *data,
				struct cpufreq_policy *policy)
{
	int ret = 0;

	pr_debug("setting new policy for CPU %u: %u - %u kHz\n", policy->cpu,
		policy->min, policy->max);

	memcpy(&policy->cpuinfo, &data->cpuinfo,
				sizeof(struct cpufreq_cpuinfo));

	if (policy->min > data->max || policy->max < data->min) {
		ret = -EINVAL;
		goto error_out;
	}

	/* verify the cpu speed can be set within this limit */
	ret = cpufreq_driver->verify(policy);
	if (ret)
		goto error_out;

	/* adjust if necessary - all reasons */
	blocking_notifier_call_chain(&cpufreq_policy_notifier_list,
			CPUFREQ_ADJUST, policy);

	/* adjust if necessary - hardware incompatibility*/
	blocking_notifier_call_chain(&cpufreq_policy_notifier_list,
			CPUFREQ_INCOMPATIBLE, policy);

	/* verify the cpu speed can be set within this limit,
	   which might be different to the first one */
	ret = cpufreq_driver->verify(policy);
	if (ret)
		goto error_out;

	/* notification of the new policy */
	blocking_notifier_call_chain(&cpufreq_policy_notifier_list,
			CPUFREQ_NOTIFY, policy);

	// Set min speed to 100mhz


	data->min = policy->min;
	data->max = policy->max;
	pr_debug("new min and max freqs are %u - %u kHz\n",
					data->min, data->max);

	if (cpufreq_driver->setpolicy) {
		data->policy = policy->policy;
		pr_debug("setting range\n");
		ret = cpufreq_driver->setpolicy(policy);
	} else {
		if (policy->governor != data->governor) {
			/* save old, working values */
			struct cpufreq_governor *old_gov = data->governor;

			pr_debug("governor switch\n");

			/* end old governor */
			if (data->governor)
				__cpufreq_governor(data, CPUFREQ_GOV_STOP);

			/* start new governor */
			data->governor = policy->governor;
			if (__cpufreq_governor(data, CPUFREQ_GOV_START)) {
				/* new governor failed, so re-start old one */
				pr_debug("starting governor %s failed\n",
							data->governor->name);
				if (old_gov) {
					data->governor = old_gov;
					__cpufreq_governor(data,
							   CPUFREQ_GOV_START);
				}
				ret = -EINVAL;
				goto error_out;
			}
			/* might be a policy change, too, so fall through */
		}
		pr_debug("governor: change or update limits\n");
		__cpufreq_governor(data, CPUFREQ_GOV_LIMITS);
	}

error_out:
	return ret;
}



// char *sgov;
static int gov_val = 2;
/*char *cpufreq_sysfs_place_holder="/sys/devices/system/cpu/cpu%i/cpufreq/scaling_governor";
char cpufreq_gov_default[32];
char *cpufreq_gov_ondemand = "ondemand";
char *cpufreq_gov_interactive = "interactive";
char *cpufreq_gov_ondemandx = "ondemandx";
char *cpufreq_gov_conservative = "conservative"; */

static ssize_t suspend_gov_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", gov_val);
}


static ssize_t suspend_gov_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{

	sscanf(buf, "%d\n", &gov_val);
	
	if (gov_val == 0) {
		strcpy(good_governor, "ondemand");
			pr_info("Suspend Governor: %s\n", good_governor);

	} else if (gov_val == 1) {
		strcpy(good_governor, "interactive");
			pr_info("Suspend Governor: %s\n", good_governor);

	} else if (gov_val == 2) {
		strcpy(good_governor, "conservative");
			pr_info("Suspend Governor: %s\n", good_governor);

	} else if (gov_val == 3) {
		strcpy(good_governor, "ondemandx");
			pr_info("Suspend Governor: %s\n", good_governor);

	} else if (gov_val < 0) {
		gov_val = 0;
		pr_info("[dtrail] suspend governor error - bailing\n");	
		return count;

	} else if (gov_val > 3) {
		gov_val = 3;
		pr_info("[dtrail] suspend governor error - bailing\n");	
		return count;
	}

	
return count;


}

int set_governor(struct cpufreq_policy *policy, char str_governor[16]) {
	unsigned int ret = -EINVAL;
	struct cpufreq_governor *t;
	struct cpufreq_policy new_policy;
	if (!policy)
		return ret;

	memcpy(&new_policy, policy, sizeof(struct cpufreq_policy));
	cpufreq_get_policy(&new_policy, policy->cpu);
	t = __find_governor_s(str_governor);
	if (t != NULL) {
		new_policy.governor = t;
	} else {
		return ret;
	}

	ret = __cpufreq_set_policy_s(policy, &new_policy);

	policy->user_policy.policy = policy->policy;
	policy->user_policy.governor = policy->governor;
	return ret;
}
EXPORT_SYMBOL(set_governor);

static ssize_t suspend_gov_version_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "version: %u.%u by dtrail\n",
		SUSPEND_GOV_VERSION_MAJOR,
		SUSPEND_GOV_VERSION_MINOR);
}

static ssize_t suspend_gov_earlysuspend_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", suspend_gov_early_suspend_active);
}

static struct kobj_attribute suspend_gov_attribute = 
	__ATTR(suspend_gov, 0666,
		suspend_gov_show,
		suspend_gov_store);

static struct kobj_attribute suspend_gov_version_attribute = 
	__ATTR(suspend_gov_version, 0444, suspend_gov_version_show, NULL);

static struct kobj_attribute suspend_gov_earlysuspend_attribute = 
	__ATTR(suspend_gov_earlysuspend, 0444, suspend_gov_earlysuspend_show, NULL);

static struct attribute *suspend_gov_attrs[] =
	{
		&suspend_gov_attribute.attr,
		&suspend_gov_version_attribute.attr,
		&suspend_gov_earlysuspend_attribute.attr,
		NULL,
	};

static struct attribute_group suspend_gov_attr_group =
	{
		.attrs = suspend_gov_attrs,
	};

static struct kobject *suspend_gov_kobj;

static void suspend_gov_early_suspend(struct early_suspend *h, struct cpufreq_policy *policy)
{
	mutex_lock(&suspend_mutex);

		suspend_gov_early_suspend_active = true;
#if 1
			pr_info("Suspend Governor : Current governor is : %s\n", policy->governor->name);
		if (policy->governor->name != good_governor) {
			strcpy(def_governor, policy->governor->name);
			set_governor(policy, good_governor);
			pr_info("Suspend Governor : Change governor to : %s\n", policy->governor->name);
			change_g = true;
		}

#endif
	mutex_unlock(&suspend_mutex);
}

static void suspend_gov_late_resume(struct early_suspend *h, struct cpufreq_policy *policy)
{
	mutex_lock(&suspend_mutex);

	if (change_g) {
			set_governor(policy, def_governor);
			pr_info("Suspend Governor : Restore default governor : %s\n", policy->governor->name);
			}	
	suspend_gov_early_suspend_active = false;
	mutex_unlock(&suspend_mutex);
}

static struct early_suspend suspend_gov_early_suspend_handler = 
	{
		.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
		.suspend = suspend_gov_early_suspend,
		.resume = suspend_gov_late_resume,
	};


static int suspend_gov_init(void)
{
	int sysfs_result;

	suspend_gov_kobj = kobject_create_and_add("suspend_gov", kernel_kobj);
	if (!suspend_gov_kobj) {
		pr_err("%s suspend_gov kobject create failed!\n", __FUNCTION__);
		return -ENOMEM;
        }

	sysfs_result = sysfs_create_group(suspend_gov_kobj,
			&suspend_gov_attr_group);

        if (sysfs_result) {
		pr_info("%s suspend_gov sysfs create failed!\n", __FUNCTION__);
		kobject_put(suspend_gov_kobj);
	}
	return sysfs_result;
}

static void suspend_gov_exit(void)
{
	unregister_early_suspend(&suspend_gov_early_suspend_handler);

	if (suspend_gov_kobj != NULL)
		kobject_put(suspend_gov_kobj);
}

module_init(suspend_gov_init);
module_exit(suspend_gov_exit);
