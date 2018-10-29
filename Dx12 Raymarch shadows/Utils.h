#pragma once
#include <Windows.h>
#include <exception>

#include <string>

inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		throw std::exception(std::to_string(hr).c_str());
	}
}