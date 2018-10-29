#pragma once

#include <map>

class Input
{
public:
	Input();
	~Input();

	void SetKeyState(unsigned int key, bool state) { keyState[key] = state; };
	bool GetKeyState(unsigned int key);

protected:
	std::map<unsigned int, bool> keyState;
};

