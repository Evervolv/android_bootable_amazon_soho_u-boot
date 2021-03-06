/*
 * (C) Copyright 2011
 * Quanta Computer Inc.
 *
 */
//**************************************************************************************
// header include
//**************************************************************************************

#include <common.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <command.h>
#include <asm/arch/mux.h>
#include <display_lcd.h>
#include <idme.h>
#include "soho_lcd.h"

#define mdelay(n) ({ unsigned long msec = (n); while (msec--) udelay(1000); })

extern char is_recovery_mode;
static int is_lp8557 = 0;

static void dsi_write_reg(unsigned char reg, unsigned char data)
{
	uint t;
	int timeout = MAX_DELAY_COUNTER;

	t  = 0x15;		/* Data type: DCS Short WRITE, 1 parameter */
	t |= (reg) << 8;	/* Data byte 1 */
	t |= (data) << 16;	/* Data byte 2 */

	RegWrite32(DSI_BASE, DSI_VC0_SHORT_PACKET_HEADER, t);

	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC0_CTRL) & (1 << 16)) == 1)  && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! send DSI VC0 command time out\n");
}

//**************************************************************************************
// Function: LCD_Normal
// Description: Normal display
//**************************************************************************************
static void LCD_Normal(void)
{
	uint t;
	int timeout = MAX_DELAY_COUNTER;

	//printf("[!!] mipi software reset..............\n");
	dsi_write_reg(0x01, 0x00);

	mdelay(20);  //need more than 20ms

	//printf("[!!] swing double mode..............\n");
	// setting to open_100ohms termination resistance @ TCON side
	dsi_write_reg(0xAE, 0x0B);

	//printf("[!!] test mode1..............\n");
	dsi_write_reg(0xEE, 0xEA);

	//printf("[!!] test mode2..............\n");
	dsi_write_reg(0xEF, 0x5F);

	//printf("[!!] bias..............\n");
	dsi_write_reg(0xF2, 0x68);

	// Color enhancement
	dsi_write_reg(0xB3, 0x03);

	dsi_write_reg(0xC8, 0x04);

	dsi_write_reg(0xEE, 0x00);

	dsi_write_reg(0xEF, 0x00);

	//After EVT1.0a, Brightness is controled by CABC
	//printf("[!!] enable CABC..............\n");
	dsi_write_reg(0xB0, 0x7E);
}


//**************************************************************************************
// Function: LCD_Bist
// Description: enter bist mode
//**************************************************************************************
static void LCD_Bist(void)
{
	uint t;

	int timeout = MAX_DELAY_COUNTER;

	printf("[!!] mipi software reset..............\n");
	t  = 0x15;		/* Data type: DCS Short WRITE, 1 parameter */
	t |= (0x01) << 8;	/* Data byte 1 */
	t |= (0x00) << 16;	/* Data byte 2 */

	RegWrite32(DSI_BASE, DSI_VC0_SHORT_PACKET_HEADER, t);

	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC0_CTRL) & (1 << 16)) == 1)  && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! send DSI VC0 command time out\n");

	udelay(20000);  //need more than 20ms

	printf("[!!] enter bist mode..............\n");
	t  = 0x15;		/* Data type: DCS Short WRITE, 1 parameter */
	t |= (0xB1) << 8;	/* Data byte 1 */
	t |= (0xEF) << 16;	/* Data byte 2 */

	RegWrite32(DSI_BASE, DSI_VC0_SHORT_PACKET_HEADER, t);

	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC0_CTRL) & (1 << 16)) == 1)  && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! send DSI VC0 command time out\n");
}

//**************************************************************************************
// Function: dsi_videomode_PostInit
// Description:
//**************************************************************************************
static void dsi_videomode_PostInit(void)
{
	ulong dwVal = 0;
	int timeout = MAX_DELAY_COUNTER;

	//disbale DSI Interface
	dwVal = RegRead32(DSI_BASE, DSI_CTRL);
	dwVal &= (~(1<<0));
	RegWrite32(DSI_BASE, DSI_CTRL, dwVal);

	//wait for interface enable
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_CTRL) & (1<<0)) != 0 ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! DSI Interface disable timeout!!!\n");

#if 0
	//disable virtual channel 1
	dwVal = RegRead32(DSI_BASE, DSI_VC1_CTRL);
	dwVal &= (~(1<<0));
	RegWrite32(DSI_BASE, DSI_VC1_CTRL, dwVal);

	//wait for virtual channel 1 disabled
	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC1_CTRL) & (1<<0)) != 0) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! virtual channel 1 disabled timeout!!!\n");
#endif

	//disable virtual channel 0
	dwVal = RegRead32(DSI_BASE, DSI_VC0_CTRL);
	dwVal &= (~(1<<0));
	RegWrite32(DSI_BASE, DSI_VC0_CTRL, dwVal);

	//wait for virtual channel 1 disabled
	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC0_CTRL) & (1<<0)) != 0) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! virtual channel 0 disabled timeout!!!\n");

	//set mode to video mode
	dwVal = RegRead32(DSI_BASE, DSI_VC0_CTRL);
	dwVal |= (1<<4);
	dwVal |= (1<<9);
	RegWrite32(DSI_BASE, DSI_VC0_CTRL, dwVal);


	/* Form the DSI video mode packet header */
	//Henry: Fix head code
	dwVal = 0x0009603e;
	RegWrite32(DSI_BASE, DSI_VC_LONG_PACKET_HEADER(0), dwVal);

	//Enable virtual channel 0
	dwVal = RegRead32(DSI_BASE, DSI_VC0_CTRL);
	dwVal |= (1<<0);
	RegWrite32(DSI_BASE, DSI_VC0_CTRL, dwVal);

	//wait for virtual channel 0 enabled
	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC0_CTRL) & (1<<0)) == 0) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! wait for virtual channel 0 enabled timeout!!!\n");

	//Enable DSI Interface
	//the data acquisition on the video port starts on the next VSYNC
	dwVal = RegRead32(DSI_BASE, DSI_CTRL);
	dwVal |= (1<<0);
	RegWrite32(DSI_BASE, DSI_CTRL, dwVal);

	//wait for interface enable
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_CTRL) & (1<<0)) == 0 ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! DSI Interface enable timeout!!!\n");

	//unset Stop state (LP-11)
	dwVal = RegRead32(DSI_BASE, DSI_TIMING1);
	dwVal &= (~(1<<15));
	RegWrite32(DSI_BASE, DSI_TIMING1, dwVal);
}

//**************************************************************************************
// Function: dsi_videomode_enable
// Description: Enable DSI video mode
//**************************************************************************************
static void dsi_video_preinit(void)
{
	ulong dwVal = 0;
	int timeout = MAX_DELAY_COUNTER;

	//set timming
	dwVal = RegRead32(DSI_BASE, DSI_CTRL);
	dwVal |= (1<<15);	/* DSI_CTRL_VP_VSYNC_START */
	dwVal |= (1<<17);	/* DSI_CTRL_VP_HSYNC_START */
	dwVal |= (1<<20);	/* DSI_CTRL_BLANKING_MODE */
	dwVal |= (1<<21);	/* DSI_CTRL_HFP_BLANKING_MODE */
	dwVal |= (1<<22);	/* DSI_CTRL_HBP_BLANKING_MODE */
	dwVal |= (1<<23);	/* DSI_CTRL_HSA_BLANKING_MODE */
	RegWrite32(DSI_BASE, DSI_CTRL, dwVal);

	//Set DSI_VM_TIMING1
	dwVal = (DSI_HSA << 24) | (DSI_HFP << 12) | DSI_HBP;
	RegWrite32(DSI_BASE, DSI_VM_TIMING1, dwVal);

	//Set DSI_VM_TIMING2
	//window_sync  = 4, 0x04000000
	dwVal = 0x04000000 | (DSI_VSA << 16) | (DSI_VFP << 8) |	DSI_VBP;
	RegWrite32(DSI_BASE, DSI_VM_TIMING2, dwVal);

	//Set DSI_VM_TIMING3 [31:16]=TL [15:0]=VACT
	dwVal = (DSI_TL << 16) | DSI_VACT;
	RegWrite32(DSI_BASE, DSI_VM_TIMING3, dwVal);

	//set DSI_VM_TIMING4
	dwVal = 0; /*0x00487296;*/
	RegWrite32(DSI_BASE, DSI_VM_TIMING4, dwVal);

	//set DSI_VM_TIMING5
	dwVal = 0x0082df3b;
	RegWrite32(DSI_BASE, DSI_VM_TIMING5, dwVal);

	//set DSI_VM_TIMING6
	dwVal = 0x00000005;
	RegWrite32(DSI_BASE, DSI_VM_TIMING6, dwVal);

	//set DSI_VM_TIMING7
	dwVal = 0x0017000E;
	//dwVal = DSI_VM_TIMING7_setting;

	RegWrite32(DSI_BASE, DSI_VM_TIMING7, dwVal);

	//set DSI_STOPCLK_TIMING
	dwVal = 0x00000007;
	RegWrite32(DSI_BASE, DSI_STOPCLK_TIMING, dwVal);

	//set TA_TO_COUNTER accordignly to kozio value
	dwVal = 0;
	dwVal |= 8191;		/* STOP_STATE_COUNTER_IO */
	dwVal |= (1<<13);	/* STOP_STATE_X4_IO */
	dwVal |= (1<<14);	/* STOP_STATE_X16_IO */
	dwVal &= ~(1<<15);	/* FORCE_TX_STOP_MODE_IO */
	dwVal |= (8191<<16);	/* TA_TO_COUNTER */
	dwVal |= (1<<29);	/* TA_TO_X8 */
	dwVal |= (1<<30);	/* TA_TO_X16 */
	/* dwVal |= (1<<31); */	/* TA_TO */

	RegWrite32(DSI_BASE, DSI_TIMING1, dwVal);

	//set TA_TO_COUNTER accordignly to kozio value
	dwVal = 0;
	dwVal |= 8191;		/* LP_RX_TO_COUNTER */
	dwVal |= (1<<13);	/* LP_RX_TO_X4 */
	dwVal |= (1<<14);	/* LP_RX_TO_X16 */
	dwVal &= ~(1<<15);	/* LP_RX_TO */
	dwVal |= (8191<<16);	/* HS_TX_TO_COUNTER */
	dwVal |= (1<<29);	/* HS_TX_TO_X16 */
	dwVal |= (1<<30);	/* HS_TX_TO_X64 */
	dwVal |= (1<<31);	/* HS_TX_TO */

	RegWrite32(DSI_BASE, DSI_TIMING2, dwVal);

	//Configure virtual channel 0
	dwVal = RegRead32(DSI_BASE, DSI_VC0_CTRL);
	dwVal &= (~0xFFEE3FDF);
	dwVal |= 0x20800580; // set mode speed to LP
	RegWrite32(DSI_BASE, DSI_VC0_CTRL, dwVal);

	//Configure virtual channel 1
#if 0
	dwVal = RegRead32(DSI_BASE, DSI_VC1_CTRL);
	dwVal &= (~0xFFEE3F4F);
	dwVal |= 0x20800D80;
	RegWrite32(DSI_BASE, DSI_VC1_CTRL, dwVal);
#endif

	//disable DSI Interface
	dwVal = RegRead32(DSI_BASE, DSI_CTRL);
	dwVal &= (~(1<<0));
	RegWrite32(DSI_BASE, DSI_CTRL, dwVal);

	//wait for interface disable
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_CTRL) & (1<<0)) != 0 ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! DSI Interface disable timeout!!!\n");

	//Enable virtual channel 0
	dwVal = RegRead32(DSI_BASE, DSI_VC0_CTRL);
	dwVal |= (1<<0);
	RegWrite32(DSI_BASE, DSI_VC0_CTRL, dwVal);

	//wait for virtual channel 0 enabled
	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC0_CTRL) & (1<<0)) == 0) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! wait for virtual channel 0 enabled timeout!!!\n");

#if 0
	//Enable virtual channel 1
	dwVal = RegRead32(DSI_BASE, DSI_VC1_CTRL);
	dwVal |= (1<<0);
	RegWrite32(DSI_BASE, DSI_VC1_CTRL, dwVal);

	//wait for virtual channel 1 enabled
	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC1_CTRL) & (1<<0)) == 0) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! wait for virtual channel 1 enabled timeout!!!\n");
#endif

	//Enable DSI Interface
	//the data acquisition on the video port starts on the next VSYNC
	dwVal = RegRead32(DSI_BASE, DSI_CTRL);
	dwVal |= (1<<0);
	RegWrite32(DSI_BASE, DSI_CTRL, dwVal);

	//wait for interface enable
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_CTRL) & (1<<0)) == 0 ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! DSI Interface enable timeout!!!\n");

	//Send null packet to start DDR clock
	dwVal = 0;
	RegWrite32(DSI_BASE, DSI_VC0_SHORT_PACKET_HEADER, dwVal);
}

//**************************************************************************************
// Function: dsi_reset
// Description: reset DSI
//**************************************************************************************
static void dsi_reset(void)
{
	ulong dwVal = 0;
	int timeout = MAX_DELAY_COUNTER;

	//Software reset
	dwVal = RegRead32(DSI_BASE, DSI_SYSCONFIG);
	dwVal |= 0x1;
	RegWrite32(DSI_BASE, DSI_SYSCONFIG, dwVal);

	//Wait until RESET_DONE
	printf("DSI module reset in progress... ");
	do{
		dwVal = RegRead32(DSI_BASE, DSI_SYSSTATUS);
		timeout--;
	}while( ((dwVal & 0x1) == 0) && (timeout > 0));

	if(timeout <=0)
		printf("Error!!! time out\n");
	else
		printf("Done\n");

	//disable IRQ
	dwVal = 0;
	RegWrite32(DSI_BASE, DSI_IRQENABLE, dwVal);
	RegWrite32(DSI_BASE, DSI_VC0_IRQENABLE, dwVal);
	RegWrite32(DSI_BASE, DSI_VC1_IRQENABLE, dwVal);
	RegWrite32(DSI_BASE, DSI_VC2_IRQENABLE, dwVal);
	RegWrite32(DSI_BASE, DSI_VC3_IRQENABLE, dwVal);

	//Reset IRQ status
	dwVal = RegRead32(DSI_BASE, DSI_IRQSTATUS);
	RegWrite32(DSI_BASE, DSI_IRQSTATUS, dwVal);

	dwVal = RegRead32(DSI_BASE, DSI_VC0_IRQSTATUS);
	RegWrite32(DSI_BASE, DSI_VC0_IRQSTATUS, dwVal);

	dwVal = RegRead32(DSI_BASE, DSI_VC1_IRQSTATUS);
	RegWrite32(DSI_BASE, DSI_VC1_IRQSTATUS, dwVal);

	dwVal = RegRead32(DSI_BASE, DSI_VC2_IRQSTATUS);
	RegWrite32(DSI_BASE, DSI_VC2_IRQSTATUS, dwVal);

	dwVal = RegRead32(DSI_BASE, DSI_VC3_IRQSTATUS);
	RegWrite32(DSI_BASE, DSI_VC3_IRQSTATUS, dwVal);

	dwVal = RegRead32(DSI_BASE, DSI_COMPLEXIO_IRQ_STATUS);
	RegWrite32(DSI_BASE, DSI_COMPLEXIO_IRQ_STATUS, dwVal);

	//DSI power management: Autoidle
	dwVal = RegRead32(DSI_BASE, DSI_SYSCONFIG);
	dwVal |= (1<<0);
	RegWrite32(DSI_BASE, DSI_SYSCONFIG, dwVal);

	//DSI power management: ENWAKEUP
	dwVal = RegRead32(DSI_BASE, DSI_SYSCONFIG);
	dwVal |= (1<<2);
	RegWrite32(DSI_BASE, DSI_SYSCONFIG, dwVal);

	//DSI power management: SIDLEMODE smart-idle
	dwVal = RegRead32(DSI_BASE, DSI_SYSCONFIG);
	dwVal &= (~(3<<3));
	dwVal |= (2 << 3);
	RegWrite32(DSI_BASE, DSI_SYSCONFIG, dwVal);
}

//**************************************************************************************
// Function: dsi_set_dpll
// Description: Set DPLL for DSI
//**************************************************************************************
static void dsi_set_dpll(void)
{
	ulong dwVal = 0;
	int timeout = MAX_DELAY_COUNTER;

	//CIO_CLK_ICG, Enable SCPClk,
	//SCPClk clock provided to DSI_PHY and PLL-CTRL module
	dwVal = RegRead32(DSI_BASE, DSI_CLK_CTRL);
	dwVal |= (1<<14);
	RegWrite32(DSI_BASE, DSI_CLK_CTRL, dwVal);

	//wait for DSI1_PLLCTRL_RESET_DONE
	timeout = MAX_DELAY_COUNTER;
	while((((RegRead32(DSI_BASE, DSI_PLL_STATUS) & (1<<0)) == 0 )) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! DSI PLL reset fail!!!\n");

	//power on both PLL and HSDIVISER
	dwVal = RegRead32(DSI_BASE, DSI_CLK_CTRL);
	dwVal |= (2<<30);
	RegWrite32(DSI_BASE, DSI_CLK_CTRL, dwVal);

	//Wait until PLL_PWR_STATUS = 0x2
	timeout = MAX_DELAY_COUNTER;
	while(((((RegRead32(DSI_BASE, DSI_CLK_CTRL) & 0x30000000) >> 28) != 0x2)) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! turn on PLL power fail!!!\n");

	// disable PLL_AUTOMODE
	dwVal = RegRead32(DSI_BASE, DSI_PLL_CONTROL);
	dwVal &= ~(1<<0);
	RegWrite32(DSI_BASE, DSI_PLL_CONTROL, dwVal);

	//Set PLL Divider by DSI_PLL_CONFIGURATION1
	dwVal = ((REGM_DSI - 1) << 26) | ((REGM_DISPC - 1) << 21) |
		(REGM << 9) | ((REGN - 1) << 1) | 1;
	RegWrite32(DSI_BASE, DSI_PLL_CONFIGURATION1, dwVal);

	//Set DSI_PLL_CONFIGURATION2
	dwVal = RegRead32(DSI_BASE, DSI_PLL_CONFIGURATION2);
	dwVal &= (~(1<<11));
	dwVal &= (~(1<<12));
	dwVal |= (1<<13);
	dwVal &= (~(1<<14));
	dwVal |= (1<<20);
	dwVal |= (3<<21);
	RegWrite32(DSI_BASE, DSI_PLL_CONFIGURATION2, dwVal);

	//Request locking
	dwVal = RegRead32(DSI_BASE, DSI_PLL_GO);
	dwVal |= (1<<0);
	RegWrite32(DSI_BASE, DSI_PLL_GO, dwVal);

	//Wait for lock
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_PLL_GO) & 0x1) != 0 ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! Wait for lock time out\n");

	// Waiting for PLL lock
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_PLL_STATUS) & 0x2 ) == 0 ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! Waiting for PLL lock time out\n");

	//Locked, change the configuration
	dwVal = 0x656008;
	RegWrite32(DSI_BASE, DSI_PLL_CONFIGURATION2, dwVal);

	//wait for M4 clock active
	timeout = MAX_DELAY_COUNTER;
	while((  (RegRead32(DSI_BASE, DSI_PLL_STATUS) & (1<<7) ) == 0 ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! wait for M4 clock active time out\n");

	// wait for M5 Protocol Engine clock active
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_PLL_STATUS) & (1<<8)) == 0 ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! wait for M5 Protocol Engine clock active time out\n");

	//Select clock source
	//FCK_CLK_SWITCH = PLL1_CLK1 selected (from DSI1 PLL)
	//DSS_CTRL(9:8) = 01
	RegSet32(DSS_BASE, DSS_CTRL, 1 << 8);
	RegClear32(DSS_BASE, DSS_CTRL, 1 << 9);

	//Select clock source
	//DSI1_CLK_SWITCH = 1
	//DSS_CTRL(1) = 1
	RegSet32(DSS_BASE, DSS_CTRL, 1 << 1);

	//Select clock source
	//LCD1_CLK_SWITCH = PLL1_CLK1 selected (from DSI1 PLL)
	//DSS_CTRL(0) = 1
	RegSet32(DSS_BASE, DSS_CTRL, 1 << 0);
}


//**************************************************************************************
// Function: dsi_set_phy
// Description: init DSI phy
//**************************************************************************************
static void dsi_set_phy(void)
{
	ulong dwVal = 0;
	int timeout = MAX_DELAY_COUNTER;

	//CIO_CLK_ICG, Enable SCPClk,
	//SCPClk clock provided to DSI_PHY and PLL-CTRL module
	dwVal = RegRead32(DSI_BASE, DSI_CLK_CTRL);
	dwVal |= (1<<14);
	RegWrite32(DSI_BASE, DSI_CLK_CTRL, dwVal);

	//HS_AUTO_STOP_ENABLE
	dwVal = RegRead32(DSI_BASE, DSI_CLK_CTRL);
	dwVal |= (1<<18);
	RegWrite32(DSI_BASE, DSI_CLK_CTRL, dwVal);

	/* A dummy read using the SCP interface to any DSIPHY register is
	 * required after DSIPHY reset to complete the reset of the DSI complex
	 * I/O. */
	dwVal = RegRead32(DSI_BASE, DSI_DSIPHY_CFG5);

	//wait for SCP clock domain of DSI phy Reset done
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_DSIPHY_CFG5) & (1<<30)) == 0 ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! DSI phy Reset time out\n");

	//Set dsi complexio_config
	dwVal = 0x54321;
	RegWrite32(DSI_BASE, DSI_COMPLEXIO_CFG1, dwVal);

	//complex I/O power on
	dwVal = RegRead32(DSI_BASE, DSI_COMPLEXIO_CFG1);
	dwVal |= (1 << 27);
	RegWrite32(DSI_BASE, DSI_COMPLEXIO_CFG1, dwVal);

	//Set the GOBIT bit
	dwVal = RegRead32(DSI_BASE, DSI_COMPLEXIO_CFG1);
	dwVal |= (1<<30);
	RegWrite32(DSI_BASE, DSI_COMPLEXIO_CFG1, dwVal);

	//wait for PWR_STATUS = on
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_COMPLEXIO_CFG1) & (0x3<<25)) != (1 << 25) ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! wait for PWR_STATUS timeout when set dsi_complexio!!!\n");

	//Wait for RESET_DONE
	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_COMPLEXIO_CFG1) & (1<<29)) == 0 ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! Wait for RESET_DONE timeout!!!\n");

	//set complexio timings
	dwVal = (THS_PREPARE << 24) | (THS_PREPARE_THE_ZERO << 16) |
		(THS_TRAIL << 8) | THS_EXIT;
	RegWrite32(DSI_BASE, DSI_DSIPHY_CFG0, dwVal);

	dwVal = RegRead32(DSI_BASE, DSI_DSIPHY_CFG1);
	dwVal &= (~0xFFFFFF);

	dwVal |= (TTAGET << 24) | (TLPX_HALF << 16) | (TCLK_TRAIL << 8) | TCLK_ZERO;
	RegWrite32(DSI_BASE, DSI_DSIPHY_CFG1, dwVal);

	dwVal = RegRead32(DSI_BASE, DSI_DSIPHY_CFG2);
	dwVal &= (~0xFF);

	dwVal |= TCLK_PREPARE;

	RegWrite32(DSI_BASE, DSI_DSIPHY_CFG2, dwVal);

}

//**************************************************************************************
// Function: dsi_set_protocol_engine
// Description: init DSI protocol engine
//**************************************************************************************
static void dsi_set_protocol_engine(void)
{
	ulong dwVal = 0;
	int timeout = MAX_DELAY_COUNTER;

	//Enable LP_CLK
	dwVal = RegRead32(DSI_BASE, DSI_CLK_CTRL);
	dwVal |= (1<<20);
	RegWrite32(DSI_BASE, DSI_CLK_CTRL, dwVal);

	//set DDR clock
	dwVal = (DDR_CLK_PRE << 8) | DDR_CLK_POST;

	RegWrite32(DSI_BASE, DSI_CLK_TIMING, dwVal);

	//set the number of TXBYTECLKHS clock cycles
	dwVal = 0xe000f;
	RegWrite32(DSI_BASE, DSI_VM_TIMING7, dwVal);

	//set lp_clk_div
	dwVal = RegRead32(DSI_BASE, DSI_CLK_CTRL);
	dwVal &= (~0x1FFF);
	dwVal |= LP_CLK_DIV;
	RegWrite32(DSI_BASE, DSI_CLK_CTRL, dwVal);

	//RX SYNCHRO ENABLE
	dwVal = RegRead32(DSI_BASE, DSI_CLK_CTRL);
	dwVal |= (1<<21);
	RegWrite32(DSI_BASE, DSI_CLK_CTRL, dwVal);

	//set TX FIFO size
	dwVal = 0x13121110;
	RegWrite32(DSI_BASE, DSI_TX_FIFO_VC_SIZE, dwVal);

	//set RX FIFO size
	dwVal = 0x13121110;
	RegWrite32(DSI_BASE, DSI_RX_FIFO_VC_SIZE, dwVal);

	//set stop_state_counter
	dwVal = RegRead32(DSI_BASE, DSI_TIMING1);
	dwVal &= (~0x0000FFFF);
	dwVal |= 0x1000;
	RegWrite32(DSI_BASE, DSI_TIMING1, dwVal);

	//set_ta_timeout
	dwVal = RegRead32(DSI_BASE, DSI_TIMING1);
	dwVal &= (~0xFFFF0000);
	dwVal |= 0x7fff0000;
	RegWrite32(DSI_BASE, DSI_TIMING1, dwVal);

	//set_lp_rx_timeout
	dwVal = RegRead32(DSI_BASE, DSI_TIMING2);
	dwVal &= (~0x0000FFFF);
	dwVal |= 0x7fff;
	RegWrite32(DSI_BASE, DSI_TIMING2, dwVal);

	//set_hs_tx_timeout
	dwVal = RegRead32(DSI_BASE, DSI_TIMING2);
	dwVal &= (~0xFFFF0000);
	dwVal |= 0x7fff0000;
	RegWrite32(DSI_BASE, DSI_TIMING2, dwVal);

	//set DSI
	dwVal = RegRead32(DSI_BASE, DSI_CTRL);
	dwVal &= (~(1<<1));	/* CS_RX_EN */
	dwVal &= (~(1<<2));	/* ECC_RX_EN */
	dwVal |= (1<<3);	/* TX_FIFO_ARBITRATION */
	dwVal |= (1<<4);	/* VP_CLK_RATIO, always 1, see errata */
	dwVal &= (~(3<<6));	/* VP_DATA_BUS_WIDTH */
	dwVal |= (2<<6);	/* VP_DATA_BUS_WIDTH, WIDTH_24*/
	dwVal &= (~(1<<8));	/* VP_CLK_POL */
	dwVal |= (1<<9);	/* VP_DE_POL */
	dwVal |= (1<<11);	/* VP_VSYNC_POL */
	dwVal &= (~(3<<12));	/* LINE_BUFFER, 2 lines */
	dwVal |= (2<<12);	/* LINE_BUFFER, 2 lines */
	dwVal |= (1<<14);	/* TRIGGER_RESET_MODE */
	dwVal |= (1<<19);	/* EOT_ENABLE */
	RegWrite32(DSI_BASE, DSI_CTRL, dwVal);

	//Initial VC0
	dwVal = 0x20061d80;
	RegWrite32(DSI_BASE, DSI_VC0_CTRL, dwVal);


	//Initial VC1
	dwVal = 0x20808f80;
	RegWrite32(DSI_BASE, DSI_VC1_CTRL, dwVal);
#if 0
	//Initial VC2
	dwVal = 0x20061d80;
	RegWrite32(DSI_BASE, DSI_VC2_CTRL, dwVal);

	//Initial VC3
	dwVal = 0x20061d80;
	RegWrite32(DSI_BASE, DSI_VC3_CTRL, dwVal);
#endif

	//Disable virtual channel 0
	dwVal = RegRead32(DSI_BASE, DSI_VC0_CTRL);
	dwVal |= (1<<0);
	RegWrite32(DSI_BASE, DSI_VC0_CTRL, dwVal);

	//wait for virtual channel 0 disabled
	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC0_CTRL) & (1<<0)) == 0) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! wait for virtual channel 0 enabled timeout!!!\n");


	//Enable virtual channel 1
	dwVal = RegRead32(DSI_BASE, DSI_VC1_CTRL);
	dwVal |= (1<<0);
	RegWrite32(DSI_BASE, DSI_VC1_CTRL, dwVal);

	//wait for virtual channel 1 enabled
	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC1_CTRL) & (1<<0)) == 0) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! wait for virtual channel 1 enabled timeout!!!\n");
#if 0
	//Enable virtual channel 2
	dwVal = RegRead32(DSI_BASE, DSI_VC2_CTRL);
	dwVal |= (1<<0);
	RegWrite32(DSI_BASE, DSI_VC2_CTRL, dwVal);

	//wait for virtual channel 2 enabled
	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC2_CTRL) & (1<<0)) == 0) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! wait for virtual channel 2 enabled timeout!!!\n");

	//Enable virtual channel 3
	dwVal = RegRead32(DSI_BASE, DSI_VC3_CTRL);
	dwVal |= (1<<0);
	RegWrite32(DSI_BASE, DSI_VC3_CTRL, dwVal);

	//wait for virtual channel 3 enabled
	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC3_CTRL) & (1<<0)) == 0) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! wait for virtual channel 1 enabled timeout!!!\n");
#endif

	//Enable DSI Interface
	//the data acquisition on the video port starts on the next VSYNC
	dwVal = RegRead32(DSI_BASE, DSI_CTRL);
	dwVal |= (1<<0);
	RegWrite32(DSI_BASE, DSI_CTRL, dwVal);

	//wait for interface enable
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_CTRL) & (1<<0)) == 0 ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! DSI Interface enable timeout!!!\n");

	//Set DSI ForceTxStopMode
	dwVal = RegRead32(DSI_BASE, DSI_TIMING1);
	dwVal |= (1<<15);
	RegWrite32(DSI_BASE, DSI_TIMING1, dwVal);

	//wait for ForceTxStopMode
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_TIMING1) & (1<<15)) != 0 ) && (timeout > 0)) {
		timeout--;
	}
	if(timeout <=0) printf("Error!!! DSI ForceTxStopMode timeout!!!\n");

}

//**************************************************************************************
// Function: set_DISPC
// Description: Set DISPC registers
//**************************************************************************************
static void set_DISPC(ulong dwFramebufferAddr)
{
	ulong dwVal = 0;

	//set STNTFT, LCD type of the primary LCD is TFT
	//DISPC_CONTROL1(3) = 1
	RegSet32(DISPC_BASE, DISPC_CONTROL1, 1 << 3);

	//DISPC_CONTROL1(11) = 0  , STALL Mode for the primary LCD output
	//DISPC_CONTROL1(15) = 1  , General Purpose Output Signal set
	//DISPC_CONTROL1(16) = 1  , General Purpose Output Signal set
	RegClear32(DISPC_BASE, DISPC_CONTROL1, 1 << 11);
	RegSet32(DISPC_BASE, DISPC_CONTROL1, 1 << 15);
	RegSet32(DISPC_BASE, DISPC_CONTROL1, 1 << 16);

	//DISPC_CONFIG1(16) = 0,
	//config BUFFERHANDCHECK
	RegClear32(DISPC_BASE, DISPC_CONFIG1, 1 << 16);

	//DISPC_CONTROL1(9, 8) = 11  , 24-bit output aligned on the LSB of the pixel data interface
	RegSet32(DISPC_BASE, DISPC_CONTROL1, 1 << 8);
	RegSet32(DISPC_BASE, DISPC_CONTROL1, 1 << 9);

	//set lcd timings, channel = 0
	dwVal = ((LCD_HBP - 1) << 20) | ((LCD_HFP - 1) << 8) | (LCD_HSW - 1);
	RegWrite32(DISPC_BASE, DISPC_TIMING_H, dwVal);

	//set lcd timings
	dwVal = (LCD_VBP << 20) | (LCD_VFP << 8) | (LCD_VSW - 1);
	RegWrite32(DISPC_BASE, DISPC_TIMING_V, dwVal);

	//configures the panel size (horizontal and vertical)
	//[26:16]=hight [10:0]=width
	dwVal = ((LCD_HIGHT - 1) << 16) | (LCD_WIDTH - 1);
	RegWrite32(DISPC_BASE, DISPC_SIZE_LCD, dwVal);

	//config clock div
	dwVal = (LCK_DIV << 16) | PCK_DIV;
	RegWrite32(DISPC_BASE, DISPC_DIVISOR, dwVal);

	//Set power management: MIDLEMODE=smart standby
	dwVal = RegRead32(DISPC_BASE, DISPC_SYSCONFIG);
	dwVal &= (~(0x3<<12));
	dwVal |= (2 << 12);

	//SIDLEMODE=smart idle
	dwVal &= (~(0x3<<3));
	dwVal |= (2 << 3);

	//ENWAKEUP
	dwVal |= (1<<2);

	//AUTOIDLE */
	dwVal |= (1<<0);

	RegWrite32(DISPC_BASE, DISPC_SYSCONFIG, dwVal);

	//set Loading Mode for the Palette/Gamma Table
	dwVal = RegRead32(DISPC_BASE, DISPC_CONFIG1);
	dwVal &= (~(0x3<<1));
	dwVal |= (2 << 1);
	RegWrite32(DISPC_BASE, DISPC_CONFIG1, dwVal);

	// Enabling the DISPC_DIVISOR
	dwVal = RegRead32(DISPC_BASE, DISPC_DIVISOR);
	dwVal &= (~0xFF0000);
	dwVal |= 0x10000;
	RegWrite32(DISPC_BASE, DISPC_DIVISOR, dwVal);

	// Disable INTR
	dwVal = 0;
	RegWrite32(DISPC_BASE, DISPC_IRQENABLE, dwVal);

	// Clear INTR
	dwVal = 0x3FFFFFFF;
	RegWrite32(DISPC_BASE, DISPC_IRQSTATUS, dwVal);

	//Graphics Channel Out configuration:TV output selected
	dwVal = RegRead32(DISPC_BASE, DISPC_GFX_ATTRIBUTES);
	dwVal |= (1<<8);
	RegWrite32(DISPC_BASE, DISPC_GFX_ATTRIBUTES, dwVal);

	//Graphics Channel Out configuration:LCD output
	//bit fields 31 and 30 defines the output associated (primary,
	//secondary or write-back).
	dwVal = RegRead32(DISPC_BASE, DISPC_GFX_ATTRIBUTES);
	dwVal &= (~(1<<8));
	RegWrite32(DISPC_BASE, DISPC_GFX_ATTRIBUTES, dwVal);

#if 0
	//VID2, Video Channel Out configuration:LCD output
	dwVal = RegRead32(DISPC_BASE, DISPC_VID2_ATTRIBUTES);
	dwVal &= (~(1<<16));
	RegWrite32(DISPC_BASE, DISPC_VID2_ATTRIBUTES, dwVal);

	//VID3, Video Channel Out configuration:LCD output
	dwVal = RegRead32(DISPC_BASE, DISPC_VID3_ATTRIBUTES);
	dwVal &= (~(1<<16));
	RegWrite32(DISPC_BASE, DISPC_VID3_ATTRIBUTES, dwVal);

	//default solid background color for the primary LCD
	dwVal = 0;
	RegWrite32(DISPC_BASE, DISPC_DEFAULT_COLOR0, dwVal);
#endif

	//set_channel_out(channel=0)
	dwVal = RegRead32(DISPC_BASE, DISPC_GFX_ATTRIBUTES);
	dwVal &= (~(1<<8));
	dwVal &= (~(3<<30));
	RegWrite32(DISPC_BASE, DISPC_GFX_ATTRIBUTES, dwVal);

	//set_color_mode [4:1] = 0x6: RGB16-565
	dwVal = RegRead32(DISPC_BASE, DISPC_GFX_ATTRIBUTES);
	dwVal &= (~(0xF<<1));
	dwVal |= (0x6<<1);
	RegWrite32(DISPC_BASE, DISPC_GFX_ATTRIBUTES, dwVal);

	//Set BA0
	dwVal = dwFramebufferAddr;
	RegWrite32(DISPC_BASE, DISPC_GFX_BA0, dwVal);

	//SetBA1
	dwVal = dwFramebufferAddr;
	RegWrite32(DISPC_BASE, DISPC_GFX_BA1, dwVal);

	//DISPC_GFX_POSITION
	dwVal = 0x0;
	RegWrite32(DISPC_BASE, DISPC_GFX_POSITION, dwVal);

	//DISPC_GFX_SIZE
	dwVal = ((LCD_HIGHT - 1) << 16) | (LCD_WIDTH - 1);
	RegWrite32(DISPC_BASE, DISPC_GFX_SIZE, dwVal);

	// dispc_enable_plane( enable=1 )
	dwVal = RegRead32(DISPC_BASE, DISPC_GFX_ATTRIBUTES);
	dwVal |= (1<<0);
	RegWrite32(DISPC_BASE, DISPC_GFX_ATTRIBUTES, dwVal);

}

//**************************************************************************************
// Function: init_DSS
// Description: Init DSS and enable all clocks
//**************************************************************************************
static void init_DSS(void)
{
	ulong dwVal = 0;

	//Enable clocks and force enable by MODULEMODE_ENABLE
	dwVal = RegRead32(CM2_DSS, CM_DSS_DSS_CLKCTRL);
	dwVal |= CM_DSS_DSS_CLKCTRL_OPTFCLKEN_SYS_CLK;
	dwVal |= CM_DSS_DSS_CLKCTRL_OPTFCLKEN_DSSCLK;
	dwVal |= CM_DSS_DSS_CLKCTRL_OPTFCLKEN_TV_FCLK;
	dwVal |= CM_DSS_DSS_CLKCTRL_OPTFCLKEN_48MHZ_CLK;
	dwVal &= (~CM_DSS_DSS_CLKCTRL_MODULEMODE_MASK);
	dwVal |= CM_DSS_DSS_CLKCTRL_MODULEMODE_ENABLE;
	RegWrite32(CM2_DSS, CM_DSS_DSS_CLKCTRL, dwVal);

	//Wait until reset completes
	//while((RegRead32(DSS_BASE, DSS_SYSSTATUS) & 0x1) == 0) {
	//   printf("DSS_SYSSTATUS = %x\n", RegRead32(DSS_BASE, DSS_SYSSTATUS));
	//}

	udelay(50000);

	//autoidle
	dwVal = RegRead32(DSS_BASE, DSS_SYSCONFIG);
	dwVal |= 0x1;
	RegWrite32(DSS_BASE, DSS_SYSCONFIG, dwVal);

	//Select DPLL
	dwVal = RegRead32(DSS_BASE, DSS_CTRL);
	dwVal &= (~0x1);
	RegWrite32(DSS_BASE, DSS_CTRL, dwVal);
}


//**************************************************************************************
// Function: init_pinMux
// Description: Init the pinmux if needed
//**************************************************************************************
static void init_pinMux(void)
{
	ulong dwVal = 0;

	//Enable DSI PHY
	dwVal = 0xFFFFC000;
	RegWrite32(SYSCTRL_PADCONF_CORE, CONTROL_DSIPHY, dwVal);
}

void set_bl_for_recovery()
{
	/* Disabling for now. When it is working in normal mode,
		we'll come back to it for recovery mode */
#if 0
	unsigned char addr = 0;
	unsigned char data = 0;

	if(is_recovery_mode)
	{
		printf("Changing bl mode in set_bl_for_recovery \n");
		/* Set I2C bus 1 @ 100KHz speed */
		if(select_bus(1, 0x64))
			puts("Error in setting I2C bus 1\n");

		/* Set BL_CTL and BRT_MODE[1:0] in LP8552 */
		addr = LP8552_DEV_CTRL_REG;
		data = 0;
		if(i2c_write(LP8552_I2C_ADDR, addr, 1, &data, 1))
			printf("Error writing to lp8552 register %x\n",addr);

		/* Set BRT[7:0] in LP8552 */
		addr = LP8552_BRT_CTRL_REG;
		data = 0;
		if(i2c_write(LP8552_I2C_ADDR, addr, 1, &data, 1))
			printf("Error writing to lp8552 register %x\n",addr);

		/* Switch back to the original I2C bus */
		if(select_bus(0, 0x64))
			puts("Error in setting I2C bus 0\n");

		is_recovery_mode = 0;
	}
	else
		printf("Doing nothing in set_bl_for_recovery \n");
#endif
}


//**************************************************************************************
// Function: NT50169_enable
// Description: Enables Vcon supply to panel
//**************************************************************************************
void NT50169_enable()
{
	u8 val = 0;

	/* Set I2C bus @ 100KHz speed */
	if(select_bus(NT50169_I2C_BUS, 100))
		printf("Error in setting panel power IC I2C bus\n");

	// Check if Vcom is already enabled. If it is, that means it's
	// either first boot after battery plug in, or after long powerbutton
	// press. Disable Vcom for 1s so panel can discharge a bit.
	i2c_read(NT50169_I2C_ADDR, NT50169_I2C_REG_0E, 1, &val, 1);

	// If read fails, val will remain 0, which is ok.
	if (val) {
		val = 0;
		i2c_write(NT50169_I2C_ADDR, NT50169_I2C_REG_0E, 1, &val, 1);
		i2c_write(NT50169_I2C_ADDR, NT50169_I2C_REG_1F, 1, &val, 1);
		mdelay(1000);
	}

	val = NT50169_RELOAD_FLASH;
	if (i2c_write(NT50169_I2C_ADDR, NT50169_I2C_REG_10, 1, &val, 1))
		printf("Error : cannot enable Vcom!\n");

	/* Switch back to the original I2C bus */
	if(select_bus(0, 100))
		printf("Error in setting I2C bus back to 0\n");
}

//**************************************************************************************
// Function: NT50169_disable
// Description: Disables Vcon supply to panel
//**************************************************************************************
void NT50169_disable()
{
	u8 val = 0;
	/* Set I2C bus @ 100KHz speed */
	if (select_bus(LP855x_I2C_BUS, 100))
		printf("Error in setting panel power IC I2C bus\n");

	if (i2c_write(NT50169_I2C_ADDR, NT50169_I2C_REG_0E, 1, &val, 1)
		|| i2c_write(NT50169_I2C_ADDR, NT50169_I2C_REG_1F, 1, &val, 1))
			printf("Error : cannot disable Vcom!\n");

	/* Switch back to the original I2C bus */
	if(select_bus(0, 100))
		printf("Error in setting I2C bus back to 0\n");
}

//**************************************************************************************
// Function: Power_on_LCD
// Description: Power_on_LCD
// Depend: GPIO driver
//**************************************************************************************
static void tate_Power_on_LCD(void)
{
	char *idme = idme=idme_get_board_id();
	//LCD Panel Enable
	//printf("LCD Panel Enable. \n");

	if (strncmp(idme, "80803", 5) > 0){
		configure_pad_mode(GPIO_LCD_ENABLE_190);
		set_gpio_output(GPIO_LCD_ENABLE_190, 1);
		configure_gpio_output(GPIO_LCD_ENABLE_190);
		//Enable pullup to keep display powered during kernel gpio6 module reset.
		RegSet32(OMAP44XX_CTRL_BASE, CONTROL_PADCONF_DPM_EMU17, PTU << 16);
	} else {
		configure_pad_mode(GPIO_LCD_ENABLE_35);
		//Enable pullup to keep display powered during kernel gpio2 module reset.
		RegSet32(OMAP44XX_CTRL_BASE, CONTROL_PADCONF_GPMC_AD10, PTU << 16);
		configure_gpio_output(GPIO_LCD_ENABLE_35);
		set_gpio_output(GPIO_LCD_ENABLE_35, 1);
	}

	//If board is Soho PVTv2 then enable the switch for powering on LCD
	if (strncmp(idme, "80808", 5) == 0) {
		configure_pad_mode(GPIO_TST_LED_41);
		set_gpio_output(GPIO_TST_LED_41, 1);
		configure_gpio_output(GPIO_TST_LED_41);
	}

	//After EVT1.0a, Brightness is controled by CABC enable CABC
	configure_pad_mode(GPIO_BACKLIGHT_CABC_EN);
	configure_gpio_output(GPIO_BACKLIGHT_CABC_EN);
	set_gpio_output(GPIO_BACKLIGHT_CABC_EN, 1);

	//If board is Soho EVT1 configure display power IC.
	if (!strncmp(idme, "80803", 5))
		NT50169_enable();
}

//**************************************************************************************
// Function: Power_off_LCD
// Description: Power_off_LCD
// Depend: GPIO driver
//**************************************************************************************
static void tate_Power_off_LCD(void)
{
	char *idme = idme=idme_get_board_id();

	Set_Brightness(0);
	if (!strncmp(idme, "80803", 5))
		NT50169_disable();
	/* FIXME: This just turns off the backlight, not the LCD panel */
	configure_pad_mode(GPIO_BACKLIGHT_CABC_EN);
	configure_gpio_output(GPIO_BACKLIGHT_CABC_EN);
	set_gpio_output(GPIO_BACKLIGHT_CABC_EN, 0);

	//LCD Panel Disable
	//printf("LCD Panel Disable. \n");

	if (strncmp(idme, "80803", 5) > 0){
		configure_pad_mode(GPIO_LCD_ENABLE_190);
		configure_gpio_output(GPIO_LCD_ENABLE_190);
		set_gpio_output(GPIO_LCD_ENABLE_190, 0);
	} else {
		configure_pad_mode(GPIO_LCD_ENABLE_35);
		configure_gpio_output(GPIO_LCD_ENABLE_35);
		set_gpio_output(GPIO_LCD_ENABLE_35, 0);
	}

	//If board is Soho PVTv2 then disable the switch for powering off LCD
	if (strncmp(idme, "80808", 5) == 0) {
		configure_pad_mode(GPIO_TST_LED_41);
		set_gpio_output(GPIO_TST_LED_41, 0);
		configure_gpio_output(GPIO_TST_LED_41);
	}

} 

//**************************************************************************************
// Function: Set_Brightness
// Description: Set_Brightness
// Depend: GPIO driver
//**************************************************************************************
struct lp855x_init_sequence {
	u8 addr;
	u8 val;
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static struct lp855x_init_sequence lp8552_init_seq[] = {
		{ LP8552_DEV_CTRL_REG, 0x00}, /* set default mode in DEV_CTRL */
		{ LP8552_BRT_CTRL_REG, 0x00}, /* set default mode in BRT_CRL */
		{ LP8552_EEPROM_ADDR0, LP8552_BRT_LEVEL}, /* set initial brightness level */
		{ LP8552_EEPROM_ADDR1, LP8552_NO_SLOPE_TIME}, /* Write in EEPROM */
};
#define LP8552_SET_BR_SEQ_NUM		2

static int lp8557_update_bit(u8 reg, u8 mask, u8 data)
{
	int ret;
	u8 tmp;

	ret = i2c_read(LP855x_I2C_ADDR, reg, 1, &tmp, 1);
	if (ret < 0) {
		printf("failed to read 0x%.2x\n", reg);
		return ret;
	}

	tmp = (u8)ret;
	tmp &= ~mask;
	tmp |= data & mask;

	return i2c_write(LP855x_I2C_ADDR, reg, 1, &tmp, 1);
}

static int lp8557_bl_on(int on)
{
	printf("LP8557 backlight => %d\n", on);
	if (on) {
		/* BL_ON = 1 after updating EPROM settings */
		return lp8557_update_bit(LP8557_BL_CMD, LP8557_BL_MASK,
				LP8557_BL_ON);
	}
	else {
		/* BL_ON = 0 before updating EPROM settings */
		return lp8557_update_bit(LP8557_BL_CMD, LP8557_BL_MASK,
				LP8557_BL_OFF);
	}
}

static struct lp855x_init_sequence lp8557_init_seq[] = {
		{ LP8557_BRIGHTNESS_CTRL, LP8557_BRT_LEVEL}, /* set default mode in BRT_CTRL*/
		{ LP8557_CONFIG, LP8557_PLAT_CONFIG}, /* set default config in CONFIG */
		{ LP8557_EPROM_ADDR4, LP8557_EPROM_CONFIG}, /* 4V OV, 5 LED string enabled */
		{ LP8557_MAXCURR, LP8557_CURRENT}, /* Set the maximum LED current to 20ma */
};

#define LP8557_SET_BR_SEQ_NUM		0

static void tate_Set_Brightness(int is_Max)
{
	unsigned char addr = 0;
	unsigned char data = 0;
	int init_seq_size;
	int i;
	char *idme = idme=idme_get_board_id();

	if(is_Max == 1) {
		/* FIXME: GPIO is not used to enable backlight driver in Tate EVT 1.x */
		//LCD Backlight Enable
		//printf("LCD Backlight Enable. \n");
		configure_pad_mode(GPIO_BACKLIGHT_EN_O2M);
		configure_gpio_output(GPIO_BACKLIGHT_EN_O2M);
		set_gpio_output(GPIO_BACKLIGHT_EN_O2M, 1);

		//printf("Set PWM to Max. \n");
		if (strncmp(idme, "80803", 5) > 0){
			configure_pad_mode(GPIO_BACKLIGHT_PWM_35);
			configure_gpio_output(GPIO_BACKLIGHT_PWM_35);
			set_gpio_output(GPIO_BACKLIGHT_PWM_35, 1);
		} else {
			configure_pad_mode(GPIO_BACKLIGHT_PWM_190);
			configure_gpio_output(GPIO_BACKLIGHT_PWM_190);
			set_gpio_output(GPIO_BACKLIGHT_PWM_190, 1);
		}

		/* Set I2C bus @ 100KHz speed */
		if(select_bus(LP855x_I2C_BUS, 0x64))
			puts("Error in setting backlight I2C bus\n");

		if (is_lp8557 == 0)
			init_seq_size = ARRAY_SIZE(lp8552_init_seq);
		else
			init_seq_size = ARRAY_SIZE(lp8557_init_seq);

		if (is_lp8557 == 1)
			/* Disable backlight before writing in EPROM registers */
			lp8557_bl_on(0);

		if (is_lp8557 == 0){
			for (i=0;i<init_seq_size;i++) {
				addr = lp8552_init_seq[i].addr;
				data = lp8552_init_seq[i].val;
				if(i2c_write(LP855x_I2C_ADDR, addr, 1, &data, 1))
					printf("Error writing to lp8552 register %x\n",addr);
			}
		} else {
			for (i=0;i<init_seq_size;i++) {
				addr = lp8557_init_seq[i].addr;
				data = lp8557_init_seq[i].val;
				if(i2c_write(LP855x_I2C_ADDR, addr, 1, &data, 1))
					printf("Error writing to lp8557 register %x\n",addr);
			}
		}

		if (is_lp8557 == 1)
			/* Enable backlight after writing in EPROM registers */
			lp8557_bl_on(1);

		/* Switch back to the original I2C bus */
		if(select_bus(0, 0x64))
			puts("Error in setting I2C bus 0\n");
	}
	else if(is_Max == 2) {
		// just turn on gpios for showing up backlight during charging
		configure_pad_mode(GPIO_BACKLIGHT_EN_O2M);
		configure_gpio_output(GPIO_BACKLIGHT_EN_O2M);
		set_gpio_output(GPIO_BACKLIGHT_EN_O2M, 1);

		if (strncmp(idme, "80803", 5) > 0){
			configure_pad_mode(GPIO_BACKLIGHT_PWM_35);
			configure_gpio_output(GPIO_BACKLIGHT_PWM_35);
			set_gpio_output(GPIO_BACKLIGHT_PWM_35, 1);
		} else {
			configure_pad_mode(GPIO_BACKLIGHT_PWM_190);
			configure_gpio_output(GPIO_BACKLIGHT_PWM_190);
			set_gpio_output(GPIO_BACKLIGHT_PWM_190, 1);
		}
	}
	else if (is_Max == 0){
		//LCD Backlight disable
		//printf("LCD Backlight Enable. \n");
		configure_pad_mode(GPIO_BACKLIGHT_EN_O2M);
		configure_gpio_output(GPIO_BACKLIGHT_EN_O2M);
		set_gpio_output(GPIO_BACKLIGHT_EN_O2M, 0);

		//printf("Set PWM to L. \n");
		if (strncmp(idme, "80803", 5) > 0){
			configure_pad_mode(GPIO_BACKLIGHT_PWM_35);
			configure_gpio_output(GPIO_BACKLIGHT_PWM_35);
			set_gpio_output(GPIO_BACKLIGHT_PWM_35, 0);
		} else {
			configure_pad_mode(GPIO_BACKLIGHT_PWM_190);
			configure_gpio_output(GPIO_BACKLIGHT_PWM_190);
			set_gpio_output(GPIO_BACKLIGHT_PWM_190, 0);
		}
	}
	else {
		configure_pad_mode(GPIO_BACKLIGHT_EN_O2M);
		configure_gpio_output(GPIO_BACKLIGHT_EN_O2M);
		set_gpio_output(GPIO_BACKLIGHT_EN_O2M, 1);

		//printf("Set PWM to Max. \n");
		if (strncmp(idme, "80803", 5) > 0){
			configure_pad_mode(GPIO_BACKLIGHT_PWM_35);
			configure_gpio_output(GPIO_BACKLIGHT_PWM_35);
			set_gpio_output(GPIO_BACKLIGHT_PWM_35, 1);
		} else {
			configure_pad_mode(GPIO_BACKLIGHT_PWM_190);
			configure_gpio_output(GPIO_BACKLIGHT_PWM_190);
			set_gpio_output(GPIO_BACKLIGHT_PWM_190, 1);
		}

		/* Set I2C bus @ 100KHz speed */
		if(select_bus(LP855x_I2C_BUS, 0x64))
			puts("Error in setting backlight I2C bus\n");

		if (is_lp8557 == 0)
			init_seq_size = ARRAY_SIZE(lp8552_init_seq);
		else
			init_seq_size = ARRAY_SIZE(lp8557_init_seq);

		if (is_lp8557 == 1)
			/* Disable backlight before writing in EPROM registers */
			lp8557_bl_on(0);

		if (is_lp8557 == 0) {
			/* Set initial brightness value */
			lp8552_init_seq[LP8552_SET_BR_SEQ_NUM].val = is_Max;
			for (i=0;i<init_seq_size;i++) {
				addr = lp8552_init_seq[i].addr;
				data = lp8552_init_seq[i].val;
				if(i2c_write(LP855x_I2C_ADDR, addr, 1, &data, 1))
					printf("Error writing to lp8552 register %x\n",addr);
			}
		} else {
			/* Set initial brightness value */
			lp8557_init_seq[LP8557_SET_BR_SEQ_NUM].val = is_Max;
			for (i=0;i<init_seq_size;i++) {
				addr = lp8557_init_seq[i].addr;
				data = lp8557_init_seq[i].val;
				if(i2c_write(LP855x_I2C_ADDR, addr, 1, &data, 1))
					printf("Error writing to lp8557 register %x\n",addr);
			}
		}

		if (is_lp8557 == 1)
			/* Enable backlight after writing in EPROM registers */
			lp8557_bl_on(1);

		/* Switch back to the original I2C bus */
		if(select_bus(0, 0x64))
			puts("Error in setting I2C bus 0\n");
	}
}

//**************************************************************************************
// Function:  Enable_LCD
// Description: Enable_LCD
//**************************************************************************************
static void Enable_LCD(void)
{
	ulong dwVal = 0;

	//goLCD
	dwVal = RegRead32(DISPC_BASE, DISPC_CONTROL1);
	dwVal |= 1<<5;
	RegWrite32(DISPC_BASE, DISPC_CONTROL1, dwVal);

	//Enable LCD
	dwVal = RegRead32(DISPC_BASE, DISPC_CONTROL1);
	dwVal |= 1<<0;
	RegWrite32(DISPC_BASE, DISPC_CONTROL1, dwVal);
}



//**************************************************************************************
// Function:  Init_LCD
// Description: configures DSS, DSI and LCD, caller is misc_init_r of omap4.c
//		Returns 0 if LCD init has to be skipped, 1 otherwise.
//**************************************************************************************
static int tate_Init_LCD(void)
{
	static int lcd_init = 0;

	/* Avoid double init */
	if (lcd_init == 1)
		return 0;

	//power on LCD
	Power_on_LCD();

	mdelay(100); // need this delay before we send any DSI command

	//turn off backlight
	Set_Brightness(0);

	//init pinMux
	init_pinMux();

	//init DSS
	init_DSS();

	//Init DISPC
	set_DISPC(FRAME_BUFFER_ADDR);

	//rset DSI
	dsi_reset();

	//Set DPLL for DSI
	dsi_set_dpll();

	//set to Videomode
	dsi_video_preinit();

	//enter Bist Mode
	//LCD_Bist();

	//Normal display
	LCD_Normal();

	//Post init for Video Mode
	dsi_videomode_PostInit();

	//Enable LCD1 output
	Enable_LCD();

	//Clear LCD, show black
	show_color((u_int16_t)(0x0000));

	lcd_init = 1;

	return 1;
}


static void display_disable()
{
	ulong dwVal;
	int timeout = MAX_DELAY_COUNTER;

	dwVal = RegRead32(DSI_BASE, DSI_VC0_CTRL);
	dwVal &= (~(1<<0));
	RegWrite32(DSI_BASE, DSI_VC0_CTRL, dwVal);

	//wait for virtual channel 0 disabled
	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC0_CTRL) & (1<<0)) != 0) && (timeout > 0)) {
		timeout--;
		udelay(1);
	}
	if(timeout <=0) printf("Error in virtual channel 0 disabled timeout!!!\n");

	dwVal = RegRead32(DSI_BASE, DSI_VC1_CTRL);
	dwVal &= (~(1<<0));
	RegWrite32(DSI_BASE, DSI_VC1_CTRL, dwVal);

	//wait for virtual channel 1 disabled
	timeout = MAX_DELAY_COUNTER;
	while(((RegRead32(DSI_BASE, DSI_VC1_CTRL) & (1<<0)) != 0) && (timeout > 0)) {
		timeout--;
		udelay(1);
	}
	if(timeout <=0) printf("Error in virtual channel 1 disabled timeout!!!\n");

	dwVal = RegRead32(DSI_BASE, DSI_CTRL);
	dwVal &= (~(1<<0));
	RegWrite32(DSI_BASE, DSI_CTRL, dwVal);

	//wait for interface disable
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_CTRL) & (1<<0)) != 0 ) && (timeout > 0)) {
		timeout--;
		udelay(1);
	}
	if(timeout <=0) printf("Error in DSI Interface disable timeout!!!\n");

	//switch dispc clock
	dwVal = RegRead32(DSS_BASE, DSS_CTRL);
	dwVal &= (~(1<<0));
	RegWrite32(DSS_BASE, DSS_CTRL, dwVal);

	//switch dsi clock
	dwVal = RegRead32(DSS_BASE, DSS_CTRL);
	dwVal &= (~(1<<1));
	RegWrite32(DSS_BASE, DSS_CTRL, dwVal);

	//complex I/O power off
	dwVal = RegRead32(DSI_BASE, DSI_COMPLEXIO_CFG1);
	dwVal &= (~(1 << 27));
	RegWrite32(DSI_BASE, DSI_COMPLEXIO_CFG1, dwVal);

	//Set the GOBIT bit
	dwVal = RegRead32(DSI_BASE, DSI_COMPLEXIO_CFG1);
	dwVal |= (1<<30);
	RegWrite32(DSI_BASE, DSI_COMPLEXIO_CFG1, dwVal);

	//wait for PWR_STATUS = off
	timeout = MAX_DELAY_COUNTER;
	while(( (RegRead32(DSI_BASE, DSI_COMPLEXIO_CFG1) & (0x3<<25)) != (0 << 25) ) && (timeout > 0)) {
		timeout--;
		udelay(1);
	}
	if(timeout <=0) printf("Error in wait for PWR_STATUS timeout when set dsi_complexio!!!\n");

	//disable scp clock
	dwVal = RegRead32(DSI_BASE, DSI_CLK_CTRL);
	dwVal &= (~(1<<14));
	RegWrite32(DSI_BASE, DSI_CLK_CTRL, dwVal);

	//power off both PLL and HSDIVISER
	dwVal = RegRead32(DSI_BASE, DSI_CLK_CTRL);
	dwVal &= (~(3<<30));
	RegWrite32(DSI_BASE, DSI_CLK_CTRL, dwVal);

	//Wait until PLL_PWR_STATUS = 0x0
	timeout = MAX_DELAY_COUNTER;
	while(((((RegRead32(DSI_BASE, DSI_CLK_CTRL) & 0x30000000) >> 28) != 0x0)) && (timeout > 0)) {
		timeout--;
		udelay(1);
	}
	if(timeout <=0) printf("Error in turn off PLL power fail!!! %x \n", RegRead32(DSI_BASE, DSI_CLK_CTRL));

	//disable scp clock
	dwVal = RegRead32(DSI_BASE, DSI_CLK_CTRL);
	dwVal &= (~(1<<14));
	RegWrite32(DSI_BASE, DSI_CLK_CTRL, dwVal);

}

static void display_reinit()
{
	//init DSS
	init_DSS();

	//Init DISPC
	set_DISPC(FRAME_BUFFER_ADDR);

	//rset DSI
	dsi_reset();

	//Set DPLL for DSI
	dsi_set_dpll();

	//set DSI Phy
	dsi_set_phy();

	//set DSI protocol engine
	dsi_set_protocol_engine();

	//set to Videomode
	dsi_video_preinit();

	//enter Bist Mode
	//LCD_Bist();

	//Normal display
	LCD_Normal();

	//Post init for Video Mode
	dsi_videomode_PostInit();

	//Enable LCD1 output
	Enable_LCD();
}




/* initlogo.rle data */
extern u_int16_t const _binary_initlogo_gz_start[];
/* lowbattery.gz data */
extern u_int16_t const _binary_lowbattery_gz_start[];
extern u_int16_t const _binary_lowbattery_gz_end[];
/* charging.gz data */
extern u_int16_t const _binary_charging_gz_start[];
extern u_int16_t const _binary_charging_gz_end[];
/* charging_tate_low.gz */
extern u_int16_t const _binary_charging_low_gz_start[];
/* devicetoohot.gz data */
extern u_int16_t const _binary_devicetoohot_gz_start[];
/* fastboot.gz data */
extern u_int16_t const _binary_fastboot_gz_start[];
extern u_int16_t const _binary_fastboot_gz_end[];
/* multi_download.gz data */
extern char const _binary_multi_download_gz_start[];
extern char const _binary_multi_download_gz_end[];


struct display_lcd_s tate_display = {
.Init_LCD = tate_Init_LCD,
.Power_on_LCD = tate_Power_on_LCD,
.Power_off_LCD = tate_Power_off_LCD,
.backlight_set = tate_Set_Brightness,
.frame_buffer= (u_int16_t *) FRAME_BUFFER_ADDR,
.gzip_buffer= (unsigned char *) GUNZIP_BUFFER_ADDR,
.width =  LCD_WIDTH,
.height =  LCD_HIGHT,
.bpp=LCD_BPP,
.frame_size = LCD_PIXELS,
.binary_initlogo_rle_start      =(u_int16_t *) NULL,
.binary_initlogo_rle_end        =(u_int16_t *) NULL,
.binary_provfailed_rle_start    =(u_int16_t *) NULL,
.binary_provfailed_rle_end      =(u_int16_t *) NULL,
.binary_initlogo_gz_start	=(void *)_binary_initlogo_gz_start,
.binary_lowbattery_gz_start     =(void *)_binary_lowbattery_gz_start,
.binary_charging_gz_start       =(void *)_binary_charging_gz_start,
.binary_charging_low_gz_start       =(void *) NULL,
.binary_devicetoohot_rle_start =(u_int16_t *) NULL,
.binary_devicetoohot_rle_end   =(u_int16_t *) NULL,
.binary_devicetoohot_gz_start  =(void *) _binary_devicetoohot_gz_start,
.binary_fastboot_gz_start       =(void *)_binary_fastboot_gz_start,
.binary_multi_download_gz_start =(void *)_binary_multi_download_gz_start,
} ;

void config_lp855x(int conf)
{
	if (conf)
		is_lp8557 = 1;
	else
		is_lp8557 = 0;
}

void register_tate_display(void)
{
	printf("Soho display registered\n");
	register_display(&tate_display);
}


