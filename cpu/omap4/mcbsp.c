#include <common.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <asm/arch/mcbsp.h>

struct omap_mcbsp omap_mcbsp_data[] =
{
	{
		.id = 1,
		.io_base = OMAP4_MCBSP1_MPU_BASE,
		.reg_size = 4,
		.reg_step = 4,
	},
	{
		.id = 2,
		.io_base = OMAP4_MCBSP2_MPU_BASE,
		.reg_size = 4,
		.reg_step = 4,
	},
	{
		.id = 3,
		.io_base = OMAP4_MCBSP3_MPU_BASE,
		.reg_size = 4,
		.reg_step = 4,
	},
	{
		.id = 4,
		.io_base = OMAP4_MCBSP4_L4_BASE,
		.reg_size = 4,
		.reg_step = 4,
	},
};

void omap_mcbsp_write(struct omap_mcbsp *mcbsp, u16 reg, u32 val)
{
	u32 addr = mcbsp->io_base + reg;

	if (mcbsp->reg_size == 2) {
		__raw_writew((u16)val, addr);
	} else {
		__raw_writel(val, addr);
	}
}

int omap_mcbsp_read(struct omap_mcbsp *mcbsp, u16 reg)
{
	u32 addr = mcbsp->io_base + reg;

	if (mcbsp->reg_size == 2) {
		return __raw_readw(addr);
	} else {
		return __raw_readl(addr);
	}
}

struct omap_mcbsp * omap_mcbsp_get_instance(int indx)
{
	if (indx < OMAP_MCBSP_1 || indx > OMAP_MCBSP_4)
		return NULL;

	return &omap_mcbsp_data[indx];
}

void omap_mcbsp_dump_reg(struct omap_mcbsp *mcbsp)
{
	printf("**** McBSP%d regs ****\n", mcbsp->id);
	printf("DRR:  0x%04x\n",
			MCBSP_READ(mcbsp, DRR));
	printf("DXR:  0x%04x\n",
			MCBSP_READ(mcbsp, DXR));
	printf("SPCR2: 0x%04x\n",
			MCBSP_READ(mcbsp, SPCR2));
	printf("SPCR1: 0x%04x\n",
			MCBSP_READ(mcbsp, SPCR1));
	printf("RCR2:  0x%04x\n",
			MCBSP_READ(mcbsp, RCR2));
	printf("RCR1:  0x%04x\n",
			MCBSP_READ(mcbsp, RCR1));
	printf("XCR2:  0x%04x\n",
			MCBSP_READ(mcbsp, XCR2));
	printf("XCR1:  0x%04x\n",
			MCBSP_READ(mcbsp, XCR1));
	printf("SRGR2: 0x%04x\n",
			MCBSP_READ(mcbsp, SRGR2));
	printf("SRGR1: 0x%04x\n",
			MCBSP_READ(mcbsp, SRGR1));
	printf("PCR0:  0x%04x\n",
			MCBSP_READ(mcbsp, PCR0));
	printf("***********************\n");
};
