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

#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define	USEC_1SEC	(1 /* sec */ * 1000 /* msec */ * 1000 /* usec */)

/**
 * 指定されたマイクロ秒待つ(CLOCK_MONOTONIC/TIMER_ABSTIME)
 *
 * @param usec [in]待ち時間
 * @param sa_flags [in]シグナル割込み再開フラグ
 *
 * @return 
 */
int tutil_wait_usec(long usec,int sa_flags)
{
	struct timespec req;
	ldiv_t cv;
	clockid_t clk_src = CLOCK_MONOTONIC;
	int res = -1;

	if((res = clock_gettime(clk_src,&req)) != 0)
		return res;

	cv = ldiv(usec,USEC_1SEC);
	req.tv_sec  += cv.quot;
	req.tv_nsec += cv.rem * 1000 /* nsec */;

	cv = ldiv(req.tv_nsec,USEC_1SEC * 1000);
	req.tv_sec  += cv.quot;
	req.tv_nsec  = cv.rem;

	while((res = clock_nanosleep(clk_src,TIMER_ABSTIME,&req,NULL)) != 0){
		if((res != EINTR) || (sa_flags == 0)){
			break;
		}
	}

	return res;
}

#if defined(__SELFTEST__)
#include <stdio.h>
#include <assert.h>
int main(void)
{
	int res;

	res = tutil_wait_usec(3 * 1000 * 1000,1);
	assert(res == 0);
	return res;
}
#endif

