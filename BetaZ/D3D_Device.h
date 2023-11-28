#pragma once
#include "Header.h"


struct THREAD_D3D //khai bao struct D3D
{
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	ULONG ref;
	void release()
	{
		if (device)
		{
			if (--ref == 0)
			{
				device->Release();
				context->Release();
				device = nullptr;
				context = nullptr;
			}
		}
	}
};

HRESULT CreateD3D11Device(ID3D11Device** hDevice, ID3D11DeviceContext** hContext);

HRESULT GetD3D11Device(THREAD_D3D** hDevice);