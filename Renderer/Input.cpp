#include "Input.h"



Input::Input()
{
}


Input::~Input()
{
}

bool Input::GetKeyState(unsigned int key)
{
	if (keyState.count(key) == 0)
		return false;
	return keyState[key];
}
