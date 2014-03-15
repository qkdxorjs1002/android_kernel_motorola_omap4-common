/*
 * OMAP4 OPP table definitions.
 *
 * Copyright (C) 2010-2011 Texas Instruments Incorporated - http://www.ti.com/
 *	Nishanth Menon
 *	Kevin Hilman
 *	Thara Gopinath
 * Copyright (C) 2010-2011 Nokia Corporation.
 *      Eduardo Valentin
 *      Paul Walmsley
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/module.h>
#include <linux/opp.h>
#include <linux/clk.h>

#include <plat/cpu.h>
#include <plat/common.h>

#include "control.h"
#include "omap_opp_data.h"
#include "pm.h"

#ifdef CONFIG_CUSTOM_VOLTAGE      
#include <linux/custom_voltage.h>
#endif

#ifdef CONFIG_LIVE_OC
#include <linux/live_oc.h>
#endif

#define OMAP4460_MPU_OPP_DPLL_TRIM             BIT(18)
#define OMAP4460_MPU_OPP_DPLL_TURBO_RBB                BIT(20)
#define OMAP4460_IVA_OPP_DPLL_TURBO_RBB                BIT(21)
#define OMAP4470_MPU_OPP_DPLL_NITRO_RBB                BIT(22)
#define OMAP4470_MPU_OPP_DPLL_NITROSB_RBB      BIT(23)
#define OMAP4470_IVA_OPP_DPLL_NITRO_RBB                BIT(24)
#define OMAP4470_IVA_OPP_DPLL_NITROSB_RBB      BIT(25)

 /*
 * STD_FUSE_OPP_DPLL_1 contains info about ABB trim type for MPU/IVA.
 * This probably is an ugly location to put the DPLL trim details.. but,
 * alternatives are even less attractive :( shrug..
 *
 * CONTROL_STD_FUSE_OPP_DPLL_1 bit fields:
 * Bit #|       Name       |        Description     |        Comment          |  Device
 * -----+------------------+------------------------+-------------------------|----------
 * 18-19|MPU_DPLL_TRIM_FREQ| 0 - 2.0GHz             | If RBB is not trimmed,  | OMAP4460
 *      |                  | 1 - 2.4GHz             | but MPU DPLL is trimmed |
 *      |                  | 2 - Reserved           | to 2.4GHz of higher,    |
 *      |                  | 3 - 3.0GHz             | it is recommended to    |
 *      |                  |                        | enable FBB for MPU at   |
 *      |                  |                        | OPPTB and OPPNT         |
 * -----+------------------+------------------------+-------------------------|----------
 *  20  |    MPU_RBB_TB    | 0 - RBB is trimmed     | If trimmed RBB can be   | OMAP4460/
 *      |                  | 1 - RBB is not trimmed | enabled at OPPTB on MPU | OMAP4470
 * -----+------------------+                        +-------------------------|----------
 *  21  |    IVA_RBB_TB    |                        | If trimmed RBB can be   | OMAP4460/
 *      |                  |                        | enabled at OPPTB on IVA | OMAP4470
 * -----+------------------+                        +-------------------------|----------
 *  22  |    MPU_RBB_NT    |                        | If trimmed RBB can be   | OMAP4470
 *      |                  |                        | enabled at OPPNT on MPU |
 * -----+------------------+                        +-------------------------|----------
 *  23  |    MPU_RBB_SB    |                        | If trimmed RBB can be   | OMAP4470
 *      |                  |                        | enabled at OPPSB on MPU |
 * -----+------------------+                        +-------------------------|----------
 *  24  |    IVA_RBB_NT    |                        | If trimmed RBB can be   | OMAP4470
 *      |                  |                        | enabled at OPPNT on IVA |
 * -----+------------------+                        +-------------------------|----------
 *  25  |    IVA_RBB_SB    |                        | If trimmed RBB can be   | OMAP4470
 *      |                  |                        | enabled at OPPSB on IVA |
 */


/**
 * struct omap4_ldo_abb_trim_data - describe ABB trim bits for specific voltage
 * @volt_data:         voltage table
 * @volt_nominal:      voltage for which ABB type should be modified according to trim bits.
 * @rbb_trim_mask:     If this bit is set in trim register, ABB type should be modified to RBB.
 * @fbb_trim_mask:     If this bit is set in trim register, ABB type should be modified to FBB.
 */
struct omap4_ldo_abb_trim_data {
	struct omap_volt_data *volt_data;
	u32 volt_nominal;
	u32 rbb_trim_mask;
	u32 fbb_trim_mask;
};

/*
 * Structures containing OMAP4430 voltage supported and various
 * voltage dependent data for each VDD. 
 * 
 * dtrail: We have default OPP1-5; extendet to steps of +100mhz = 12 OPPs
 * 	   Because we have limited eFuses, we must define one eFuse per each OPP in header file 
 *   	   and adjust voltages for OPPs
 *
 * 		Define voltage for and fuses for new OPPs
 */
#ifdef CONFIG_OMAP_OCFREQ_12
#define OMAP4430_VDD_MPU_OPP_UVA_UV		 990000			/* 100 */
#define OMAP4430_VDD_MPU_OPP50_UV		1005000			/* 200 */
#define OMAP4430_VDD_MPU_OPP100_UV		1020000			/* 300 */
#define OMAP4430_VDD_MPU_OPP150_UV		1075000			/* 400 */
#define OMAP4430_VDD_MPU_OPP200_UV		1125000			/* 500 */
#define OMAP4430_VDD_MPU_OPP250_UV		1195000			/* 600 */
#define OMAP4430_VDD_MPU_OPPTURBO_UV		1275000			/* 700 */
#define OMAP4430_VDD_MPU_OPPTURBOB_UV		1320000			/* 800 */
#define OMAP4430_VDD_MPU_OPPTURBOC_UV		1355000			/* 900 */
#define OMAP4430_VDD_MPU_OPPNITRO_UV		1385000			/* 1000 */
#define OMAP4430_VDD_MPU_OPPNITROS_UV		1390000			/* 1100 */
#define OMAP4430_VDD_MPU_OPPNITROSB_UV		1395000			/* 1200 */
#define OMAP4430_VDD_MPU_OPPNITROSBA_UV		1410000			/* 1300 */
#else
#define OMAP4430_VDD_MPU_OPP25_UV		 990000			/* 100 */
#define OMAP4430_VDD_MPU_OPP50_UV		1010000			/* 200 */
#define OMAP4430_VDD_MPU_OPP100_UV		1020000			/* 300 */
#define OMAP4430_VDD_MPU_OPPTURBO_UV		1080000			/* 400 */
#define OMAP4430_VDD_MPU_OPPTURBOB_UV		1195000			/* 600 */
#define OMAP4430_VDD_MPU_OPPNITRO_UV		1320000			/* 800 */
#define OMAP4430_VDD_MPU_OPPNITROSB_UV		1383000			/* 1000 */
#endif

/* dtrail: connecting OPPs to fuses and set smartreflex values */

#ifdef CONFIG_OMAP_SMARTREFLEX_CUSTOM_SENSOR
struct omap_volt_data omap443x_vdd_mpu_volt_data[] = {
#ifdef CONFIG_OMAP_OCFREQ_12
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP_UVA_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP50, 0, 0, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP50_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP50, 0, 0, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP100_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP50, 0, 0, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP150_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP200_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP250_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPTURBO_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBO, 0, 0, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPTURBOB_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBO, 0, 0, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPTURBOC_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBO, 0, 0, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPNITRO_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITRO, 0, 0, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPNITROS_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITRO, 0, 0, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPNITROSB_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITROSB, 0, 0, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPNITROSBA_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITROSB, 0, 0, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0, 0, 0),
};
#else
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP25_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP25, 0, 0, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP50_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP50, 0, 0, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP100_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP100, 0, 0, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPTURBO_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBO, 0, 0, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPTURBOB_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBOB, 0, 0, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPNITRO_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITRO, 0, 0, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPNITROSB_UV, 0, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0, 0, 0),
};
#endif
#else
struct omap_volt_data omap443x_vdd_mpu_volt_data[] = {
#ifdef CONFIG_OMAP_OCFREQ_12
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP_UVA_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP50, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP50_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP50, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP100_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP50, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP150_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP100, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP200_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP100, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP250_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP100, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPTURBO_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBO, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPTURBOB_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBO, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPTURBOC_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBO, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPNITRO_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITRO, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPNITROS_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITRO, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPNITROSB_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITROSB, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPNITROSBA_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITROSB, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};
#else
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP25_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP25, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP50_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP50, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPP100_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP100, 0xf4, 0x0c,  OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPTURBO_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBO, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPTURBOB_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBOB, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPNITRO_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITRO, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_MPU_OPPNITROSB_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITROSB, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};
#endif
#endif									
#ifdef CONFIG_BATTERY_FRIEND
#define OMAP4430_VDD_IVA_OPP50_UV		 950000
#define OMAP4430_VDD_IVA_OPP100_UV		1114000
#define OMAP4430_VDD_IVA_OPPTURBO_UV		1291000
#define OMAP4430_VDD_IVA_OPPNITRO_UV    	1375000
#else
#define OMAP4430_VDD_IVA_OPP50_UV		 910000
#define OMAP4430_VDD_IVA_OPP100_UV		1075000
#define OMAP4430_VDD_IVA_OPPTURBO_UV		1175000
#define OMAP4430_VDD_IVA_OPPNITRO_UV    	1275000
#endif

#ifdef CONFIG_OMAP_SMARTREFLEX_CUSTOM_SENSOR
struct omap_volt_data omap443x_vdd_iva_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP4430_VDD_IVA_OPP50_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPP50, 0, 0, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_IVA_OPP100_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_IVA_OPPTURBO_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPPTURBO, 0, 0, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_IVA_OPPNITRO_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPPNITRO, 0, 0, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0, 0, 0),
};
#else
struct omap_volt_data omap443x_vdd_iva_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP4430_VDD_IVA_OPP50_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPP50, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_IVA_OPP100_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPP100, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_IVA_OPPTURBO_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPPTURBO, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4430_VDD_IVA_OPPNITRO_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPPNITRO, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};
#endif
/* We define two more GPU scaling steps and adjust voltages */
#define OMAP4430_VDD_CORE_OPP25_UV		 902000
#define OMAP4430_VDD_CORE_OPP50_UV	         962000
#define OMAP4430_VDD_CORE_OPP100_UV		1127000
#define OMAP4430_VDD_CORE_OPP100_OV_UV		1250000

#ifdef CONFIG_OMAP_SMARTREFLEX_CUSTOM_SENSOR
struct omap_volt_data omap443x_vdd_core_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP4430_VDD_CORE_OPP25_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP50, 0, 0, 0xf4, 0x0c, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(OMAP4430_VDD_CORE_OPP50_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP100, 0, 0, 0xf4, 0x0c, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(OMAP4430_VDD_CORE_OPP100_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(OMAP4430_VDD_CORE_OPP100_OV_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0, 0, 0),
};
#else
struct omap_volt_data omap443x_vdd_core_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP4430_VDD_CORE_OPP25_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP50, 0xf4, 0x0c, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(OMAP4430_VDD_CORE_OPP50_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP100, 0xf4, 0x0c, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(OMAP4430_VDD_CORE_OPP100_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP100, 0xf9, 0x16, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(OMAP4430_VDD_CORE_OPP100_OV_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP100, 0xf9, 0x16, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};
#endif

/* Dependency of domains are as follows for OMAP4430 (OPP based):
 *
 *	MPU	IVA	CORE
 *	50	50	50+
 *	50	100+	100
 *	100+	50	100
 *	100+	100+	100
 */

/* OMAP 4430 MPU Core VDD dependency table */
static struct omap_vdd_dep_volt omap443x_vdd_mpu_core_dep_data[] = {
#ifdef CONFIG_OMAP_OCFREQ_12
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPP_UVA_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP50_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPP50_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP50_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPP100_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP50_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPP150_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPP200_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPP250_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPPTURBO_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPPTURBOB_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPPTURBOC_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPPNITRO_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPPNITROS_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPPNITROSB_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPPNITROSBA_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
};
#else
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPP25_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP50_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPP50_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP50_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPP100_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP50_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPPTURBO_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPPTURBOB_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPPNITRO_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_MPU_OPPNITROSB_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
};
#endif

struct omap_vdd_dep_info omap443x_vddmpu_dep_info[] = {
	{
		.name	= "core",
		.dep_table = omap443x_vdd_mpu_core_dep_data,
		.nr_dep_entries = ARRAY_SIZE(omap443x_vdd_mpu_core_dep_data),
	},
	{.name = NULL, .dep_table = NULL, .nr_dep_entries = 0},
};

/* OMAP 4430 MPU IVA VDD dependency table */
static struct omap_vdd_dep_volt omap443x_vdd_iva_core_dep_data[] = {
	{.main_vdd_volt = OMAP4430_VDD_IVA_OPP50_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP50_UV},
	{.main_vdd_volt = OMAP4430_VDD_IVA_OPP100_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_IVA_OPPTURBO_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4430_VDD_IVA_OPPNITRO_UV, .dep_vdd_volt = OMAP4430_VDD_CORE_OPP100_UV},
};

struct omap_vdd_dep_info omap443x_vddiva_dep_info[] = {
	{
		.name	= "core",
		.dep_table = omap443x_vdd_iva_core_dep_data,
		.nr_dep_entries = ARRAY_SIZE(omap443x_vdd_iva_core_dep_data),
	},
	{.name = NULL, .dep_table = NULL, .nr_dep_entries = 0},
};

static struct omap_opp_def __initdata omap443x_opp_def_list[] = {
#ifdef CONFIG_OMAP_OCFREQ_12
	/* MPU OPP1 - OPP25 */
 	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 100000000, OMAP4430_VDD_MPU_OPP_UVA_UV),
	/* MPU OPP1 - OPP50 */
 	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 200000000, OMAP4430_VDD_MPU_OPP50_UV),
	/* MPU OPP1 - OPP100 */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 300000000, OMAP4430_VDD_MPU_OPP100_UV),
	/* MPU OPP1 - OPP150 */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 400000000, OMAP4430_VDD_MPU_OPP150_UV),
	/* MPU OPP1 - OPP200 */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 500000000, OMAP4430_VDD_MPU_OPP200_UV),
	/* MPU OPP1 - OPP250 */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 600000000, OMAP4430_VDD_MPU_OPP250_UV),
	/* MPU OPP1 - OPP300 */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 700000000, OMAP4430_VDD_MPU_OPPTURBO_UV),
	/* MPU OPP2 - OPP-TB */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 800000000, OMAP4430_VDD_MPU_OPPTURBOB_UV),
	/* MPU OPP2 - OPP-TBB */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 900000000, OMAP4430_VDD_MPU_OPPTURBOC_UV),
	/* MPU OPP3 - OPP-NT */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 1000000000, OMAP4430_VDD_MPU_OPPNITRO_UV),
	/* MPU OPP3 - OPP-NTS */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 1100000000, OMAP4430_VDD_MPU_OPPNITROS_UV),
	/* MPU OPP4 - OPP-NTSB */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 1200000000, OMAP4430_VDD_MPU_OPPNITROSB_UV),
	/* MPU OPP4 - OPP-NTSB1 */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 1300000000, OMAP4430_VDD_MPU_OPPNITROSBA_UV),
#else
	/* MPU OPP1 - OPP25 */
 	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 100000000, OMAP4430_VDD_MPU_OPP25_UV),
	/* MPU OPP1 - OPP50 */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 200000000, OMAP4430_VDD_MPU_OPP50_UV),
	/* MPU OPP1 - OPP100 */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 300000000, OMAP4430_VDD_MPU_OPP100_UV),
	/* MPU OPP1 - OPP100B */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 400000000, OMAP4430_VDD_MPU_OPPTURBO_UV),
	/* MPU OPP2 - OPP-TB */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", true, 600000000, OMAP4430_VDD_MPU_OPPTURBOB_UV),
	/* MPU OPP3 - OPP-NT */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", false, 800000000, OMAP4430_VDD_MPU_OPPNITRO_UV),
	/* MPU OPP4 - OPP-NTSB */
	OPP_INITIALIZER("mpu", "dpll_mpu_ck", "mpu", false, 1000000000, OMAP4430_VDD_MPU_OPPNITROSB_UV),
#endif
	/* MPU OPP1 - OPP50  */
	OPP_INITIALIZER("l3_main_1", "virt_l3_ck", "core", true, 100000000, OMAP4430_VDD_CORE_OPP50_UV),
	/* L3 OPP2 - OPP100, OPP-Turbo, OPP-SB */
	OPP_INITIALIZER("l3_main_1", "virt_l3_ck", "core", true, 200000000, OMAP4430_VDD_CORE_OPP100_UV),
	/* L3 OPP3 - GIve core the abbillity to use higher voltage if needed - this can prevent crashes during OC */
	OPP_INITIALIZER("l3_main_1", "virt_l3_ck", "core", true, 200000000, OMAP4430_VDD_CORE_OPP100_OV_UV),
	/* IVA OPP1 - OPP50 */
	OPP_INITIALIZER("iva", "dpll_iva_m5x2_ck", "iva", true, 133000000, OMAP4430_VDD_IVA_OPP50_UV),
	/* IVA OPP2 - OPP100 */
	OPP_INITIALIZER("iva", "dpll_iva_m5x2_ck", "iva", true, 266100000, OMAP4430_VDD_IVA_OPP100_UV),
	/* IVA OPP3 - OPP-Turbo */
	OPP_INITIALIZER("iva", "dpll_iva_m5x2_ck", "iva", true, 332000000, OMAP4430_VDD_IVA_OPPTURBO_UV),
	/* IVA OPP4 - OPP-Nitro */
	OPP_INITIALIZER("iva", "dpll_iva_m5x2_ck", "iva", true, 430000000, OMAP4430_VDD_IVA_OPPNITRO_UV),
	/* SGX OPP1 - OPP50 */
	OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 100300000, OMAP4430_VDD_CORE_OPP25_UV),
	/* SGX OPP1 - OPP50 */
	OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 153600000, OMAP4430_VDD_CORE_OPP50_UV),
	/* SGX OPP2 - OPP100 */
	OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 307200000, OMAP4430_VDD_CORE_OPP100_UV),
	/* SGX OPP3 - OPPOV  dtrail: Added third GPU OPP and overclocked to factory default */
	OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 384000000, OMAP4430_VDD_CORE_OPP100_UV),
	/* SGX OPP4 - OPPOV  dtrail: Added fourth GPU OPP and overclocked to 416mhz */
	OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", false, 416000000, OMAP4430_VDD_CORE_OPP100_OV_UV),
	/* FDIF OPP1 - OPP25 */
	OPP_INITIALIZER("fdif", "fdif_fck", "core", true, 32000000, OMAP4430_VDD_CORE_OPP50_UV),
	/* FDIF OPP2 - OPP50 */
	OPP_INITIALIZER("fdif", "fdif_fck", "core", true, 64000000, OMAP4430_VDD_CORE_OPP50_UV),
	/* FDIF OPP3 - OPP100 */
	OPP_INITIALIZER("fdif", "fdif_fck", "core", true, 128000000, OMAP4430_VDD_CORE_OPP100_UV),
	/* DSP OPP1 - OPP50 */
	OPP_INITIALIZER("dsp", "dpll_iva_m4x2_ck", "iva", true, 232750000, OMAP4430_VDD_IVA_OPP50_UV),
	/* DSP OPP2 - OPP100 */
	OPP_INITIALIZER("dsp", "dpll_iva_m4x2_ck", "iva", true, 465500000, OMAP4430_VDD_IVA_OPP100_UV),
	/* DSP OPP3 - OPPTB */
#ifdef CONFIG_BATTERY_FRIEND
	OPP_INITIALIZER("dsp", "dpll_iva_m4x2_ck", "iva", false, 496000000, OMAP4430_VDD_IVA_OPPTURBO_UV),
#else
	OPP_INITIALIZER("dsp", "dpll_iva_m4x2_ck", "iva", true, 496000000, OMAP4430_VDD_IVA_OPPTURBO_UV),
#endif
	/* HSI OPP1 - OPP50 */
	OPP_INITIALIZER("hsi", "hsi_fck", "core", true, 96000000, OMAP4430_VDD_CORE_OPP50_UV),
	/* HSI OPP2 - OPP100 */
	OPP_INITIALIZER("hsi", "hsi_fck", "core", true, 192000000, OMAP4430_VDD_CORE_OPP100_UV),
	/* ABE OPP1 - OPP50 */
	OPP_INITIALIZER("aess", "abe_clk", "iva", true, 98304000, OMAP4430_VDD_IVA_OPP50_UV),
	/* ABE OPP2 - OPP100 */
	OPP_INITIALIZER("aess", "abe_clk", "iva", true, 196608000, OMAP4430_VDD_IVA_OPP100_UV),
	/* DSS OPP1 - OPP50 */
	OPP_INITIALIZER("dss_dispc", "virt_lcd_pclk", "core", true, 91000000, OMAP4430_VDD_CORE_OPP50_UV),
	/* DSS OPP2 - OPP100 */
	OPP_INITIALIZER("dss_dispc", "virt_lcd_pclk", "core", true, 170000000, OMAP4430_VDD_CORE_OPP100_UV),
};

#define OMAP4460_VDD_MPU_OPP50_UV    1025000
#define OMAP4460_VDD_MPU_OPP100_UV    1203000
#define OMAP4460_VDD_MPU_OPPTURBO_UV    1317000
#define OMAP4460_VDD_MPU_OPPNITRO_UV    1380000
#define OMAP4460_VDD_MPU_OPPNITROSB_UV    1390000

#ifdef CONFIG_OMAP_SMARTREFLEX_CUSTOM_SENSOR
struct omap_volt_data omap446x_vdd_mpu_volt_data[] = {
  VOLT_DATA_DEFINE(OMAP4460_VDD_MPU_OPP50_UV, 10000, OMAP44XX_CONTROL_FUSE_MPU_OPP50, 0, 0, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_MPU_OPP100_UV, 10000, OMAP44XX_CONTROL_FUSE_MPU_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_MPU_OPPTURBO_UV, 10000, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBO, 0, 0, 0xfa, 0x23, OMAP_ABB_FAST_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_MPU_OPPNITRO_UV, 10000, OMAP44XX_CONTROL_FUSE_MPU_OPPNITRO, 0, 0, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_MPU_OPPNITROSB_UV, 10000, OMAP44XX_CONTROL_FUSE_MPU_OPPNITROSB, 0, 0, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
  VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0, 0, 0),
};
#else
struct omap_volt_data omap446x_vdd_mpu_volt_data[] = {
  VOLT_DATA_DEFINE(OMAP4460_VDD_MPU_OPP50_UV, 10000, OMAP44XX_CONTROL_FUSE_MPU_OPP50, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_MPU_OPP100_UV, 10000, OMAP44XX_CONTROL_FUSE_MPU_OPP100, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_MPU_OPPTURBO_UV, 10000, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBO, 0xfa, 0x23, OMAP_ABB_FAST_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_MPU_OPPNITRO_UV, 10000, OMAP44XX_CONTROL_FUSE_MPU_OPPNITRO, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_MPU_OPPNITROSB_UV, 10000, OMAP44XX_CONTROL_FUSE_MPU_OPPNITROSB, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
  VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};
#endif

#define OMAP4460_VDD_IVA_OPP50_UV     950000
#define OMAP4460_VDD_IVA_OPP100_UV    1140000
#define OMAP4460_VDD_IVA_OPPTURBO_UV    1291000
#define OMAP4460_VDD_IVA_OPPNITRO_UV    1375000
#define OMAP4460_VDD_IVA_OPPNITROSB_UV		1376000

#ifdef CONFIG_OMAP_SMARTREFLEX_CUSTOM_SENSOR
struct omap_volt_data omap446x_vdd_iva_volt_data[] = {
  VOLT_DATA_DEFINE(OMAP4460_VDD_IVA_OPP50_UV, 10000, OMAP44XX_CONTROL_FUSE_IVA_OPP50, 0, 0, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_IVA_OPP100_UV, 10000, OMAP44XX_CONTROL_FUSE_IVA_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_IVA_OPPTURBO_UV, 10000, OMAP44XX_CONTROL_FUSE_IVA_OPPTURBO, 0, 0, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_IVA_OPPNITRO_UV, 10000, OMAP44XX_CONTROL_FUSE_IVA_OPPNITRO, 0, 0, 0xfa, 0x23, OMAP_ABB_FAST_OPP),
  VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0, 0, 0),
};
#else
struct omap_volt_data omap446x_vdd_iva_volt_data[] = {
  VOLT_DATA_DEFINE(OMAP4460_VDD_IVA_OPP50_UV, 10000, OMAP44XX_CONTROL_FUSE_IVA_OPP50, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_IVA_OPP100_UV, 10000, OMAP44XX_CONTROL_FUSE_IVA_OPP100, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_IVA_OPPTURBO_UV, 10000, OMAP44XX_CONTROL_FUSE_IVA_OPPTURBO, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
  VOLT_DATA_DEFINE(OMAP4460_VDD_IVA_OPPNITRO_UV, 10000, OMAP44XX_CONTROL_FUSE_IVA_OPPNITRO, 0xfa, 0x23, OMAP_ABB_FAST_OPP),
  VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};
#endif

#define OMAP4460_VDD_CORE_OPP50_UV     962000
#define OMAP4460_VDD_CORE_OPP100_UV    1127000
#define OMAP4460_VDD_CORE_OPP100_OV_UV    1250000

#ifdef CONFIG_OMAP_SMARTREFLEX_CUSTOM_SENSOR
struct omap_volt_data omap446x_vdd_core_volt_data[] = {
  VOLT_DATA_DEFINE(OMAP4460_VDD_CORE_OPP50_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP50, 0, 0, 0xf4, 0x0c, OMAP_ABB_NONE),
  VOLT_DATA_DEFINE(OMAP4460_VDD_CORE_OPP100_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NONE),
  VOLT_DATA_DEFINE(OMAP4460_VDD_CORE_OPP100_OV_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP100OV, 0, 0, 0xf9, 0x16, OMAP_ABB_NONE),
  VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0, 0, 0),
};
#else
struct omap_volt_data omap446x_vdd_core_volt_data[] = {
  VOLT_DATA_DEFINE(OMAP4460_VDD_CORE_OPP50_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP50, 0xf4, 0x0c, OMAP_ABB_NONE),
  VOLT_DATA_DEFINE(OMAP4460_VDD_CORE_OPP100_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP100, 0xf9, 0x16, OMAP_ABB_NONE),
  VOLT_DATA_DEFINE(OMAP4460_VDD_CORE_OPP100_OV_UV, 10000, OMAP44XX_CONTROL_FUSE_CORE_OPP100OV, 0xf9, 0x16, OMAP_ABB_NONE),
  VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};
#endif

/* OMAP 4460 MPU Core VDD dependency table */
static struct omap_vdd_dep_volt omap446x_vdd_mpu_core_dep_data[] = {
  {.main_vdd_volt = OMAP4460_VDD_MPU_OPP50_UV, .dep_vdd_volt = OMAP4460_VDD_CORE_OPP50_UV},
  {.main_vdd_volt = OMAP4460_VDD_MPU_OPP100_UV, .dep_vdd_volt = OMAP4460_VDD_CORE_OPP100_UV},
  {.main_vdd_volt = OMAP4460_VDD_MPU_OPPTURBO_UV, .dep_vdd_volt = OMAP4460_VDD_CORE_OPP100_UV},
  {.main_vdd_volt = OMAP4460_VDD_MPU_OPPNITRO_UV, .dep_vdd_volt = OMAP4460_VDD_CORE_OPP100_UV},
  {.main_vdd_volt = OMAP4460_VDD_MPU_OPPNITROSB_UV, .dep_vdd_volt = OMAP4460_VDD_CORE_OPP100_UV},
};

struct omap_vdd_dep_info omap446x_vddmpu_dep_info[] = {
  {
    .name  = "core",
    .dep_table = omap446x_vdd_mpu_core_dep_data,
    .nr_dep_entries = ARRAY_SIZE(omap446x_vdd_mpu_core_dep_data),
  },
  {.name = NULL, .dep_table = NULL, .nr_dep_entries = 0},
};

/* OMAP 4460 MPU IVA VDD dependency table */
static struct omap_vdd_dep_volt omap446x_vdd_iva_core_dep_data[] = {
  {.main_vdd_volt = OMAP4460_VDD_IVA_OPP50_UV, .dep_vdd_volt = OMAP4460_VDD_CORE_OPP50_UV},
  {.main_vdd_volt = OMAP4460_VDD_IVA_OPP100_UV, .dep_vdd_volt = OMAP4460_VDD_CORE_OPP100_UV},
  {.main_vdd_volt = OMAP4460_VDD_IVA_OPPTURBO_UV, .dep_vdd_volt = OMAP4460_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4460_VDD_IVA_OPPNITRO_UV, .dep_vdd_volt = OMAP4460_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4460_VDD_IVA_OPPNITROSB_UV, .dep_vdd_volt = OMAP4460_VDD_CORE_OPP100_UV},
};

struct omap_vdd_dep_info omap446x_vddiva_dep_info[] = {
  {
    .name  = "core",
    .dep_table = omap446x_vdd_iva_core_dep_data,
    .nr_dep_entries = ARRAY_SIZE(omap446x_vdd_iva_core_dep_data),
  },
  {.name = NULL, .dep_table = NULL, .nr_dep_entries = 0},
};

static struct omap4_ldo_abb_trim_data __initdata omap446x_ldo_abb_trim_data[] = {
	{.volt_data = omap446x_vdd_mpu_volt_data, .volt_nominal = OMAP4460_VDD_MPU_OPPTURBO_UV, .rbb_trim_mask = OMAP4460_MPU_OPP_DPLL_TURBO_RBB,
			.fbb_trim_mask = OMAP4460_MPU_OPP_DPLL_TRIM},
	{.volt_data = omap446x_vdd_iva_volt_data, .volt_nominal = OMAP4460_VDD_IVA_OPPTURBO_UV, .rbb_trim_mask = OMAP4460_IVA_OPP_DPLL_TURBO_RBB},
	{.volt_data = NULL},
};

static struct omap_opp_def __initdata omap446x_opp_def_list[] = {
  /* MPU OPP1 - OPP50 */
  OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", true, 350000000, OMAP4460_VDD_MPU_OPP50_UV),
  /* MPU OPP2 - OPP100 */
  OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", true, 700000000, OMAP4460_VDD_MPU_OPP100_UV),
  /* MPU OPP3 - OPP-Turbo */
  OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", true, 920000000, OMAP4460_VDD_MPU_OPPTURBO_UV),
  /* MPU OPP4 - OPP-Nitro */
  OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", false, 1200000000, OMAP4460_VDD_MPU_OPPNITRO_UV),
  /* MPU OPP4 - OPP-Nitro SpeedBin */
  OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", false, 1500000000, OMAP4460_VDD_MPU_OPPNITROSB_UV),
  /* L3 OPP1 - OPP50 */
  OPP_INITIALIZER("l3_main_1", "virt_l3_ck", "core", true, 100000000, OMAP4460_VDD_CORE_OPP50_UV),
  /* L3 OPP2 - OPP100 */
  OPP_INITIALIZER("l3_main_1", "virt_l3_ck", "core", true, 200000000, OMAP4460_VDD_CORE_OPP100_UV),
  /* L3 OPP3 - OPP100_OV */
  OPP_INITIALIZER("l3_main_1", "virt_l3_ck", "core", true, 200000000, OMAP4460_VDD_CORE_OPP100_OV_UV),
  /* IVA OPP1 - OPP50 */
  OPP_INITIALIZER("iva", "dpll_iva_m5x2_ck", "iva", true, 133000000, OMAP4460_VDD_IVA_OPP50_UV),
  /* IVA OPP2 - OPP100 */
  OPP_INITIALIZER("iva", "dpll_iva_m5x2_ck", "iva", true, 266100000, OMAP4460_VDD_IVA_OPP100_UV),
  /*
   * IVA OPP3 - OPP-Turbo + Disabled as the reference schematics
   * recommends Phoenix VCORE2 which can supply only 600mA - so the ones
   * above this OPP frequency, even though OMAP is capable, should be
   * enabled by board file which is sure of the chip power capability
   */
  OPP_INITIALIZER("iva", "dpll_iva_m5x2_ck", "iva", false, 332000000, OMAP4460_VDD_IVA_OPPTURBO_UV),
  /* IVA OPP4 - OPP-Nitro */
  OPP_INITIALIZER("iva", "dpll_iva_m5x2_ck", "iva", false, 430000000, OMAP4460_VDD_IVA_OPPNITRO_UV),
  /* IVA OPP5 - OPP-Nitro SpeedBin*/
  OPP_INITIALIZER("iva", "dpll_iva_m5x2_ck", "iva", false, 500000000, OMAP4460_VDD_IVA_OPPNITRO_UV),

  /* SGX OPP1 - OPP50 */
  OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 153600000, OMAP4460_VDD_CORE_OPP50_UV),
  /* SGX OPP2 - OPP100 */
  OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 307200000, OMAP4460_VDD_CORE_OPP100_UV),
  /* SGX OPP3 - OPPOV */
  OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 384000000, OMAP4460_VDD_CORE_OPP100_OV_UV),
  /* FDIF OPP1 - OPP25 */
  OPP_INITIALIZER("fdif", "fdif_fck", "core", true, 32000000, OMAP4460_VDD_CORE_OPP50_UV),
  /* FDIF OPP2 - OPP50 */
  OPP_INITIALIZER("fdif", "fdif_fck", "core", true, 64000000, OMAP4460_VDD_CORE_OPP50_UV),
  /* FDIF OPP3 - OPP100 */
  OPP_INITIALIZER("fdif", "fdif_fck", "core", true, 128000000, OMAP4460_VDD_CORE_OPP100_UV),
  /* DSP OPP1 - OPP50 */
  OPP_INITIALIZER("dsp", "dpll_iva_m4x2_ck", "iva", true, 232750000, OMAP4460_VDD_IVA_OPP50_UV),
  /* DSP OPP2 - OPP100 */
  OPP_INITIALIZER("dsp", "dpll_iva_m4x2_ck", "iva", true, 465500000, OMAP4460_VDD_IVA_OPP100_UV),
  /* DSP OPP3 - OPPTB */
  OPP_INITIALIZER("dsp", "dpll_iva_m4x2_ck", "iva", false, 496000000, OMAP4460_VDD_IVA_OPPTURBO_UV),
  /* HSI OPP1 - OPP50 */
  OPP_INITIALIZER("hsi", "hsi_fck", "core", true, 96000000, OMAP4460_VDD_CORE_OPP50_UV),
  /* HSI OPP2 - OPP100 */
  OPP_INITIALIZER("hsi", "hsi_fck", "core", true, 192000000, OMAP4460_VDD_CORE_OPP100_UV),
  /* ABE OPP1 - OPP50 */
  OPP_INITIALIZER("aess", "abe_clk", "iva", true, 98304000, OMAP4460_VDD_IVA_OPP50_UV),
  /* ABE OPP2 - OPP100 */
  OPP_INITIALIZER("aess", "abe_clk", "iva", true, 196608000, OMAP4460_VDD_IVA_OPP100_UV),
  /* DSS OPP1 - OPP50 */
  OPP_INITIALIZER("dss_dispc", "virt_lcd_pclk", "core", true, 93000000, OMAP4460_VDD_CORE_OPP50_UV),
  /* DSS OPP2 - OPP100 */
#ifdef CONFIG_PVR_GOVERNOR
  OPP_INITIALIZER("dss_dispc", "virt_lcd_pclk", "core", true, 170000000, OMAP4460_VDD_CORE_OPP75_UV),
#else
  OPP_INITIALIZER("dss_dispc", "virt_lcd_pclk", "core", true, 170000000, OMAP4460_VDD_CORE_OPP100_UV),
#endif 
}; 

/*
 * Structures containing OMAP4470 voltage supported and various
 * voltage dependent data for each VDD.
 */
#define OMAP4470_VDD_MPU_OPP50_UV              1025000
#define OMAP4470_VDD_MPU_OPP100_UV             1200000
#define OMAP4470_VDD_MPU_OPPTURBO_UV           1312000
#define OMAP4470_VDD_MPU_OPPNITRO_UV           1375000
#define OMAP4470_VDD_MPU_OPPNITROSB_UV         1380000

#ifdef CONFIG_OMAP_SMARTREFLEX_CUSTOM_SENSOR
struct omap_volt_data omap447x_vdd_mpu_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP4470_VDD_MPU_OPP50_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP50, 0, 0, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_MPU_OPP100_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_MPU_OPPTURBO_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBO, 0, 0, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_MPU_OPPNITRO_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITRO, 0, 0, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_MPU_OPPNITROSB_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITROSB, 0, 0,  0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0, 0, 0),
};
#else
struct omap_volt_data omap447x_vdd_mpu_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP4470_VDD_MPU_OPP50_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP50, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_MPU_OPP100_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPP100, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_MPU_OPPTURBO_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPTURBO, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_MPU_OPPNITRO_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITRO, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_MPU_OPPNITROSB_UV, 0, OMAP44XX_CONTROL_FUSE_MPU_OPPNITROSB, 0xfa, 0x27, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};
#endif

#define OMAP4470_VDD_IVA_OPP50_UV               950000
#define OMAP4470_VDD_IVA_OPP100_UV             1137000
#define OMAP4470_VDD_IVA_OPPTURBO_UV           1287000
#define OMAP4470_VDD_IVA_OPPNITRO_UV           1375000
#define OMAP4470_VDD_IVA_OPPNITROSB_UV         1376000

#ifdef CONFIG_OMAP_SMARTREFLEX_CUSTOM_SENSOR
struct omap_volt_data omap447x_vdd_iva_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP4470_VDD_IVA_OPP50_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPP50, 0, 0, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_IVA_OPP100_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_IVA_OPPTURBO_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPPTURBO, 0, 0, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_IVA_OPPNITRO_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPPNITRO, 0, 0, 0xfa, 0x23, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_IVA_OPPNITROSB_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPPNITROSB, 0, 0, 0xfa, 0x23, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0, 0, 0),
};
#else
struct omap_volt_data omap447x_vdd_iva_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP4470_VDD_IVA_OPP50_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPP50, 0xf4, 0x0c, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_IVA_OPP100_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPP100, 0xf9, 0x16, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_IVA_OPPTURBO_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPPTURBO, 0xfa, 0x23, OMAP_ABB_NOMINAL_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_IVA_OPPNITRO_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPPNITRO, 0xfa, 0x23, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(OMAP4470_VDD_IVA_OPPNITROSB_UV, 0, OMAP44XX_CONTROL_FUSE_IVA_OPPNITROSB, 0xfa, 0x23, OMAP_ABB_FAST_OPP),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};
#endif

/*
 * OPP100H is OPP100 in High Performance OPP set. It uses the same nTarget.
 * The 1mV difference is introduced just to distinguish a new OPP100 in
 * OPP dependencies tables and maintain correct dependencies in both OPP sets:
 * - in Low set if MPU/IVA OPP >= OPP100 then CORE will be at least at OPP100.
 * - in High set OPP100H voltage is lower than requested OPP100,
 *   so next higher OPP will be used which is OPP100OV.
 */
#define OMAP4470_VDD_CORE_OPP50_UV              962000
#define OMAP4470_VDD_CORE_OPP100H_UV           1124000
#define OMAP4470_VDD_CORE_OPP100_UV            1125000
#define OMAP4470_VDD_CORE_OPP100_OV_UV         1250000

#ifdef CONFIG_OMAP_SMARTREFLEX_CUSTOM_SENSOR
struct omap_volt_data omap447x_vdd_core_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP4470_VDD_CORE_OPP50_UV, 0, OMAP44XX_CONTROL_FUSE_CORE_OPP50, 0, 0, 0xf4, 0x0c, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(OMAP4470_VDD_CORE_OPP100H_UV, 0, OMAP44XX_CONTROL_FUSE_CORE_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(OMAP4470_VDD_CORE_OPP100_UV, 0, OMAP44XX_CONTROL_FUSE_CORE_OPP100, 0, 0, 0xf9, 0x16, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(OMAP4470_VDD_CORE_OPP100_OV_UV, 0, OMAP44XX_CONTROL_FUSE_CORE_OPP100OV, 0, 0, 0xf9, 0x16, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0, 0, 0),
};
#else
struct omap_volt_data omap447x_vdd_core_volt_data[] = {
	VOLT_DATA_DEFINE(OMAP4470_VDD_CORE_OPP50_UV, 0, OMAP44XX_CONTROL_FUSE_CORE_OPP50, 0xf4, 0x0c, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(OMAP4470_VDD_CORE_OPP100H_UV, 0, OMAP44XX_CONTROL_FUSE_CORE_OPP100, 0xf9, 0x16, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(OMAP4470_VDD_CORE_OPP100_UV, 0, OMAP44XX_CONTROL_FUSE_CORE_OPP100, 0xf9, 0x16, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(OMAP4470_VDD_CORE_OPP100_OV_UV, 0, OMAP44XX_CONTROL_FUSE_CORE_OPP100OV, 0xf9, 0x16, OMAP_ABB_NONE),
	VOLT_DATA_DEFINE(0, 0, 0, 0, 0, 0),
};
#endif

/* OMAP 4470 MPU Core VDD dependency table */
static struct omap_vdd_dep_volt omap447x_vdd_mpu_core_dep_data[] = {
	{.main_vdd_volt = OMAP4470_VDD_MPU_OPP50_UV, .dep_vdd_volt = OMAP4470_VDD_CORE_OPP50_UV},
	{.main_vdd_volt = OMAP4470_VDD_MPU_OPP100_UV, .dep_vdd_volt = OMAP4470_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4470_VDD_MPU_OPPTURBO_UV, .dep_vdd_volt = OMAP4470_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4470_VDD_MPU_OPPNITRO_UV, .dep_vdd_volt = OMAP4470_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4470_VDD_MPU_OPPNITROSB_UV, .dep_vdd_volt = OMAP4470_VDD_CORE_OPP100_UV},
};

struct omap_vdd_dep_info omap447x_vddmpu_dep_info[] = {
	{
		.name   = "core",
		.dep_table = omap447x_vdd_mpu_core_dep_data,
		.nr_dep_entries = ARRAY_SIZE(omap447x_vdd_mpu_core_dep_data),
	},
	{.name = NULL, .dep_table = NULL, .nr_dep_entries = 0},
};
/* OMAP 4470 MPU IVA VDD dependency table */
static struct omap_vdd_dep_volt omap447x_vdd_iva_core_dep_data[] = {
	{.main_vdd_volt = OMAP4470_VDD_IVA_OPP50_UV, .dep_vdd_volt = OMAP4470_VDD_CORE_OPP50_UV},
	{.main_vdd_volt = OMAP4470_VDD_IVA_OPP100_UV, .dep_vdd_volt = OMAP4470_VDD_CORE_OPP100H_UV},
	{.main_vdd_volt = OMAP4470_VDD_IVA_OPPTURBO_UV, .dep_vdd_volt = OMAP4470_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4470_VDD_IVA_OPPNITRO_UV, .dep_vdd_volt = OMAP4470_VDD_CORE_OPP100_UV},
	{.main_vdd_volt = OMAP4470_VDD_IVA_OPPNITROSB_UV, .dep_vdd_volt = OMAP4470_VDD_CORE_OPP100_UV},
};

struct omap_vdd_dep_info omap447x_vddiva_dep_info[] = {
	{
		.name   = "core",
		.dep_table = omap447x_vdd_iva_core_dep_data,
		.nr_dep_entries = ARRAY_SIZE(omap447x_vdd_iva_core_dep_data),
	},
	{.name = NULL, .dep_table = NULL, .nr_dep_entries = 0},
};
static struct omap4_ldo_abb_trim_data __initdata omap447x_ldo_abb_trim_data[] = {
	{.volt_data = omap447x_vdd_mpu_volt_data, .volt_nominal = OMAP4470_VDD_MPU_OPPTURBO_UV, .rbb_trim_mask = OMAP4460_MPU_OPP_DPLL_TURBO_RBB},
	{.volt_data = omap447x_vdd_mpu_volt_data, .volt_nominal = OMAP4470_VDD_MPU_OPPNITRO_UV, .rbb_trim_mask = OMAP4470_MPU_OPP_DPLL_NITRO_RBB},
	{.volt_data = omap447x_vdd_mpu_volt_data, .volt_nominal = OMAP4470_VDD_MPU_OPPNITROSB_UV, .rbb_trim_mask = OMAP4470_MPU_OPP_DPLL_NITROSB_RBB},
	{.volt_data = omap447x_vdd_iva_volt_data, .volt_nominal = OMAP4470_VDD_IVA_OPPTURBO_UV, .rbb_trim_mask = OMAP4460_IVA_OPP_DPLL_TURBO_RBB},
	{.volt_data = omap447x_vdd_iva_volt_data, .volt_nominal = OMAP4470_VDD_IVA_OPPNITRO_UV, .rbb_trim_mask = OMAP4470_IVA_OPP_DPLL_NITRO_RBB},
	{.volt_data = omap447x_vdd_iva_volt_data, .volt_nominal = OMAP4470_VDD_IVA_OPPNITROSB_UV, .rbb_trim_mask = OMAP4470_IVA_OPP_DPLL_NITROSB_RBB},
	{.volt_data = NULL},
};

#define OMAP4470_LP_OPP_SET_CORE_DPLL_FREQ 1600000000

static struct omap_opp_def __initdata omap447x_opp_low_def_list[] = {
	/* MPU OPP1 - OPP50 */
	OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", true, 396800000, OMAP4470_VDD_MPU_OPP50_UV),
	/* MPU OPP2 - OPP100 */
	OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", true, 800000000, OMAP4470_VDD_MPU_OPP100_UV),
	/* MPU OPP3 - OPP-Turbo */
	OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", true, 1100000000, OMAP4470_VDD_MPU_OPPTURBO_UV),
	/* MPU OPP4 - OPP-Nitro */
	OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", true, 1300000000, OMAP4470_VDD_MPU_OPPNITRO_UV),
	/* MPU OPP4 - OPP-Nitro SpeedBin */
	OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", false, 1500000000, OMAP4470_VDD_MPU_OPPNITROSB_UV),
	/* L3 OPP1 - OPP50 */
	OPP_INITIALIZER("l3_main_1", "virt_l3_ck", "core", true, 100000000, OMAP4470_VDD_CORE_OPP50_UV),
	/* L3 OPP2 - OPP100 */
	OPP_INITIALIZER("l3_main_1", "virt_l3_ck", "core", true, 200000000, OMAP4470_VDD_CORE_OPP100_UV),
	OPP_INITIALIZER("l3_main_1", "virt_l3_ck", "core", true, 200000000, OMAP4470_VDD_CORE_OPP100_OV_UV),
	/* IVA OPP1 - OPP50 */
	OPP_INITIALIZER("iva", "virt_iva_ck", "iva", true, 133000000, OMAP4470_VDD_IVA_OPP50_UV),
	/* IVA OPP2 - OPP100 */
	OPP_INITIALIZER("iva", "virt_iva_ck", "iva", true, 266100000, OMAP4470_VDD_IVA_OPP100_UV),
	/* IVA OPP3 - OPP-Turbo */
	OPP_INITIALIZER("iva", "virt_iva_ck", "iva", true, 332000000, OMAP4470_VDD_IVA_OPPTURBO_UV),
	/* IVA OPP4 - OPP-Nitro */
	OPP_INITIALIZER("iva", "virt_iva_ck", "iva", false, 430000000, OMAP4470_VDD_IVA_OPPNITRO_UV),
	/* IVA OPP5 - OPP-Nitro SpeedBin*/
	OPP_INITIALIZER("iva", "virt_iva_ck", "iva", false, 500000000, OMAP4470_VDD_IVA_OPPNITROSB_UV),

	/* SGX OPP1 - OPP50 */
	OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 153600000, OMAP4470_VDD_CORE_OPP50_UV),
	/* SGX OPP2 - OPP100 */
	OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 307200000, OMAP4470_VDD_CORE_OPP100_UV),
	/* SGX OPP3 - OPPOV */
	OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 384000000, OMAP4470_VDD_CORE_OPP100_OV_UV),
	/* BB2D OPP1 - OPP50 */
	OPP_INITIALIZER("bb2d", "dpll_per_m6x2_ck", "core", true, 153600000, OMAP4470_VDD_CORE_OPP50_UV),
	/* BB2D OPP1 - OPP100 */
	OPP_INITIALIZER("bb2d", "dpll_per_m6x2_ck", "core", true, 307200000, OMAP4470_VDD_CORE_OPP100_UV),
	/* BB2D OPP2 - OPP-OV */
	OPP_INITIALIZER("bb2d", "dpll_per_m6x2_ck", "core", true, 384000000, OMAP4470_VDD_CORE_OPP100_OV_UV),
	/* FDIF OPP1 - OPP25 */
	OPP_INITIALIZER("fdif", "fdif_fck", "core", true, 32000000, OMAP4470_VDD_CORE_OPP50_UV),
	/* FDIF OPP2 - OPP50 */
	OPP_INITIALIZER("fdif", "fdif_fck", "core", true, 64000000, OMAP4470_VDD_CORE_OPP50_UV),
	/* FDIF OPP3 - OPP100 */
	OPP_INITIALIZER("fdif", "fdif_fck", "core", true, 128000000, OMAP4470_VDD_CORE_OPP100_UV),
	/* DSP OPP1 - OPP50 */
	OPP_INITIALIZER("dsp_c0", "virt_dsp_ck", "iva", true, 232750000, OMAP4470_VDD_IVA_OPP50_UV),
	/* DSP OPP2 - OPP100 */
	OPP_INITIALIZER("dsp_c0", "virt_dsp_ck", "iva", true, 465500000, OMAP4470_VDD_IVA_OPP100_UV),
	/* DSP OPP3 - OPPTB */
	OPP_INITIALIZER("dsp_c0", "virt_dsp_ck", "iva", true, 496000000, OMAP4470_VDD_IVA_OPPTURBO_UV),
	/* HSI OPP1 - OPP50 */
	OPP_INITIALIZER("hsi", "hsi_fck", "core", true, 96000000, OMAP4470_VDD_CORE_OPP50_UV),
	/* HSI OPP2 - OPP100 */
	OPP_INITIALIZER("hsi", "hsi_fck", "core", true, 192000000, OMAP4470_VDD_CORE_OPP100_UV),
	/* ABE OPP1 - OPP50 */
	OPP_INITIALIZER("aess", "abe_clk", "iva", true, 98304000, OMAP4470_VDD_IVA_OPP50_UV),
	/* ABE OPP2 - OPP100 */
	OPP_INITIALIZER("aess", "abe_clk", "iva", true, 196608000, OMAP4470_VDD_IVA_OPP100_UV),
	/* DSS OPP1 - OPP50 */
	OPP_INITIALIZER("dss_dispc", "virt_lcd_pclk", "core", true, 93000000, OMAP4470_VDD_CORE_OPP50_UV),
	/* DSS OPP2 - OPP100 */
	OPP_INITIALIZER("dss_dispc", "virt_lcd_pclk", "core", true, 170000000, OMAP4470_VDD_CORE_OPP100_UV),
};

static struct omap_opp_def __initdata omap447x_opp_high_def_list[] = {
	/* MPU OPP1 - OPP50 */
	OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", true, 396800000, OMAP4470_VDD_MPU_OPP50_UV),
	/* MPU OPP2 - OPP100 */
	OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", true, 800000000, OMAP4470_VDD_MPU_OPP100_UV),
	/* MPU OPP3 - OPP-Turbo */
	OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", true, 1100000000, OMAP4470_VDD_MPU_OPPTURBO_UV),
	/* MPU OPP4 - OPP-Nitro */
	OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", true, 1300000000, OMAP4470_VDD_MPU_OPPNITRO_UV),
	/* MPU OPP4 - OPP-Nitro SpeedBin */
	OPP_INITIALIZER("mpu", "virt_dpll_mpu_ck", "mpu", false, 1500000000, OMAP4470_VDD_MPU_OPPNITROSB_UV),
	/* L3 OPP1 - OPP50 */
	OPP_INITIALIZER("l3_main_1", "virt_l3_ck", "core", true, 116000000, OMAP4470_VDD_CORE_OPP50_UV),
	/* L3 OPP2 - OPP100 */
	OPP_INITIALIZER("l3_main_1", "virt_l3_ck", "core", true, 116000000, OMAP4470_VDD_CORE_OPP100H_UV),
	/* L3 OPP3 - OPP-OV */
	OPP_INITIALIZER("l3_main_1", "virt_l3_ck", "core", true, 233000000, OMAP4470_VDD_CORE_OPP100_OV_UV),
	/* IVA OPP1 - OPP50 */
	OPP_INITIALIZER("iva", "virt_iva_ck", "iva", true, 133000000, OMAP4470_VDD_IVA_OPP50_UV),
	/* IVA OPP2 - OPP100 */
	OPP_INITIALIZER("iva", "virt_iva_ck", "iva", true, 266100000, OMAP4470_VDD_IVA_OPP100_UV),
	/* IVA OPP3 - OPP-Turbo */
	OPP_INITIALIZER("iva", "virt_iva_ck", "iva", true, 332000000, OMAP4470_VDD_IVA_OPPTURBO_UV),
	/* IVA OPP4 - OPP-Nitro */
	OPP_INITIALIZER("iva", "virt_iva_ck", "iva", false, 430000000, OMAP4470_VDD_IVA_OPPNITRO_UV),
	/* IVA OPP5 - OPP-Nitro SpeedBin*/
	OPP_INITIALIZER("iva", "virt_iva_ck", "iva", false, 500000000, OMAP4470_VDD_IVA_OPPNITROSB_UV),
	/* SGX OPP1 - OPP50 */
	OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 192000000, OMAP4470_VDD_CORE_OPP50_UV),
	/* SGX OPP2 - OPP100*/
	OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 307200000, OMAP4470_VDD_CORE_OPP100H_UV),
	/* SGX OPP3 - OPPOV */
	OPP_INITIALIZER("gpu", "dpll_per_m7x2_ck", "core", true, 384000000, OMAP4470_VDD_CORE_OPP100_OV_UV),
	/* BB2D OPP1 - OPP50 */
	OPP_INITIALIZER("bb2d", "dpll_per_m6x2_ck", "core", true, 192000000, OMAP4470_VDD_CORE_OPP50_UV),
	/* BB2D OPP2 - OPP100 */
	OPP_INITIALIZER("bb2d", "dpll_per_m6x2_ck", "core", true, 307200000, OMAP4470_VDD_CORE_OPP100H_UV),
	/* BB2D OPP3 - OPP-OV */
	OPP_INITIALIZER("bb2d", "dpll_per_m6x2_ck", "core", true, 384000000, OMAP4470_VDD_CORE_OPP100_OV_UV),
	/* FDIF OPP1 - OPP25 */
	OPP_INITIALIZER("fdif", "fdif_fck", "core", true, 32000000, OMAP4470_VDD_CORE_OPP50_UV),
	/* FDIF OPP2 - OPP50 */
	OPP_INITIALIZER("fdif", "fdif_fck", "core", true, 64000000, OMAP4470_VDD_CORE_OPP50_UV),
	/* FDIF OPP3 - OPP100 */
	OPP_INITIALIZER("fdif", "fdif_fck", "core", true, 128000000, OMAP4470_VDD_CORE_OPP100H_UV),
	/* DSP OPP1 - OPP50 */
	OPP_INITIALIZER("dsp_c0", "virt_dsp_ck", "iva", true, 232750000, OMAP4470_VDD_IVA_OPP50_UV),
	/* DSP OPP2 - OPP100 */
	OPP_INITIALIZER("dsp_c0", "virt_dsp_ck", "iva", true, 465500000, OMAP4470_VDD_IVA_OPP100_UV),
	/* DSP OPP3 - OPPTB */
	OPP_INITIALIZER("dsp_c0", "virt_dsp_ck", "iva", true, 496000000, OMAP4470_VDD_IVA_OPPTURBO_UV),
	/* HSI OPP1 - OPP50 */
	OPP_INITIALIZER("hsi", "hsi_fck", "core", true, 96000000, OMAP4470_VDD_CORE_OPP50_UV),
	/* HSI OPP2 - OPP100 */
	OPP_INITIALIZER("hsi", "hsi_fck", "core", true, 192000000, OMAP4470_VDD_CORE_OPP100H_UV),
	/* ABE OPP1 - OPP50 */
	OPP_INITIALIZER("aess", "abe_clk", "iva", true, 98304000, OMAP4470_VDD_IVA_OPP50_UV),
	/* ABE OPP2 - OPP100 */
	OPP_INITIALIZER("aess", "abe_clk", "iva", true, 196608000, OMAP4470_VDD_IVA_OPP100_UV),
	/* DSS OPP1 - OPP50 */
	OPP_INITIALIZER("dss_dispc", "virt_lcd_pclk", "core", true, 93000000, OMAP4470_VDD_CORE_OPP50_UV),
	/* DSS OPP2 - OPP100 */
	OPP_INITIALIZER("dss_dispc", "virt_lcd_pclk", "core", true, 170000000, OMAP4470_VDD_CORE_OPP100H_UV),
};

/**
 * omap4_mpu_opp_enable() - helper to enable the OPP
 * @freq:	frequency to enable
 */
static void __init omap4_mpu_opp_enable(unsigned long freq)
{
	struct device *mpu_dev;
	int r;

	mpu_dev = omap2_get_mpuss_device();
	if (!mpu_dev) {
		pr_err("%s: no mpu_dev, did not enable f=%ld\n", __func__,
			freq);
		return;
	}

	r = opp_enable(mpu_dev, freq);
	if (r < 0)
		dev_err(mpu_dev, "%s: opp_enable failed(%d) f=%ld\n", __func__,
			r, freq);
}

 /**
 * omap4_abb_update() - update the ABB map for a specific voltage in table
 * @vtable:    voltage table to update
 * @voltage:   voltage whose voltage data needs update
 * @abb_type:  what ABB type should we update it to?
 */
static void __init omap4_abb_update(struct omap_volt_data *vtable,
				    unsigned long voltage, int abb_type)
{
	/* scan through and update the voltage table */
	while (vtable->volt_nominal) {
		if (vtable->volt_nominal == voltage) {
			vtable->abb_type = abb_type;
			return;
		}
		vtable++;
	}
	/* WARN noticably to get the developer to fix */
	WARN(1, "%s: voltage %ld could not be set to ABB %d\n",
	    __func__, voltage, abb_type);
}

/**
 * omap4_abb_trim_update() - update the ABB mapping quirks for OMAP4460/4470
 */
static void __init omap4_abb_trim_update(
		struct omap4_ldo_abb_trim_data *trim_data)
{
	u32 reg;
	int abb_type;

	if (!trim_data) {
		pr_err("%s: Trim data is not valid\n", __func__);
		return;
	}

	reg = omap_ctrl_readl(OMAP4_CTRL_MODULE_CORE_STD_FUSE_OPP_DPLL_1);

	/* Update ABB configuration if at least one of trim bits is set
	 * Leave default configuration in opposite case. */
	for (; trim_data->volt_data; trim_data++) {
		if (reg & trim_data->rbb_trim_mask)
			abb_type = OMAP_ABB_SLOW_OPP;
		else if (reg & trim_data->fbb_trim_mask)
			abb_type = OMAP_ABB_FAST_OPP;
		else
			continue;

		omap4_abb_update(trim_data->volt_data, trim_data->volt_nominal,
				abb_type);
	}
}

/**
 * omap4_gpu_opp_enable() - helper to enable the OPP
 * @freq:	frequency to enable
 */
static void __init omap4_gpu_opp_enable(unsigned long freq)
{
	struct device *gpu_dev;
	int r;

	gpu_dev = omap2_get_gpu_device();
	if (!gpu_dev) {
		pr_err("%s: no gpu_dev, did not enable f=%ld\n", __func__,
			freq);
		return;
	}

	r = opp_enable(gpu_dev, freq);
	if (r < 0)
		dev_err(gpu_dev, "%s: opp_enable failed(%d) f=%ld\n", __func__,
			r, freq);
}

/**
 * omap4_opp_init() - initialize omap4 opp table
 */
int __init omap4_opp_init(void)
{
	int r = -ENODEV;
	int trimmed = 1;

	if (!cpu_is_omap44xx())
		return r;
	if (cpu_is_omap443x())
		r = omap_init_opp_table(omap443x_opp_def_list,
			ARRAY_SIZE(omap443x_opp_def_list));
	else if (cpu_is_omap446x()) {
		omap4_abb_trim_update(omap446x_ldo_abb_trim_data);

		r = omap_init_opp_table(omap446x_opp_def_list,
			ARRAY_SIZE(omap446x_opp_def_list));
		trimmed = omap_readl(0x4a002268) & ((1 << 18) | (1 << 19));
		/* if device is untrimmed override DPLL TRIM register */
		if (!trimmed)
			omap_writel(0x29, 0x4a002330);
	} else if (cpu_is_omap447x()) {
		struct clk *dpll_core_ck;
		unsigned long rate = 0;

		omap4_abb_trim_update(omap447x_ldo_abb_trim_data);

		dpll_core_ck = clk_get(NULL, "dpll_core_ck");
		if (dpll_core_ck) {
			rate = clk_get_rate(dpll_core_ck);
			clk_put(dpll_core_ck);
		}

		if (rate > OMAP4470_LP_OPP_SET_CORE_DPLL_FREQ / 2)
			r = omap_init_opp_table(omap447x_opp_high_def_list,
					ARRAY_SIZE(omap447x_opp_high_def_list));
		else
			r = omap_init_opp_table(omap447x_opp_low_def_list,
					ARRAY_SIZE(omap447x_opp_low_def_list));
	}

	if (!r) {
		if (omap4_has_mpu_1_2ghz()) 
			omap4_mpu_opp_enable(1200000000);
			omap4_mpu_opp_enable(1350000000);
	
		if (!trimmed)
			pr_info("This is DPLL un-trimmed SOM. OPP is limited at 1.2 GHz\n"); 
		if (omap4_has_mpu_1_5ghz() && trimmed)
			omap4_mpu_opp_enable(1500000000); 
	}

	/*struct device *dev;

        dev = omap_hwmod_name_get_dev("gpu");
	opp_enable(dev, 512000000); */

			omap4_gpu_opp_enable(153600000);
			omap4_gpu_opp_enable(384000000);
			omap4_gpu_opp_enable(416000000);

#ifdef CONFIG_CUSTOM_VOLTAGE
	customvoltage_init();
#endif

#ifdef CONFIG_LIVE_OC
	liveoc_init();
#endif
	return r;
}
device_initcall(omap4_opp_init);

