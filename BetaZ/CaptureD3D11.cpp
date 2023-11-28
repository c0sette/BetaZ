#include "CaptureD3D11.h"
#include "D3D_Device.h"
#include "Header.h"
typedef BOOL(WINAPI* fn_GetDxSharedSurface)(HANDLE hHandle, HANDLE* phSurface, LUID* pAdapterLuid, ULONG* pFmtWindow, ULONG* pPresentFlags, ULONGLONG* pWin32kUpdateId);
fn_GetDxSharedSurface DwmGetDxSharedSurface;
HANDLE GetDWMSharedHandle(HWND hwnd)
{
    HANDLE    surface = nullptr;
    LUID      luid = { 0, };
    ULONG     format = 0;
    ULONG     flags = 0;
    ULONGLONG update_id = 0; //Must define 0 to all variables

    if (!DwmGetDxSharedSurface(hwnd, &surface, &luid, &format, &flags, &update_id)) {
        printf("Failed to get surface!");
        return nullptr;
    }
    
    std::cout << "Adapter DWM : " << luid.LowPart<< std::endl;
    std::cout << "Surface DWM : " << surface << std::endl;
    return (HANDLE)surface;
}

HRESULT CaptureD3D11::Init()
{
    if (!DwmGetDxSharedSurface)
    {
        DwmGetDxSharedSurface = ((fn_GetDxSharedSurface)GetProcAddress(LoadLibrary("user32.dll"), "DwmGetDxSharedSurface"));
    }
    if (!DwmGetDxSharedSurface)
    {
        MessageBox(NULL, "Error 0x1\n\n( Please contact Admin to fix )", "", MB_OK);
        exit(1);
        return E_FAIL;
        
    }
    HRESULT hr = S_OK;

    if (FAILED(hr = CreateD3D11Device(&m_hDevice, &m_hContext)))
    {
        std::cout << "Cant create d11 , error code: " << hr << std::endl;
        return hr;
    }
    std::cout << "D3D11 Device :" << m_hDevice << std::endl;
    std::cout << "D3D11 Device Create Status :" << hr << std::endl;
    
    auto& texturedesc = m_tex.desc;
    texturedesc.Usage = D3D11_USAGE_STAGING;
    texturedesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    texturedesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texturedesc.BindFlags = 0;
    texturedesc.MiscFlags = 0;
    texturedesc.MipLevels = 1;
    texturedesc.ArraySize = 1;
    texturedesc.SampleDesc.Count = 1;
    texturedesc.SampleDesc.Quality = 0;
    return S_OK;
}
void CaptureD3D11::SaveImage(D3D11_MAPPED_SUBRESOURCE data,std::string key)
{

    try {
        printf("I'm here\n");
        HRESULT hr;

        Microsoft::WRL::ComPtr<IWICImagingFactory> wicFactory;
        Microsoft::WRL::ComPtr<IWICFormatConverter> formatConverter;
        CoInitialize(nullptr);
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            __uuidof(wicFactory),
            reinterpret_cast<void**>(wicFactory.GetAddressOf()));
        if (FAILED(hr))
        {
            printf("Expcetion #! in CocREATEiNSTANCE\n");
        }

        Microsoft::WRL::ComPtr<IWICBitmapEncoder> wicEncoder;
        hr = wicFactory->CreateEncoder(
            GUID_ContainerFormatJpeg,
            nullptr,
            &wicEncoder);
        if (FAILED(hr))
        {
            printf("Exception #2 in WIC\n");
            //throw MyException::Make(hr, L"Failed to create BMP encoder");
        }

        Microsoft::WRL::ComPtr<IWICStream> wicStream;
        hr = wicFactory->CreateStream(&wicStream);
        if (FAILED(hr)) 
        {
            printf("Exception in ####\n");
            //throw MyException::Make(hr, L"Failed to create IWICStream");
        }

        std::string pic_name = "vl" + std::to_string(capture_times) +"_" +key + ".jpeg";
        std::wstring p_name = std::wstring(pic_name.begin(), pic_name.end());

        hr = wicStream->InitializeFromFilename(p_name.c_str(), GENERIC_WRITE);
        if (FAILED(hr)) 
        {
            printf("Exception in ####\n");
            //throw MyException::Make(hr, L"Failed to initialize stream from file name");
        }

        hr = wicEncoder->Initialize(wicStream.Get(), WICBitmapEncoderNoCache);
        if (FAILED(hr))
        {
            printf("Exception in ####\n");
            // throw MyException::Make(hr, L"Failed to initialize bitmap encoder");
        }
        // Encode and commit the frame
        {
            Microsoft::WRL::ComPtr<IWICBitmapFrameEncode> frameEncode;
            Microsoft::WRL::ComPtr<IPropertyBag2> properties;
            wicEncoder->CreateNewFrame(&frameEncode, properties.GetAddressOf());
            if (FAILED(hr)) {
                //   throw MyException::Make(hr, L"Failed to create IWICBitmapFrameEncode");
            }
            PROPBAG2 name = { 0 };
            VARIANT quality[1];

            name.dwType = PROPBAG2_TYPE_DATA;
            name.vt = VT_R4;
            name.pstrName = (LPOLESTR)"ImageQuality";
            quality[0].fltVal = 1.0f;
            hr = properties->Write(1, &name, quality);
            //SetEncodingProperties()
            hr = frameEncode->Initialize(properties.Get());
            
            if (FAILED(hr)) 
            {
                std::cout << "Fail" << std::endl;
                // throw MyException::Make(hr, L"Failed to initialize IWICBitmapFrameEncode");
            }

            
            hr = frameEncode->SetSize((UINT)m_tex.desc.Width, (UINT)m_tex.desc.Height);
            GUID guid = GUID_WICPixelFormat24bppBGR;
            hr = frameEncode->SetPixelFormat(&guid);
            //
           

            if (FAILED(hr)) 
            {
                std::cout << "Fail HR code:" << hr << std::endl;
                //throw MyException::Make(hr, L"SetSize(...) failed.");
            }
           // IWICBitmapSource *m_pEmbeddedBitmap;
           
          
            Microsoft::WRL::ComPtr<IWICBitmap> m_pEmbeddedBitmapSource = 0;
           
            hr = wicFactory->CreateBitmapFromMemory((UINT)m_tex.desc.Width, (UINT)m_tex.desc.Height, GUID_WICPixelFormat32bppBGRA, data.RowPitch, data.RowPitch * m_tex.desc.Height, (BYTE*)data.pData,&m_pEmbeddedBitmapSource);
            
            wicFactory->CreateFormatConverter(&formatConverter);

            hr = formatConverter->Initialize(m_pEmbeddedBitmapSource.Get(), GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, 0, 0, WICBitmapPaletteTypeCustom);

            WICRect rect = { 0, 0, (INT)m_tex.desc.Width,(INT)m_tex.desc.Height };
           
            hr = frameEncode->WriteSource(formatConverter.Get(), &rect);
            if (FAILED(hr)) 
            {
                printf("Exception in ####\n");
                //  throw MyException::Make(

            }
            hr = frameEncode->Commit();
            if (FAILED(hr)) {
                printf("Exception in ####\n");
            }
        }
      
        hr = wicEncoder->Commit();
        if (FAILED(hr))
        {
            printf("Exception in @@@@\n");
        }
    }
    catch (std::exception e)
    {
        printf("Error exception\n");
    }
}
HRESULT CaptureD3D11::CaptureScreenD3D11_Test(HWND hwndTarget,std::string key)
{

    HANDLE textureHandle = NULL;
    textureHandle = GetDWMSharedHandle(hwndTarget);
    std::cout << "HWND Target : " << hwndTarget << std::endl;
    if (!textureHandle)
    {
        printf("Expcetion in #1\n");
        return DWM_E_COMPOSITIONDISABLED;
    }
    ID3D11Resource* pResource = nullptr;
   
    HRESULT hr = m_hDevice->OpenSharedResource((HANDLE)textureHandle, __uuidof(ID3D11Resource), (LPVOID*)&pResource);
    if (FAILED(hr))
    {
        printf("Expcetion in #2\n");
        return hr;
    }

    //0x8964 section
    ID3D11Texture2D* tex;
    hr = pResource->QueryInterface<ID3D11Texture2D>(&tex);
    pResource->Release();
    if (FAILED(hr)) {
        printf("Expcetion in #3\n");
        return hr;
    }
    D3D11_TEXTURE2D_DESC desc;
    RECT window_rect{}, client_rect{};
    bool range = false;
    tex->GetDesc(&desc);
    int w = desc.Width, h = desc.Height;

    std::cout << "Width  HWND: " << w << std::endl;
    std::cout << "Height HWND: " << h << std::endl;
    if (m_tex.p) m_tex.p->Release(), m_tex.p = nullptr;
    m_tex.desc.Width = w;
    m_tex.desc.Height = h;
    if (FAILED(hr = m_hDevice->CreateTexture2D(&m_tex.desc, nullptr, &m_tex.p))) 
    {
        tex->Release();
        printf("Expcetion in #4\n");
        return hr;
    }
    m_hContext->CopyResource(m_tex.p, tex);
    tex->Release();

    D3D11_MAPPED_SUBRESOURCE mapped;
    BYTE* out;
    if (SUCCEEDED(hr = m_hContext->Map(m_tex.p, 0, D3D11_MAP_READ, 0, &mapped)))
    {

        out = (BYTE*)mapped.pData;
        printf("Capture success\n");
        CaptureD3D11::SaveImage(mapped,key);
        m_hContext->Unmap(m_tex.p, 0);
        std::cout << "Mapped Row Pitch: " << mapped.pData << std::endl;
    }
    return S_OK;
}