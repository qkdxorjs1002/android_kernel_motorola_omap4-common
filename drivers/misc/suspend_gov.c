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

unsigned int suspend_gov;
int gov_val = 2;

char *cpufreq_conservative_gov;

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
		char *cpufreq_conservative_gov = "ondemand";
			pr_info("Suspend Governor: %s\n", cpufreq_conservative_gov);

	} else if (gov_val == 1) {
		char *cpufreq_conservative_gov = "interactive";
			pr_info("Suspend Governor: %s\n", cpufreq_conservative_gov);

	} else if (gov_val == 2) {
		char *cpufreq_conservative_gov = "conservative";
			pr_info("Suspend Governor: %s\n", cpufreq_conservative_gov);

	} else if (gov_val == 3) {
		char *cpufreq_conservative_gov = "ondemandx";		
			pr_info("Suspend Governor: %s\n", cpufreq_conservative_gov);

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

char cpufreq_default_gov[CONFIG_NR_CPUS][MAX_GOV_NAME_LEN];

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
EXPORT_SYMBOL(cpufreq_store_default_gov);

int cpufreq_change_gov(char *target_gov)
	{
	unsigned int cpu = 0;
	for_each_online_cpu(cpu)
	return cpufreq_set_gov(target_gov, cpu);
	pr_info("Suspend Governor: Set");
	}
EXPORT_SYMBOL(cpufreq_change_gov);

int cpufreq_restore_default_gov(void)
	{

int ret = 0;
unsigned int cpu;

	for (cpu = 0; cpu < CONFIG_NR_CPUS; cpu++) {
		if (strlen((const char *)&cpufreq_default_gov[cpu])) {
			ret = cpufreq_set_gov(cpufreq_default_gov[cpu], cpu);
	if (ret < 0)

	/* Unable to restore gov for the cpu as
	* It was online on suspend and becomes
	* offline on resume.
	*/
		pr_info("Unable to restore gov:%s for cpu:%d,"
		, cpufreq_default_gov[cpu]
		, cpu);
	else
		pr_info("Suspend Governor: Restored default governor");
								}
		cpufreq_default_gov[cpu][0] = '\0';
		pr_info("Suspend Governor: Restored default governor");
	}
			return ret;
}
EXPORT_SYMBOL(cpufreq_restore_default_gov);

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
EXPORT_SYMBOL(suspend_gov_init);

/* static void suspend_gov_exit(void)
{

	if (suspend_gov_kobj != NULL)
		kobject_put(suspend_gov_kobj);
} */

module_init(suspend_gov_init);
// module_exit(suspend_gov_exit);
