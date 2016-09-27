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

/*****************************************************************************
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
#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
    #include <platform/disp_drv_platform.h>
#elif defined(BUILD_UBOOT)
    #include <asm/arch/mt_gpio.h>
#else
    #include <linux/delay.h>
    #include <mach/mt_gpio.h>
#endif
const static unsigned char lcd_ID_cmi = 0x00;
const static unsigned char lcd_ID_auo = 0x03;
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE									1
#if (LCM_DSI_CMD_MODE)
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#else
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1023)
#endif

#define REGFLAG_DELAY             							0xFE
#define REGFLAG_END_OF_TABLE      							0xFC   // END OF REGISTERS MARKER


#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif
#define NT35590    0
static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
//#define LCM_DSI_CMD_MODE									1

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------


const static unsigned int BL_MIN_LEVEL = 20;

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

//#define UDELAY(n) 											(lcm_util.udelay(n))
//#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};

//update initial param for IC 1.1
static struct LCM_setting_table lcm_initialization_setting_cmi[] = {

{0xff,3,{0x12,0x82,0x01}},        //EXTC=1
{0x00,1,{0x80}},                    //Orise mode enable
{0xff,2,{0x12,0x82}},

//------- LTPS Timing Setting ------------------------------------------------------------------------------//
{0x00,1,{0x82}},
{0xA5,1,{0x02}},                //LTPS Timing Setting

{0x00,1,{0x80}},
{0xC0,10,{0x00,0x8B,0x00,0x0A,0x0A,0x00,0x8B,0x0A,0x0A,0x00}}, //TCON Setting 1

{0x00,1,{0xA0}},
{0xC0,12,{0x00,0x00,0x00,0x03,0x00,0x1E,0x06,0x00,0x00,0x00,0x00,0x00}}, //LTPS SELR/G/B Setting

{0x00,1,{0x80}},
{0xC2,8,{0x82,0x03,0x00,0x00,0x00,0x00,0x00,0x00}}, //LTPS STV Setting

{0x00,1,{0x90}},
{0xC2,15,{0x87,0x0C,0x01,0x07,0x06,0x86,0x0C,0x01,0x07,0x06,0x85,0x0D,0x01,0x07,0x06}}, //LTPS CKV Setting 1

{0x00,1,{0xA0}},
{0xC2,15,{0x84,0x0D,0x01,0x07,0x06,0x87,0x0B,0x01,0x07,0x06,0x86,0x0B,0x01,0x07,0x06}}, //LTPS CKV Setting 2

{0x00,1,{0xB0}},
{0xC2,10,{0x85,0x0D,0x01,0x07,0x06,0x84,0x0D,0x01,0x07,0x06}}, //LTPS CKV Setting 3

{0x00,1,{0xEA}},
{0xC2,3,{0x22,0x02,0x11}},        //LTPS CKV Setting 4

{0x00,1,{0xF9}},
{0xC2,4,{0x08,0x00,0x0C,0x01}},

{0x00,1,{0x90}},
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0xf0}},

{0x00,1,{0xA0}},
{0xCB,15,{0xf0,0x00,0x00,0x00,0x00,0xc0,0x00,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xB0}},
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x50,0xc0,0x00,0x00,0xc0,0x00}},

//{0x00,1,{0xA0}},// only for video mode
//{0xC1,2,{0x00,0xEE}},
{0x00,1,{0x00}},
{0x1C,1,{0x20}},/*orise advise*/
{0x00,1,{0xB3}}, //column inverse
{0xC0,1,{0x33}},

{0x00,1,{0xB4}},
{0xC0,1,{0x40}},//10

{0x00,1,{0x90}},
{0xA5,1,{0x00}},

{0x00,1,{0x80}},//only for CUT 1.0, CUT1.1 not needed
{0xF5,2,{0x21,0x18}},

{0x00,1,{0x91}},//only for CUT 1.0, CUT1.1 not needed
{0xB0,1,{0x9E}},

{0x00,1,{0xD2}},//only for CUT 1.0, CUT1.1 not needed
{0xF5,1,{0x20}},
/*orise advise used default
{0x00,1,{0x90}},//only for CUT 1.0, CUT1.1 not needed
{0xF6,1,{0x20}},
*/
{0x00,1,{0x81}},//only for CUT 1.0, CUT1.1 not needed
{0xC4,1,{0x04}},

{0x00,1,{0xC0}},
{0xCB,15,{0x14,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x14,0x04,0x04,0x14,0x3b,0x3b}},

{0x00,1,{0xD0}},
{0xCB,15,{0x3b,0x14,0x04,0x04,0x30,0x35,0x04,0xf7,0x04,0x14,0x14,0x14,0x14,0x04,0x04}},

{0x00,1,{0xE0}},
{0xCB,15,{0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x15,0x15,0x07,0x14,0x14,0x00,0x00}},

{0x00,1,{0xF0}},
{0xCB,12,{0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x80}},
{0xCC,10,{0x0f,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x10}},

{0x00,1,{0xB0}},
{0xCC,10,{0x0f,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x10}},

{0x00,1,{0x8A}},
{0xCD,1,{0x0B}},

{0x00,1,{0xA0}},
{0xCD,15,{0x06,0x07,0x08,0x09,0x14,0x2d,0x14,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d}},

{0x00,1,{0xB0}},
{0xCD,15,{0x2d,0x2d,0x2d,0x15,0x12,0x2c,0x0c,0x0b,0x13,0x0e,0x0f,0x10,0x2d,0x2d,0x2d}},

{0x00,1,{0xC0}},
{0xCD,10,{0x2d,0x2d,0x2d,0x27,0x28,0x29,0x2a,0x2b,0x1d,0x2d}},

{0x00,1,{0xD0}},
{0xCD,15,{0x02,0x03,0x04,0x05,0x14,0x2d,0x14,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d}},

{0x00,1,{0xE0}},
{0xCD,15,{0x2d,0x2d,0x2d,0x15,0x12,0x2c,0x0c,0x0b,0x13,0x0e,0x0f,0x10,0x2d,0x2d,0x2d}},

{0x00,1,{0xF0}},
{0xCD,10,{0x2d,0x2d,0x2d,0x27,0x28,0x29,0x2a,0x2b,0x1d,0x2d}},

{0x00,1,{0xD0}},
{0xC0,12,{0x00,0x00,0x00,0x03,0x00,0x1e,0x06,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x80}},
{0xC3,8,{0x82,0x03,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x90}},
{0xC3,15,{0x87,0x0c,0x01,0x07,0x06,0x86,0x0c,0x01,0x07,0x06,0x85,0x0d,0x01,0x07,0x06}},

{0x00,1,{0xA0}},
{0xC3,15,{0x84,0x0d,0x01,0x07,0x06,0x87,0x0b,0x01,0x07,0x06,0x86,0x0b,0x01,0x07,0x06}},

{0x00,1,{0xB0}},
{0xC3,10,{0x85,0x0d,0x01,0x07,0x06,0x84,0x0d,0x01,0x07,0x06}},

{0x00,1,{0xEA}},
{0xC3,3,{0x22,0x02,0x11}},

{0x00,1,{0x90}},
{0xC5,10,{0x92,0xD6,0xB5,0xAF,0xB2,0xAA,0x44,0x44,0x40,0x88}},

{0x00,1,{0xA0}},
{0xC5,10,{0x92,0xD6,0xB5,0xAF,0xB2,0xAA,0x44,0x44,0x40,0x88}},
{0x00,1,{0x00}},
{0xD8,2,{0x2C,0x2C}},/*improve white color*/
//{0x00,1,{0x00}}, //vcom setting
//{0xD9,1,{0x78}},

/*optimize gamma 2.5*/
//gamma setting
{0x00,1,{0x00}},
{0xE1,24,{0x59,0x5B,0x5C,0x60,0x64,0x66,0x6D,0x77,0x7D,0x88,0x90,0x96,0x64,0x5F,0x59,0x4D,0x3E,0x33,0x2C,0x28,0x27,0x24,0x23,0x20}},

{0x00,1,{0x00}},
{0xE2,24,{0x59,0x5B,0x5D,0x60,0x64,0x66,0x6D,0x77,0x7D,0x88,0x90,0x96,0x64,0x5F,0x59,0x4D,0x3E,0x33,0x2C,0x28,0x27,0x24,0x23,0x20}},

{0x00,1,{0x00}},
{0xE3,24,{0x5F,0x60,0x62,0x65,0x67,0x6A,0x70,0x79,0x7E,0x89,0x90,0x96,0x64,0x5F,0x59,0x4D,0x3E,0x33,0x2C,0x28,0x27,0x24,0x23,0x20}},

{0x00,1,{0x00}},
{0xE4,24,{0x5F,0x61,0x62,0x65,0x68,0x6B,0x70,0x79,0x7E,0x89,0x90,0x96,0x64,0x5F,0x59,0x4D,0x3E,0x33,0x2C,0x28,0x27,0x24,0x23,0x20}},

{0x00,1,{0x00}},
{0xE5,24,{0x04,0x1C,0x24,0x34,0x3F,0x47,0x54,0x68,0x76,0x85,0x90,0x96,0x64,0x5F,0x59,0x4D,0x3E,0x33,0x2C,0x28,0x27,0x24,0x23,0x20}},

{0x00,1,{0x00}},
{0xE6,24,{0x04,0x1C,0x24,0x34,0x3F,0x47,0x54,0x68,0x76,0x85,0x90,0x96,0x64,0x5F,0x59,0x4D,0x3E,0x33,0x2C,0x28,0x27,0x24,0x23,0x20}},
{0x00,1,{0x80}},
{0xFF,2,{0x00,0x00}},

{0x00,1,{0x00}},
{0xFF,3,{0x00,0x00,0x00}},

{0x35, 1, {0x00}},

{0x11, 1, {0x00}},
{REGFLAG_DELAY, 120, {}},

{0x29, 1, {0x00}},
{REGFLAG_DELAY, 20, {}},
{REGFLAG_END_OF_TABLE, 0x00, {}},

};
static struct LCM_setting_table lcm_initialization_setting_auo[] = {
{0xff,3,{0x12,0x82,0x01}},        //EXTC=1
{0x00,1,{0x80}},                    //Orise mode enable
{0xff,2,{0x12,0x82}},

/*-------Power Setting ------------------------------------------------------------------------------*/
/*TE CLK change from 65Hz to 60 Hz*/
{0x00,1,{0x80}},
{0xC0,14,{0x00,0x85,0x00,0x2C,0x2C,0x00,0x7B,0x2C,0x2C,0x00,0x7B,0x00,0x2C,0x2C}}, //TCON Setting 1

{0x00,1,{0xA0}},
{0xC0,7,{0x00,0x00,0x00,0x07,0x00,0x19,0x09}},

{0x00,1,{0xD0}},
{0xC0,7,{0x00,0x00,0x00,0x07,0x00,0x19,0x09}},

{0x00,1,{0x80}},
{0xC1,2,{0x55,0x55}},

{0x00,1,{0x90}},
{0xC1,3,{0x66,0x00,0x00}},

{0x00,1,{0x80}},
{0xC2,4,{0x83,0x01,0x45,0x45}},

{0x00,1,{0x90}},
{0xC2,15,{0xA9,0x2C,0x01,0x00,0x00,0xAB,0x2C,0x01,0x00,0x00,0xAA,0x2C,0x01,0x00,0x00}},

{0x00,1,{0xA0}},
{0xC2,5,{0xA8,0x2C,0x01,0x00,0x00}},

{0x00,1,{0xEC}},
{0xC2,1,{0x00}},

{0x00,1,{0xFA}},
{0xC2,3,{0x00,0x80,0x01}},

{0x00,1,{0x80}},
{0xC3,4,{0x83,0x01,0x45,0x45}},

{0x00,1,{0x90}},
{0xC3,15,{0xA9,0x2C,0x01,0x00,0x00,0xAB,0x2C,0x01,0x00,0x00,0xAA,0x2C,0x01,0x00,0x00}},

{0x00,1,{0xA0}},
{0xC3,5,{0xA8,0x2C,0x01,0x00,0x00}},

{0x00,1,{0xEC}},
{0xC3,1,{0x00}},

{0x00,1,{0x90}},
{0xCB,15,{0xC0,0x00,0xC0,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0x00}},

{0x00,1,{0xA0}},
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xB0}},
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x50,0xC0,0x00,0x00,0x00,0x00}},
/* AUO advise use default value
{0x00,1,{0xB3}},
{0xC0,1,{0x33}},
*/
{0x00,1,{0xB4}},
{0xC0,1,{0x40}},

{0x00,1,{0xC0}},
{0xCB,15,{0xFF,0x28,0xEB,0x28,0xEB,0x14,0x14,0x14,0x14,0x00,0x14,0x14,0xD7,0x28,0x28}},

{0x00,1,{0xD0}},
{0xCB,15,{0x28,0x28,0x14,0x14,0x14,0xD7,0x04,0xF7,0x04,0x14,0x14,0x14,0x14,0x04,0x04}},

{0x00,1,{0xE0}},
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x15,0x15,0x07,0x14,0x14,0x00,0x00}},

{0x00,1,{0xF0}},
{0xCB,12,{0x30,0x03,0xFC,0x03,0xF0,0x30,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x80}},
{0xCC,10,{0x0F,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x10}},

{0x00,1,{0xB0}},
{0xCC,10,{0x0F,0x06,0x05,0x04,0x03,0x07,0x08,0x09,0x0A,0x10}},

{0x00,1,{0x8A}},
{0xCD,1,{0x0B}},

{0x00,1,{0xA0}},
{0xCD,15,{0x12,0x14,0x15,0x04,0x05,0x01,0x0A,0x2D,0x2D,0x2D,0x2D,0x2D,0x26,0x25,0x24}},

{0x00,1,{0xB0}},
{0xCD,15,{0x23,0x22,0x21,0x20,0x1F,0x2D,0x2D,0x2D,0x2D,0x13,0x0B,0x0C,0x2D,0x2D,0x2D}},

{0x00,1,{0xC0}},
{0xCD,10,{0x2D,0x2D,0x2D,0x27,0x28,0x29,0x2A,0x2B,0x1D,0x2D}},

{0x00,1,{0xD0}},
{0xCD,15,{0x12,0x14,0x15,0x02,0x03,0x01,0x0A,0x2D,0x2D,0x2D,0x2D,0x2D,0x26,0x25,0x24}},

{0x00,1,{0xE0}},
{0xCD,15,{0x23,0x22,0x21,0x20,0x1F,0x2D,0x2D,0x2D,0x2D,0x13,0x0B,0x0C,0x2D,0x2D,0x2D}},

{0x00,1,{0xF0}},
{0xCD,10,{0x2D,0x2D,0x2D,0x27,0x28,0x29,0x2A,0x2B,0x1D,0x2D}},

{0x00,1,{0x93}},
{0xF5,1,{0x10}},

{0x00,1,{0x90}},
{0xC5,4,{0x92,0xD6,0xAD,0xB0}},

{0x00,1,{0xA0}},
{0xC5,4,{0x92,0xD6,0xAD,0xB0}},

{0x00,1,{0x00}},
{0xD8,2,{0x34,0x34}},
/*gamma 2.5 setting (delete gamma 2.2)*/
{0x00,1,{0x00}},
/*Gamma Red+ setting*/
{0xE1,24,{0x2A,0x35,0x3D,0x4A,0x53,0x58,0x65,0x78,0x7F,0x93,0x9B,0xA1,0x57,0x56,0x50,0x45,0x37,0x26,0x1E,0x19,0x16,0x0E,0x13,0x09}},
{0x00,1,{0x00}},
/*Gamma Red- setting*/
{0xE2,24,{0x2A,0x35,0x3E,0x4A,0x52,0x5B,0x66,0x76,0x84,0x91,0x9D,0xA2,0x5B,0x56,0x54,0x47,0x37,0x2A,0x20,0x1C,0x15,0x10,0x10,0x09}},
{0x00,1,{0x00}},
/*Gamma Green+ setting*/
{0xE3,24,{0x2A,0x35,0x3D,0x4A,0x53,0x58,0x65,0x78,0x7F,0x93,0x9B,0xA1,0x57,0x56,0x50,0x45,0x37,0x26,0x1E,0x19,0x16,0x0E,0x13,0x09}},
{0x00,1,{0x00}},
/*Gamma Green- setting*/
{0xE4,24,{0x2A,0x35,0x3E,0x4A,0x52,0x5B,0x66,0x76,0x84,0x91,0x9D,0xA2,0x5B,0x56,0x54,0x47,0x37,0x2A,0x20,0x1C,0x15,0x10,0x10,0x09}},
{0x00,1,{0x00}},
/*Gamma Blue+ setting*/
{0xE5,24,{0x2A,0x35,0x3D,0x4A,0x53,0x58,0x65,0x78,0x7F,0x93,0x9B,0xA1,0x57,0x56,0x50,0x45,0x37,0x26,0x1E,0x19,0x16,0x0E,0x13,0x09}},
{0x00,1,{0x00}},
/*Gamma Blue- setting*/
{0xE6,24,{0x2A,0x35,0x3E,0x4A,0x52,0x5B,0x66,0x76,0x84,0x91,0x9D,0xA2,0x5B,0x56,0x54,0x47,0x37,0x2A,0x20,0x1C,0x15,0x10,0x10,0x09}},
/*Exit Gamma setting*/
/*Disable level 2 command*/
{0x00,1,{0x80}},
{0xFF,2,{0x00,0x00}},

{0x00,1,{0x00}},
{0xFF,3,{0x00,0x00,0x00}},

{0x11, 1, {0x00}},
{REGFLAG_DELAY, 120, {}},

{0x35,1,{0x00}},

{0x29, 1, {0x00}},
{REGFLAG_DELAY, 20, {}},
{REGFLAG_END_OF_TABLE, 0x00, {}},
};
#if 0
static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    //Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},

    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_compare_id_setting[] = {
    //Display off sequence
    {0xB9,	3,	{0xFF, 0x83, 0x69}},
    {REGFLAG_DELAY, 10, {}},

    // Sleep Mode On
    //{0xC3, 1, {0xFF}},

    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_backlight_level_setting[] = {
    {0x51, 1, {0xFF}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++)
    {
        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {

            case REGFLAG_DELAY :
                if(table[i].count <= 20)
                    mdelay(table[i].count);
                else
                    msleep(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
    unsigned int div2_real=0;
    unsigned int cycle_time = 0;
    unsigned int ui = 0;
    unsigned int hs_trail_m, hs_trail_n;

    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DSI;

    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

#if (LCM_DSI_CMD_MODE)
    params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity		= LCM_POLARITY_FALLING;
#endif

#if (LCM_DSI_CMD_MODE)
    params->dsi.mode   = CMD_MODE;
#else
    params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_FOUR_LANE;
    //The following defined the fomat for data coming from LCD engine
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

    // Highly depends on LCD driver capability.
    // Not support in MT6573
    params->dsi.packet_size=256;
    params->dsi.vertical_active_line				= FRAME_HEIGHT;
    // Video mode setting
    params->dsi.intermediat_buffer_num = 0;

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active				= 2;
    params->dsi.vertical_backporch					= 8;
    params->dsi.vertical_frontporch					= 10;
    params->dsi.vertical_active_line				= FRAME_HEIGHT;

    params->dsi.horizontal_sync_active				= 10;
    params->dsi.horizontal_backporch				= 20;
    params->dsi.horizontal_frontporch				= 40;
    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
    params->dsi.CLK_HS_POST = 14;/*improve the clk_hs_post time*/
    //improve clk quality
    params->dsi.PLL_CLOCK = LCM_DSI_6589_PLL_CLOCK_240_5; //this value must be in MTK suggested table

}

/******************************************************************************
Function:       lcm_reset
Description:    LCD reset funcation
Input:          none
Output:         none
Return:         none
Others:         none
******************************************************************************/
static void lcm_reset(void)
{
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
    mdelay(5);
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
    mdelay(15);
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
    mdelay(15);
}
/******************************************************************************
Function:       lcm_power_on
Description:    LCD power on seq.
Input:          none
Output:         none
Return:         none
Others:         none
******************************************************************************/
//after VSP&VSN power on, reset twice (low to high,then low to high)
static void lcm_power_on(void)
{

    //reset low to high
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
    mdelay(5);
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
    mdelay(5);

    //reset low to high
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
    mdelay(5);
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
    mdelay(5);

}

/******************************************************************************
Function:       lcm_resume_on
Description:    LCD resume on seq.
Input:          none
Output:         none
Return:         none
Others:         none
******************************************************************************/
//after VSP&VSN power on, reset twice (low to high,then low to high)
static void lcm_resume_on(void)
{
    //enable VSP & VSN
    lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ONE);
    mdelay(5);

    //reset high
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
    mdelay(5);

    //reset low to high
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
    mdelay(5);
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
    mdelay(5);
}
/******************************************************************************
Function:       lcm_id_pin_cmi
Description:    operate GPIO to prevent electric leakage
Input:          none
Output:         none
Return:         none
Others:         cmi id0:0;id1:0,so pull down GPIO_DISP_ID0_PIN and GPIO_DISP_ID1_PIN
******************************************************************************/
static void lcm_id_pin_cmi(void)
{
    mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);
    mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,GPIO_PULL_DOWN);
}
/*divide lcm_init into lcm_init_cmi and lcm_init_auo,for cmi and auo*/
static void lcm_init_cmi(void)
{
    lcm_power_on();
    lcm_id_pin_cmi();
    push_table(lcm_initialization_setting_cmi, sizeof(lcm_initialization_setting_cmi) / sizeof(struct LCM_setting_table), 1);  

    #ifdef BUILD_LK
        printf("uboot:cmi_otm1282a_lcm_init\n");
    #else
        printk("kernel:cmi_otm1282a_lcm_init\n");
    #endif
}
/******************************************************************************
Function:       lcm_id_pin_auo
Description:    operate GPIO to prevent electric leakage
Input:          none
Output:         none
Return:         none
Others:         cmi id0:1;id1:1,so pull up GPIO_DISP_ID0_PIN and GPIO_DISP_ID1_PIN
******************************************************************************/
static void lcm_id_pin_auo(void)
{
    mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_UP);
    mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,GPIO_PULL_UP);
}
static void lcm_init_auo(void)
{
    lcm_power_on();
    lcm_id_pin_auo();
    push_table(lcm_initialization_setting_auo, sizeof(lcm_initialization_setting_auo) / sizeof(struct LCM_setting_table), 1);

    #ifdef BUILD_LK
        printf("uboot:auo_otm1282a_lcm_init\n");
    #else
        printk("kernel:auo_otm1282a_lcm_init\n");
    #endif
}
static void lcm_suspend(void)
{
    push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
    //reset low
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
    mdelay(5);
    //disable VSP & VSN
    lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ZERO);
    mdelay(5);
    #ifdef BUILD_LK
        printf("uboot:otm1282a_lcm_suspend\n");
    #else
        printk("kernel:otm1282a_lcm_suspend\n");
    #endif
}

static void lcm_resume_cmi(void)
{

    lcm_resume_on();
    push_table(lcm_initialization_setting_cmi, sizeof(lcm_initialization_setting_cmi) / sizeof(struct LCM_setting_table), 1);
    #ifdef BUILD_LK
        printf("uboot:cmi_otm1282a_lcm_resume\n");
    #else
        printk("kernel:cmi_otm1282a_lcm_resume\n");
    #endif
}
static void lcm_resume_auo(void)
{
    lcm_resume_on();
    push_table(lcm_initialization_setting_auo, sizeof(lcm_initialization_setting_auo) / sizeof(struct LCM_setting_table), 1);
    #ifdef BUILD_LK
        printf("uboot:auo_otm1282a_lcm_resume\n");
    #else
        printk("kernel:auo_otm1282a_lcm_resume\n");
    #endif
}

static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
         //delete high speed packet
	//data_array[0]=0x00290508;
	//dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}


static void lcm_setbacklight(unsigned int level)
{
	unsigned int default_level = 145;
	unsigned int mapped_level = 0;

	//for LGE backlight IC mapping table
	if(level > 255) 
			level = 255;

	if(level >0) 
			mapped_level = default_level+(level)*(255-default_level)/(255);
	else
			mapped_level=0;

	// Refresh value of backlight level.
	lcm_backlight_level_setting[0].para_list[0] = mapped_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}
/******************************************************************************
  Function:       lcm_set_pwm_level_XXX
  Description:    set different values for each LCD
  Input:          level
  Output:         NONE
  Return:         mapped_level
  Others:         none
******************************************************************************/
static unsigned int lcm_set_pwm_level_cmi(unsigned int level )
{
    unsigned int mapped_level = 0;
    if( 0 == level)
    {
        mapped_level = level;
    }
    else if(( 0 < level ) && (  BL_MIN_LEVEL > level ))
    {
        //Some 3rd APK will set values < 20 , set value(1-19) is 9
        mapped_level = 9;
    }
    else
    {
        //Reduce min brightness value
        mapped_level = (unsigned int)((level-8) * 8 /10);
    }
    #ifdef BUILD_LK
        printf("uboot:otm1282a_lcm_set_pwm mapped_level = %d,level=%d\n",mapped_level,level);
    #else
        printk("kernel:otm1282a_lcm_set_pwm mapped_level = %d,level=%d\n",mapped_level,level);
    #endif
    return mapped_level;
}
static unsigned int lcm_set_pwm_level_auo(unsigned int level )
{
    unsigned int mapped_level = 0;
    if( 0 == level)
    {
        mapped_level = level;
    }
    else if(( 0 < level ) && (  BL_MIN_LEVEL > level ))
    {
        //Some 3rd APK will set values <= 20 , set value(1-20) is 9
        mapped_level = 8;
    }
    else
    {
        //Reduce min brightness value
        mapped_level = (unsigned int)((level-6) * 7 /10);
    }
    #ifdef BUILD_LK
        printf("uboot:auo_otm1282a_lcm_set_pwm mapped_level = %d,level=%d\n",mapped_level,level);
    #else
        printk("kernel:auo_otm1282a_lcm_set_pwm mapped_level = %d,level=%d\n",mapped_level,level);
    #endif
    return mapped_level;
}
static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_UBOOT
        if(lcm_esd_test)
        {
            lcm_esd_test = FALSE;
            return TRUE;
        }

        /// please notice: the max return packet size is 1
        /// if you want to change it, you can refer to the following marked code
        /// but read_reg currently only support read no more than 4 bytes....
        /// if you need to read more, please let BinHan knows.
        /*
                unsigned int data_array[16];
                unsigned int max_return_size = 1;
                
                data_array[0]= 0x00003700 | (max_return_size << 16);    
                
                dsi_set_cmdq(&data_array, 1, 1);
        */

        if(read_reg(0xB6) == 0x42)
        {
            return FALSE;
        }
        else
        {            
            return TRUE;
        }
#endif
}
static unsigned int lcm_esd_recover(void)
{
    unsigned char para = 0;

    lcm_reset();
    push_table(lcm_initialization_setting_cmi, sizeof(lcm_initialization_setting_cmi) / sizeof(struct LCM_setting_table), 1);
    mdelay(10);
    push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
    mdelay(10);
    dsi_set_cmdq_V2(0x35, 1, &para, 1);//enable TE
    mdelay(10);

    return TRUE;
}
static unsigned int lcm_compare_id_cmi(void)
{
    unsigned char module_id = which_lcd_module();
    return ((lcd_ID_cmi == module_id )? 1 : 0);
}
static unsigned int lcm_compare_id_auo(void)
{
    unsigned char module_id = which_lcd_module();
    return ((lcd_ID_auo == module_id )? 1 : 0);
}
// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
/*add module vendor into LCM driver name*/
LCM_DRIVER cmi_otm1282a_hd720_dsi_cmd_lcm_drv =
{
    .name           = "cmi_otm1282a_hd720_dsi_cmd",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init_cmi,/*cmi init fun.*/
    .suspend        = lcm_suspend,
    .resume         = lcm_resume_cmi,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
    .set_backlight  = lcm_setbacklight,
    .set_pwm_level = lcm_set_pwm_level_cmi,
//  .set_pwm        = lcm_setpwm,
//  .get_pwm        = lcm_getpwm,
//  .esd_check      = lcm_esd_check,
//  .esd_recover    = lcm_esd_recover,
    .compare_id     = lcm_compare_id_cmi,
#endif
};

LCM_DRIVER auo_otm1282a_hd720_dsi_cmd_lcm_drv =
{
    .name           = "auo_otm1282a_hd720_dsi_cmd",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init_auo,/*auo init fun.*/
    .suspend        = lcm_suspend,
    .resume         = lcm_resume_auo,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
    .set_backlight  = lcm_setbacklight,
    .set_pwm_level = lcm_set_pwm_level_auo,
//  .set_pwm        = lcm_setpwm,
//  .get_pwm        = lcm_getpwm,
//  .esd_check      = lcm_esd_check,
//  .esd_recover    = lcm_esd_recover,
    .compare_id     = lcm_compare_id_auo,
#endif
};
