/* include/linux/suspend_gov.h */

#ifndef _LINUX_SUSPEND_GOV_H
#define _LINUX_SUSPEND_GOV_H

#include <linux/suspend_gov.h>
#include <linux/input.h>
#include <linux/cpufreq.h>

#define MAX_GOV_NAME_LEN 16
extern char def_governor[16];
extern char good_governor[16];

bool change_g;

static struct cpufreq_policy *policy;
static struct cpufreq_governor *__find_governor(const char *str_governor);
static int __cpufreq_set_policy(struct cpufreq_policy *data,
				struct cpufreq_policy *policy);


//extern char cpufreq_default_gov[CONFIG_NR_CPUS][MAX_GOV_NAME_LEN];
//extern void cpufreq_store_default_gov(void);
//extern int cpufreq_change_gov(char *target_gov);
//extern int cpufreq_restore_default_gov(void);
//extern void cpufreq_restore_default_governor(void);
//extern void cpufreq_set_user_governor(void);
extern int set_governor(struct cpufreq_policy *policy, char str_governor[16]);
// extern int cpufreq_put_gov(void);
// extern void cpufreq_set_governor(char *governor);
#endif

