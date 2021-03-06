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
#include <cust_i2c.h>
#else
#include <linux/time.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include "lcm_drv.h"
#include <upmu_common.h>
//#include <mt_gpio.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/irq.h>
/*#include <linux/jiffies.h> */
#include <linux/uaccess.h>
/*#include <linux/delay.h> */
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include "ddp_hal.h"
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

#define FRAME_WIDTH  										(1920)
#define FRAME_HEIGHT 										(1080)


#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))
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


extern void DSI_clk_HS_mode(DISP_MODULE_ENUM module, void *cmdq, bool enter);


#ifdef CONFIG_MTK_LEGACY
#define TPS_I2C_BUSNUM  I2C_I2C_LCD_BIAS_CHANNEL/*for I2C channel 0 */
#endif
#define I2C_ID_NAME 	"tc358767"
#define TPS_ADDR		 (0x1e >> 1)
/*****************************************************************************
* GLobal Variable
*****************************************************************************/
#ifdef CONFIG_MTK_LEGACY
static struct i2c_board_info tc358767_board_info __initdata = {I2C_BOARD_INFO(I2C_ID_NAME, TPS_ADDR)};
#endif

#if !defined(CONFIG_MTK_LEGACY)
static const struct of_device_id lcm_of_match[] = {
		{.compatible = "mediatek,i2c_lcd_bias"},
		{},
};
#endif


#if 1
static struct i2c_client *tc358767_i2c_client;
#define SYN_I2C_RETRY_TIMES 10
#define MASK_8BIT 0xFF
#define PAGE_SELECT_LEN (2)


/*****************************************************************************
* Function Prototype
*****************************************************************************/
static int tc358767_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tc358767_remove(struct i2c_client *client);
/*****************************************************************************
* Data Structure
*****************************************************************************/

struct tc358767_dev	{
	struct i2c_client	*client;
	u8 current_page;
};

//static struct mutex io_ctrl_mutex;  
//static struct tc358767_dev *ts;

static const struct i2c_device_id tc358767_id[] = {
	{ I2C_ID_NAME, 0 },
	{ }
};

/*
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
static struct i2c_client_address_data addr_data = { .forces = forces,};
#endif
*/
static struct i2c_driver tc358767_iic_driver = {
	.id_table	= tc358767_id,
	.probe		= tc358767_probe,
	.remove		= tc358767_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "tc358767",
#if !defined(CONFIG_MTK_LEGACY)
		.of_match_table = lcm_of_match,
#endif
	},
};
/*****************************************************************************
* Extern Area
*****************************************************************************/

/*****************************************************************************
* Function
*****************************************************************************/
static int tc358767_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk("xuzhoubin TPS: info==>name=%s addr=0x%x\n", client->name, client->addr);
	tc358767_i2c_client  = client;
	tc358767_i2c_client->addr =  TPS_ADDR;
	// mutex_init(&io_ctrl_mutex);
	return 0;
}


static int tc358767_remove(struct i2c_client *client)
{
	pr_debug("tps65132_remove\n");
	tc358767_i2c_client = NULL;
	i2c_unregister_device(client);
	return 0;
}

#if 0
static int tc358767_set_page(struct i2c_client *client, unsigned int address)
{
	int retval = 0; 
	unsigned char retry = 0;
	unsigned char buf[PAGE_SELECT_LEN];
	unsigned char page;
    
    page = ((address >> 8) & MASK_8BIT);
	if (page != ts->current_page) {
		buf[0] = MASK_8BIT;
		buf[1] = page;
		for ( retry = 0; retry < SYN_I2C_RETRY_TIMES; retry++) {
			retval = i2c_master_send(client, buf, PAGE_SELECT_LEN);
			if (retval != PAGE_SELECT_LEN) {
//dev_err(&client->dev, "%s: I2C retry %d\n", __func__, retry + 1);
				MDELAY(20);
			} else {
				ts->current_page = page;
				break;
		}    
	}    
    }else{
		 retval = PAGE_SELECT_LEN;
	}
	
    return retval;
}

static int i2c_common_send(struct i2c_client *client, unsigned short addr, unsigned char *data, unsigned short length)
{
	u8 retval = 0;
	u8 retry = 0;
	u8 *pData = data;
	u8 buf[5] = { 0 };
	int tmp_addr = addr;
	int left_len = length;
	
	 //mutex_lock(&io_ctrl_mutex);
	  retval = tc358767_set_page(client, addr);
	   if (retval != PAGE_SELECT_LEN) {
		    printk("tc358767_set_page fail, retval = %d\n", retval);
			 retval = -EIO;
			  goto exit;
	   }
	 while (left_len > 0) {
		 buf[0] = tmp_addr;
		 for (retry = 0; retry < SYN_I2C_RETRY_TIMES; retry++) {
			 if (left_len > 4) {
				 memcpy(buf + 1, pData, 4);
				 retval = i2c_master_send(client, buf, 5);
				 } else {
					 memcpy(buf + 1, pData, left_len);
					  retval = i2c_master_send(client, buf, left_len + 1);
					  }
			if (retval > 0)
				break;
			printk("%s: I2C retry %d\n", __func__, retry + 1);
			MDELAY(20);
			}
		left_len -= 4;
		pData += 4;
		tmp_addr += 4;
	}
	
exit:
	//mutex_unlock(&io_ctrl_mutex);
	return retval;
}

#endif
  
int tc358767_write_bytes(unsigned short RegAddr, unsigned int value)
{

	#if 0
	int ret = 0;
	struct i2c_client *client = tc358767_i2c_client;
	char puSendCmd[6] = { (char)((RegAddr >> 8) & 0xFF), (char)(RegAddr & 0xFF), (char)(value & 0xFF),(char) (value >> 8) & 0xFF , (char) (value >> 16) & 0xFF, (char) (value >> 24) & 0xFF };
	if (client == NULL) {
		pr_debug("ERROR!!tps65132_i2c_client is null\n");
		return 0;
	}

	ret = i2c_master_send(client, puSendCmd, 6);

	if (ret < 0){
		pr_debug("tps65132 write data fail !!\n");
	}
	return ret;
	#else
	
	unsigned char len = 6;
	struct i2c_client *client = tc358767_i2c_client;
	char puSendCmd[6] = { (char)((RegAddr >> 8) & 0xFF), (char)(RegAddr & 0xFF), (char)(value & 0xFF),(char) (value >> 8) & 0xFF , (char) (value >> 16) & 0xFF, (char) (value >> 24) & 0xFF };
	int err = 0;
	unsigned char *buffer = NULL;
	struct i2c_msg msg;
	memset(&msg, 0, sizeof(struct i2c_msg));
	buffer = kzalloc(len+ 1, GFP_KERNEL);
	if (buffer == NULL)
		return -1;
	memcpy((buffer), puSendCmd, len);
	//msg.addr = client->addr |= I2C_DMA_FLAG;
	msg.addr = client->addr & (I2C_MASK_FLAG|I2C_DMA_FLAG);
	msg.ext_flag = client->ext_flag;
	msg.timing = 100;
	msg.flags = 0;
	msg.len = len + 1;
	msg.buf = buffer;
	err = i2c_transfer(client->adapter, &msg, 1);
	if (err != 1)
		err = -EIO;
	else
		err = 0;
	kfree(buffer);
	return err;
	  
	#endif
	
}
EXPORT_SYMBOL_GPL(tc358767_write_bytes);


/*
* module load/unload record keeping
*/

static int __init tc358767_iic_init(void)
{
	pr_debug("tc358767_iic_init\n");
#ifdef CONFIG_MTK_LEGACY
	i2c_register_board_info(TPS_I2C_BUSNUM, &tps65132_board_info, 1);
	pr_debug("tc358767_iic_init2\n");
#endif
	i2c_add_driver(&tc358767_iic_driver);
	pr_debug("tc358767_iic_init success\n");
	return 0;
}

static void __exit tc358767_iic_exit(void)
{
	pr_debug("tc358767_iic_exit\n");
	i2c_del_driver(&tc358767_iic_driver);
}

module_init(tc358767_iic_init);
module_exit(tc358767_iic_exit);

MODULE_AUTHOR("Zhoubin Xu");
MODULE_DESCRIPTION("MTK tc3587672 I2C Driver");
MODULE_LICENSE("GPL");

#endif

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

static void init_tc358767(void)
{
	printk("xuzhoubin ...lcm_init\r\n");
	
	MDELAY(100);
	tc358767_write_bytes( 0x06A0,0x11113087); //DP0_SrcCtrl"
	tc358767_write_bytes( 0x07A0,0x11113002); //DP1_SrcCtrl"
	tc358767_write_bytes( 0x0918,0x00000101); //SYS_PLLPARAM"

	//DP-PHY/PLLs
	tc358767_write_bytes( 0x0800,0x03000007); //DP_PHY_CTRL"
	tc358767_write_bytes( 0x0900,0x00000005); //DP0_PLLCTRL"
	tc358767_write_bytes( 0x0904,0x00000005); //DP1_PLLCTRL"

	MDELAY(1);

	tc358767_write_bytes( 0x0914,0x01330141); //PXL_PLLPARAM"
	tc358767_write_bytes( 0x0908,0x00000005); //PXL_PLLCTRL"

	MDELAY(1);

	//Reset/Enable Main Link(s)
	tc358767_write_bytes( 0x0800,0x13001107); //DP_PHY_CTRL"
	tc358767_write_bytes( 0x0800,0x03000007); //DP_PHY_CTRL"

	MDELAY(35);

	//Set Link Rate and Lane Count (DPCD 00100h,00101h)"
	tc358767_write_bytes( 0x0668,0x00000100); //DP0_AuxAddr"
	tc358767_write_bytes( 0x066C,0x0000020A); //DP0_AuxWData0"
	tc358767_write_bytes( 0x0660,0x00000108); //DP0_AuxCfg0"

	//Set Main Link Channel Coding (DPCD 00108h)
	tc358767_write_bytes( 0x0668,0x00000108); //DP0_AuxAddr"
	tc358767_write_bytes( 0x066C,0x00000001); //DP0_AuxWData0"
	tc358767_write_bytes( 0x0660,0x00000008); //DP0_AuxCfg0"

	//Write DPCD 00103h~00104h before Training Pat1
	tc358767_write_bytes( 0x0668,0x00000103); //DP0_AuxAddr"
	tc358767_write_bytes( 0x066C,0x00000101); //DP0_AuxWData0"
	tc358767_write_bytes( 0x0660,0x00000108); //DP0_AuxCfg0"

	//Set DPCD 00102h for Training Pat 1
	tc358767_write_bytes( 0x06E4,0x00000021); //DP0_SnkLTCtrl"
	tc358767_write_bytes( 0x06D8,0xF600000D); //DP0_LTLoopCtrl"

	//Set DP0 Trainin Pattern 1
	tc358767_write_bytes( 0x06A0,0x01013086); //DP0_SrcCtrl"
	tc358767_write_bytes( 0x06A0,0x01013187); //DP0_SrcCtrl"

	//Enable DP0 to start Link Training
	tc358767_write_bytes( 0x0600,0x00000001); //DP0Ctl"

	MDELAY(10);

	//Set DPCD 00102h for Link Traing Pat 2
	tc358767_write_bytes( 0x06E4,0x00000022); //DP0_SnkLTCtrl"

	//Write DPCD 00103h~00104h before Training Pat2
	tc358767_write_bytes( 0x0668,0x00000103); //DP0_AuxAddr"
	tc358767_write_bytes( 0x066C,0x00000909); //DP0_AuxWData0"
	tc358767_write_bytes( 0x0660,0x00000108); //DP0_AuxCfg0"

	//Set DP0 Trainin Pattern 2
	tc358767_write_bytes( 0x06A0,0x11113086); //DP0_SrcCtrl"
	tc358767_write_bytes( 0x06A0,0x11113287); //DP0_SrcCtrl"

	MDELAY(10);

	//Clear DPCD 00102h
	tc358767_write_bytes( 0x0668,0x00000102); //AuxAddr"
	tc358767_write_bytes( 0x066C,0x00000000); //AuxWData0"
	tc358767_write_bytes( 0x0660,0x00000008); //AuxCfg0"

	//Clear DP0 Trainin Pattern
	tc358767_write_bytes( 0x06A0,0x11111087); //DP0_SrcCtrl"

	//Write DPCD 0010Ah (ASSR, Enhanced Framing)"
	tc358767_write_bytes( 0x0668,0x0000010A); //AuxAddr"
	tc358767_write_bytes( 0x066C,0x00000000); //AuxWData0"
	tc358767_write_bytes( 0x0660,0x00000008); //AuxCfg0"

	//Write DPCD 00107h (SSCG)
	tc358767_write_bytes( 0x0668,0x00000107); //AuxAddr"
	tc358767_write_bytes( 0x066C,0x00000000); //AuxWData0"
	tc358767_write_bytes( 0x0660,0x00000008); //AuxCfg0"
	tc358767_write_bytes( 0x013C,0x00070009); //PPI_TX_RX_TA"
	tc358767_write_bytes( 0x0114,0x00000006); //PPI_LPTXTIMECNT"
	tc358767_write_bytes( 0x0164,0x00000008); //CLRSIPOCOUNT"
	tc358767_write_bytes( 0x0168,0x00000008); //CLRSIPOCOUNT"
	tc358767_write_bytes( 0x016C,0x00000008); //CLRSIPOCOUNT"
	tc358767_write_bytes( 0x0170,0x00000008); //CLRSIPOCOUNT"
	tc358767_write_bytes( 0x0134,0x0000001F); //PPI_LANEENABLE"
	tc358767_write_bytes( 0x0210,0x0000001F); //DSI_LANEENABLE"
	tc358767_write_bytes( 0x0104,0x00000001); //PPI_STARTPPI"
	tc358767_write_bytes( 0x0204,0x00000001); //DSI_STARTDSI"

	//LCD Ctl Frame Size
	tc358767_write_bytes( 0x0450,0x06400100); //VPCTRL0"
	tc358767_write_bytes( 0x0454,0x005C0020); //HTIM01"
	tc358767_write_bytes( 0x0458,0x00280780); //HTIM02"
	tc358767_write_bytes( 0x045C,0x00140006); //VTIM01"
	tc358767_write_bytes( 0x0460,0x00030438); //VTIM02"
	tc358767_write_bytes( 0x0464,0x00000001); //VFUEN0"

	//DP Main Stream Attirbutes
	tc358767_write_bytes( 0x0644,0x003E07FC); //DP0_VidSyncDly"
	tc358767_write_bytes( 0x0648,0x04550824); //DP0_TotalVal"
	tc358767_write_bytes( 0x064C,0x001A007C); //DP0_StartVal"
	tc358767_write_bytes( 0x0650,0x04380780); //DP0_ActiveVal"
	tc358767_write_bytes( 0x0654,0x80068020); //DP0_SyncVal"

	//DP Flow Shape & TimeStamp
	tc358767_write_bytes( 0x0658,0x1F3F0020); //DP0_Misc"

	//For Color Bar Test		
	//tc358767_write_bytes( 0x0A00,0x78006312); //D2DPTestCtl
	
	//Nvid/Mvid setting
	tc358767_write_bytes( 0x0610,0x00006180); //DP0_Vid_fM"
	tc358767_write_bytes( 0x0614,0x0000BDD8); //DP0_Vid_N"

	//Enable Video and DP link
	tc358767_write_bytes( 0x0600,0x00000041); //DP0Ctl"
	tc358767_write_bytes( 0x0600,0x00000043); //DP0Ctl"
	tc358767_write_bytes( 0x0510,0x00000001); //SYSCTRL"
	//tc358767_write_bytes( 0x0510,0x00000003); //SYSCTRL"
	MDELAY(100);

}


static void lcm_init(void)
{
	DSI_clk_HS_mode(0, NULL, 0);
	MDELAY(100);
	init_tc358767();
	DSI_clk_HS_mode(0, NULL, 1);
	MDELAY(100);
}

static void lcm_suspend(void)
{
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);

	/*SET_RESET_PIN(0);*/
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

