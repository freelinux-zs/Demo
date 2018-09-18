/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/***************************************************************
**************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/


#ifdef BUILD_LK
#include <debug.h>
#include "lcm_drv.h"
#include <platform/upmu_common.h>
#include <platform/upmu_hw.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h> 
#include <platform/mt_pmic.h>
#include <string.h>
#include <cust_gpio_usage.h>
#else
#include <linux/string.h>
#include <linux/kernel.h>
#include "lcm_drv.h"
#include <upmu_common.h>
//#include <mt_gpio.h>
#include <linux/gpio.h>
#endif

#ifndef MACH_FPGA
#include <cust_i2c.h>
#endif

#define LOG_TAG "LCM"

#ifdef BUILD_LK
#define LCM_LOGI(string, args...)  dprintf(0, "[LK/"LOG_TAG"]"string, ##args)
#define LCM_LOGD(string, args...)  dprintf(1, "[LK/"LOG_TAG"]"string, ##args)
#else
#define LCM_LOGI(fmt, args...)  pr_notice("[KERNEL/"LOG_TAG"]"fmt, ##args)
#define LCM_LOGD(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#endif




// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (1920)
#define FRAME_HEIGHT 										(1080)
#define LCM_ID                       						(0x8006)

#define REGFLAG_DELAY             							(0XFFFE)
#define REGFLAG_END_OF_TABLE      							(0xFff0)	// END OF REGISTERS MARKER


#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#ifndef MACH_FPGA
#define GPIO_KTD2152_V5P_EN GPIO_LCD_BIAS_ENP_PIN
#define GPIO_KTD2152_V5N_EN GPIO_LCD_BIAS_ENN_PIN
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

#define LCM_FT8006M_ID 		(0x38)

//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)


#define set_gpio_lcd_enp(cmd) \
		lcm_util.set_gpio_lcd_enp_bias(cmd)



#define SLAVE_ADDR_WRITE      0x1e
#define I2C_ID                I2C0

static struct mt_i2c_t i2c;


u32 i2c_d2dp_write(u16 addr, u32 value)
{
	 u32 ret_code = I2C_OK;
	u8  write_data[6];
	u8  len,i;
	write_data[0] = addr>>8;
	write_data[1] = addr;
	
	write_data[2] = value;
	write_data[3] = value>>8;
	write_data[4] = value>>16;
	write_data[5] = value>>24;

	i2c.id    = I2C_ID;
	i2c.addr = (SLAVE_ADDR_WRITE >> 1);
 	i2c.mode  = ST_MODE;
 	i2c.speed = 100;
	len = 6;

	ret_code = i2c_write(&i2c, write_data, len);
	return ret_code;
	
}



 struct LCM_setting_table {
    u16 cmd;
    u32 data;
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x06A0,0x11113087}, //DP0_SrcCtrl"
	{0x07A0,0x11113002}, //DP1_SrcCtrl"
	{0x0918,0x00000101}, //SYS_PLLPARAM"

	//DP-PHY/PLLs
	{0x0800,0x03000007}, //DP_PHY_CTRL"
	{0x0900,0x00000005}, //DP0_PLLCTRL"
	{0x0904,0x00000005}, //DP1_PLLCTRL"

	//MDELAY(1);
	{REGFLAG_DELAY,1},

	{0x0914,0x01330141}, //PXL_PLLPARAM"
	{0x0908,0x00000005}, //PXL_PLLCTRL"

	//   MDELAY(1);
	{REGFLAG_DELAY,1},

	//Reset/Enable Main Link(s)
	{0x0800,0x13001107}, //DP_PHY_CTRL"
	{0x0800,0x03000007}, //DP_PHY_CTRL"

	// MDELAY(35);
	{REGFLAG_DELAY,35},

	//Set Link Rate and Lane Count (DPCD 00100h,00101h)"
	{0x0668,0x00000100}, //DP0_AuxAddr"
	{0x066C,0x0000020A}, //DP0_AuxWData0"
	{0x0660,0x00000108}, //DP0_AuxCfg0"

	//Set Main Link Channel Coding (DPCD 00108h)
	{0x0668,0x00000108}, //DP0_AuxAddr"
	{0x066C,0x00000001}, //DP0_AuxWData0"
	{0x0660,0x00000008}, //DP0_AuxCfg0"

	//Write DPCD 00103h~00104h before Training Pat1
	{0x0668,0x00000103}, //DP0_AuxAddr"
	{0x066C,0x00000101}, //DP0_AuxWData0"
	{0x0660,0x00000108}, //DP0_AuxCfg0"

	//Set DPCD 00102h for Training Pat 1
	{0x06E4,0x00000021}, //DP0_SnkLTCtrl"
	{0x06D8,0xF600000D}, //DP0_LTLoopCtrl"

	//Set DP0 Trainin Pattern 1
	{0x06A0,0x01013086}, //DP0_SrcCtrl"
	{0x06A0,0x01013187}, //DP0_SrcCtrl"

	//Enable DP0 to start Link Training
	{0x0600,0x00000001}, //DP0Ctl"

	{REGFLAG_DELAY,10},
	// MDELAY(10);

	//Set DPCD 00102h for Link Traing Pat 2
	{0x06E4,0x00000022}, //DP0_SnkLTCtrl"

	//Write DPCD 00103h~00104h before Training Pat2
	{0x0668,0x00000103}, //DP0_AuxAddr"
	{ 0x066C,0x00000909}, //DP0_AuxWData0"
	{0x0660,0x00000108}, //DP0_AuxCfg0"

	//Set DP0 Trainin Pattern 2
	{0x06A0,0x11113086}, //DP0_SrcCtrl"
	{0x06A0,0x11113287}, //DP0_SrcCtrl"

	{REGFLAG_DELAY,10},
	//   MDELAY(10);
	{REGFLAG_DELAY,10},

	//Clear DPCD 00102h
	{0x0668,0x00000102}, //AuxAddr"
	{0x066C,0x00000000}, //AuxWData0"
	{0x0660,0x00000008}, //AuxCfg0"

	//Clear DP0 Trainin Pattern
	{0x06A0,0x11111087}, //DP0_SrcCtrl"

	//Write DPCD 0010Ah (ASSR, Enhanced Framing)"
	{0x0668,0x0000010A}, //AuxAddr"
	{0x066C,0x00000000}, //AuxWData0"
	{0x0660,0x00000008}, //AuxCfg0"

	//Write DPCD 00107h (SSCG)
	{0x0668,0x00000107}, //AuxAddr"
	{0x066C,0x00000000}, //AuxWData0"
	{0x0660,0x00000008}, //AuxCfg0"
	{0x013C,0x00070009}, //PPI_TX_RX_TA"
	{0x0114,0x00000006}, //PPI_LPTXTIMECNT"
	{0x0164,0x00000008}, //CLRSIPOCOUNT"
	{0x0168,0x00000008}, //CLRSIPOCOUNT"
	{0x016C,0x00000008}, //CLRSIPOCOUNT"
	{0x0170,0x00000008}, //CLRSIPOCOUNT"
	{0x0134,0x0000001F}, //PPI_LANEENABLE"
	{0x0210,0x0000001F}, //DSI_LANEENABLE"
	{0x0104,0x00000001}, //PPI_STARTPPI"
	{0x0204,0x00000001}, //DSI_STARTDSI"

	//LCD Ctl Frame Size
	{0x0450,0x06400100}, //VPCTRL0"
	{0x0454,0x005C0020}, //HTIM01"
	{0x0458,0x00280780}, //HTIM02"
	{0x045C,0x00140006}, //VTIM01"
	{0x0460,0x00030438}, //VTIM02"
	{0x0464,0x00000001}, //VFUEN0"

	//DP Main Stream Attirbutes
	{0x0644,0x003E07FC}, //DP0_VidSyncDly"
	{0x0648,0x04550824}, //DP0_TotalVal"
	{0x064C,0x001A007C}, //DP0_StartVal"
	{0x0650,0x04380780}, //DP0_ActiveVal"
	{0x0654,0x80068020}, //DP0_SyncVal"

	//DP Flow Shape & TimeStamp
	{0x0658,0x1F3F0020}, //DP0_Misc"

	//For Color Bar Test					
	//    {0x0A00,0x78006312}, //D2DPTestCtl
	//Nvid/Mvid setting
	{0x0610,0x00006180}, //DP0_Vid_fM"
	{0x0614,0x0000BDD8}, //DP0_Vid_N"

	//Enable Video and DP link
	{0x0600,0x00000041},//DP0Ctl"
	{0x0600,0x00000043}, //DP0Ctl"
	{0x0510,0x00000001}, //SYSCTRL"
};


/*
static struct LCM_setting_table lcm_sleep_out_setting[] = {
	// Sleep Out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	// Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
*/



static void push_table(struct LCM_setting_table *table, unsigned int count,
		unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {

		u16 cmd;
		cmd = table[i].cmd;

		switch (cmd) {

		case REGFLAG_DELAY:
			MDELAY(table[i].data);
			break;

		case REGFLAG_END_OF_TABLE:
			break;

		default:
			i2c_d2dp_write(cmd, table[i].data);
		}
	}

}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS * util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS * params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;


	params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//SYNC_EVENT_VDO_MODE;//BURST_VDO_MODE;////
	params->dsi.word_count =  FRAME_WIDTH  * 3;
	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
 	
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
	
	
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	
//	params->dsi.intermediat_buffer_num = 0;	//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

	params->dsi.pll_select = 0;
	params->dsi.cont_clock = 1;

	params->dsi.vertical_sync_active				=  6;
	params->dsi.vertical_backporch					= 20;//16 25 30 35 12 8
	params->dsi.vertical_frontporch					= 3;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 32;
	params->dsi.horizontal_backporch				= 192; 
	params->dsi.horizontal_frontporch				= 140;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK						= 469;

}

static void lcm_init(void)
{

	DSI_clk_HS_mode(0, NULL, 0);
	
	MDELAY(100);

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	DSI_clk_HS_mode(0, NULL, 1);
	MDELAY(100);
}

static void lcm_suspend(void)
{
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	
	SET_RESET_PIN(0);
	MDELAY(10);
}


static void lcm_resume(void)
{
	lcm_init();
}


static unsigned int lcm_compare_id(void)
{

	return 1;


}
LCM_DRIVER ft8006m_with_ktd2151_dsi_vdo_h570tax0_ns570hd3901az01_rx_mx2532e_ztr_s562z_lcm_drv = 
{
	.name			= "ft8006m_with_ktd2151_dsi_vdo_h570tax0_ns570hd3901az01_rx_mx2532e_ztr_s562z",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,	
	.compare_id     = lcm_compare_id,	

#if (LCM_DSI_CMD_MODE)
    //.update         = lcm_update,
#endif	//wqtao
};

