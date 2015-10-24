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

#include "compliance.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "minilcddisp.h"

extern int Oled704_1306_13SU(int cmd,void *args);
static struct LCDDrivers_s LCD_Driver_List[2] = {
	{ "Oled704_1306_13SU", Oled704_1306_13SU },
	{ NULL,NULL }
};

struct LCDDrivers_s *Use_LCD_Driver = &LCD_Driver_List[0];
int DEBUGLEVEL = 0;

static char *myname = NULL;
static char driver_opt_str[1024];
static int do_test = 0;
static int do_clear = 0;
static int do_reset = 0;
static int do_rotation = 0;

static char disp_str[128];

static void print_version(void)
{
	printf("%s: version 00.01 (BUILD:%s %s)\n",myname,__DATE__,__TIME__);
}

static int parse_option(int argc,char *argv[])
{
	int opt;

	/* 小文字のオプションは，プログラム全体の振舞いを決める */
	/* 大文字のオプションは，LCDドライバ固有の振舞いを決める */
	while ((opt = getopt(argc, argv, "vtcrdO:o")) != -1){
		switch(opt){
			case 'v':	/* バージョン出力して終了 */
				print_version();
				exit(0);
				break;
			case 'O':	/* LCD ドライバへ引き渡すオプションを指定する */
				if(snprintf(driver_opt_str,sizeof(driver_opt_str),"%s",optarg) >= (int)(sizeof(driver_opt_str))){
					fprintf(stderr,"%s : Driver option too long (%s)\n",myname,driver_opt_str);
					return -1;
				}
				if(DEBUGLEVEL){
					printf("Driver option : [%s]\n",driver_opt_str);
				}
				break;
			case 't':	/* LCD テスト表示を行う */
				do_test = 1;
				break;
			case 'c':	/* 表示に先立ち，LCD をクリアする */
				do_clear = 1;
				break;
			case 'r':	/* 表示に先立ち，LCD をリセットする */
				do_reset = 1;
				break;
			case 'd':	/* デバッグレベルを上げる */
				DEBUGLEVEL++;
				if(DEBUGLEVEL > 1){
					printf("DEBUGLEVEL : %d\n",DEBUGLEVEL);
				}
				break;
			case 'o':	/* 可能であれば，表示を180度回転する(上下左右反転) */
				do_rotation = 1;
				break;
		}
	}
	disp_str[0] = (char)('\0');
	if(optind < argc){
		snprintf(disp_str,sizeof(disp_str),"%s",argv[optind]);
	}
	if(DEBUGLEVEL){
		printf("disp_str : [%s]\n",disp_str);
	}
	return 0;
}

static int Display(char *text)
{
	struct LCD_Write_s wparam;
	char *_tmp;
	const char *tkn = "\t ";
	char *saveptr;
	int ret = -1;

	_tmp = strtok_r(text,tkn,&saveptr);
	switch(*_tmp){
		case 'T':	/* Text draw */
			_tmp = strtok_r(NULL,tkn,&saveptr);
			assert(_tmp != NULL);
			wparam.x = strtol(_tmp,NULL,10);
			_tmp = strtok_r(NULL,tkn,&saveptr);
			assert(_tmp != NULL);
			wparam.y = strtol(_tmp,NULL,10);
			_tmp = strtok_r(NULL,tkn,&saveptr);
			assert(_tmp != NULL);
			wparam.fontNo = strtol(_tmp,NULL,10);
			_tmp = strtok_r(NULL,"",&saveptr);
			assert(_tmp != NULL);
			wparam.text = _tmp;
			ret = Use_LCD_Driver->funccall(_LCD_CMD_WRITE,&wparam);
			assert(ret == 0);
			break;
		case 'C':	/* Display clear */
			ret = Use_LCD_Driver->funccall(_LCD_CMD_CLEAR,NULL);
			assert(ret == 0);
			break;
		default :
			ret = -1;
	}
	return ret;
}

int main(int argc,char *argv[])
{
	int ret = -1;
	myname = argv[0];
	if(parse_option(argc,argv) != 0){
		return 1;
	}
	ret = Use_LCD_Driver->funccall(_LCD_CMD_INIT,NULL);
	assert(ret == 0);
	if(getuid() == 0){
		/* もし，自身が root 権限で動作していれば，INIT 処理後は降格する */
		ret = setgid((gid_t)65534);
		assert(ret == 0);
		ret = setuid((uid_t)65534);
		assert(ret == 0);
		if(DEBUGLEVEL){
			printf("getuid() = %d\n",getuid());
		}
	}
	if(do_reset){
		ret = Use_LCD_Driver->funccall(_LCD_CMD_RESET,NULL);
		assert(ret == 0);
	}
	if(do_clear){
		ret = Use_LCD_Driver->funccall(_LCD_CMD_CLEAR,NULL);
		assert(ret == 0);
	}
	if(do_test){
		ret = Use_LCD_Driver->funccall(_LCD_CMD_TEST,NULL);
		assert(ret == 0);
	}
	if(disp_str[0] != (char)('\0')){
		ret = Display(disp_str);
		assert(ret == 0);
	} else {
		char buf[128];
		int slen;
		while(fgets(buf,sizeof(buf)-1,stdin) != NULL){
			if((slen = strlen(buf)) <= 1)
				continue;
			buf[slen-1] = (char)('\0');
			ret = Display(buf);
			assert(ret == 0);
		}
	}
	ret = Use_LCD_Driver->funccall(_LCD_CMD_FINI,NULL);
	assert(ret == 0);
	return 0;
}

