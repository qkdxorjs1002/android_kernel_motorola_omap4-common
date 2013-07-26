/*
 * Author: Manuel Manz alias dtrail <mnl.manz@gmail.com>
 *
 * Copyright 2013 Manuel Manz
 * Copyright 2012 Manuel Manz
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
 */

#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

bool battery_friend_active __read_mostly = true;


static ssize_t battery_friend_active_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", (battery_friend_active ? 1 : 0));
}


static ssize_t battery_friend_active_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int data;

	if(sscanf(buf, "%u\n", &data) == 1) {
		if (data == 1) {
			pr_info("%s: battery friend enabled\n", __FUNCTION__);
			battery_friend_active = true;
		}
		else if (data == 0) {
			pr_info("%s: battery friend disabled\n", __FUNCTION__);
			battery_friend_active = false;
		}
		else
			pr_info("%s: bad value: %u\n", __FUNCTION__, data);
	} else
		pr_info("%s: unknown input!\n", __FUNCTION__);

	return count;
}

static struct kobj_attribute battery_friend_active_attribute = 
	__ATTR(battery_friend_active, 0666,
		battery_friend_active_show;

static struct attribute *battery_friend_active_attrs[] =
	{
		&battery_friend_active.attr,
		NULL,
	};

static struct attribute_group battery_friend_active_attr_group =
	{
		.attrs = battery_friend_active_attrs,
	};

static struct kobject *battery_friend_kobj;



static int battery_friend_init(void)
{
	int sysfs_result;


	battery_friend_kobj = kobject_create_and_add("dyn_fsync", kernel_kobj);
	if (!battery_friend_kobj) {
		pr_err("%s battery_friend kobject create failed!\n", __FUNCTION__);
		return -ENOMEM;
        }

	sysfs_result = sysfs_create_group(battery_friend_kobj,
			&battery_friend_active_attr_group);

        if (sysfs_result) {
		pr_info("%s battery_friend sysfs create failed!\n", __FUNCTION__);
		kobject_put(battery_friend_kobj);
	}
	return sysfs_result;
}

static void battery_friend_exit(void)
{

	if (battery_friend_kobj != NULL)
		kobject_put(battery_friend_kobj);
}

module_init(battery_friend_init);
module_exit(battery_friend_exit);
