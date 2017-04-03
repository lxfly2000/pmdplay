#include<DxLib.h>
#include"DxKeyTrigger.h"

DxKeyTrigger::DxKeyTrigger() :pressed(0), pressedOld(0) {}

DxKeyTrigger::DxKeyTrigger(int key) : DxKeyTrigger()
{
	SetKey(key);
}

void DxKeyTrigger::SetKey(int key)
{
	m_key = key;
}

bool DxKeyTrigger::Pressed()
{
	UpdateState();
	return (pressed) && (!pressedOld);
}

bool DxKeyTrigger::Released()
{
	UpdateState();
	return (!pressed)&&(pressedOld);
}

bool DxKeyTrigger::Pressed(int key)
{
	UpdateState(key);
	return (pressedKey == key) && (pressedKeyOld != key);
}

bool DxKeyTrigger::Released(int key)
{
	UpdateState(key);
	return (pressedKey != key)&&(pressedKeyOld == key);
}

void DxKeyTrigger::UpdateState(int key)
{
	pressedKeyOld = pressedKey;
	pressedKey = (CheckHitKey(key) ? key : (pressedKey == key ? 0 : pressedKey));
}

void DxKeyTrigger::UpdateState()
{
	pressedOld = pressed;
	pressed = CheckHitKey(m_key);
}

int DxKeyTrigger::pressedKey = 0;
int DxKeyTrigger::pressedKeyOld = 0;

bool KeyPressed(int key)
{
	return DxKeyTrigger::Pressed(key);
}

bool KeyReleased(int key)
{
	return DxKeyTrigger::Released(key);
}