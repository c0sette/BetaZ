#include "HWND.h"
FindHWND fhwnd;

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
	std::string Window_Text;
	std::string Class_Name;
	char cques[500];
	char classn[500];
	
	GetWindowTextA(hWnd, cques, sizeof(cques));
	GetClassName(hWnd, classn, sizeof(classn));

	Window_Text += cques;
	Class_Name += classn;

	//Debug Window_Text if you want 
	//std::cout << Window_Text << std::endl;
	if (fhwnd.enable && fhwnd.content.length() > 0)
	{
		if (Window_Text == fhwnd.content)
		{
			fhwnd.result_HWND = hWnd;
		}
	}
	return true;
}


HWND GetHWNDFromText(HWND input_HWND,std::string content)
{
	fhwnd.result_HWND = 0;
	fhwnd.enable = true;
	fhwnd.content = content;
	EnumChildWindows(input_HWND, EnumChildProc, 0);
	fhwnd.enable = false;
	return fhwnd.result_HWND;
}