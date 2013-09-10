/* include/linux/suspend_gov.h */

#ifndef _LINUX_SUSPEND_GOV_H
#define _LINUX_SUSPEND_GOV_H

#include <linux/suspend_gov.h>
#include <linux/input.h>
#include <linux/cpufreq.h>

#define MAX_GOV_NAME_LEN 16
extern char *cpufreq_conservative_gov;
extern char cpufreq_default_gov[CONFIG_NR_CPUS][MAX_GOV_NAME_LEN];
extern void cpufreq_store_default_gov(void);
extern int cpufreq_change_gov(char *target_gov);
extern int cpufreq_restore_default_gov(void);
#endif

