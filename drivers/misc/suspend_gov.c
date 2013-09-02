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

static DEFINE_MUTEX(suspend_mutex);

char *cpufreq_ondemand_gov;

int suspend_gov;
unsigned int gov_val;

static ssize_t suspend_gov_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", gov_val);
}


static ssize_t suspend_gov_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{

	sscanf(buf, "%d\n", &gov_val);

	if (gov_val < 0) gov_val == 0;
	if (gov_val > 3) gov_val == 3;
		// shouldn't be here
		pr_info("[dtrail] suspend governor error - bailing\n");	
		return count;

	if (gov_val == 0) {
		cpufreq_ondemand_gov = "ondemand";
			pr_info("Suspend Governor: Ondemand\n");

	} else if (gov_val == 1) {
		cpufreq_ondemand_gov = "ktoonservative";
			pr_info("Suspend Governor: Ktoonservative\n");

	} else if (gov_val == 2) {
		cpufreq_ondemand_gov = "conservative";
			pr_info("Suspend Governor: Conservative\n");

	} else if (gov_val == 3) {
		cpufreq_ondemand_gov = "ondemandx";		
			pr_info("Suspend Governor: OndemandX\n");
	
return count;


}

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

static void suspend_gov_exit(void)
{

	if (suspend_gov_kobj != NULL)
		kobject_put(suspend_gov_kobj);
}

module_init(suspend_gov_init);
module_exit(suspend_gov_exit);
