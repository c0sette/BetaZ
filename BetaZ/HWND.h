#pragma once

#include <Windows.h>
#include <string>
#include <iostream>
class FindHWND
{
	public:
		bool enable;
		std::string content;
		HWND result_HWND;
};
BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);
HWND GetHWNDFromText(HWND input_HWND,std::string content);
