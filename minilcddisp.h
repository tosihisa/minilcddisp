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

#ifndef	__MINILCDDISP_H /* { */
#define	__MINILCDDISP_H

typedef unsigned char VRAM_val_t;

struct _s_LCDinfo {
	int width;
	int height;
	VRAM_val_t *VRAMptr;
};

struct _s_LCDdriver {
	char *name;								/** LCD ドライバ名 */
	int (*init)(int argc,char *argv[]);		/** LCD 処理部分の初期化 */
	int (*reset)(void);						/** LCD のリセット */
	int (*flush)(void);						/** LCD VRAM の内容を LCD へ転送する */
	int (*test)(void);						/** LCD のテスト表示 */
	int (*getinfo)(struct _s_LCDinfo *inf);
};

#define	_LCD_CMD_INIT	(0)
#define	_LCD_CMD_RESET	(1)
#define	_LCD_CMD_FLUSH	(2)
#define	_LCD_CMD_TEST	(3)
#define	_LCD_CMD_WRITE	(4)
#define	_LCD_CMD_CLEAR	(5)
#define	_LCD_CMD_FINI	(9999)

struct LCD_Write_s {
	int x;
	int y;
	int fontNo;
	char *text;
};

struct LCDDrivers_s {
	char *name;
	int (*funccall)(int cmd,void *args);
};

extern int DEBUGLEVEL;

#endif	/* } __MINILCDDISP_H */

