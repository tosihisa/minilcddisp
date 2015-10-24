
/*
The MIT License (MIT)

Copyright (c) 2015 Toshihisa Tanaka <tosihisa@netfort.gr.jp>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
 * GPIO 17 : RST
 * GPIO  4 : D/C
 */

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "minilcddisp.h"
#include "linuxfont.h"

#define	LCD_MAX_X	(128)
#define	LCD_MAX_Y	(32)
#define	VRAM_BYTE_SIZE	(((LCD_MAX_X * LCD_MAX_Y)/8))

/* SPI LCD 用の VRAM．ここに一旦描いてコピーする． */
/* サイズは VRAM_BYTE_SIZE である */
static uint8_t *VRAMptr = NULL;

static const struct font_desc *fontTbl[] = {
	&font_vga_8x8,		// 0
	&font_vga_8x16,		// 1
	&font_pearl_8x8,	// 2
	&font_vga_6x11,		// 3
	&font_7x14,			// 4
	&font_10x18,		// 5
	&font_sun_8x16,		// 6
	&font_sun_12x22,	// 7
	&font_acorn_8x8,	// 8
	&font_mini_4x6,		// 9
	NULL
};

static int GPIO_RST = -1;
static int GPIO_D_C = -1;
static int SPI_fd = -1;
static int VRAM_fd = -1;

static int GPIO_open(int GPIOnum,int isOUT)
{
	char gpio_path[PATH_MAX];
	int ecnt = 0;
	char *wk;
	int gpio_fd;
	int ret = -1;

	snprintf(gpio_path,sizeof(gpio_path),"/sys/class/gpio/gpio%d/direction",GPIOnum);
	ecnt = 0;
	while((gpio_fd = open(gpio_path,O_RDWR)) < 0){
		char n_str[10];
		wk = "/sys/class/gpio/export";
		if((gpio_fd = open(wk,O_RDWR)) < 0){
			perror(wk);
			return -1;
		}
		snprintf(n_str,sizeof(n_str),"%d",GPIOnum);
		if(write(gpio_fd,n_str,strlen(n_str)) != (ssize_t)strlen(n_str)){
			perror(wk);
			close(gpio_fd);
			return -1;
		}
		close(gpio_fd);
		ecnt++;
		if(ecnt > 2){
			return -2;
		}
	}
	/* ラズパイの GPIO ドライバは，direction を上書きすると値が初期化されるようだ */
	/* 従って，out -> out と，同じ direction であっても値を初期化してしまうので，
	   ここでは値が異なる場合にのみ変更する */
	if(1){
		char chk[1];
		ret = read(gpio_fd,chk,sizeof(chk));
		assert(ret == sizeof(chk));
		if(isOUT){
			ret = (chk[0] == 'o') ? 0 : 1;
		} else {
			ret = (chk[0] == 'i') ? 0 : 1;
		}
		if(ret){
			wk = (isOUT) ? "out" : "in";
			if(write(gpio_fd,wk,strlen(wk)) != (ssize_t)strlen(wk)){
				perror(gpio_path);
				close(gpio_fd);
				return -1;
			}
		}
	}
	close(gpio_fd);
	snprintf(gpio_path,sizeof(gpio_path),"/sys/class/gpio/gpio%d/value",GPIOnum);
	if((gpio_fd = open(gpio_path,O_RDWR)) < 0){
		perror(gpio_path);
	}
	return gpio_fd;
}

static int GPIO_write(int gpio_fd,int onoff)
{
	ssize_t ret;
	char str[1];

	str[0] = ((char)'0') + onoff;
	ret = write(gpio_fd,str,sizeof(str));
	return (ret == sizeof(str)) ? 0 : 1;
}

static int wait_msec(int msec)
{
	struct timespec req;

	req.tv_sec = msec / 1000;
	req.tv_nsec = (msec % 1000) * 1000 * 1000;
	return nanosleep(&req,NULL);
}

static uint8_t mode = 0 ;
static uint8_t bits = 8;
static uint32_t speed = 200000;
static uint16_t delay = 1;

static int SPItransfer(int fd,uint8_t *tx,int txsize,uint8_t *rx)
{
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = txsize,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	assert(ret == txsize);
	return ret;
}

static void LCDmoveto(int fd,int x, int y)
{
	uint8_t tx[3];
	uint8_t rx[3];
	int ret;
    tx[0] = (0xb0 | (y & 0x0f)); // Page Address Set (see 2.4.3)
    tx[1] = (0x10 | (x >> 4 & 0x0f)); // Column Address Set (see 2.4.4)
    tx[2] = (x & 0x0f);
	ret = SPItransfer(fd,tx,sizeof(tx),rx);
	assert(ret == sizeof(tx));
}

static int LCDinit(void)
{
	int ret = 0;
	if(1){
		GPIO_RST = GPIO_open(17,1);
		assert(GPIO_RST >= 0);
		GPIO_D_C = GPIO_open(4,1);
		assert(GPIO_D_C >= 0);
		SPI_fd = open("/dev/spidev0.0", O_RDWR);
		assert(SPI_fd >= 0);
	}
	ret = ioctl(SPI_fd, SPI_IOC_WR_MODE, &mode);
	assert(ret == 0);
	ret = ioctl(SPI_fd, SPI_IOC_RD_MODE, &mode);
	assert(ret == 0);
	ret = ioctl(SPI_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	assert(ret == 0);
	ret = ioctl(SPI_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	assert(ret == 0);
	ret = ioctl(SPI_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	assert(ret == 0);
	ret = ioctl(SPI_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	assert(ret == 0);
	return 0;
}


/**
 * VRAM マッピング処理
 *
 * VRAM は，テンポラリファイルを生成して mmap() でマッピングする．
 * 理由は，VRAM をプログラムに抱き込むと，毎回初期化してしまうため．
 * LCD 側は毎回初期化することは無くても，VRAM 全画面を転送すると，
 * 前回実行した画面は消されてしまう．
 * 他にも理由があって，テンポラリファイルをロックすることで，
 * 同時に描画処理が走らないようにする．
 * また，テンポラリファイルを読み出せば，その時どの様な画面を
 * 描画していたのか分かる．
 *
 * @return 
 */
static int VRAMattach(int *is1st)
{
	int ret = -1;
	struct flock lck;
	char *VRAMfile = "/tmp/.minilcddisp.VRAM.Oled704_1306_13SU.v1";
	VRAM_fd = open(VRAMfile,O_RDWR | O_CREAT | O_EXCL,0600);
	assert((VRAM_fd >= 0) || (errno == EEXIST));
	if(VRAM_fd < 0){
		VRAM_fd = open(VRAMfile,O_RDWR);
		assert(VRAM_fd >= 0);
		*is1st = 0;
	} else {
		*is1st = 1;
	}
	lck.l_type = F_WRLCK;
	lck.l_whence = SEEK_SET;
	lck.l_start = lck.l_len = 0;
	ret = fcntl(VRAM_fd,F_SETLKW,&lck);
	assert(ret == 0);
	ret = ftruncate(VRAM_fd,VRAM_BYTE_SIZE);
	assert(ret == 0);
	VRAMptr = mmap(NULL,VRAM_BYTE_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,VRAM_fd,0);
	assert(VRAMptr != MAP_FAILED);
	return 0;
}

static void VRAMPlot(int x, int y,int fb)
{
	int ly;
	int my;
	unsigned char *pVRAM;
	unsigned char w;

	if((x < 0) || (x >= LCD_MAX_X))
		return;
	if((y < 0) || (y >= LCD_MAX_Y))
		return;

	ly = y / 8;
	my = y % 8;

	pVRAM = &VRAMptr[(LCD_MAX_X*ly)+x];
	w = 1 << my;
	if(fb){
		*pVRAM |= w;
	} else {
		*pVRAM &= ~w;
	}
}

static void VRAMclear(void)
{
	memset(VRAMptr,0,VRAM_BYTE_SIZE);
}

static char *VRAMGetCharImage(int c,const struct font_desc *fontdesc) {
    int w;
    w = (fontdesc->width > 8) ? 2 : 1;
    return ((char *)fontdesc->data) + ((w*fontdesc->height)*(c & 0xff));
}

/* It corresponds only when the width is below 16dots. */
static void VRAMDrawChar(int x,int y,int c,const struct font_desc *fontdesc) {
    int w,h;
    char *imagePtr = VRAMGetCharImage(c,fontdesc);
    unsigned long mask = 0;
    unsigned long maskTbl[] = { 0x80,0x8000 };
    unsigned short yline;
    int wcol;

    for (h = 0;h < fontdesc->height;h++) {
        mask = maskTbl[ (fontdesc->width - 1) / 8 ];
        yline = (unsigned short)(*(imagePtr + 0));
        if(fontdesc->width > 8){
            yline = yline << 8;
            yline |= (unsigned short)(*(imagePtr + 1));
        }
        for (w = 0;w < fontdesc->width;w++) {
            wcol = (yline & mask) ? 1 : 0;
            VRAMPlot(x+w,y+h,wcol);
            mask = mask >> 1;
        }
        imagePtr += (fontdesc->width > 8) ? 2 : 1;
    }
}

static void VRAMDrawStr(int x,int y,char *str,const struct font_desc *fontdesc) {
    char c;
	int t_x = x;
    int i = 0;
    for (i = 0;(c = *(str+i)) != (char)('\0');i++) {
        VRAMDrawChar(t_x,y,c,fontdesc);
        t_x += fontdesc->width;
    }
}

static void VRAMupdateLCD(void)
{
    int x, y;
	int ret;
	uint8_t _rx[LCD_MAX_X];
	ret = msync(VRAMptr,VRAM_BYTE_SIZE,MS_SYNC);
	assert(ret == 0);
	x = 0;
    for(y = 0; y < 4; y++){
		ret = GPIO_write(GPIO_D_C,0);
		assert(ret == 0);
		LCDmoveto(SPI_fd,x,y);
		ret = GPIO_write(GPIO_D_C,1);
		assert(ret == 0);
		ret = SPItransfer(SPI_fd,&VRAMptr[(LCD_MAX_X*y)+x],LCD_MAX_X,_rx);
		assert(ret == LCD_MAX_X);
    }
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static int LCDreset(void)
{
	int ret;

	if(1){
		ret = GPIO_write(GPIO_RST,0);
		assert(ret == 0);
		wait_msec(10);
		ret = GPIO_write(GPIO_RST,1);
		assert(ret == 0);
		wait_msec(10);
	}

	if(1){
		uint8_t tx[] = {
			0xAE,    //DISP OFF
			0xD5,    //SET_CLK
			0x80,
			0xA8,   //SET_MUX
			0x1F,
			0xD3,   //SET_OFFSET
			0x00,
			0x40,   //SET_STARTLINE
			0x8D,   //SET_CHG
			0x14,
			0xA1,   //SEG_REMAP
			0xC8,   //COM_DIR
			0xDA,   //COM_SET
			0x02,
			0x81,   //SET_CONT
			0x40,   //cont
			0xD9,   //SET_PRECHARGE
			0xF1,
			0xDB,   //VCOMH_DESEL
			0x40,
			0xA4,   //ENT_DISP
			0xA6,   //SET_DISP
			0xAF,   //DISP_ON
		};
		uint8_t rx[ARRAY_SIZE(tx)] = {0, };
		ret = GPIO_write(GPIO_D_C,0);
		assert(ret == 0);
		ret = SPItransfer(SPI_fd,tx,ARRAY_SIZE(tx),rx);
		assert(ret == ARRAY_SIZE(tx));
		ret = GPIO_write(GPIO_D_C,1);
		assert(ret == 0);
	}
	return 0;
}

static int LCDtest(void)
{
	int ret;
	if(1){
		int x,y;
		uint8_t _tx[1];
		uint8_t _rx[1];
		for(y = 0;y < 4;y++){
			ret = GPIO_write(GPIO_D_C,0);
			assert(ret == 0);
			LCDmoveto(SPI_fd,0,y);
			ret = GPIO_write(GPIO_D_C,1);
			assert(ret == 0);
			for(x = 0;x < 128;x++){
				ret = GPIO_write(GPIO_D_C,0);
				assert(ret == 0);
				LCDmoveto(SPI_fd,x,y);
				ret = GPIO_write(GPIO_D_C,1);
				assert(ret == 0);
				_tx[0] = (x & 1) ? 0x00 : 0xFF;
				ret = SPItransfer(SPI_fd,_tx,sizeof(_tx),_rx);
				assert(ret == 1);
           	}
		}
		wait_msec(1000);
	}
	if(1){
		VRAMclear();
		VRAMDrawStr(0,0,"Test123",fontTbl[0]);
		VRAMDrawStr(0,8,"Test123",fontTbl[1]);
		VRAMDrawStr(0,8+16,"Test123",fontTbl[9]);
		VRAMDrawStr(64,0,"ABC19",fontTbl[7]);
		VRAMDrawStr(64,22,"ABC9876",fontTbl[2]);
		VRAMupdateLCD();
	}

	return 0;
}

static int LCDwrite(struct LCD_Write_s *wparam)
{
	VRAMDrawStr(wparam->x,wparam->y,wparam->text,fontTbl[wparam->fontNo]);
	VRAMupdateLCD();
	return 0;
}

static int LCDclear(void)
{
	VRAMclear();
	VRAMupdateLCD();
	return 0;
}

static int CLOSE_IF_OPEN(int fd)
{
	int ret = 0;
	if(fd >= 0){
		ret = close(fd);
	}
	return ret;
}

static int LCDfini(void)
{
	int ret;
	ret = msync(VRAMptr,VRAM_BYTE_SIZE,MS_SYNC);
	assert(ret == 0);
	ret = munmap(VRAMptr,VRAM_BYTE_SIZE);
	assert(ret == 0);
	ret = CLOSE_IF_OPEN(GPIO_RST);
	assert(ret == 0);
	ret = CLOSE_IF_OPEN(GPIO_D_C);
	assert(ret == 0);
	ret = CLOSE_IF_OPEN(SPI_fd);
	assert(ret == 0);
	/* VRAM_fd は排他制御を兼ねているので最後に close() すること */
	ret = CLOSE_IF_OPEN(VRAM_fd);
	assert(ret == 0);
	return 0;
}

int Oled704_1306_13SU(int cmd,void *args)
{
	int ret = -1;
	int is1st = 0;
	if(DEBUGLEVEL){
		printf("CALL %s(%d,%p)\n",__FUNCTION__,cmd,args);
	}
	switch(cmd){
		case _LCD_CMD_INIT:
			is1st = 0;
			ret = VRAMattach(&is1st);
			assert(ret == 0);
			ret = LCDinit();
			assert(ret == 0);
			if(is1st){
				ret = LCDreset();
				assert(ret == 0);
			}
			break;
		case _LCD_CMD_RESET:
			ret = LCDreset();
			break;
		case _LCD_CMD_TEST:
			ret = LCDtest();
			break;
		case _LCD_CMD_WRITE:
			ret = LCDwrite(args);
			break;
		case _LCD_CMD_CLEAR:
			ret = LCDclear();
			break;
		case _LCD_CMD_FINI:
			ret = LCDfini();
			break;
		default:
			break;
	}
	return ret;
}

