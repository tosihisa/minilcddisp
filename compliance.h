
#ifndef	__COMPLIANCE_H__	/* { */
#define	__COMPLIANCE_H__

#if 0
/* clock_getres(), clock_gettime(), clock_settime():_POSIX_C_SOURCE >= 199309L */
/* snprintf(), vsnprintf():_BSD_SOURCE || _XOPEN_SOURCE >= 500 || _ISOC99_SOURCE || _POSIX_C_SOURCE >= 200112L; or cc -std=c99 */
/*
       sigaltstack():
           Since glibc 2.12
               _BSD_SOURCE || _XOPEN_SOURCE >= 500 || _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED || _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700
           Before glibc 2.12
               _BSD_SOURCE || _XOPEN_SOURCE >= 500 || _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED
*/
#define	_POSIX_C_SOURCE (200809L)

/* sigaction(): _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE */
/* sigaction() は，_POSIX_C_SOURCE が 1以上であれば良いが，SA_RESETHAND は UNIX98 であるらしい．UNIX98 相当であるためには，_XOPEN_SOURCE >= 500 にする */
#define	_XOPEN_SOURCE (500)

#if 0
#define _GNU_SOURCE        /* or _BSD_SOURCE or _SVID_SOURCE */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
int syscall(int number, ...);
/* syscall() を使うには，_GNU_SOURCE || _BSD_SOURCE || _SVID_SOURCE のいずれか．
 * _GNU_SOURCE を定義すると，strerror_r() の振舞いが変わるので，_SVID_SOURCE を使う */
#endif
#define	_SVID_SOURCE

/* http://linuxjm.sourceforge.jp/html/LDP_man-pages/man7/feature_test_macros.7.html
 * _FORTIFY_SOURCE (glibc 2.3.4 以降)
 *     このマクロを定義すると、文字列やメモリの操作を行う様々な関数を使用する際にバッファオーバーフローを検出するための軽めのチェックが 実行されるようになる。
 *     すべてのバッファオーバーフローが検出される わけではなく、あくまでよくある例についてだけである。
 *     現在の実装では、以下の関数にチェックが追加されている:
 *     memcpy(3), mempcpy(3), memmove(3), memset(3), stpcpy(3), strcpy(3), strncpy(3), strcat(3), strncat(3), sprintf(3), snprintf(3), vsprintf(3), vsnprintf(3), gets(3).
 *     _FORTIFY_SOURCE が 1 に設定された場合、コンパイラの最適化レベルが 1 (gcc -O1) かそれ以上であれば、規格に準拠するプログラムの振る舞いを変化させないようなチェックが実行される。
 *     _FORTIFY_SOURCE が 2 に設定された場合、さらなるチェックが追加されるが、 規格に準拠するプログラムのいくつかが失敗する可能性がある。
 *     いくつかのチェックはコンパイル時に実行でき、コンパイラの警告として 表示される。他のチェックは実行時に行われ、チェックに失敗した場合 には実行時エラーとなる。
 *     このマクロを使用するにはコンパイラの対応が必要であり、 バージョン 4.0 以降の gcc(1) で利用できる。 
 * 注記：このマクロを有効にすると，関数呼び出しで戻り値をチェックすべきだがチェックしていない場合にも警告が出る．
 *       この警告は，(void)func() でも出るので，ignore (無視) を意味する変数を定義し，ignore に代入する形で対応する．
 */
#define	_FORTIFY_SOURCE	(2)
#endif

#define	_POSIX_C_SOURCE	200809L
#define	_XOPEN_SOURCE	500
#define	_SVID_SOURCE
#define	_FORTIFY_SOURCE	2

/* 2GByte を超えるファイルサイズへの対処 */
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

#endif	/* __COMPLIANCE_H__ } */

