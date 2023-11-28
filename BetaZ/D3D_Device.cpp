#include "D3D_Device.h"

thread_local static THREAD_D3D g_d3d = { 0 };


HRESULT CreateD3D11Device(ID3D11Device** hDevice, ID3D11DeviceContext** hContext)
{
	HRESULT hr = S_OK;

	//Driver D3D11 create types
	//Only support hardware Driver D3D11 -> ADAPTER must be NULL -> Default ADAPTER
	D3D_DRIVER_TYPE DriverTypes[] = { D3D_DRIVER_TYPE_HARDWARE};
	UINT num_DriverTpypes = ARRAYSIZE(DriverTypes);

	D3D_FEATURE_LEVEL FeatureLevels[] = {D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_10_1 };
	UINT num_FeatureLevels = ARRAYSIZE(FeatureLevels);
	//Nếu bật chế độ debug những máy mới sẽ bị lỗi
	D3D_FEATURE_LEVEL FeatureLevel;
	for (UINT index = 0; index < num_DriverTpypes; ++index)
	{
		hr = D3D11CreateDevice(0, DriverTypes[index], NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, FeatureLevels, num_FeatureLevels, D3D11_SDK_VERSION, hDevice, &FeatureLevel, hContext);
		if (SUCCEEDED(hr))
		{
			printf("Create D3D11 Success\n");
			break;
		}
		else
		{
			std::cout << "Error D3D11 Create: " << hr << std::endl;
		}
	}
	return hr;
}

HRESULT GetD3D11Device(THREAD_D3D** hDevice)
{
	HRESULT hr;
	if (!g_d3d.device)
	{
		if (FAILED(hr = CreateD3D11Device(&g_d3d.device, &g_d3d.context)))
		{
			return hr;
		}
	}
	if (g_d3d.device)
	{
		*hDevice = &g_d3d;
		g_d3d.ref++;
		return S_OK;
	}
	return E_FAIL;
}