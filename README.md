# minilcddisp
minilcddisp - Mini LCD display for Raspberry Pi

minilcddisp は，小さいLCD を使って Raspberry Pi で各種表示をするプログラムです．

# 特徴

* フォントは Linux カーネルのフォントをソースコードレベルで使用しています．
    * 10種類のフォントが使用できます．
* 表示可能 LCD は Oled704_1306_13SU です．
    * Oled704_1306_13SU : 大阪 日本橋 デジットで販売している 128x32 ドット表示のミニOLEDディスプレイです．
* Raspberry Pi と，glibc 以外の依存ライブラリはありません．Raspberry Pi でよく使われている WiringPi も使っていません．

# LICENSE

minilcddisp.* と Oled704_1306_13SU.c は MIT ライセンスです．

linuxfont/ 以下は，Linux カーネルと同じライセンス(例外条項付き GPLv2)です．

ソースレベルでは上記のとおりのライセンスですが，ビルドすると一つのプログラムになりますので，プログラムは例外条項付き GPLv2 です．

