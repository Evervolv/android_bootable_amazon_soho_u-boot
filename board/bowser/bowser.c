/*
 * (C) Copyright 2004-2009
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <mmc.h>
#include <idme.h>


#define		OMAP44XX_WKUP_CTRL_BASE		0x4A31E000
#if 1
#define M0_SAFE M0
#define M1_SAFE M1
#define M2_SAFE M2
#define M4_SAFE M4
#define M7_SAFE M7
#define M3_SAFE M3
#define M5_SAFE M5
#define M6_SAFE M6
#else
#define M0_SAFE M7
#define M1_SAFE M7
#define M2_SAFE M7
#define M4_SAFE M7
#define M7_SAFE M7
#define M3_SAFE M7
#define M5_SAFE M7
#define M6_SAFE M7
#endif
#define		MV(OFFSET, VALUE)\
			__raw_writew((VALUE), OMAP44XX_CTRL_BASE + (OFFSET));
#define		MV1(OFFSET, VALUE)\
			__raw_writew((VALUE), OMAP44XX_WKUP_CTRL_BASE + (OFFSET));

#define		CP(x)	(CONTROL_PADCONF_##x)
#define		WK(x)	(CONTROL_WKUP_##x)

/*Platform pinmux setting*/
#include "soho_pinmux.c"

#define MUX_BOWSER_EMMC_8BIT() \
	MV(CP(GPMC_AD0) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat0 */ \
	MV(CP(GPMC_AD1) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat1 */ \
	MV(CP(GPMC_AD2) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat2 */ \
	MV(CP(GPMC_AD3) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat3 */ \
	MV(CP(GPMC_AD4) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat4 */ \
	MV(CP(GPMC_AD5) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat5 */ \
	MV(CP(GPMC_AD6) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat6 */ \
	MV(CP(GPMC_AD7) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat7 */ \
	MV(CP(GPMC_NOE) , ( PTU | IEN | OFF_EN | OFF_OUT_PTD | M1))  /* sdmmc2_clk */ \
	MV(CP(GPMC_NWE) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_cmd */

/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */
/**********************************************************
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers
 *              specific to the hardware. Many pins need
 *              to be moved from protect to primary mode.
 *********************************************************/
void set_muxconf_regs(void)
{
	/*
	 * No pad muxes should be needed early.
	 * Any conflicting pad mux that need to be initialized early need
	 * to be in xloader. Make an exception for eMMC.
	 */
	MUX_BOWSER_EMMC_8BIT();
	return;
}

/**********************************************************
 * Routine: set_muxconf_late
 * Description: Setting up the configuration Mux registers
 *              specific to the hardware. Many pins need
 *              to be moved from protect to primary mode.
 *********************************************************/
void set_muxconf_late(void)
{
	u8 *idme;

	idme=idme_get_board_id();

#if defined (CONFIG_MACH_BOWSER) || defined(CONFIG_MACH_BOWSER_SUBTYPE_SOHO)
	extern void register_tate_display(void);
	register_tate_display();
	extern void config_lp855x(int conf);
	if (strncmp(idme, "80801", 5) == 0)
		config_lp855x(0);
	else
		config_lp855x(1);
#endif

	if (strncmp(idme, "808", 3) == 0 ) {
		printf("SOHO Pinmux configuration\n");
		MUX_SOHO();
	}
	
	return;
}






/*****************************************
 * Routine: get_bowser_board_revision 
 * Description: Uses 4 reserved board id pins as GPIO's
 *   to determine the bowser board id
 *****************************************/
u32 get_bowser_board_revision()
{
#if !defined(CONFIG_MACH_BOWSER_SUBTYPE_TATE) /* Bowser has board ID pins */
	u32 val;
	u32 rev = BOARD_REVISION_INVALID;
	u16 data11_backup, data12_backup, data13_backup, data14_backup;

	const u32 gpio4_base_address = 0x48059000;
	const u32 gpio_oe_offset = 0x0134;
	const u32 gpio_datain_offset = 0x0138;

	// backup pad configuration values
	data11_backup = __raw_readw(OMAP44XX_CTRL_BASE + CONTROL_PADCONF_C2C_DATA11);
	data12_backup = __raw_readw(OMAP44XX_CTRL_BASE + CONTROL_PADCONF_C2C_DATA12);
	data13_backup = __raw_readw(OMAP44XX_CTRL_BASE + CONTROL_PADCONF_C2C_DATA13);
	data14_backup = __raw_readw(OMAP44XX_CTRL_BASE + CONTROL_PADCONF_C2C_DATA14);

	// set pad configuration values for ID detection
	MV(CONTROL_PADCONF_C2C_DATA11, (IEN | PTU | M3));
	MV(CONTROL_PADCONF_C2C_DATA12, (IEN | PTU | M3));
	MV(CONTROL_PADCONF_C2C_DATA13, (IEN | PTU | M3));
	MV(CONTROL_PADCONF_C2C_DATA14, (IEN | PTU | M3));

	// configure board id pin's GPIO settings as inputs
	val = __raw_readl(gpio4_base_address + gpio_oe_offset);
	val |= 0x000000F0;
	__raw_writel(val, gpio4_base_address + gpio_oe_offset);

	// read values of board id pins
	val = __raw_readl(gpio4_base_address + gpio_datain_offset);
	val = ((val & 0x000000F0) >> 4);

	// translate board id pin values into board revisions
	switch (val)
	{
		case 0xF: rev = BOARD_REVISION_BOWSER_1; break;
		case 0x8: rev = BOARD_REVISION_BOWSER_2; break;
		default:
			  printf("ERROR: invalid board revision detected: 0x%x!\n", val);
			  rev = BOARD_REVISION_INVALID;  
			  break;
	}
	
	// restore original pad configuration values
	MV(CONTROL_PADCONF_C2C_DATA11, data11_backup);
	MV(CONTROL_PADCONF_C2C_DATA12, data12_backup);
	MV(CONTROL_PADCONF_C2C_DATA13, data13_backup);
	MV(CONTROL_PADCONF_C2C_DATA14, data14_backup);
#else /* Tate does not have board ID pins */
        u32 rev = BOARD_REVISION_INVALID;
#endif
	return rev;
}

/*****************************************
 * Routine: SWBootingConfiguration
 * Description:  SW Booting Configuration
 *****************************************/
void set_SWBootingCfg()
{
  unsigned val;

  val = __raw_readl(0x4A307B08);

  printf("Before write, PRM_RSTTIME = %x\n", val);

  val &= ~(0x3ff);
  val |= 0x190;

 __raw_writel(val, 0x4A307B08);

  val = __raw_readl(0x4A307B08);

  printf("After write, PRM_RSTTIME = %x\n", val);


  __raw_writel( 0x4A326A0C , 0x4A326A00 );   // Address in Public Internal SRAM where SW Booting Configuration is
  __raw_writel( 0xCF00AA01 , 0x4A326A0C );   // Header for SW Booting Configuration
  __raw_writel( 0x0000000C , 0x4A326A10 );
  __raw_writel( 0x00450000 , 0x4A326A14 );   // USB Boot First
  __raw_writel( 0x00000000 , 0x4A326A18 );
  __raw_writel( 0x00000000 , 0x4A326A1C );
  /* now warm reset the silicon */
  __raw_writel(PRM_RSTCTRL_RESET_WARM_BIT,PRM_RSTCTRL);
}

/*****************************************
 * Routine: board_init
 * Description: Early hardware init.
 *****************************************/
int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gpmc_init();

#if 0 /* No eMMC env partition for now */
	/* Intializing env functional pointers with eMMC */
	boot_env_get_char_spec = mmc_env_get_char_spec;
	boot_env_init = mmc_env_init;
	boot_saveenv = mmc_saveenv;
	boot_env_relocate_spec = mmc_env_relocate_spec;
	env_ptr = (env_t *) (CFG_FLASH_BASE + CFG_ENV_OFFSET);
	env_name_spec = mmc_env_name_spec;
#endif

	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP4_BOWSER;
	gd->bd->bi_boot_params = (0x80000000 + 0x100); /* boot param addr */

	// get bowser board revision
	gd->bd->bi_board_revision = get_bowser_board_revision();


	return 0;
}

/****************************************************************************
 * check_fpga_revision number: the rev number should be a or b
 ***************************************************************************/
inline u16 check_fpga_rev(void)
{
	return __raw_readw(FPGA_REV_REGISTER);
}

/****************************************************************************
 * check_uieeprom_avail: Check FPGA Availability
 * OnBoard DEBUG FPGA registers need to be ready for us to proceed
 * Required to retrieve the bootmode also.
 ***************************************************************************/
int check_uieeprom_avail(void)
{
	volatile unsigned short *ui_brd_name =
	    (volatile unsigned short *)EEPROM_UI_BRD + 8;
	int count = 1000;

	/* Check if UI revision Name is already updated.
	 * if this is not done, we wait a bit to give a chance
	 * to update. This is nice to do as the Main board FPGA
	 * gets a chance to know off all it's components and we can continue
	 * to work normally
	 * Currently taking 269* udelay(1000) to update this on poweron
	 * from flash!
	 */
	while ((*ui_brd_name == 0x00) && count) {
		udelay(200);
		count--;
	}
	/* Timed out count will be 0? */
	return count;
}

/***********************************************************************
 * get_board_type() - get board revision if it wasn't already set in the
 * global data board information structure
 ************************************************************************/
u32 get_board_type(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (gd->bd->bi_board_revision == BOARD_REVISION_INVALID)
	{
		gd->bd->bi_board_revision = get_bowser_board_revision();
	}

	return gd->bd->bi_board_revision;
}
