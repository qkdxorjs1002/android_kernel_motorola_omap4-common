/*
 * arch/arm/mach-omap2/board-mapphone-vibrator.c
 *
 * Copyright (C) 2011 Motorola Mobility Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include <linux/module.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/vib-timed.h>
#include <linux/hrtimer.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/wakelock.h>

#ifdef CONFIG_VIBRATOR_CONTROL
#include <linux/delay.h>
#endif

#include <plat/dmtimer.h>

#include "dt_path.h"
#include <linux/of.h>

#define DM_FILLER		20
#define MAX_VIBS		2
#define MAX_PWMS		8
#define MAX_VOLT		4

#define VIB_TYPE_GENENIC_ROTARY	0x00000017
#define VIB_TYPE_GENENIC_LINEAR	0x00000002

#define ACTIVE_HIGH		1
#define ACTIVE_LOW		0

#define SIGNAL_GPIO		0x1F

#define MIN_TIMEOUT		0
#define MAX_TIMEOUT		60000000

#define REGULATOR_NAME_SIZE	16

#define VIB_EN			"vib_en"
#define VIB_DIR			"vib_dir"

#define SIGNAL_ENABLE		0x2A
#define SIGNAL_DIRECTION	0x3A

#define GPIO_STAGE_ACTIVE	1
#define GPIO_STAGE_INACTIVE	2

struct vib_ctrl_gpio {
	struct hrtimer hrtimer;
	int stage;
	unsigned int active_us;
	unsigned int inactive_us;
};

struct vib_signal;

struct vib_of_signal {
	int type;
	int active_level;
	int pwm;
	int gpio;
};

struct vib_ctrl_ops {
	int (*init)(struct vib_signal *);
	int (*configure)(struct vib_signal *, unsigned int,
			unsigned int, unsigned int);
	int (*activate)(struct vib_signal *);
	int (*deactivate)(struct vib_signal *);
};

struct vib_signal {
	int signal_type;
	struct vib_of_signal of;
	struct vib_ctrl_gpio gpioc;
	struct vib_ctrl_ops *ops;
	const char *name;
};


struct vib_voltage {
	unsigned int time;
	u32 min_uV;
	u32 max_uV;
};

struct vib_regulator {
	struct regulator *regulator;
	int enabled;
	u32 deferred_off;	/* in us */
	struct vib_voltage volt[MAX_VOLT];
	char name[REGULATOR_NAME_SIZE];
	struct mutex lock;	/* to defer regulator_disable */
	struct delayed_work work;
};

struct vibrator {
	int type;
	struct vib_regulator reg;
	struct wake_lock wakelock;
	unsigned int min_us;
	unsigned int max_us;
};

struct vibrator vibrators[MAX_VIBS]; /* dev_data */
struct vib_timed vib_timeds[MAX_VIBS]; /* pdata */
const char *vib_name[MAX_VIBS] = {"vibrator", "vibrator1"};

#ifdef CONFIG_VIBRATOR_CONTROL
extern void vibratorcontrol_register_vibstrength(int vibstrength);

void vibratorcontrol_update(int vibstrength)
{
    while (vibdata.enabled || !mutex_trylock(&vibdata.lock))
  msleep(50);

    omap_dm_timer_set_load(vibdata.gptimer, 1, -vibstrength);
    vibdata.gptimer->context.tldr = (unsigned int)-vibstrength;

    omap_dm_timer_set_match(vibdata.gptimer, 1, -vibstrength+10);
    vibdata.gptimer->context.tmar = (unsigned int)(-vibstrength+10);

    mutex_unlock(&vibdata.lock);

    return;
}
EXPORT_SYMBOL(vibratorcontrol_update);
#endif 

static enum hrtimer_restart gpioc_hrtimer_func(struct hrtimer *hrtimer)
{
	struct vib_ctrl_gpio *gpioc = container_of(hrtimer,
				struct vib_ctrl_gpio, hrtimer);
	struct vib_signal *vibs = container_of(gpioc,
				struct vib_signal, gpioc);
	struct vib_of_signal *of = &vibs->of;

	if (gpioc->stage == GPIO_STAGE_ACTIVE) {
		if (vibs->signal_type == SIGNAL_ENABLE) {
			gpio_direction_output(of->gpio, !of->active_level);
			dvib_tprint("g-t %s\n", vibs->name);
			gpioc->stage = GPIO_STAGE_INACTIVE;
		}
		if (gpioc->inactive_us) {
			if (vibs->signal_type == SIGNAL_DIRECTION) {
				gpio_direction_output(of->gpio,
						!of->active_level);
				dvib_tprint("g-t %s\n", vibs->name);
				gpioc->stage = GPIO_STAGE_INACTIVE;
			}
			hrtimer_start(&gpioc->hrtimer,
				ns_to_ktime((u64) gpioc->inactive_us
					* NSEC_PER_USEC),
				HRTIMER_MODE_REL);
		}
	} else {
		if (gpioc->active_us) {
			dvib_tprint("g+t %s\n", vibs->name);
			gpio_direction_output(of->gpio, of->active_level);
			gpioc->stage = GPIO_STAGE_ACTIVE;
			hrtimer_start(&gpioc->hrtimer,
				ns_to_ktime((u64) gpioc->active_us
					* NSEC_PER_USEC),
				HRTIMER_MODE_REL);
		}
	}
	return HRTIMER_NORESTART;
}

static int vib_ctrl_gpio_init(struct vib_signal *vibs)
{
	struct vib_ctrl_gpio *gpioc = &vibs->gpioc;
	struct vib_of_signal *of = &vibs->of;
	int ret;
	ret = gpio_request(of->gpio, "vib_ctrl");
	if (ret) {
		zfprintk("gpio request %d failed %d\n", of->gpio, ret);
		return ret;
	}
	ret = gpio_direction_output(of->gpio, !of->active_level);
	if (ret) {
		zfprintk("gpio %d output %d failed %d\n", of->gpio,
			!of->active_level, ret);
		return ret;
	}

	hrtimer_init(&gpioc->hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	gpioc->hrtimer.function = gpioc_hrtimer_func;
	return 0;
}

static int vib_ctrl_gpio_activate(struct vib_signal *vibs)
{
	struct vib_ctrl_gpio *gpioc = &vibs->gpioc;
	struct vib_of_signal *of = &vibs->of;
	int ret;
	dvib_tprint("g+ %s\n", vibs->name);
	if (!gpioc->active_us)
		return 0;
	ret = gpio_direction_output(of->gpio, of->active_level);
	if (ret) {
		zfprintk("gpio %d output %d failed %d\n", of->gpio,
			of->active_level, ret);
		return ret;
	}
	gpioc->stage = GPIO_STAGE_ACTIVE;
	ret = hrtimer_start(&gpioc->hrtimer,
		ns_to_ktime((u64) gpioc->active_us * NSEC_PER_USEC),
		HRTIMER_MODE_REL);
	if (ret)
		dvib_tprint("started timer %p while active.\n",
				&gpioc->hrtimer);
#ifdef CONFIG_VIBRATOR_
	return 0;
}

static int vib_ctrl_gpio_deactivate(struct vib_signal *vibs)
{
	struct vib_ctrl_gpio *gpioc = &vibs->gpioc;
	struct vib_of_signal *of = &vibs->of;
	int ret;
	ret = hrtimer_cancel(&gpioc->hrtimer);
	dvib_tprint("g- %s %s\n", vibs->name, ret ? "a" : "na");
	ret = gpio_direction_output(of->gpio, !of->active_level);
	if (ret) {
		zfprintk("gpio %d output %d failed %d\n", of->gpio,
			!of->active_level, ret);
		return ret;
	}
	gpioc->stage = GPIO_STAGE_INACTIVE;
	return 0;
}

static int vib_ctrl_gpio_config(struct vib_signal *vibs, unsigned int total_us,
			unsigned int period_us, unsigned int duty_us)
{
	struct vib_ctrl_gpio *gpioc = &vibs->gpioc;
	vib_ctrl_gpio_deactivate(vibs);
	gpioc->active_us = duty_us;
	gpioc->inactive_us = period_us - duty_us;
	dvib_print("\t\t%s\tT %u A %u N %u\n", vibs->name,
		total_us, gpioc->active_us, gpioc->inactive_us);
	return 0;
}

static struct vib_ctrl_ops vib_ctrl_gpio_ops = {
	.init		= vib_ctrl_gpio_init,
	.configure	= vib_ctrl_gpio_config,
	.activate	= vib_ctrl_gpio_activate,
	.deactivate	= vib_ctrl_gpio_deactivate,
};

static int vib_signal_init(struct vib_signal *vibs)
{
	struct vib_of_signal *of = &vibs->of;
	int ret;

	switch (of->type) {
	case SIGNAL_GPIO:
		ret = vib_ctrl_gpio_init(vibs);
		vibs->ops = &vib_ctrl_gpio_ops;
		break;

	default:
		zfprintk("%s unknown signal type %d\n", vibs->name, of->type);
		of->type = 0;
		ret = -1;
	}
	return ret;
}

