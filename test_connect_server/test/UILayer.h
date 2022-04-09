#pragma once
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#pragma comment(lib, "D2D1.lib")
using namespace D2D1;



#define SafeRelease(p) { if(p) { (p)->Release(); (p)=NULL; } }

struct TextBlock
{
    std::wstring        strText;
    D2D1_RECT_F         d2dLayoutRect;
    IDWriteTextFormat*  pdwFormat;
};

class UILayer
{
public:
    UILayer(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor);

    virtual void UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
    virtual void Render(UINT nFrame);
    void ReleaseResources();
    void Resize(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height, UINT TextAlignment, UINT ParagraphAlignment);
    void setAlpha(float Layout_a, float Text_a);

 

protected:
    UINT GetRenderTargetsCount() { return static_cast<UINT>(m_vWrappedRenderTargets.size()); }
    void Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor);

    float m_fWidth;
    float m_fHeight;
    D2D1::ColorF m_TextColor = D2D1::ColorF::White;

    ID3D11DeviceContext*            m_pd3d11DeviceContext = NULL;
    ID3D11On12Device*               m_pd3d11On12Device = NULL;
    IDWriteFactory*                 m_pd2dWriteFactory = NULL;
    ID2D1Factory3*                  m_pd2dFactory = NULL;
    ID2D1Device2*                   m_pd2dDevice = NULL;
    ID2D1DeviceContext2*            m_pd2dDeviceContext = NULL;
    ID2D1SolidColorBrush*           m_pd2dTextBrush = NULL;
    IDWriteTextFormat*              m_pdwTextFormat = NULL;
    
  

    std::vector<ID3D11Resource*>    m_vWrappedRenderTargets;
    std::vector<ID2D1Bitmap1*>      m_vd2dRenderTargets;
    std::vector<TextBlock>          m_vTextBlocks;

    ID2D1SolidColorBrush*           m_pBrush;
};





class UIBar : public UILayer
{
private:
    UINT                            Basic_LeftTop_x;
    UINT                            Basic_LeftTop_y;
    UINT                            Basic_RightBottom_x;
    UINT                            Basic_RightBottom_y;

    D2D1_RECT_F                     Behind_Bar;
    D2D1_RECT_F                     Color_Bar;
    ID2D1SolidColorBrush* m_pColorBrush;
    ID2D1SolidColorBrush* m_pBehindBrush;

    IWICImagingFactory* imagingFactory = 0;

    ID2D1Bitmap* bitmap = 0;

public:
    UIBar(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor);
    ~UIBar();
    void UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
    void Render(UINT nFrame);
    void SetBehindBrush(D2D1::ColorF::Enum c, float a, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
    void SetColorBrush(D2D1::ColorF::Enum c, float a, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);


    HRESULT WICInit(IWICImagingFactory** factory)

    {

        // COM 초기화

        CoInitialize(0);



        // 인터페이스 생성

        return CoCreateInstance(

            CLSID_WICImagingFactory,

            0, CLSCTX_INPROC_SERVER,

            IID_PPV_ARGS(factory)

        );

    }

    HRESULT D2DLoadBitmap(LPCWSTR fileName, ID2D1RenderTarget* target,

        IWICImagingFactory* factory, ID2D1Bitmap** bitmap)

    {

        HRESULT hr;



        // 디코더 생성

        IWICBitmapDecoder* decoder = 0;

        hr = factory->CreateDecoderFromFilename(fileName,

            0,

            GENERIC_READ,

            WICDecodeMetadataCacheOnDemand,

            &decoder

        );

        if (FAILED(hr))

            return hr;



        // 프레임 얻기

        IWICBitmapFrameDecode* frameDecode = 0;


        // 0번 프레임을 읽어들임.

        hr = decoder->GetFrame(0, &frameDecode);

        if (FAILED(hr))

        {

            decoder->Release();

            return hr;

        }



        // 컨버터 생성

        IWICFormatConverter* converter = 0;

        hr = factory->CreateFormatConverter(&converter);

        if (FAILED(hr))

        {

            decoder->Release();

            return hr;

        }



        // 컨버터 초기화

        hr = converter->Initialize(

            frameDecode,

            GUID_WICPixelFormat32bppPBGRA,

            WICBitmapDitherTypeNone,

            0, 0.0, WICBitmapPaletteTypeCustom

        );

        if (FAILED(hr))

        {

            decoder->Release();

            frameDecode->Release();

            converter->Release();

            return hr;

        }



        // WIC 비트맵으로부터 D2D 비트맵 생성

        hr = target->CreateBitmapFromWicBitmap(converter, 0, bitmap);


        // 자원 해제

        decoder->Release();

        frameDecode->Release();

        converter->Release();


        return hr;

    }


    bool Setup()

    {

        if (FAILED(WICInit(&imagingFactory)))

        {

            MessageBox(0, L"Imaging  Factory", 0, 0);

            return false;

        }

        if (FAILED(D2DLoadBitmap(L"\Image/마나2.png", m_pd2dDeviceContext, imagingFactory, &bitmap)))

            return false;

        return true;

    }

    void Display(DWORD timeDelta)

    {

        m_pd2dDeviceContext->BeginDraw();

        m_pd2dDeviceContext->Clear();


        m_pd2dDeviceContext->DrawBitmap(

            bitmap,

            D2D1::RectF(0.0f, 0.0f, 300.0f, 300.0f)

        );


    };

    void Clean()

    {

        SafeRelease(bitmap);

        SafeRelease(imagingFactory);

        SafeRelease(m_pd2dDeviceContext);


    }


};

