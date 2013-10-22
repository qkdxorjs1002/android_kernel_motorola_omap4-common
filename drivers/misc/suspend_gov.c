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

#define SUSPEND_GOV_VERSION_MAJOR 1
#define SUSPEND_GOV_VERSION_MINOR 1

static DEFINE_MUTEX(suspend_mutex);

bool suspend_gov_early_suspend_active __read_mostly;

static int suspend_gov;

bool change_g;

static int gov_val = 2;

char def_governor;
char good_governor;

unsigned int cpu;


bool change_g;


static ssize_t suspend_gov_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", gov_val);
}


static ssize_t suspend_gov_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{


	/* Read usersapce input */

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


#endif
	mutex_unlock(&suspend_mutex);
}

static void suspend_gov_late_resume(struct early_suspend *h, struct cpufreq_policy *policy)
{
	mutex_lock(&suspend_mutex);
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
