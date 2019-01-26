#pragma once
//按键按下/松开的触发状态检测
class DxKeyTrigger
{
public:
	//按键检测，需要调用 SetKey 设置按键
	DxKeyTrigger();
	//指定用于检测按下/松开的触发状态的键
	DxKeyTrigger(int key);
	//设置待检测的按键
	void SetKey(int key);
	//检测是否按下了按键
	bool Pressed();
	//检测是否松开了按键
	bool Released();
	//检测是否按下了按键，参数为要检测的键
	static bool Pressed(int key);
	//检测是否松开了按键，参数为要检测的键
	static bool Released(int key);
private:
	static void UpdateState(int key);
	void UpdateState();
	static int pressedKey, pressedKeyOld;
	int m_key, pressed, pressedOld;
};
//检测是否按下了按键，参数为要检测的键
bool KeyPressed(int key);
//检测是否松开了按键，参数为要检测的键
bool KeyReleased(int key);