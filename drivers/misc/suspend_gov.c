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


#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>
#include <linux/suspend_gov.h>

#define SUSPEND_GOV_VERSION_MAJOR 1
#define SUSPEND_GOV_VERSION_MINOR 1

#define MAX_GOV_NAME_LEN 16

static DEFINE_MUTEX(suspend_mutex);

char def_governor[16];
char good_governor[16];

static struct cpufreq_policy *policy;
static struct cpufreq_governor *__find_governor(const char *str_governor);
static int __cpufreq_set_policy(struct cpufreq_policy *data,
				struct cpufreq_policy *policy);


bool change;

// static char *sgov;
int gov_val = 2;
// static char *cpufreq_sysfs_place_holder="/sys/devices/system/cpu/cpu%i/cpufreq/scaling_governor";

static char *cpufreq_gov_ondemand = "ondemand";
static char *cpufreq_gov_interactive = "interactive";
static char *cpufreq_gov_ondemandx = "ondemandx";
static char *cpufreq_gov_conservative = "conservative";

static ssize_t suspend_gov_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", gov_val);
}


static ssize_t suspend_gov_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{

	sscanf(buf, "%d\n", &gov_val);
	
/*	if (gov_val == 0) {
		sgov = &cpufreq_gov_ondemand;
			pr_info("Suspend Governor: %s\n", cpufreq_gov_ondemand);

	} else if (gov_val == 1) {
		sgov = &cpufreq_gov_interactive;
			pr_info("Suspend Governor: %s\n", cpufreq_gov_interactive);

	} else if (gov_val == 2) {
		sgov = &cpufreq_gov_conservative;
			pr_info("Suspend Governor: %s\n", cpufreq_gov_conservative);

	} else if (gov_val == 3) {
		sgov = &cpufreq_gov_ondemandx;
			pr_info("Suspend Governor: %s\n", cpufreq_gov_ondemandx);

	} else if (gov_val < 0) {
		gov_val = 0;
		pr_info("[dtrail] suspend governor error - bailing\n");	
		return count;

	} else if (gov_val > 3) {
		gov_val = 3;
		pr_info("[dtrail] suspend governor error - bailing\n");	
		return count;
	}  */


	if (gov_val == 0) {
		strcpy(good_governor, "ondemand");
			pr_info("Suspend Governor: %s\n", cpufreq_gov_ondemand);

	} else if (gov_val == 1) {
		strcpy(good_governor, "interactive");
			pr_info("Suspend Governor: %s\n", cpufreq_gov_interactive);

	} else if (gov_val == 2) {
		strcpy(good_governor, "conservative");
			pr_info("Suspend Governor: %s\n", cpufreq_gov_conservative);

	} else if (gov_val == 3) {
		strcpy(good_governor, "ondemandx");
			pr_info("Suspend Governor: %s\n", cpufreq_gov_ondemandx);

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

/* char cpufreq_default_gov[CONFIG_NR_CPUS][MAX_GOV_NAME_LEN];

void cpufreq_store_default_gov(void)
{
unsigned int cpu;
struct cpufreq_policy *policy;

	for (cpu = 0; cpu < CONFIG_NR_CPUS; cpu++) {
		policy = cpufreq_cpu_get(cpu);
		if (policy) {
			sprintf(cpufreq_default_gov[cpu], "%s",
					policy->governor->name);
			cpufreq_cpu_put(policy);
			}
		}
	}
// EXPORT_SYMBOL(cpufreq_store_default_gov); */


/* void cpufreq_set_governor(char *governor)
{
    struct file *scaling_gov = NULL;
    char    buf[128];
    int i;
    loff_t offset = 0;

    if (governor == NULL)
        return;

    for_each_online_cpu(i) {
        sprintf(buf, cpufreq_sysfs_place_holder, i);
        scaling_gov = filp_open(buf, O_RDWR, 0);
        if (scaling_gov != NULL) {
            if (scaling_gov->f_op != NULL &&
                scaling_gov->f_op->write != NULL)
                scaling_gov->f_op->write(scaling_gov,
                        governor,
                        strlen(governor),
                        &offset);
            else
                pr_err("f_op might be null\n");

            filp_close(scaling_gov, NULL);
        } else {
            pr_err("%s. Can't open %s\n", __func__, buf);
        }
    }
} */




int set_governor(struct cpufreq_policy *policy, char str_governor[16]) {
	unsigned int ret = -EINVAL;
	struct cpufreq_governor *t;
	struct cpufreq_policy new_policy;
	if (!policy)
		return ret;

	memcpy(&new_policy, policy, sizeof(struct cpufreq_policy));
	cpufreq_get_policy(&new_policy, policy->cpu);
	t = __find_governor(str_governor);
	if (t != NULL) {
		new_policy.governor = t;
	} else {
		return ret;
	}

	ret = __cpufreq_set_policy(policy, &new_policy);

	policy->user_policy.policy = policy->policy;
	policy->user_policy.governor = policy->governor;
	return ret;
}




/* int cpufreq_put_gov(void)
{
		pr_info("Suspend Governor : Current governor is : %s\n", policy->governor->name);
		if (policy->governor->name != good_governor) {
			strcpy(def_governor, policy->governor->name);
			set_governor(policy, good_governor);
			change = true;
			pr_info("Suspend Governor : Change governor to : %s\n", policy->governor->name);
} */









// EXPORT_SYMBOL(cpufreq_set_governor);

/* void cpufreq_restore_default_governor(void)
{
    cpufreq_set_governor(cpufreq_gov_default);
} */

/* void cpufreq_set_user_governor(void)
{
    cpufreq_set_governor(sgov);
} */


/* int cpufreq_change_gov(char *target_gov)
	{
	unsigned int cpu = 0;
	for_each_online_cpu(cpu)
	return cpufreq_set_gov(target_gov, cpu);
	pr_info("Suspend Governor: Set");
	}
// EXPORT_SYMBOL(cpufreq_change_gov); */

/* int cpufreq_restore_default_gov(void)
	{

int ret = 0;
unsigned int cpu;

	for (cpu = 0; cpu < CONFIG_NR_CPUS; cpu++) {
		if (strlen((const char *)&cpufreq_default_gov[cpu])) {
			ret = cpufreq_set_gov(cpufreq_default_gov[cpu], cpu);
	if (ret < 0) */

	/* Unable to restore gov for the cpu as
	* It was online on suspend and becomes
	* offline on resume.
	*/
/*		pr_info("Unable to restore gov:%s for cpu:%d\n,"
		, cpufreq_default_gov[cpu]
		, cpu);
							}
		cpufreq_default_gov[cpu][0] = '\0';
	}
			return ret;
}
// EXPORT_SYMBOL(cpufreq_restore_default_gov); */

static ssize_t suspend_gov_version_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "version: %u.%u by dtrail\n",
		SUSPEND_GOV_VERSION_MAJOR,
		SUSPEND_GOV_VERSION_MINOR);
}

static struct kobj_attribute suspend_gov_attribute = 
	__ATTR(suspend_gov, 0666,
		suspend_gov_show,
		suspend_gov_store);

static struct kobj_attribute suspend_gov_version_attribute = 
	__ATTR(suspend_gov_version, 0444, suspend_gov_version_show, NULL);

static struct attribute *suspend_gov_attrs[] =
	{
		&suspend_gov_attribute.attr,
		&suspend_gov_version_attribute.attr,
		NULL,
	};

static struct attribute_group suspend_gov_attr_group =
	{
		.attrs = suspend_gov_attrs,
	};

static struct kobject *suspend_gov_kobj;

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
// EXPORT_SYMBOL(suspend_gov_init);

static void suspend_gov_exit(void)
{

	if (suspend_gov_kobj != NULL)
		kobject_put(suspend_gov_kobj);
}

module_init(suspend_gov_init);
module_exit(suspend_gov_exit);
