# pmdplay
用于 [Yamaha YM2203 (OPN)](https://en.wikipedia.org/wiki/Yamaha_YM2203) 和 [YM2608 (OPNA)](https://en.wikipedia.org/wiki/Yamaha_YM2608) 的 [FM](https://en.wikipedia.org/wiki/Frequency_modulation_synthesis) 音源播放器。

支持的文件格式：.M, .M2 以及其他 PMD 支持的格式。

## 下载
[GitHub](https://github.com/lxfly2000/pmdplay/releases)  
[Old Versions](https://pan.baidu.com/s/1b0Tkce)

## 运行环境
经测试可以运行的系统：
* Windows 7
* Windows 8.1
* Windows 10

以下系统理论上可以运行但未测试：
* Windows Vista
* Windows 8

## 编译
请用 Visual Studio 2017 编译，需要下载 [DxLib](http://dxlib.o.oo7.jp).

## 参考的代码
* Ｐrofessional Ｍusic Ｄriver (Ｐ.Ｍ.Ｄ.) - [M.Kajihara](http://www5.airnet.ne.jp/kajapon/)
* [OPNA FM Generator](http://retropc.net/cisc/sound/) - cisc
* PPZ8 PCM Driver - UKKY
* PMDWin - [C60](http://c60.la.coocan.jp/)
* [pmdmini](https://github.com/mistydemeo/pmdmini) - BouKiCHi

### 程序中未标示的功能
1. 在播放时按 Z 键可加速2倍播放，C 键减速2倍，X 键恢复原速；
2. 按键盘的 0～9 键可分别静音对应的通道，R 键静音 YM2608 的节奏声音；
3. 通道信息中的三列数字从左到右所表示的意思分别为“音高”，“音色”（Ch7～9 不适用），“该通道的音量”（Ch1～6 的最大值是128，Ch7～9 是16）；
4. 当 PMD 节奏开启时（按 0 键切换静音），Ch9 表示的是 PMD 的节奏，否则为 Ch9 的演奏。其中 PMD 节奏哪个声音被演奏是根据该通道音高的二进制值决定的，从左到右分别表示低位到高位。如果不出意外情况的话只有前11个 bit 位被用到，各 bit 的意义见下表。

   |Bit位（0起）|数值表示（十进制）|含义|
   |:-:|:-:|:-:|
   |0|1|Bass Drum|
   |1|2|Snare Drum|
   |2|4|Low Tom|
   |3|8|Mid Tom|
   |4|16|Hi Tom|
   |5|32|Rimshot|
   |6|64|Snare Drum 2|
   |7|128|Closed Hat|
   |8|256|Open Hat|
   |9|512|Crash Cymbal|
   |10|1024|Ride Cymbal|

   其他有效数值表示同时演奏多个 PMD 节奏声音，由上述数值按位或得到并分别表示相应的声音，0 表示不演奏任何节奏声音。

5. 按 F11 可全屏。
6. 启动参数：“600p”以 960x600 的窗口启动，“720p”以 1280x720 的窗口启动。
7. 按左右键控制播放位置。
8. 按M显示各通道的内存状态，‘<’、‘>’键切换通道。

### 补充说明
* 如果播放时声音卡顿严重，你可以在程序相同目录下建立一个`pmdplay.ini`文件（注意编码只能使用ANSI或Unicode），里面写上以下内容：
```ini
[PMDPlayer]
NoXAudio2=1 ;不使用XAudio2
DSoundNotifyCount=4 ;DSound缓冲区块数
DSoundBufferTime=50 ;DSound单块缓冲区大小（用时长表示，毫秒）
XAudio2BufferTime=20 ;XAudio2缓冲区大小（用时长表示，毫秒）
```
