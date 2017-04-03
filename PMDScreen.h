#pragma once
class PMDScreen
{
public:
	PMDScreen();
	//绘制键盘图
	void Draw();
	//pKeys: 设置表示按键的数组，num: 设置要绘制前多少个通道
	void SetKeyNotesSrc(const int *pkeys, int num);
	//设置绘图区域，如果不指定高度的话则会自动按标准比例计算高度
	void SetRectangle(int _x, int _y, int _w, int _h = 0);
	//设置绘图区域，如果不指定高度的话则会自动按标准比例计算高度
	void SetRectangle(float _x, float _y, float _w, float _h = 0.0f);
	//设置白键颜色，不支持透明 (0x00RRGGBB)
	void SetWhiteKeyColor(int color);
	//设置黑键颜色，不支持透明 (0x00RRGGBB)
	void SetBlackKeyColor(int color);
	//设置白键按下时的颜色，不支持透明 (0x00RRGGBB)
	void SetWhiteKeyPressedColor(int color);
	//设置黑键按下时的颜色，不支持透明 (0x00RRGGBB)
	void SetBlackKeyPressedColor(int color);
private:
	//绘制白键
	void DrawWhiteKey();
	//绘制黑键
	void DrawBlackKey();
	//获取白键序号，如果出错会返回 -1, takeRightVar 为 true 时会向后查找并返回正确结果。
	int GetNumWhiteKey(int n, bool takeRightVar = false);
	//获取黑键序号，如果出错会返回 -1.（注意中间没有黑键的部分也会有一个序号用于充位）
	int GetNumBlackKey(int n);
	int x, y, w, h, drawLength_keyWhite, drawWidth_keyWhite, start_keyBlackX, drawWidth_keyBlack, drawLength_keyBlack;
	int width_avgKey;
	int colorWhiteKey, colorBlackKey, colorWhiteKeyPressed, colorBlackKeyPressed;
	static int tableWhiteKey[];
	static int tableBlackKey[];

	const int *channel_key_notes;//各通道按下的键
	int num_channel;//要绘制前多少个通道
};

