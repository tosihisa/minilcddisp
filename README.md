NAME
----

minilcddisp - Mini LCD display for Raspberry Pi

<img src="https://www.netfort.gr.jp/~tosihisa/notebook/lib/exe/fetch.php/raspberrypi/img_0395.jpg" alt="Drawing" width="480" />

SYNOPSIS
--------

`minilcddisp` [`-vtcrd4`] [`"Display Command"`]

DESCRIPTION
-----------

minilcddisp は，小さいLCD を使って Raspberry Pi で各種表示をするプログラムです．

* フォントは Linux カーネルのフォントをソースコードレベルで使用しています．
    * 10種類のフォントが使用できます．
* 表示可能 LCD は Oled704_1306_13SU です．
    * Oled704_1306_13SU : 大阪 日本橋 デジットで販売している 128x32 ドット表示のミニOLEDディスプレイです．
* Raspberry Pi と，glibc 以外の依存ライブラリはありません．Raspberry Pi でよく使われている WiringPi も使っていません．
* minilcddisp の起動には，Raspberry Pi の GPIO と /dev/spidev0.0 へのアクセス権が必要です．
    * 手っ取り早い方法は，minilcddisp を sudo コマンドで root 権限で起動することです．
    * minilcddisp は，root 権限で起動された場合，root 権限で必要な最小限の処理を行った後は uid=65534(nobody) に降格します．
* eth0 の IPv4 アドレスを表示する事ができます．Raspberry Pi を Headless (モニタ無し)で使用している場合，IPアドレスを忘れてしまってリモート接続するのに思い出す時間がかかってしまいますが，minilcddisp を使うことで，小さいLCDにIPアドレスを出すことが出来ます．IPアドレスを得るために外部コマンドは使っていません．

OPTIONS
-------

`-v`
  バージョンを出力して終了します．

`-t`
  テスト表示を行います．

`-c`
  表示に先立ち，画面をクリアします．

`-r`
  表示に先立ち，LCD をリセットします．初回起動時は必ず設定してください．

`-d`
  デバッグ出力を行います．

`-4`
  eth0 の IPv4 アドレスを表示します．

Display Command
---------------

Display Command は，minilcddisp への表示命令です．コマンドラインから指定するか，コマンドラインで指定しない場合は，標準入力から読み取ります．

以下の表示命令が使用できます．表示命令は，文字列の終わりか，改行まで有効です


`"C"`
  画面をクリアします．

`"T x y fontno Text"`
  (x,y) の位置に，fontno で示すフォント番号に `Text` を描画します．
  
  コマンドラインから指定する場合は，Tコマンド全体をダブルクォーテーションでくくってください．

EXAMPLE
-------

https://www.netfort.gr.jp/~tosihisa/notebook/doku.php/raspberrypi/minilcddisp にコマンド実行例を掲載しています．

FILES
-----

`/tmp/.minilcddisp.VRAM.Oled704_1306_13SU.v1`
  minilcddisp のVRAMです．

ENVIRONMENT
-----------

必要な環境変数は特にありませんが，Raspberry Pi と LCD は以下の様に結線してください．

`Oled704_1306_13SU`
````
Raspberry Pi <==> Oled704_1306_13SU
-----------------------------------
 GND ------------------ GND
 3v3 ------------------ VIN
 GPIO 10(MOSI) -------- DATA
 GPIO 11(SCLK) -------- CLK
 GPIO 8(CE0) ---------- CS
 GPIO 4 --------------- D/C
 GPIO 17 -------------- RST
````

DIAGNOSTICS
-----------

BUGS
----

* minilcddisp を多重起動すると，片方の minilcddisp は，もう片方の minilcddisp が終了するまで待ちます．
  これはバグではなく，内部的に仮想VRAMを作っており，この仮想VRAMのセマフォ待ちのためです．
* minilcddisp は，何か異常があると assert() で終了するので，異常終了した時は，assert() で出力される部分に何か問題があります．

AUTHOR
------

Toshihisa Tanaka

SEE ALSO
--------

https://www.netfort.gr.jp/~tosihisa/notebook/doku.php/raspberrypi/minilcddisp

LICENSE
-------

minilcddisp.* と Oled704_1306_13SU.c は MIT ライセンスです．

linuxfont/ 以下は，Linux カーネルと同じライセンス(例外条項付き GPLv2)です．

ソースレベルでは上記のとおりのライセンスですが，ビルドすると一つのプログラムになりますので，プログラムは例外条項付き GPLv2 です．

INSTALL
-------

あらかじめ gcc 等をインストールしてください．

minilcddisp は，C言語で作成しており，make clean all でビルドできるはずです．

私は，Raspbian Jessie でビルドと動作を確認しています．

インストールですが，ビルドして出来上がる minilcddisp を，手動で /usr/local/bin 等にコピーしてください．
minilcddisp は，フォントもプログラムに入るので，実行ファイルは minilcddisp 一つだけです．

Raspberry Pi 側では，raspi-config を起動して SPI を使用できるようにしてください．

