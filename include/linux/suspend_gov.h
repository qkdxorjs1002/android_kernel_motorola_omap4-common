/* include/linux/suspend_gov.h */

#ifndef _LINUX_SUSPEND_GOV_H
#define _LINUX_SUSPEND_GOV_H

#include <linux/suspend_gov.h>
#include <linux/input.h>
#include <linux/cpufreq.h>
#include <linux/uaccess.h>

#define MAX_GOV_NAME_LEN 16

extern char good_governor;
extern char def_governor;

extern unsigned int cpu;

extern bool change_g;

//extern int cpufreq_set_gov(char *target_gov, unsigned int cpu);

#endif

