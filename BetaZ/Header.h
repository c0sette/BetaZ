#pragma once
#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <dwmapi.h>
#include <dxgi.h>
#include <shlwapi.h>
#include <gdiplus.h>
#include <fstream>
#include <wincodec.h>
#include <wrl.h>
#include <chrono>
#include <variant>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <ctime>
#include <tlhelp32.h>
//Add-on library //////////////////// 
#include "library/interception.h"
#include "utils.h"
#include "HWND.h"
#include "CaptureD3D11.h"
#include "types.h"
#include "D3D_Device.h"
#include "curl/curl.h"
#include "ini.h"
#include "json.hpp"
////////////////////////////////////

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment (lib,"Shlwapi.lib")
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "Windowscodecs.lib")


extern int capture_times;
