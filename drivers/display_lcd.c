#include <common.h>
#include <asm/byteorder.h>
#include <command.h>
#include <display_lcd.h>
#include "font5x12.h"



static struct display_lcd_s *display = (struct display_lcd_s *) NULL;
static unsigned int fb_width=10,fb_height=10;


void register_display(struct display_lcd_s *d)
{
	display=d;
}


int Init_LCD(void){
	if (display) {
		if (display->Init_LCD) 
			return (*display->Init_LCD)();
	}
	return 0;
}

void Power_on_LCD(void)
{
	if (display) {
		if (display->Power_on_LCD) 
			(*display->Power_on_LCD)();
	}
}

void Power_off_LCD(void)
{
	if (display) {
		if (display->Power_off_LCD) 
			(*display->Power_off_LCD)();
	}
} 



//**************************************************************************************
// Function: load_rle
// Description: load rle data onto RAM
//**************************************************************************************
void load_rle_shim(u_int16_t *fb, u_int16_t *logo_start, u_int16_t *logo_end)
{
	u_int16_t *target_addr = fb;
	u_int16_t *start = logo_start;
	u_int16_t *end = logo_end;
	u_int16_t count = start[0];

	/* Convert the RLE data into RGB565 */
	for (; start != end; start += 2) {
		count = start[0];

		while (count--)
			*target_addr++ = start[1];
	}
}

// TBD fix -  hack to realign, since objcopy does not seem to always align correctly
void load_rle(u_int16_t *fb, u_int16_t *l_start, u_int16_t *l_end)
{
	char *logo_start;
	char *logo_end;
	logo_start = (char *) l_start;
	logo_end = (char *) l_end;

	if (((unsigned int) logo_start) &1) {
		if ( (display!=NULL) && (display->gzip_buffer!=NULL) && display->frame_size) {
			unsigned int size;
			char *end;
			size= (unsigned int) logo_end - (unsigned int) logo_start;
			if (size < display->frame_size) {
				memcpy(display->gzip_buffer,logo_start, size);
				end= (unsigned char *) display->gzip_buffer + size;
				load_rle_shim(fb, display->gzip_buffer, end);
			}
	

		}
	}
	else
	{
		load_rle_shim(fb, logo_start, logo_end);
	}


}



//**************************************************************************************
// Function: Show_Logo
// Description: show init logo to LCD
//**************************************************************************************
void Show_Logo(void)
{
	if (!display || !display->frame_buffer) return;

	if (display->binary_initlogo_gz_start) {
		show_gzip_rle(display->binary_initlogo_gz_start,
					BRIGHTNESS_NORMAL_ON);
	} else if (display->binary_initlogo_rle_start
		   && display->binary_initlogo_rle_end) {
			load_rle(display->frame_buffer,
				(u_int16_t *) display->binary_initlogo_rle_start,
				(u_int16_t *) display->binary_initlogo_rle_end);
			Set_Brightness(BRIGHTNESS_NORMAL_ON);
	}
}

void show_provfailed(void)
{
	if ( (!display) || !display->frame_buffer ) return;

	if (!display->binary_provfailed_rle_start || !display->binary_provfailed_rle_end) {
		show_color((u_int16_t)(0xF800));
	}
	else {
	load_rle(display->frame_buffer,	display->binary_provfailed_rle_start,
			display->binary_provfailed_rle_end);
	}
}

int show_gzip_rle(void *gzip_data,int brightness)
{
	unsigned char *dst_addr = (unsigned char *)NULL;
	unsigned long dst_len = ~0UL;

	if ( (!display) || !display->frame_buffer || !display->gzip_buffer ||
		!gzip_data || !display->frame_size) return 0;

	dst_addr = (unsigned char *)display->gzip_buffer;

	if (gunzip(dst_addr, display->frame_size,
			gzip_data, &dst_len) != 0) {
		printf("Can not uncompress *.gz file!\n");
		return 1;
	}

	load_rle((u_int16_t *)display->frame_buffer,
			(u_int16_t *)(dst_addr),
			(u_int16_t *)(dst_addr + dst_len));

	if (display->backlight_set) {
		(*display->backlight_set)(brightness);
	}
	return 0;
}

void Set_Brightness(int brightness)
{
	if (display && display->backlight_set) {
		(*display->backlight_set)(brightness);
	}

}

//**************************************************************************************
// Function: show_lowbattery
// Description: show lowbattery logo (compressed as lowbattery.gz) to LCD
//**************************************************************************************
void show_lowbattery(void)
{
	if (!display) return;
	show_gzip_rle(display->binary_lowbattery_gz_start,BRIGHTNESS_LOWBATTERY_ON);
}


//**************************************************************************************
// Function: show_charging
// Description: show charging logo (compressed as charging.gz) to LCD
//**************************************************************************************
void show_charging(void)
{
	if (!display) return;
	show_gzip_rle(display->binary_charging_gz_start,BRIGHTNESS_LOWBATTERY_ON);
}

//**************************************************************************************
// Function: show_charging_low
// Description: show charging logo for SDP charger
//		(compressed as charging_low.gz) to LCD
//**************************************************************************************
void show_charging_low()
{
	if (!display)
		return;

	show_gzip_rle(display->binary_charging_low_gz_start,BRIGHTNESS_LOWBATTERY_ON);
}

//**************************************************************************************
// Function: show_devicetoohot
// Description: show device too hot logo (compressed as devicetoohot.gz) to LCD
//**************************************************************************************
void show_devicetoohot(void)
{
	if (!display) return;
	if (display->binary_devicetoohot_gz_start) {
		printf("Device too hot gzip\n");
		show_gzip_rle(display->binary_devicetoohot_gz_start,BRIGHTNESS_NORMAL_ON);
	} else
	if (display->binary_devicetoohot_rle_start && display->binary_devicetoohot_rle_start) {
		printf("Device too hot rle\n");
		load_rle(display->frame_buffer,	display->binary_devicetoohot_rle_start,
			display->binary_devicetoohot_rle_end);
		Set_Brightness(BRIGHTNESS_NORMAL_ON);
	}
}


//**************************************************************************************
// Function: show_fastboot
// Description: show fastboot logo (compressed as fastboot.gz) to LCD
//**************************************************************************************
void show_fastboot(void)
{
	if (!display) return;
	show_gzip_rle(display->binary_fastboot_gz_start,BRIGHTNESS_NORMAL_ON);
}


//**************************************************************************************
// Function: show_multidownload
// Description: show multi-download logo (compressed as multi_download.gz) to LCD
//**************************************************************************************
void show_multidownload(void)
{
	if (!display) return;
	show_gzip_rle(display->binary_multi_download_gz_start,BRIGHTNESS_NORMAL_ON);
}


int lcd_DrawPixel(int x, int y, ulong dwColor)
{
	ulong *pfb ; 
	ulong dwWidth = 0;
	ulong dwHeight = 0;

	if ( (!display) || !display->frame_buffer ) return 0;

	pfb = (ulong *) display->frame_buffer;
	dwWidth = display->width;
	dwHeight = display->height; 


	if (0 > x || dwWidth - 2 <= x) {
		return 0;
	}
	if (0 > y || dwHeight - 2 <= y) {
		return 0;
	}

	//dwColor |= 0xFF000000;

	pfb[y * dwWidth + x + 1] = dwColor;
	return 0;
}

void fastboot_putc(char c)
{
	unsigned int *glyph=font5x12 + (c - 32) * 2;
	unsigned int xx, yy, datadd;
	datadd = glyph[0];
	if(c>127)
		return;
	if(c==10){
		fb_width=10;
		fb_height+=12;
		return;
	}
	for(yy = 0  ; yy < 6; yy++) {
		for(xx = 0; xx < 5; xx++) {
			if(datadd & 1)
				lcd_DrawPixel(xx+fb_width, yy+fb_height, 0xffffff);
			datadd >>= 1;
		}
	}
	datadd = glyph[1];
	for(yy = 0; yy < 6; yy++) {
		for(xx = 0; xx < 5; xx++) {
			if(datadd & 1)
				lcd_DrawPixel(xx+fb_width, yy+6+fb_height, 0xffffff);
				datadd >>= 1;
		}
	}
	fb_width+=6;
}
void fastboot_printf(char *str)
{
	int i=0;

	while(*(str+i)!=0){
		fastboot_putc(*(str+i));
		i++;
	}
}

//**************************************************************************************
// Function: show_color
// Description: show color on LCD
//**************************************************************************************
void show_color(u_int16_t color)
{
	u_int16_t *target_addr;
	unsigned int pixels;

	if ( (!display) || !display->frame_buffer ||
		!display->bpp || !display->frame_size) return ;

	target_addr = (u_int16_t *)display->frame_buffer;
	pixels = display->frame_size / display->bpp;

	do {
		*target_addr++ = color;
	} while (--pixels);
}


//**************************************************************************************
// Function: show_main
// Description: main function of show color on LCD
//**************************************************************************************
int show_main(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc != 2) {
		goto show_cmd_usage;
	} else {
		if (strncmp(argv[1], "init", 4) ==0) { /* initialize DSI and LCD */
			Init_LCD();
		} else if (strncmp(argv[1], "blue", 4) == 0) {
			show_color((u_int16_t)(0x001F));
		} else if (strncmp(argv[1], "green", 5) == 0) {
			show_color((u_int16_t)(0x07E0));
		} else if (strncmp(argv[1], "red", 3) == 0) {
			show_color((u_int16_t)(0xF800));
		} else if (strncmp(argv[1], "white", 5) == 0) {
			show_color((u_int16_t)(0xFFFF));
		} else if (strncmp(argv[1], "black", 5) == 0) {
			show_color((u_int16_t)(0x0000));
		} else if (strncmp(argv[1], "logo", 4) == 0) { /* show init logo */
			Show_Logo();
		} else if (strncmp(argv[1], "lbc", 3) == 0) {
			/* show low battery charging logo */
			show_charging();
		} else if (strncmp(argv[1], "lb", 2) == 0) {
			/* show low battery logo */
			show_lowbattery();
		} else if (strncmp(argv[1], "devicehot", 9) == 0) {
			/* show device too hot logo */
			show_devicetoohot();
		} else if (strncmp(argv[1], "fastboot", 8) == 0) {
			/* show low fastboot logo */
			show_fastboot();
		} else if (strncmp(argv[1], "multidownload", 8) == 0) {
			/* show low multidownload logo */
			show_multidownload();
		} else if (strncmp(argv[1], "provfailed", 10) == 0) {
			show_provfailed();
 		} else {
			goto show_cmd_usage;
		}

		return 0;
	}

show_cmd_usage:
	printf("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

//**************************************************************************************
// Function:  U_BOOT_CMD
// Description: Uboot command define
//**************************************************************************************
U_BOOT_CMD(
	show, 2, 1, show_main,
	"show   - show color or logo on the DSI display\n",
	"show [blue, green, red, black, white, logo, lb, lbc, devicehot, fastboot, multidownload, provfailed] \n"
);

