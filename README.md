NAME
----

minilcddisp - Mini LCD display for Raspberry Pi

SYNOPSIS
--------

`minilcddisp` [`-vtcrd`] `"Display Command"`

DESCRIPTION
-----------

minilcddisp は，小さいLCD を使って Raspberry Pi で各種表示をするプログラムです．

* フォントは Linux カーネルのフォントをソースコードレベルで使用しています．
    * 10種類のフォントが使用できます．
* 表示可能 LCD は Oled704_1306_13SU です．
    * Oled704_1306_13SU : 大阪 日本橋 デジットで販売している 128x32 ドット表示のミニOLEDディスプレイです．
* Raspberry Pi と，glibc 以外の依存ライブラリはありません．Raspberry Pi でよく使われている WiringPi も使っていません．

OPTIONS
-------

`-v`
  バージョンを出力して終了します．

`-t`
  テスト表示を行います．

`-c`
  表示に先立ち，画面をクリアします．

`-r`
  表示に先立ち，LCD をリセットします．

`-d`
  デバッグ出力を行います．

Display Command
---------------

Display Command は，minilcddisp への表示命令です．
以下の表示命令が使用できます．表示命令は，文字列の終わりか，改行まで有効です

`"C"`
  画面をクリアします．

`"T x y fontno Text"`
  (x,y) の位置に，fontno で示すフォント番号に `Text` を描画します．
  コマンドラインから指定する場合は，Tコマンド全体をダブルクォーテーションでくくってください．

EXAMPLE
-------

FILES
-----

ENVIRONMENT
-----------

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

LICENSE
-------

minilcddisp.* と Oled704_1306_13SU.c は MIT ライセンスです．

linuxfont/ 以下は，Linux カーネルと同じライセンス(例外条項付き GPLv2)です．

ソースレベルでは上記のとおりのライセンスですが，ビルドすると一つのプログラムになりますので，プログラムは例外条項付き GPLv2 です．

INSTALL
-------

あらかじめ gcc 等をインストールしてください．
make clean all でビルドできるはずです．

私は，Raspbian Jessie でビルドと動作を確認しています．

インストールですが，ビルドして出来上がる minilcddisp を，手動で /usr/local/bin 等にコピーしてください．
minilcddisp は，フォントもプログラムに入るので，実行ファイルは minilcddisp 一つだけです．

