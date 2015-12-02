#ifndef __MCBSP_H
#define __MCBSP_H


enum {
	OMAP_MCBSP_1 = 0,
	OMAP_MCBSP_2,
	OMAP_MCBSP_3,
	OMAP_MCBSP_4,
	OMAP_MCBSP_MAX,
};

#define OMAP4_CM1_ABE_MCBSP1_CLKCTRL	0x4a004548
#define OMAP4_CM1_ABE_MCBSP2_CLKCTRL	0x4a004550
#define OMAP4_CM1_ABE_MCBSP3_CLKCTRL	0x4a004558

#define OMAP4_MCBSP1_L3_BASE		0x49022000
#define OMAP4_MCBSP2_L3_BASE		0x49024000
#define OMAP4_MCBSP3_L3_BASE		0x49026000

#define OMAP4_MCBSP1_MPU_BASE		0x40122000
#define OMAP4_MCBSP2_MPU_BASE		0x40124000
#define OMAP4_MCBSP3_MPU_BASE		0x40126000

#define OMAP4_MCBSP4_L4_BASE		0x48096000


#define OMAP_MCBSP_REG_DRR		0x00
#define OMAP_MCBSP_REG_DXR		0x08
#define OMAP_MCBSP_REG_SPCR2		0x10
#define OMAP_MCBSP_REG_SPCR1		0x14
#define OMAP_MCBSP_REG_RCR2		0x18
#define OMAP_MCBSP_REG_RCR1		0x1c
#define OMAP_MCBSP_REG_XCR2		0x20
#define OMAP_MCBSP_REG_XCR1		0x24
#define OMAP_MCBSP_REG_SRGR2		0x28
#define OMAP_MCBSP_REG_SRGR1		0x2c
#define OMAP_MCBSP_REG_MCR2		0x30
#define OMAP_MCBSP_REG_MCR1		0x34
#define OMAP_MCBSP_REG_RCERA		0x38
#define OMAP_MCBSP_REG_RCERB		0x3c
#define OMAP_MCBSP_REG_XCERA		0x40
#define OMAP_MCBSP_REG_XCERB		0x44
#define OMAP_MCBSP_REG_PCR0		0x48
#define OMAP_MCBSP_REG_RCERC		0x4c
#define OMAP_MCBSP_REG_RCERD		0x50
#define OMAP_MCBSP_REG_XCERC		0x54
#define OMAP_MCBSP_REG_XCERD		0x58
#define OMAP_MCBSP_REG_RCERE		0x5c
#define OMAP_MCBSP_REG_RCERF		0x60
#define OMAP_MCBSP_REG_XCERE		0x64
#define OMAP_MCBSP_REG_XCERF		0x68
#define OMAP_MCBSP_REG_RCERG		0x6c
#define OMAP_MCBSP_REG_RCERH		0x70
#define OMAP_MCBSP_REG_XCERG		0x74
#define OMAP_MCBSP_REG_XCERH		0x78
#define OMAP_MCBSP_REG_REV		0x7c
#define OMAP_MCBSP_REG_RINTCLR		0x80
#define OMAP_MCBSP_REG_XINTCLR		0x84
#define OMAP_MCBSP_REG_ROVFLCLR		0x88
#define OMAP_MCBSP_REG_SYSCONFIG	0x8c
#define OMAP_MCBSP_REG_THRSH2		0x90
#define OMAP_MCBSP_REG_THRSH1		0x94
#define OMAP_MCBSP_REG_IRQSTATUS	0xa0
#define OMAP_MCBSP_REG_IRQENABLE	0xa4
#define OMAP_MCBSP_REG_WAKEUPEN		0xa8
#define OMAP_MCBSP_REG_XCCR		0xac
#define OMAP_MCBSP_REG_RCCR		0xb0
#define OMAP_MCBSP_REG_XBUFFSTAT	0xb4
#define OMAP_MCBSP_REG_RBUFFSTAT	0xb8

/************************** McBSP SPCR1 bit definitions ***********************/
#define RRST			(1 << 0)
#define RRDY			(1 << 1)
#define RFULL			(1 << 2)
#define RSYNC_ERR		(1 << 3)
#define RINTM(value)		(((value) & 0x3) << 4)	/* bits 4:5 */
#define ABIS			(1 << 6)
#define DXENA			(1 << 7)
#define CLKSTP(value)		(((value) & 0x3) << 11)	/* bits 11:12 */
#define RJUST(value)		(((value) & 0x3) << 13)	/* bits 13:14 */
#define ALB			(1 << 15)
#define DLB			(1 << 15)

/************************** McBSP SPCR2 bit definitions ***********************/
#define XRST			(1 << 0)
#define XRDY			(1 << 1)
#define XEMPTY			(1 << 2)
#define XSYNC_ERR		(1 << 3)
#define XINTM(value)		(((value) & 0x3) << 4)	/* bits 4:5 */
#define GRST			(1 << 6)
#define FRST			(1 << 7)
#define SOFT			(1 << 8)
#define FREE			(1 << 9)

/************************** McBSP PCR bit definitions *************************/
#define CLKRP			(1 << 0)
#define CLKXP			(1 << 1)
#define FSRP			(1 << 2)
#define FSXP			(1 << 3)
#define DR_STAT			(1 << 4)
#define DX_STAT			(1 << 5)
#define CLKS_STAT		(1 << 6)
#define SCLKME			(1 << 7)
#define CLKRM			(1 << 8)
#define CLKXM			(1 << 9)
#define FSRM			(1 << 10)
#define FSXM			(1 << 11)
#define RIOEN			(1 << 12)
#define XIOEN			(1 << 13)
#define IDLE_EN			(1 << 14)

/************************** McBSP RCR1 bit definitions ************************/
#define RWDLEN1(value)		(((value) & 0x7) << 5)	/* Bits 5:7 */
#define RFRLEN1(value)		(((value) & 0x7f) << 8)	/* Bits 8:14 */

/************************** McBSP XCR1 bit definitions ************************/
#define XWDLEN1(value)		(((value) & 0x7) << 5)	/* Bits 5:7 */
#define XFRLEN1(value)		(((value) & 0x7f) << 8)	/* Bits 8:14 */

/*************************** McBSP RCR2 bit definitions ***********************/
#define RDATDLY(value)		((value) & 0x3)		/* Bits 0:1 */
#define RFIG			(1 << 2)
#define RCOMPAND(value)		(((value) & 0x3) << 3)	/* Bits 3:4 */
#define RWDLEN2(value)		(((value) & 0x7) << 5)	/* Bits 5:7 */
#define RFRLEN2(value)		(((value) & 0x7f) << 8)	/* Bits 8:14 */
#define RPHASE			(1 << 15)

/*************************** McBSP XCR2 bit definitions ***********************/
#define XDATDLY(value)		((value) & 0x3)		/* Bits 0:1 */
#define XFIG			(1 << 2)
#define XCOMPAND(value)		(((value) & 0x3) << 3)	/* Bits 3:4 */
#define XWDLEN2(value)		(((value) & 0x7) << 5)	/* Bits 5:7 */
#define XFRLEN2(value)		(((value) & 0x7f) << 8)	/* Bits 8:14 */
#define XPHASE			(1 << 15)

/************************* McBSP SRGR1 bit definitions ************************/
#define CLKGDV(value)		((value) & 0x7f)		/* Bits 0:7 */
#define FWID(value)		(((value) & 0xff) << 8)	/* Bits 8:15 */

/************************* McBSP SRGR2 bit definitions ************************/
#define FPER(value)		((value) & 0x0fff)	/* Bits 0:11 */
#define FSGM			(1 << 12)
#define CLKSM			(1 << 13)
#define CLKSP			(1 << 14)
#define GSYNC			(1 << 15)

/************************* McBSP MCR1 bit definitions *************************/
#define RMCM			(1 << 0)
#define RCBLK(value)		(((value) & 0x7) << 2)	/* Bits 2:4 */
#define RPABLK(value)		(((value) & 0x3) << 5)	/* Bits 5:6 */
#define RPBBLK(value)		(((value) & 0x3) << 7)	/* Bits 7:8 */

/************************* McBSP MCR2 bit definitions *************************/
#define XMCM(value)		((value) & 0x3)		/* Bits 0:1 */
#define XCBLK(value)		(((value) & 0x7) << 2)	/* Bits 2:4 */
#define XPABLK(value)		(((value) & 0x3) << 5)	/* Bits 5:6 */
#define XPBBLK(value)		(((value) & 0x3) << 7)	/* Bits 7:8 */

/*********************** McBSP XCCR bit definitions *************************/
#define XDISABLE		(1 << 0)
#define XDMAEN			(1 << 3)
#define DILB			(1 << 5)
#define XFULL_CYCLE		(1 << 11)
#define DXENDLY(value)		(((value) & 0x3) << 12)	/* Bits 12:13 */
#define PPCONNECT		(1 << 14)
#define EXTCLKGATE		(1 << 15)

/********************** McBSP RCCR bit definitions *************************/
#define RDISABLE		(1 << 0)
#define RDMAEN			(1 << 3)
#define RFULL_CYCLE		(1 << 11)

/********************** McBSP SYSCONFIG bit definitions ********************/
#define SOFTRST			(1 << 1)
#define ENAWAKEUP		(1 << 2)
#define SIDLEMODE(value)	(((value) & 0x3) << 3)
#define CLOCKACTIVITY(value)	(((value) & 0x3) << 8)

/********************** McBSP SSELCR bit definitions ***********************/
#define SIDETONEEN		(1 << 10)

/********************** McBSP Sidetone SYSCONFIG bit definitions ***********/
#define ST_AUTOIDLE		(1 << 0)

/********************** McBSP Sidetone SGAINCR bit definitions *************/
#define ST_CH0GAIN(value)	((value) & 0xffff)	/* Bits 0:15 */
#define ST_CH1GAIN(value)	(((value) & 0xffff) << 16) /* Bits 16:31 */

/********************** McBSP Sidetone SFIRCR bit definitions **************/
#define ST_FIRCOEFF(value)	((value) & 0xffff)	/* Bits 0:15 */

/********************** McBSP Sidetone SSELCR bit definitions **************/
#define ST_SIDETONEEN		(1 << 0)
#define ST_COEFFWREN		(1 << 1)
#define ST_COEFFWRDONE		(1 << 2)

/********************** McBSP DMA operating modes **************************/
#define MCBSP_DMA_MODE_ELEMENT		0
#define MCBSP_DMA_MODE_THRESHOLD	1

/********************** McBSP WAKEUPEN/IRQST/IRQEN bit definitions *********/
#define RSYNCERREN		(1 << 0)
#define RFSREN			(1 << 1)
#define REOFEN			(1 << 2)
#define RRDYEN			(1 << 3)
#define RUNDFLEN		(1 << 4)
#define ROVFLEN			(1 << 5)
#define XSYNCERREN		(1 << 7)
#define XFSXEN			(1 << 8)
#define XEOFEN			(1 << 9)
#define XRDYEN			(1 << 10)
#define XUNDFLEN		(1 << 11)
#define XOVFLEN			(1 << 12)
#define XEMPTYEOFEN		(1 << 14)

/* Clock signal muxing options */
#define CLKR_SRC_CLKR		0 /* CLKR signal is from the CLKR pin */
#define CLKR_SRC_CLKX		1 /* CLKR signal is from the CLKX pin */
#define FSR_SRC_FSR		2 /* FSR signal is from the FSR pin */
#define FSR_SRC_FSX		3 /* FSR signal is from the FSX pin */

/* McBSP functional clock sources */
#define MCBSP_CLKS_PRCM_SRC	0
#define MCBSP_CLKS_PAD_SRC	1

struct omap_mcbsp_reg_cfg {
	u16 spcr2;
	u16 spcr1;
	u16 rcr2;
	u16 rcr1;
	u16 xcr2;
	u16 xcr1;
	u16 srgr2;
	u16 srgr1;
	u16 mcr2;
	u16 mcr1;
	u16 pcr0;
	u16 rcerc;
	u16 rcerd;
	u16 xcerc;
	u16 xcerd;
	u16 rcere;
	u16 rcerf;
	u16 xcere;
	u16 xcerf;
	u16 rcerg;
	u16 rcerh;
	u16 xcerg;
	u16 xcerh;
	u16 xccr;
	u16 rccr;
};

struct omap_mcbsp {
	u8 id;
	u32 io_base;
	u8 reg_size;
	u8 reg_step;
};

void omap_mcbsp_write(struct omap_mcbsp *mcbsp, u16 reg, u32 val);
int omap_mcbsp_read(struct omap_mcbsp *mcbsp, u16 reg);
struct omap_mcbsp * omap_mcbsp_get_instance(int indx);
void omap_mcbsp_dump_reg(struct omap_mcbsp *mcbsp);

#define MCBSP_READ(mcbsp, reg) \
		omap_mcbsp_read(mcbsp, OMAP_MCBSP_REG_##reg)
#define MCBSP_WRITE(mcbsp, reg, val) \
		omap_mcbsp_write(mcbsp, OMAP_MCBSP_REG_##reg, val)

#endif
