#pragma once
#include "Header.h"
#include "types.h"

class CaptureD3D11
{
	public:
		HRESULT Init();
		HRESULT CaptureScreenD3D11(HWND hwndTarget, BOX* box, CAPTURE_DATA* data);
		HRESULT CaptureScreenD3D11_Test(HWND hwndTarget,std::string key);
		void SaveImage(D3D11_MAPPED_SUBRESOURCE data, std::string key);
		void releaseTexture() 
		{
			RESET_OBJECT(m_tex.p);
		}

		~CaptureD3D11() {
			RESET_OBJECT(m_tex.p);
			RESET_OBJECT(m_hDevice);
			RESET_OBJECT(m_hContext);
		}

	private:
		ID3D11Device* m_hDevice;
		ID3D11DeviceContext* m_hContext;
		HANDLE m_sharedTexture;
	
		struct
		{
			ID3D11Texture2D* p;
			D3D11_TEXTURE2D_DESC desc;//Descrption
		} 
		m_tex = { 0 };
};