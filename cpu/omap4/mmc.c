/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 * Syed Mohammed Khasim <khasim@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
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
#include <config.h>
#include <common.h>
#include <mmc.h>
#include <part.h>
#include <i2c.h>

#ifdef CONFIG_MMC
#include "mmc_host_def.h"
#include "mmc_protocol.h"

#define OMAP_MMC_MASTER_CLOCK   96000000
extern int fat_register_device(block_dev_desc_t *dev_desc, int part_no);
static unsigned int mmc_stat_last;
int wait_card_state_tran( mmc_card_data *mmc_card_cur, mmc_controller_data *mmc_cont_cur);

mmc_card_data cur_card_data[2];
mmc_controller_data cur_controller_data[2];

static block_dev_desc_t mmc_blk_dev[2];

block_dev_desc_t *mmc_get_dev(int dev)
{
	if ((dev == 0) || (dev == 1))
		return (block_dev_desc_t *) &mmc_blk_dev[dev];
	else
		return NULL;
}

unsigned char mmc_board_init(mmc_controller_data *mmc_cont_cur)
{
	unsigned char ret = 1;
	unsigned int value = 0;
	unsigned char data = 0;

	switch (mmc_cont_cur->slot) {
	case 0:
		/* Phoenix LDO config */
		data = 0x01;
		i2c_write(0x48, 0x98, 1, &data, 1);
		data = 0x03;
		i2c_write(0x48, 0x99, 1, &data, 1);
		data = 0x21;
		i2c_write(0x48, 0x9A, 1, &data, 1);
		data = 0x15;
		i2c_write(0x48, 0x9B, 1, &data, 1);

		/* SLOT-0 PBIAS config */
		value = CONTROL_PBIAS_LITE;
		CONTROL_PBIAS_LITE = value | (1 << 22) | (1 << 26);
		value = CONTROL_CONF_MMC1;
		CONTROL_CONF_MMC1  = value | (1 << 31) | (1 << 30) |
					(1 << 27) | (1 << 26) | (1 << 25);
		break;
	case 1:
		break;
	default:
		return ret;
	}
	return ret;
}

void mmc_init_stream(mmc_controller_data *mmc_cont_cur)
{
	OMAP_HSMMC_CON(mmc_cont_cur->base) |= INIT_INITSTREAM;

	OMAP_HSMMC_CMD(mmc_cont_cur->base) = MMC_CMD0;
	while (1) {
		if ((OMAP_HSMMC_STAT(mmc_cont_cur->base) & CC_MASK))
			break;
	}
	OMAP_HSMMC_STAT(mmc_cont_cur->base) = CC_MASK;

	OMAP_HSMMC_CMD(mmc_cont_cur->base) = MMC_CMD0;
	while (1) {
		if ((OMAP_HSMMC_STAT(mmc_cont_cur->base) & CC_MASK))
			break;
	}

	OMAP_HSMMC_STAT(mmc_cont_cur->base) =
				OMAP_HSMMC_STAT(mmc_cont_cur->base);
	OMAP_HSMMC_CON(mmc_cont_cur->base) &= ~INIT_INITSTREAM;
}

unsigned char mmc_clock_config(mmc_controller_data *mmc_cont_cur,
			unsigned int iclk, unsigned short clk_div)
{
	unsigned int val;

	mmc_reg_out(OMAP_HSMMC_SYSCTL(mmc_cont_cur->base),
			(ICE_MASK | DTO_MASK | CEN_MASK),
			(ICE_STOP | DTO_15THDTO | CEN_DISABLE));

	switch (iclk) {
	case CLK_INITSEQ:
		val = MMC_INIT_SEQ_CLK / 2;
		break;
	case CLK_400KHZ:
		val = MMC_400kHz_CLK;
		break;
	case CLK_MISC:
		val = clk_div;
		break;
	default:
		return 0;
	}
	mmc_reg_out(OMAP_HSMMC_SYSCTL(mmc_cont_cur->base),
	    ICE_MASK | CLKD_MASK, (val << CLKD_OFFSET) | ICE_OSCILLATE);

	while (1) {
		if ((OMAP_HSMMC_SYSCTL(mmc_cont_cur->base) & ICS_MASK)
							!= ICS_NOTREADY)
			break;
	}

	OMAP_HSMMC_SYSCTL(mmc_cont_cur->base) |= CEN_ENABLE;
	return 1;
}

unsigned char mmc_init_setup(mmc_controller_data *mmc_cont_cur)
{
	unsigned int reg_val;

	mmc_board_init(mmc_cont_cur);

	OMAP_HSMMC_SYSCONFIG(mmc_cont_cur->base) |= MMC_SOFTRESET;
	while (1) {
		if ((OMAP_HSMMC_SYSSTATUS(mmc_cont_cur->base)
						& RESETDONE) != 0)
		break;
	}

	OMAP_HSMMC_SYSCTL(mmc_cont_cur->base) |= SOFTRESETALL;
	while (1) {
		if ((OMAP_HSMMC_SYSCTL(mmc_cont_cur->base)
					& SOFTRESETALL) == 0)
		break;
	}
	OMAP_HSMMC_HCTL(mmc_cont_cur->base) =
					DTW_1_BITMODE | SDBP_PWROFF | SDVS_3V0;
	OMAP_HSMMC_CAPA(mmc_cont_cur->base) |= VS30_3V0SUP | VS18_1V8SUP;

	reg_val = OMAP_HSMMC_CON(mmc_cont_cur->base) & RESERVED_MASK;

	OMAP_HSMMC_CON(mmc_cont_cur->base) = CTPL_MMC_SD | reg_val |
		WPP_ACTIVEHIGH | CDP_ACTIVEHIGH | MIT_CTO | DW8_1_4BITMODE |
		MODE_FUNC | STR_BLOCK | HR_NOHOSTRESP | INIT_NOINIT
		| NOOPENDRAIN;

	mmc_clock_config(mmc_cont_cur, CLK_INITSEQ, 0);
	OMAP_HSMMC_HCTL(mmc_cont_cur->base) |= SDBP_PWRON;

	OMAP_HSMMC_IE(mmc_cont_cur->base) = 0x307f0033;

	mmc_init_stream(mmc_cont_cur);
	return 1;
}

struct mmc_cmd {
	ushort cmdidx;
	uint resp_type;
	uint cmdarg;
	uint response[4];
	uint flags;
};

struct mmc_data {
	union {
		char *dest;
		const char *src; /* src buffers don't get written to */
	};
	uint flags;
	uint blocks;
	uint blocksize;
};

typedef struct hsmmc {
	volatile unsigned char res1[0x10];
	volatile unsigned int sysconfig; /* 0x10 */
	volatile unsigned int sysstatus; /* 0x14 */
	volatile unsigned char res2[0x14];
	volatile unsigned int con; /* 0x2C */
	volatile unsigned char res3[0xD4];
	volatile unsigned int blk; /* 0x104 */
	volatile unsigned int arg; /* 0x108 */
	volatile unsigned int cmd; /* 0x10C */
	volatile unsigned int rsp10; /* 0x110 */
	volatile unsigned int rsp32; /* 0x114 */
	volatile unsigned int rsp54; /* 0x118 */
	volatile unsigned int rsp76; /* 0x11C */
	volatile unsigned int data; /* 0x120 */
	volatile unsigned int pstate; /* 0x124 */
	volatile unsigned int hctl; /* 0x128 */
	volatile unsigned int sysctl; /* 0x12C */
	volatile unsigned int stat; /* 0x130 */
	volatile unsigned int ie; /* 0x134 */
	volatile unsigned char res4[0x8];
	volatile unsigned int capa; /* 0x140 */
	// OMAP4_PANDA_ST
	volatile unsigned char res5[0x4];
	volatile unsigned int cur_capa; /* 0x148 */
	volatile unsigned char res6[0x4];
	volatile unsigned int fe; /* 0x150 */
	volatile unsigned int admaes; /* 0x154 */
	volatile unsigned int admasal; /* 0x158 */
// OMAP4_PANDA_ED
} hsmmc_t;

struct adma_table_type
{
	u64	valid:1,
		end:1,
		irq:1,
		zero:1,
		act1:1,
		act2:1,
		rsvd:10,
		length:16,
		address:32;
} adma_table[MAX_ADMA_TABLE];

static void mmc_print_erri(hsmmc_t *mmc_base, unsigned int stat) {
	if (stat & (1 << 29))
		printf("buffer bad access\n");
	if (stat & (1 << 28))
		printf("card error\n");
	if (stat & (1 << 25))
		printf("adma error (ADMAES: 0x%08x)\n", mmc_base->admaes);
	if (stat & (1 << 24))
		printf("auto cmd12 error\n");
	if (stat & (1 << 22))
		printf("data end bit error\n");
	if (stat & (1 << 21))
		printf("data crc error\n");
	if (stat & (1 << 20))
		printf("data timeout error\n");
	if (stat & (1 << 19))
		printf("command index error\n");
	if (stat & (1 << 18))
		printf("command end bit error\n");
	if (stat & (1 << 17))
		printf("command crc error\n");
	if (stat & (1 << 16))
		printf("command timeout error\n");
}

#define TIMEOUT			-19
#define MAX_RETRY_MS	1000
#define SD_CMD_APP_SEND_SCR		51
#define	MMC_SUCCESS			(0x00000000)

#define MSBS_SGLEBLK                   (0x0 << 5)
#define MSBS_MULTIBLK                  (0x1 << 5)

#define BCE_DISABLE                    (0x0 << 1)
#define BCE_ENABLE                     (0x1 << 1)

#define DMA_MNS                        (0x1 << 20)
#define DE_ENABLE                      (0x1 << 0)
#define DMA_MASK                       (0x1 << 3)      // adma transfer complete
#define AMDAE_MASK                     (0x1 << 25)     // adma error

static int mmc_wait_busy(unsigned int base) {
	hsmmc_t *mmc_base = (hsmmc_t *) base;
	unsigned int mmc_stat;
	do {
		mmc_stat = mmc_base->stat;
		if ((mmc_stat & ERRI_MASK) != 0) {
			mmc_print_erri(mmc_base, mmc_stat);
			return (1);
		}
//		LED_D2_toggle();
	} while ((mmc_base->pstate & (1 << 1)) != 0);
	return (0);
}

static int mmc_read_data_ex(hsmmc_t *mmc_base, char *buf, unsigned int size)
{
       unsigned int *output_buf = (unsigned int *)buf;
       unsigned int mmc_stat;
       unsigned int count;

       /*
        * Start Polled Read
        */
       count = (size > MMCSD_SECTOR_SIZE) ? MMCSD_SECTOR_SIZE : size;
       count /= 4;

       while (size) {
               ulong start = 0;
               do {
                       mmc_stat = mmc_base->stat;
                       udelay(1000);
                       start++;
                       if (start > MAX_RETRY_MS) {
                               printf("%s: timedout waiting for status!\n",
                                               __func__);
                               return TIMEOUT;
                       }
               } while (mmc_stat == 0);

               if ((mmc_stat & ERRI_MASK) != 0)
               {
                       mmc_print_erri(mmc_base, mmc_stat);
                       return 1;
               }

               if (mmc_stat & BRR_MASK) {
                       unsigned int k;

                       mmc_base->stat |= BRR_MASK;
                       for (k = 0; k < count; k++) {
                               *output_buf = mmc_base->data;
                               output_buf++;
                       }
                       size -= (count*4);
               }

               if (mmc_stat & BWR_MASK)
                       mmc_base->stat |= BWR_MASK;

               if (mmc_stat & TC_MASK) {
                       mmc_base->stat |= TC_MASK;
                       break;
               }
       }
       return 0;
}

static int mmc_write_data_ex(hsmmc_t *mmc_base, const char *buf, unsigned int size)
{
       unsigned int *input_buf = (unsigned int *)buf;
       unsigned int mmc_stat;
       unsigned int count;

       /*
        * Start Polled Read
        */
       count = (size > MMCSD_SECTOR_SIZE) ? MMCSD_SECTOR_SIZE : size;
       count /= 4;

       while (size) {
               ulong start = 0;
               do {
                       mmc_stat = mmc_base->stat;
                       udelay(1000);
                       start++;
                       if (start > MAX_RETRY_MS) {
                               printf("%s: timedout waiting for status!\n",
                                               __func__);
                               return TIMEOUT;
                       }
               } while (mmc_stat == 0);

               if ((mmc_stat & ERRI_MASK) != 0)
               {
                       mmc_print_erri(mmc_base, mmc_stat);
                       return 1;
               }

               if (mmc_stat & BWR_MASK) {
                       unsigned int k;

                       mmc_base->stat |= BWR_MASK;
                       for (k = 0; k < count; k++) {
                               mmc_base->data = *input_buf;
                               input_buf++;
                       }
                       size -= (count*4);
               }

               if (mmc_stat & BRR_MASK)
                       mmc_base->stat |= BRR_MASK;

               if (mmc_stat & TC_MASK) {
                       mmc_base->stat |= TC_MASK;
                       break;
               }
       }
       return 0;
}

static int mmc_send_cmd_ex(unsigned int base,
		struct mmc_cmd *cmd, struct mmc_data *data)
{
	hsmmc_t *mmc_base = (hsmmc_t *) base;
	unsigned int flags, mmc_stat;
	ulong start;

	if ((cmd->cmdidx == 1) || (cmd->cmdidx == 2) || (cmd->cmdidx == 3)
			|| (cmd->cmdidx == 40))
		mmc_base->con &= ~0x1;
	else
		mmc_base->con |= 0x1; // push pull

	if (cmd->cmdidx == 12) {
		mmc_base->sysctl |= 1 << 26;
		start = 0;
		while ((mmc_base->sysctl & (1 << 26)) != 0) {
			udelay(1000);
			start++;
			if (start > MAX_RETRY_MS) {
				printf("%s: timedout waiting for softresetall!\n", __func__);
				return TIMEOUT;
			}
		}
	}

	mmc_base->stat = 0xFFFFFFFF;
	start = 0;
	while (mmc_base->stat) {
		udelay(1000);
		start++;
		if (start > MAX_RETRY_MS) {
			printf("%s: timedout waiting for stat!\n", __func__);
			return TIMEOUT;
		}
	}
	/*
	 * CMDREG
	 * CMDIDX[13:8]	: Command index
	 * DATAPRNT[5]	: Data Present Select
	 * ENCMDIDX[4]	: Command Index Check Enable
	 * ENCMDCRC[3]	: Command CRC Check Enable
	 * RSPTYP[1:0]
	 *	00 = No Response
	 *	01 = Length 136
	 *	10 = Length 48
	 *	11 = Length 48 Check busy after response
	 */
	/* Delay added before checking the status of frq change
	 * retry not supported by mmc.c(core file)
	 */
	if (cmd->cmdidx == SD_CMD_APP_SEND_SCR)
		udelay(50000); /* wait 50 ms */

	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags = 0;
	else if (cmd->resp_type & MMC_RSP_136)
		flags = RSP_TYPE_LGHT136 | CICE_NOCHECK;
	else if (cmd->resp_type & MMC_RSP_BUSY)
		flags = RSP_TYPE_LGHT48B;
	else
		flags = RSP_TYPE_LGHT48;

	/* enable default flags */
	flags = flags | (CMD_TYPE_NORMAL | CICE_NOCHECK | CCCE_NOCHECK
			| MSBS_SGLEBLK | ACEN_DISABLE | BCE_DISABLE | DE_DISABLE);

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= CCCE_CHECK;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= CICE_CHECK;

	if (data) {
		if ((cmd->cmdidx == 18) || (cmd->cmdidx
				== 25) || (cmd->cmdidx
				== 0)) { // read boot data
			flags |= (MSBS_MULTIBLK | BCE_ENABLE);
			mmc_base->blk = data->blocksize | (data->blocks << 16);
		} else
			mmc_base->blk = data->blocksize | NBLK_STPCNT;

		if (data->flags & MMC_DATA_READ)
			flags |= (DP_DATA | DDIR_READ);
		else
			flags |= (DP_DATA | DDIR_WRITE);
		if (data->flags & MMC_DATA_ADMA) {
			int i = 0;
			unsigned int addr = (unsigned int) data->dest;
			unsigned int size = data->blocksize * data->blocks;
			unsigned int length;
			mmc_base->con |= DMA_MNS; // adma select
			do {
				length = (size > MAX_ADMA_LENGTH) ? MAX_ADMA_LENGTH : size;
				adma_table[i].address = addr;
				adma_table[i].length = length;
				adma_table[i].act2 = 1;
				adma_table[i].act1 = 0;
				adma_table[i].irq = (size == length) ? 1 : 0;
				adma_table[i].end = (size == length) ? 1 : 0;
				adma_table[i].valid = 1;
				addr += length;
				size -= length;
				i++;
			} while (size > 0);

			mmc_base->admasal= (unsigned int) adma_table;

			mmc_base->hctl |= (0x2 << 3); // 32bit address adma
			mmc_base->ie |= (0x1 << 25); // adma error interrupt enable
			mmc_base->ie |= (0x1 << 3); // adma interrupt enable
			flags |= (BCE_ENABLE | DE_ENABLE);
		} else {
			mmc_base->con &= ~DMA_MNS;
		}
	}
	mmc_base->arg = cmd->cmdarg;
	mmc_base->cmd = (cmd->cmdidx << 24) | flags;
	start = 0;
	do {
		mmc_stat = mmc_base->stat;
		if ((mmc_stat & ERRI_MASK) != 0) {
			printf("%s: ERR STAT=%08X\n", __func__, mmc_stat);
			mmc_print_erri(mmc_base, mmc_stat);
			return (mmc_stat & (1 << 16)) != 0 ? TIMEOUT : -1;
		}
		udelay(1000);
		start++;
		if (start > MAX_RETRY_MS) {
			printf("%s : timeout: command is not completed\n", __func__);
			printf("%s :          CMD%d 0x%08x\n", __func__, cmd->cmdidx,
					cmd->cmdarg);
			printf("%s :          RESP  0x%08x\n", __func__, mmc_base->rsp10);
			return TIMEOUT;
		}
	} while (!(mmc_stat & CC_MASK));

	if (mmc_stat & CC_MASK) {
		mmc_base->stat = CC_MASK;
		if (cmd->resp_type & MMC_RSP_PRESENT) {
			if (cmd->resp_type & MMC_RSP_136) {
				/* response type 2 */
				cmd->response[3] = mmc_base->rsp10;
				cmd->response[2] = mmc_base->rsp32;
				cmd->response[1] = mmc_base->rsp54;
				cmd->response[0] = mmc_base->rsp76;
			} else {
				/* response types 1, 1b, 3, 4, 5, 6 */
				cmd->response[0] = mmc_base->rsp10;
			}
		}
	}

	if (data && (data->flags & MMC_DATA_ADMA)) {
		return 0;
	}

	if (data && (data->flags & MMC_DATA_READ)) {
		mmc_read_data_ex(mmc_base, data->dest, data->blocksize * data->blocks);
	} else if (data && (data->flags & MMC_DATA_WRITE)) {
		mmc_write_data_ex(mmc_base, data->src, data->blocksize * data->blocks);
	}
	return 0;
}

static int omap_mmc_erase_ex(unsigned int base, unsigned int start, unsigned int end, unsigned int opt)
{
	int ret;
	struct mmc_cmd cmd;

	cmd.cmdidx = 35;
	cmd.cmdarg = start;
	cmd.resp_type = MMC_RSP_R1;
	cmd.flags = 0;
	cmd.response[0] = 0xffffffff;
	ret = mmc_send_cmd_ex(base, &cmd, NULL);
	if (ret)
		return ret;

	cmd.cmdidx = 36;
	cmd.cmdarg = end;
	cmd.resp_type = MMC_RSP_R1;
	cmd.flags = 0;
	cmd.response[0] = 0xffffffff;
	ret = mmc_send_cmd_ex(base, &cmd, NULL);
	if (ret)
		return ret;

	cmd.cmdidx = 38;
	cmd.cmdarg = opt;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.flags = 0;
	cmd.response[0] = 0xffffffff;
	ret = mmc_send_cmd_ex(base, &cmd, NULL);
	if (ret)
		return ret;

	ret = mmc_wait_busy(base);
	if (ret)
		return ret;

	printf("Erase done: start=%08X; end=%08X; opt=%08X\n", start, end, opt);

	return MMC_SUCCESS;
}

unsigned char mmc_send_cmd(unsigned int base, unsigned int cmd,
			unsigned int arg, unsigned int *response)
{
	unsigned int mmc_stat;
	unsigned int cmd_index = cmd >> 24;

	while (1) {
		if ((OMAP_HSMMC_PSTATE(base) & DATI_MASK) != DATI_CMDDIS)
		break;
	}

	OMAP_HSMMC_STAT(base) = 0xFFFFFFFF;
	OMAP_HSMMC_ARG(base) = arg;
	if (cmd_index == 0x19) { /* CMD25: Multi block write */
		OMAP_HSMMC_CMD(base) = cmd | CMD_TYPE_NORMAL | CICE_NOCHECK |
			CCCE_NOCHECK | MSBS | BCE | ACEN_DISABLE | DE_DISABLE;
	} else if (cmd_index == 0xC) {
		OMAP_HSMMC_CMD(base) = cmd | 0x3 << 22 | CICE_NOCHECK |
		CCCE_NOCHECK | ACEN_DISABLE | BCE_DISABLE |
			DE_DISABLE;
	} else {
		OMAP_HSMMC_BLK(base) = BLEN_512BYTESLEN | NBLK_STPCNT;
		OMAP_HSMMC_CMD(base) = cmd | CMD_TYPE_NORMAL | CICE_NOCHECK |
		CCCE_NOCHECK | MSBS_SGLEBLK | ACEN_DISABLE | BCE_DISABLE |
			DE_DISABLE;
	}

	while (1) {
		do {
			mmc_stat = OMAP_HSMMC_STAT(base);
		} while (mmc_stat == 0);

		if ((mmc_stat & ERRI_MASK) != 0)
			return (unsigned char)mmc_stat;


		if (mmc_stat & CC_MASK) {
			OMAP_HSMMC_STAT(base) = CC_MASK;
			response[0] = OMAP_HSMMC_RSP10(base);
			if ((cmd & RSP_TYPE_MASK) == RSP_TYPE_LGHT136) {
				response[1] = OMAP_HSMMC_RSP32(base);
				response[2] = OMAP_HSMMC_RSP54(base);
				response[3] = OMAP_HSMMC_RSP76(base);
			}
			break;
		}
	}
	return 1;
}

unsigned char mmc_read_data(unsigned int base, unsigned int *output_buf)
{
	unsigned int mmc_stat;
	unsigned int read_count = 0;

	/*
	 * Start Polled Read
	 */
	while (1) {
		do {
			mmc_stat = OMAP_HSMMC_STAT(base);
		} while (mmc_stat == 0);
		mmc_stat_last=mmc_stat;

		if ((mmc_stat & ERRI_MASK) != 0) {
			printf("mmc read data error  0x%x\n", mmc_stat);
			return (unsigned char)mmc_stat;
		}

		if (mmc_stat & BRR_MASK) {
			unsigned int k;

			OMAP_HSMMC_STAT(base) |= BRR_MASK;
			for (k = 0; k < MMCSD_SECTOR_SIZE / 4; k++) {
				*output_buf = OMAP_HSMMC_DATA(base);
				output_buf++;
				read_count += 4;
			}
		}

		if (mmc_stat & BWR_MASK)
			OMAP_HSMMC_STAT(base) |= BWR_MASK;

		if (mmc_stat & TC_MASK) {
			OMAP_HSMMC_STAT(base) |= TC_MASK;
			break;
		}
	}
	return 1;
}

int mmc_write_data(unsigned int base, unsigned int *input_buf)
{
	unsigned int mmc_stat;
	int count = 0;

	/*
	 * Start Polled Write
	 */
	while (1) {
		do {
			mmc_stat = OMAP_HSMMC_STAT(base);
		} while (mmc_stat == 0);

		if ((mmc_stat & ERRI_MASK) != 0) {
			printf("mmc write error %08x\n", mmc_stat);
			return -1;
		}

		if (mmc_stat & BWR_MASK) {
			unsigned int k;

			OMAP_HSMMC_STAT(base) |= BWR_MASK;
			for (k = 0; k < MMCSD_SECTOR_SIZE / 4; k++) {
				OMAP_HSMMC_DATA(base) = *input_buf;
				input_buf++;
			}
			count++;
		}

		if (mmc_stat & BRR_MASK)
			OMAP_HSMMC_STAT(base) |= BRR_MASK;

		if (mmc_stat & TC_MASK) {
			OMAP_HSMMC_STAT(base) |= TC_MASK;
			break;
		}
	}
	return count;
}

unsigned char mmc_detect_card(mmc_card_data *mmc_card_cur,
				mmc_controller_data *mmc_contr_cur)
{
	unsigned char err;
	unsigned int argument = 0;
	unsigned int ocr_value = 0;
	unsigned int ocr_recvd = 0;
	unsigned int ret_cmd41 = 0;
	unsigned int hcs_val = 0;
	unsigned int resp[4];
#ifdef CONFIG_MACH_OTTER2
	unsigned short retry_cnt = 6000;
#else
	unsigned short retry_cnt = 2000;
#endif
	/* Set to Initialization Clock */
	err = mmc_clock_config(mmc_contr_cur, CLK_400KHZ, 0);
	if (err != 1)
		return err;

	mmc_card_cur->RCA = MMC_RELATIVE_CARD_ADDRESS;
	argument = 0x00000000;

	switch (mmc_contr_cur->slot) {
	case 0:
		ocr_value = (0x1FF << 15);
		break;
	case 1:
		ocr_value = 0x80;
		break;
	default:
		printf("mmc_detect_card:Invalid Slot\n");
	}
	err = mmc_send_cmd(mmc_contr_cur->base, MMC_CMD0, argument, resp);
	if (err != 1)
		return err;

	argument = SD_CMD8_CHECK_PATTERN | SD_CMD8_2_7_3_6_V_RANGE;
	err = mmc_send_cmd(mmc_contr_cur->base, MMC_SDCMD8, argument, resp);
	hcs_val = (err == 1) ?
	    MMC_OCR_REG_HOST_CAPACITY_SUPPORT_SECTOR :
	    MMC_OCR_REG_HOST_CAPACITY_SUPPORT_BYTE;

	argument = 0x0000 << 16;
	err = mmc_send_cmd(mmc_contr_cur->base, MMC_CMD55, argument, resp);
	if (err == 1) {
		mmc_card_cur->card_type = SD_CARD;
		ocr_value |= hcs_val;
		ret_cmd41 = MMC_ACMD41;
	} else {
		mmc_card_cur->card_type = MMC_CARD;
		ocr_value |= MMC_OCR_REG_ACCESS_MODE_SECTOR;
		ret_cmd41 = MMC_CMD1;
		OMAP_HSMMC_CON(mmc_contr_cur->base) &= ~OD;
		OMAP_HSMMC_CON(mmc_contr_cur->base) |= OPENDRAIN;
	}

	argument = ocr_value;
	err = mmc_send_cmd(mmc_contr_cur->base, ret_cmd41, argument, resp);
	if (err != 1)
		return err;

	ocr_recvd = ((mmc_resp_r3 *) resp)->ocr;

	while (!(ocr_recvd & (0x1 << 31)) && (retry_cnt > 0)) {
		retry_cnt--;
		if (mmc_card_cur->card_type == SD_CARD) {
			argument = 0x0000 << 16;
			err = mmc_send_cmd(mmc_contr_cur->base, MMC_CMD55,
								argument, resp);
		}

		argument = ocr_value;
		err = mmc_send_cmd(mmc_contr_cur->base, ret_cmd41,
								argument, resp);
		if (err != 1)
			return err;
		ocr_recvd = ((mmc_resp_r3 *) resp)->ocr;
	}

	if (!(ocr_recvd & (0x1 << 31)))
		return 0;

	if (mmc_card_cur->card_type == MMC_CARD) {
		if ((ocr_recvd & MMC_OCR_REG_ACCESS_MODE_MASK) ==
		    MMC_OCR_REG_ACCESS_MODE_SECTOR) {
			mmc_card_cur->mode = SECTOR_MODE;
		} else {
			mmc_card_cur->mode = BYTE_MODE;
		}

		ocr_recvd &= ~MMC_OCR_REG_ACCESS_MODE_MASK;
	} else {
		if ((ocr_recvd & MMC_OCR_REG_HOST_CAPACITY_SUPPORT_MASK)
		    == MMC_OCR_REG_HOST_CAPACITY_SUPPORT_SECTOR) {
			mmc_card_cur->mode = SECTOR_MODE;
		} else {
			mmc_card_cur->mode = BYTE_MODE;
		}
		ocr_recvd &= ~MMC_OCR_REG_HOST_CAPACITY_SUPPORT_MASK;
	}

	ocr_recvd &= ~(0x1 << 31);
	if (!(ocr_recvd & ocr_value))
		return 0;

	err = mmc_send_cmd(mmc_contr_cur->base, MMC_CMD2, argument, resp);
	if (err != 1)
		return err;

	if (mmc_card_cur->card_type == MMC_CARD) {
		argument = mmc_card_cur->RCA << 16;
		err = mmc_send_cmd(mmc_contr_cur->base, MMC_CMD3,
							argument, resp);
		if (err != 1)
			return err;
	} else {
		argument = 0x00000000;
		err = mmc_send_cmd(mmc_contr_cur->base, MMC_SDCMD3,
							argument, resp);
		if (err != 1)
			return err;

		mmc_card_cur->RCA = ((mmc_resp_r6 *) resp)->newpublishedrca;
	}

	OMAP_HSMMC_CON(mmc_contr_cur->base) &= ~OD;
	OMAP_HSMMC_CON(mmc_contr_cur->base) |= NOOPENDRAIN;
	return 1;
}

unsigned char mmc_read_cardsize(unsigned int base, mmc_card_data *mmc_dev_data,
				mmc_csd_reg_t *cur_csd)
{
	mmc_extended_csd_reg_t ext_csd;
	unsigned int size, count, blk_len, blk_no, card_size, argument;
	unsigned char err;
	unsigned int resp[4];

	if (mmc_dev_data->mode == SECTOR_MODE) {
		if (mmc_dev_data->card_type == SD_CARD) {
			card_size =
			    (((mmc_sd2_csd_reg_t *) cur_csd)->
			     c_size_lsb & MMC_SD2_CSD_C_SIZE_LSB_MASK) |
			    ((((mmc_sd2_csd_reg_t *) cur_csd)->
			      c_size_msb & MMC_SD2_CSD_C_SIZE_MSB_MASK)
			     << MMC_SD2_CSD_C_SIZE_MSB_OFFSET);
			mmc_dev_data->size = (card_size + 1) * 1024;
			if (mmc_dev_data->size == 0)
				return 0;
		} else {
			argument = 0x00000000;
			err = mmc_send_cmd(base, MMC_CMD8, argument, resp);
			if (err != 1)
				return err;
			err = mmc_read_data(base, (unsigned int *)&ext_csd);
			if (err != 1)
				return err;
			mmc_dev_data->size = ext_csd.sectorcount;

			if (mmc_dev_data->size == 0)
				mmc_dev_data->size = 8388608;
		}
	} else {
		if (cur_csd->c_size_mult >= 8)
			return 0;

		if (cur_csd->read_bl_len >= 12)
			return 0;

		/* Compute size */
		count = 1 << (cur_csd->c_size_mult + 2);
		card_size = (cur_csd->c_size_lsb & MMC_CSD_C_SIZE_LSB_MASK) |
		    ((cur_csd->c_size_msb & MMC_CSD_C_SIZE_MSB_MASK)
		     << MMC_CSD_C_SIZE_MSB_OFFSET);
		blk_no = (card_size + 1) * count;
		blk_len = 1 << cur_csd->read_bl_len;
		size = blk_no * blk_len;
		mmc_dev_data->size = size / MMCSD_SECTOR_SIZE;
		if (mmc_dev_data->size == 0)
			return 0;
	}
	return 1;
}

unsigned char omap_mmc_read_sect(unsigned int start_sec, unsigned int num_bytes,
			mmc_controller_data *mmc_cont_cur,
			mmc_card_data *mmc_c, unsigned int *output_buf)
{
	unsigned char err;
	unsigned int argument;
	unsigned int resp[4];
	unsigned int num_sec_val =
	    (num_bytes + (MMCSD_SECTOR_SIZE - 1)) / MMCSD_SECTOR_SIZE;
	unsigned int sec_inc_val;

	if (num_sec_val == 0) {
		printf("mmc read: Invalid size\n");
		return 1;
	}
	if (mmc_c->mode == SECTOR_MODE) {
		argument = start_sec;
		sec_inc_val = 1;
	} else {
		argument = start_sec * MMCSD_SECTOR_SIZE;
		sec_inc_val = MMCSD_SECTOR_SIZE;
	}
	while (num_sec_val) {
		err = mmc_send_cmd(mmc_cont_cur->base, MMC_CMD17,
							argument, resp);
		if (err != 1) {
			printf("mmc read cmd sector 0x%x error  0x%x\n",argument, mmc_stat_last);
		    printf("OMAP_HSMMC_SYSCTL(mmc_cont_cur->base)= 0x%x\n",OMAP_HSMMC_SYSCTL(mmc_cont_cur->base));
			printf("OMAP_HSMMC_CAPA(mmc_cont_cur->base) = 0x%x\n", OMAP_HSMMC_CAPA(mmc_cont_cur->base));

			return err;
		}


		err = mmc_read_data(mmc_cont_cur->base, output_buf);
		if (err != 1) {
			printf("mmc read data  sector 0x%x error  0x%x\n",argument, mmc_stat_last);
		    printf("OMAP_HSMMC_SYSCTL(mmc_cont_cur->base)= 0x%x\n",OMAP_HSMMC_SYSCTL(mmc_cont_cur->base));
			printf("OMAP_HSMMC_CAPA(mmc_cont_cur->base) = 0x%x\n", OMAP_HSMMC_CAPA(mmc_cont_cur->base));
			return err;
		}

		output_buf += (MMCSD_SECTOR_SIZE / 4);
		argument += sec_inc_val;
		num_sec_val--;
	}
	return 1;
}

unsigned char omap_mmc_write_sect(unsigned int *input_buf,
		unsigned int num_bytes,
		mmc_controller_data *mmc_cont_cur,
		mmc_card_data *mmc_c, unsigned long start_sec)
{
	unsigned char err;
	unsigned int argument;
	unsigned int resp[4];
	unsigned int num_sec_val =
		(num_bytes + (MMCSD_SECTOR_SIZE - 1)) / MMCSD_SECTOR_SIZE;
	unsigned int sec_inc_val;
	unsigned int blk_cnt_current_tns;
	int r;

	if (num_sec_val == 0) {
		printf("mmc write: Invalid size\n");
		return 1;
	}

	if (mmc_c->mode == SECTOR_MODE) {
		argument = start_sec;
		sec_inc_val = 1;
	} else {
		argument = start_sec * MMCSD_SECTOR_SIZE;
		sec_inc_val = MMCSD_SECTOR_SIZE;
	}

	while (num_sec_val) {
		if (num_sec_val > 0xFFFF)
			/* Max number of blocks per cmd */
			blk_cnt_current_tns = 0xFFFF;
		else
			blk_cnt_current_tns = num_sec_val;

		/* check for Multi Block */
		if (blk_cnt_current_tns > 1) {
#if !defined(CONFIG_4430PANDA)
			err = mmc_send_cmd(mmc_cont_cur->base, MMC_CMD23,
					blk_cnt_current_tns, resp);
			if (err != 1)
				return err;
#endif
			OMAP_HSMMC_BLK(mmc_cont_cur->base) = BLEN_512BYTESLEN |
						(blk_cnt_current_tns << 16);

			err = mmc_send_cmd(mmc_cont_cur->base,
						MMC_CMD25, argument, resp);
			if (err != 1)
				return err;
		} else {
			err = mmc_send_cmd(mmc_cont_cur->base, MMC_CMD24,
								argument, resp);
			if (err != 1)
				return err;
		}
		r = mmc_write_data(mmc_cont_cur->base, input_buf);
		if (r < 0)
			return 1;
		blk_cnt_current_tns = r;

#if defined(CONFIG_4430PANDA)
		if (blk_cnt_current_tns > 1) {
			err = mmc_send_cmd(mmc_cont_cur->base,
						MMC_CMD12, 0, resp);

			if (err != 1) {
				printf("MMC_CMD12 failed 0x%x\n", err);
				return err;
			}
		}
#endif

		input_buf += (MMCSD_SECTOR_SIZE / 4) * blk_cnt_current_tns;
		argument += sec_inc_val * blk_cnt_current_tns;
		num_sec_val -= blk_cnt_current_tns;
	}
	return 1;
}

unsigned char omap_mmc_erase_sect(unsigned int start,
	mmc_controller_data *mmc_cont_cur, mmc_card_data *mmc_c, int size)
{
	unsigned char err;
	unsigned int argument;
	unsigned int num_sec_val;
	unsigned int sec_inc_val;
	unsigned int resp[4];
	unsigned int mmc_stat;
	unsigned int blk_cnt_current_tns;

	if ((start / MMCSD_SECTOR_SIZE) > mmc_c->size ||
			((start + size) / MMCSD_SECTOR_SIZE) > mmc_c->size) {
		printf("mmc erase: erase to Sector is\n"
			"out of card range\n");
		return 1;
	}
	num_sec_val = (size + (MMCSD_SECTOR_SIZE - 1)) / MMCSD_SECTOR_SIZE;
	if (mmc_c->mode == SECTOR_MODE) {
		argument = start;
		sec_inc_val = 1;
	} else {
		argument = start * MMCSD_SECTOR_SIZE;
		sec_inc_val = MMCSD_SECTOR_SIZE;
	}
	while (num_sec_val) {
		if (num_sec_val > 0xFFFF)
			blk_cnt_current_tns = 0xFFFF;
		else
			blk_cnt_current_tns = num_sec_val;

		/* check for Multi Block */
		if (blk_cnt_current_tns > 1) {

			OMAP_HSMMC_BLK(mmc_cont_cur->base) = BLEN_512BYTESLEN |
						(blk_cnt_current_tns << 16);

			err = mmc_send_cmd(mmc_cont_cur->base,
					MMC_CMD25, argument, resp);
			if (err != 1)
				return err;

		} else {
			err = mmc_send_cmd(mmc_cont_cur->base, MMC_CMD24,
								argument, resp);
			if (err != 1)
				return err;
		}
		while (1) {
			do {
				mmc_stat = OMAP_HSMMC_STAT(mmc_cont_cur->base);
			} while (mmc_stat == 0);

			if ((mmc_stat & ERRI_MASK) != 0)
				return (unsigned char)mmc_stat;

			if (mmc_stat & BWR_MASK) {
				unsigned int k;

				OMAP_HSMMC_STAT(mmc_cont_cur->base) |= BWR_MASK;
				for (k = 0; k < MMCSD_SECTOR_SIZE / 4; k++) {
					OMAP_HSMMC_DATA(mmc_cont_cur->base) =
								0XFFFFFFFF;
				}
			}

			if (mmc_stat & BRR_MASK)
				OMAP_HSMMC_STAT(mmc_cont_cur->base) |= BRR_MASK;

			if (mmc_stat & TC_MASK) {
				OMAP_HSMMC_STAT(mmc_cont_cur->base) |= TC_MASK;
				break;
			}
		}

		if (blk_cnt_current_tns > 1) {
			err = mmc_send_cmd(mmc_cont_cur->base,
						MMC_CMD12, 0, resp);

			if (err != 1) {
				printf("MMC_CMD12 failed 0x%x\n", err);
				return err;
			}
		}

		argument += sec_inc_val * blk_cnt_current_tns;
		num_sec_val -= blk_cnt_current_tns;
	}
	return 1;
}

unsigned char omap_mmc_health_status(char cmd_index, int tbl_cnt,
                        mmc_controller_data *mmc_cont_cur,
                        mmc_card_data *mmc_c, unsigned int *output_buf)
{
        unsigned char err;
        unsigned int argument = 0;
        unsigned int resp[4];
	int tbl_index;

	argument = READ_MODE | (cmd_index << 1) | (tbl_cnt << 8);

	printf("mmc_health_status:argument: 0x%x\n", argument);

	err = mmc_send_cmd(mmc_cont_cur->base, MMC_CMD56,
				argument, resp);
        if (err != 1)
	        return err;

        err = mmc_read_data(mmc_cont_cur->base, output_buf);
        if (err != 1)
		return err;

        return 1;
}

unsigned char configure_controller(mmc_controller_data *cur_controller_data,
								int slot)
{
	int ret = 0;

	cur_controller_data->slot = slot;
	switch (slot) {
	case 0:
		cur_controller_data->base = OMAP_HSMMC1_BASE;
		break;
	case 1:
		cur_controller_data->base = OMAP_HSMMC2_BASE;
		break;
	default:
		printf("MMC on SLOT=%d not Supported\n", slot);
		ret = 1;
	}
	return ret;
}

#ifndef CONFIG_MACH_OTTER2
static int mmc_send_status( mmc_card_data *mmc_card_cur, mmc_controller_data *mmc_cont_cur, unsigned int *mstatus)
{
	unsigned char ret_val;
	unsigned int argument;
	unsigned int resp[4];

	argument = mmc_card_cur->RCA << 16;
	ret_val = mmc_send_cmd(mmc_cont_cur->base, MMC_CMD13, argument, resp);

	if (ret_val != 1) {
		printf("error in sending cmd13\n");
		if (mstatus!=NULL) *mstatus=mmc_stat_last;
		return ret_val;
	}

	if (mstatus) *mstatus=resp[0];
	return 1;
}


int mmc_get_status(int mmc_cont, unsigned int *mstatus)
{
	int ret_val;
	unsigned int status;

	if (mmc_blk_dev[mmc_cont].dev == -1) {
		printf("Read not permitted as Card on SLOT-%d"
			" not Initialized\n", mmc_cont);
	} else {
		ret_val=mmc_send_status( &cur_card_data[mmc_cont],  &cur_controller_data[mmc_cont], &status);

		if (ret_val != 1) {
			printf("error in sending cmd13 STATUS=%x\n",status);
			if (mstatus) *mstatus=status;
			return 0;
		}
	}
	if (mstatus) *mstatus=status;
	return ret_val;
}

static int wait_card_state_prg( mmc_card_data *mmc_card_cur, mmc_controller_data *mmc_cont_cur)
{
#define RETRY_WAIT_CARD_PRG_COUNT 100
	int count = 0;
	unsigned int status;
	int err;

	for (count=0;count<RETRY_WAIT_CARD_PRG_COUNT;count++)
	{
		err= mmc_send_status(mmc_card_cur, mmc_cont_cur, &status);
		if (err!=1) {
			printf("Error getting card status\n");
			return err;
		}
		if  (MMC_R1_CURRENT_STATE(status) != MMC_R1_STATE_PRG) break;
		printf("wait for mmc card to exit prg state\n");
		udelay(1000);
	}
	if (count>=RETRY_WAIT_CARD_PRG_COUNT) {
		printf("mmc wait prg state timeout\n");
		return 0;
	}

	return 1;
}
#endif

unsigned char configure_mmc(mmc_card_data *mmc_card_cur,
				mmc_controller_data *mmc_cont_cur)
{
	unsigned char ret_val;
	unsigned int argument;
	unsigned int resp[4];
	unsigned int trans_fact, trans_unit, retries = 2;
	unsigned int max_dtr;
	int dsor;
	mmc_csd_reg_t Card_CSD;
	unsigned char trans_speed;

	ret_val = mmc_init_setup(mmc_cont_cur);
	if (ret_val != 1)
		return ret_val;


	do {
		ret_val = mmc_detect_card(mmc_card_cur, mmc_cont_cur);
		retries--;
	} while ((retries > 0) && (ret_val != 1));

	argument = mmc_card_cur->RCA << 16;
	ret_val = mmc_send_cmd(mmc_cont_cur->base, MMC_CMD9, argument, resp);
	if (ret_val != 1) {
		printf("failed cmd9\n");
		return ret_val;
	}

	((unsigned int *)&Card_CSD)[3] = resp[3];
	((unsigned int *)&Card_CSD)[2] = resp[2];
	((unsigned int *)&Card_CSD)[1] = resp[1];
	((unsigned int *)&Card_CSD)[0] = resp[0];

	if (mmc_card_cur->card_type == MMC_CARD)
		mmc_card_cur->version = Card_CSD.spec_vers;

	trans_speed = Card_CSD.tran_speed;
	ret_val = mmc_send_cmd(mmc_cont_cur->base, MMC_CMD4,
					MMC_DSR_DEFAULT << 16, resp);
	if (ret_val != 1) {
		printf("failed get transfer speed\n");
		return ret_val;
	}

	trans_unit = trans_speed & MMC_CSD_TRAN_SPEED_UNIT_MASK;
	trans_fact = trans_speed & MMC_CSD_TRAN_SPEED_FACTOR_MASK;

	if (trans_unit > MMC_CSD_TRAN_SPEED_UNIT_100MHZ)
		return 0;

	if ((trans_fact < MMC_CSD_TRAN_SPEED_FACTOR_1_0) ||
	    (trans_fact > MMC_CSD_TRAN_SPEED_FACTOR_8_0))
		return 0;

	trans_unit >>= 0;
	trans_fact >>= 3;

	max_dtr = tran_exp[trans_unit] * tran_mant[trans_fact];
	dsor = OMAP_MMC_MASTER_CLOCK / max_dtr;

	if (OMAP_MMC_MASTER_CLOCK / dsor > max_dtr)
                        dsor++;

	ret_val = mmc_clock_config(mmc_cont_cur, CLK_MISC, dsor);
	if (ret_val != 1)
		return ret_val;

	argument = mmc_card_cur->RCA << 16;
	ret_val = mmc_send_cmd(mmc_cont_cur->base, MMC_CMD7_SELECT,
							argument, resp);
	if (ret_val != 1)
		return ret_val;

	if (mmc_cont_cur->slot == 1) {
		/* Switch eMMC on OMAP4 to HS timing */
		argument = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
			(EXT_CSD_HS_TIMING << 16) | (1 << 8) |
			EXT_CSD_CMD_SET_NORMAL;
		ret_val = mmc_send_cmd(mmc_cont_cur->base, MMC_CMD6,
				argument, resp);
		if (ret_val != 1)
		{
  		    printf("failed ot set high speed mode\n");
			return ret_val;
		}
#ifndef CONFIG_MACH_OTTER2
		wait_card_state_prg( mmc_card_cur, mmc_cont_cur);
#endif
		/* Switch the clock to 48MHz */
		ret_val = mmc_clock_config(mmc_cont_cur, CLK_MISC, 2);
		if (ret_val != 1)
			return ret_val;
	}

	/* Configure the block length to 512 bytes */
	argument = MMCSD_SECTOR_SIZE;
	ret_val = mmc_send_cmd(mmc_cont_cur->base, MMC_CMD16, argument, resp);
	if (ret_val != 1) {
		printf("failed sector size\n");
		return ret_val;
	}

	/* get the card size in sectors */
	ret_val = mmc_read_cardsize(mmc_cont_cur->base,
					mmc_card_cur, &Card_CSD);
	if (ret_val != 1) {
		printf("failed to get CSD\n");
		return ret_val;
	}

	if (mmc_cont_cur->slot == 1) {
		/* Switch the eMMC on OMAP4 to 8-bit mode */
		argument = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
			(EXT_CSD_BUS_WIDTH << 16) | (EXT_CSD_BUS_WIDTH_8 << 8) |
			EXT_CSD_CMD_SET_NORMAL;
		ret_val = mmc_send_cmd(mmc_cont_cur->base, MMC_CMD6,
				argument, resp);
		if (ret_val != 1) {
			printf("failed to set 8-bit mmc mode\n");
			return ret_val;
		}

		OMAP_HSMMC_CON(mmc_cont_cur->base) |= (1 << 5);
#ifndef CONFIG_MACH_OTTER2
		wait_card_state_prg( mmc_card_cur, mmc_cont_cur);
#endif
	}

	return 1;
}

unsigned long mmc_bread(int dev_num, ulong blknr, ulong blkcnt, ulong *dst)
{
	unsigned long ret;

	ret = (unsigned long)omap_mmc_read_sect(blknr,
				(blkcnt * MMCSD_SECTOR_SIZE),
			&cur_controller_data[dev_num], &cur_card_data[dev_num],
					(unsigned int *)dst);
	return ret;
}

int mmc_init(int slot)
{
	int ret = 1;

	switch (slot) {
	case 0:
		configure_controller(&cur_controller_data[slot], slot);
		ret = configure_mmc(&cur_card_data[slot],
					&cur_controller_data[slot]);
		break;
	case 1:
		configure_controller(&cur_controller_data[slot], slot);
		ret = configure_mmc(&cur_card_data[slot],
					&cur_controller_data[slot]);
		break;
	default:
		printf("mmc_init:mmc slot is not supported%d\n", slot);
	}

	if (ret != 1) {
		mmc_blk_dev[slot].dev = -1;
	} else {
		mmc_blk_dev[slot].if_type = IF_TYPE_MMC;
		mmc_blk_dev[slot].part_type = PART_TYPE_DOS;
		mmc_blk_dev[slot].dev = cur_controller_data[slot].slot;
		mmc_blk_dev[slot].lun = 0;
		mmc_blk_dev[slot].type = 0;
		/* FIXME fill in the correct size (is set to 32MByte) */
		mmc_blk_dev[slot].blksz = MMCSD_SECTOR_SIZE;
		mmc_blk_dev[slot].lba = 0x10000;
		mmc_blk_dev[slot].removable = 0;
		mmc_blk_dev[slot].block_read = mmc_bread;
		fat_register_device(&mmc_blk_dev[slot], 1);
	}
  	return 0;
}

int mmc_read(int mmc_cont, unsigned int src, unsigned char *dst, int size)
{
	int ret = 1;

	if (mmc_blk_dev[mmc_cont].dev == -1) {
		printf("Read not permitted as Card on SLOT-%d"
				" not Initialized\n", mmc_cont);
	} else {
		ret = omap_mmc_read_sect(src, size,
			&cur_controller_data[mmc_cont],
			&cur_card_data[mmc_cont], (unsigned int *)dst);
	}
	return ret;
}

int mmc_write(int mmc_cont, unsigned char *src, unsigned long dst, int size)
{
	int ret = 1;

	if (mmc_blk_dev[mmc_cont].dev == -1) {
		printf("Write not permitted as Card on SLOT-%d"
					" not Initialized\n", mmc_cont);
	} else {
		ret = omap_mmc_write_sect((unsigned int *)src, size,
			&cur_controller_data[mmc_cont],
			&cur_card_data[mmc_cont], dst);
	}
	return ret;
}

#define MMC_ERASE_MODE_TRIM 1

int mmc_erase(int mmc_cont, unsigned int start, int size)
{
	int ret = 1;

	if (mmc_blk_dev[mmc_cont].dev == -1) {
		printf("Erase not permitted as Card on SLOT-%d"
					" not Initialized\n", mmc_cont);
	} else {
		/*
		ret = omap_mmc_erase_sect(start, &cur_controller_data[mmc_cont],
						&cur_card_data[mmc_cont], size);
						*/
		if(size > 0)
			ret = omap_mmc_erase_ex(cur_controller_data[mmc_cont].base, start,
				start+size-1, MMC_ERASE_MODE_TRIM);
	}
	return ret;
}

int mmc_info(int mmc_cont, unsigned int *blksize, unsigned int *sectors)
{
	if (blksize)
		*blksize = mmc_blk_dev[mmc_cont].blksz;
	if (sectors)
		*sectors = cur_card_data[mmc_cont].size;
	return 0;
}

static unsigned char mmc_health_status[512];
void print_blk_erase_info()
{
	block_erase_count_retrieve *hbeci = (void*) mmc_health_status;
	int i;
	printf("|-----------------------------------------------------|\n");
        printf("| Block Erase count |\n");
        printf("|-----------------------------------------------------|\n");
        printf("| Physical block address | Erase Count |\n");
        printf("|-----------------------------------------------------|\n");
        for (i = 0; i < 128; i++) {
		printf("| %-22d | %-11d |\n",
		__swap_16(hbeci->phy_blk_add[i]),
		__swap_16(hbeci->blk_erase_cnt[i]));
	}
        printf("|----------------------------------------------------|\n");
}

int mmc_health(int mmc_cont, char cmd_index)
{
	int ret = 1;
	int tbl_cnt = 0, tbl_index = 0;
	void *output_buf = (void*) mmc_health_status;
	bad_block_info *hbblk;
	block_erase_count_info *heblk;
	printf("MMC slot:%d initialize:cmd_index:0x%x\n", mmc_cont, cmd_index);

	if ((cmd_index != BAD_BLK) && (cmd_index != BLK_ERASE_CNT) &&
		(cmd_index != BLK_ERASE_CNT_RET)) {
		printf("Invalid mmc health cmd index");
		return -1;
	}
	if (mmc_blk_dev[mmc_cont].dev == -1) {
		printf("Read not permitted as Card on SLOT-%d"
				" not Initialized\n", mmc_cont);
		return -1;
	}

	if (cmd_index == BLK_ERASE_CNT_RET) {
		ret = omap_mmc_health_status(BLK_ERASE_CNT_TBL, 0,
                         &cur_controller_data[mmc_cont],
			 &cur_card_data[mmc_cont], output_buf);
		if (ret > 0) {
			tbl_cnt = (int) mmc_health_status[0];
			while (tbl_cnt > 0) {
				ret = omap_mmc_health_status(cmd_index, tbl_index,
		                        &cur_controller_data[mmc_cont],
					&cur_card_data[mmc_cont],
					output_buf);
				printf("eMMC-Health Status:tbl_cnt:%d, tbl_index:%d\n",
					tbl_cnt, tbl_index);
				print_blk_erase_info();
				tbl_index++;
				tbl_cnt--;
				printf("Incr Status:tbl_cnt:%d, tbl_index:%d\n", tbl_cnt, tbl_index);
			}
		}
	} else {

		ret = omap_mmc_health_status(cmd_index, 0,
                         &cur_controller_data[mmc_cont],
			 &cur_card_data[mmc_cont], output_buf);
	}

	if (cmd_index == BAD_BLK) {
		hbblk = (void*) mmc_health_status;
		printf("|------------------------------------------------|\n");
                printf("| eMMC Health status Bad Block information |\n");
                printf("|------------------------------------------------|\n");
                printf("| Bad block 	       | Bad block Count |\n");
                printf("|------------------------------------------------|\n");
                printf("| initial_bad_blk_cnt  | %-14d  |\n", __swap_16(hbblk->initial_bad_blk_cnt));
                printf("| later_bad_blk_cnt    | %-14d  |\n", __swap_16(hbblk->later_bad_blk_cnt));
                printf("| spare_bad_blk_cnt    | %-14d  |\n", __swap_16(hbblk->spare_bad_blk_cnt));
                printf("|------------------------------------------------|\n");
	}
	if (cmd_index == BLK_ERASE_CNT) {
		heblk = (void*) mmc_health_status;
		printf("|------------------------------------------------|\n");
                printf("| eMMC Health status Erase Block information |\n");
                printf("|------------------------------------------------|\n");
                printf("| Erase block 	     | Erase block Count |\n");
                printf("|------------------------------------------------|\n");
		printf("| min_blk_erase_cnt  | %-16d  |\n", __swap_16(heblk->min_blk_erase_cnt));
		printf("| max_blk_erase_cnt  | %-16d  |\n", __swap_16(heblk->max_blk_erase_cnt));
		printf("| avg_blk_erase_cnt  | %-16d  |\n", __swap_16(heblk->avg_blk_erase_cnt));
                printf("|------------------------------------------------|\n");

	}

	return ret;
}

int mmc_sw_part(int mmc_cont, unsigned int part)
{
  int ret = 1;

  if (mmc_blk_dev[mmc_cont].dev == -1) {
    printf("sw_part not permitted as Card on SLOT-%d not Initialized\n",mmc_cont);
  } else {
    unsigned int resp[4];
    ret = mmc_send_cmd(cur_controller_data[mmc_cont].base, MMC_CMD6, (3<<24) | (0xb3<<16) | ((part&7)<<8), resp);
  }
  return ret;
}

#endif
