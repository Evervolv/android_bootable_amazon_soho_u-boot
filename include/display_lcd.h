#include <common.h>
#include <asm/byteorder.h>

struct   display_lcd_s {
int (*Init_LCD)(void);
void (*Power_on_LCD)(void);
void (*Power_off_LCD)(void);
void (*backlight_set)(int);
u_int16_t *frame_buffer;
unsigned char *gzip_buffer;
ulong width;
ulong  height;
ulong	bpp;
ulong  frame_size;
u_int16_t *binary_initlogo_rle_start;
u_int16_t *binary_initlogo_rle_end;
u_int16_t *binary_provfailed_rle_start;
u_int16_t *binary_provfailed_rle_end;
u_int16_t *binary_devicetoohot_rle_start;
u_int16_t *binary_devicetoohot_rle_end;
void *binary_initlogo_gz_start;
void *binary_lowbattery_gz_start;
void *binary_charging_gz_start;
void *binary_charging_low_gz_start;
void *binary_devicetoohot_gz_start;
void *binary_fastboot_gz_start;
void *binary_multi_download_gz_start;
};


void register_display(struct display_lcd_s *d);
int Init_LCD(void);
void Power_on_LCD(void);
void Power_off_LCD(void);
void load_rle(u_int16_t *fb, u_int16_t *logo_start, u_int16_t *logo_end);
void Show_Logo(void);
void show_provfailed(void);
void show_lowbattery(void);
void show_charging(void);
void show_devicetoohot(void);
void show_fastboot(void);
void show_multidownload(void);
int lcd_DrawPixel(int x, int y, ulong dwColor);
void fastboot_putc(char c);
void fastboot_printf(char *str);
void Set_Brightness(int brightness);




