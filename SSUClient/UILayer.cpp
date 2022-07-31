#include "stdafx.h"
#include "UILayer.h"
#include "Network.h"

using namespace std;

int buff_ui_num[5] = { -1 };
int skill_ui_num[3] = { 0,1,2 };
clock_t start_buff_0;
clock_t start_buff_1;
clock_t start_buff_2;
clock_t start_buff_3;
clock_t start_buff_4;
clock_t end_buff_0;
clock_t end_buff_1;
clock_t end_buff_2;
clock_t end_buff_3;
clock_t end_buff_4;

bool first_skill_used = false;
bool second_skill_used = false;
bool third_skill_used = false;
clock_t start_skill[3]; 
clock_t end_skill[3];

UILayer::UILayer(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor)
{
    m_fWidth = 0.0f;
    m_fHeight = 0.0f;
    m_vWrappedRenderTargets.resize(nFrame);
    m_vd2dRenderTargets.resize(nFrame);
    m_vTextBlocks.resize(1);
    Initialize(pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor);
}

void UILayer::Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor)
{
    UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D2D1_FACTORY_OPTIONS d2dFactoryOptions = { };

#if defined(_DEBUG) || defined(DBG)
    d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ID3D11Device* pd3d11Device = NULL;
    ID3D12CommandQueue* ppd3dCommandQueues[] = { pd3dCommandQueue };
    ::D3D11On12CreateDevice(pd3dDevice, d3d11DeviceFlags, nullptr, 0, reinterpret_cast<IUnknown**>(ppd3dCommandQueues), _countof(ppd3dCommandQueues), 0, (ID3D11Device **)&pd3d11Device, (ID3D11DeviceContext **)&m_pd3d11DeviceContext, nullptr);

    pd3d11Device->QueryInterface(__uuidof(ID3D11On12Device), (void **)&m_pd3d11On12Device);
    pd3d11Device->Release();

#if defined(_DEBUG) || defined(DBG)
    ID3D12InfoQueue* pd3dInfoQueue;
    if (SUCCEEDED(pd3dDevice->QueryInterface(IID_PPV_ARGS(&pd3dInfoQueue))))
    {
        D3D12_MESSAGE_SEVERITY pd3dSeverities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO,
        };

        D3D12_MESSAGE_ID pd3dDenyIds[] =
        {
            D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
        };

        D3D12_INFO_QUEUE_FILTER d3dInforQueueFilter = { };
        d3dInforQueueFilter.DenyList.NumSeverities = _countof(pd3dSeverities);
        d3dInforQueueFilter.DenyList.pSeverityList = pd3dSeverities;
        d3dInforQueueFilter.DenyList.NumIDs = _countof(pd3dDenyIds);
        d3dInforQueueFilter.DenyList.pIDList = pd3dDenyIds;

        pd3dInfoQueue->PushStorageFilter(&d3dInforQueueFilter);
    }
    pd3dInfoQueue->Release();
#endif

    IDXGIDevice* pdxgiDevice = NULL;
    m_pd3d11On12Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pdxgiDevice);

    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, (void **)&m_pd2dFactory);
    HRESULT hResult = m_pd2dFactory->CreateDevice(pdxgiDevice, (ID2D1Device2 **)&m_pd2dDevice);

 
    m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, (ID2D1DeviceContext2 **)&m_pd2dDeviceContext);

    m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(TextColor), (ID2D1SolidColorBrush **)&m_pd2dTextBrush);

    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&m_pd2dWriteFactory);
    pdxgiDevice->Release();

    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(LayoutColor), (ID2D1SolidColorBrush**)&m_pBrush);
    m_pBrush->SetColor(D2D1::ColorF(LayoutColor));
    m_pBrush->SetOpacity(0.5);
    m_TextColor = TextColor;
}

void UILayer::UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y)
{
    m_vTextBlocks[0] = { strUIText, D2D1::RectF(LeftTop_x, LeftTop_y, RightBottom_x, RightBottom_y), m_pdwTextFormat };
}

void UILayer::Render(UINT nFrame)
{
    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

    m_pd2dDeviceContext->BeginDraw();

    for (const auto& textBlock : m_vTextBlocks)
    {
        m_pd2dDeviceContext->FillRectangle(textBlock.d2dLayoutRect, m_pBrush);
        m_pd2dDeviceContext->DrawRectangle(textBlock.d2dLayoutRect, m_pBrush);
        m_pd2dDeviceContext->DrawText(textBlock.strText.c_str(), static_cast<UINT>(textBlock.strText.length()), textBlock.pdwFormat, textBlock.d2dLayoutRect, m_pd2dTextBrush);
    }
    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
}

void UILayer::ReleaseResources()
{
    for (UINT i = 0; i < GetRenderTargetsCount(); i++)
    {
        ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[i] };
        m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    }
    m_pd2dDeviceContext->SetTarget(nullptr);
    m_pd3d11DeviceContext->Flush();
    for (UINT i = 0; i < GetRenderTargetsCount(); i++)
    {
        m_vd2dRenderTargets[i]->Release();
        m_vWrappedRenderTargets[i]->Release();
    }
    m_pd2dTextBrush->Release();
    m_pd2dDeviceContext->Release();
    m_pdwTextFormat->Release();
    m_pd2dWriteFactory->Release();
    m_pd2dDevice->Release();
    m_pd2dFactory->Release();
    m_pd3d11DeviceContext->Release();
    m_pd3d11On12Device->Release();
}

void UILayer::Resize(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight, UINT TextAlignment, UINT ParagraphAlignment)
{
    m_fWidth = static_cast<float>(nWidth);
    m_fHeight = static_cast<float>(nHeight);

    D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

    for (UINT i = 0; i < GetRenderTargetsCount(); i++)
    {
       
        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        m_pd3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&m_vWrappedRenderTargets[i]));
        IDXGISurface* pdxgiSurface = NULL;
        m_vWrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void **)&pdxgiSurface);

    
        m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &m_vd2dRenderTargets[i]);
        pdxgiSurface->Release();
    }

    if (m_pd2dDeviceContext) m_pd2dDeviceContext->Release();
    m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2dDeviceContext);
    m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    if (m_pd2dTextBrush) m_pd2dTextBrush->Release();
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(m_TextColor), &m_pd2dTextBrush);

    const float fFontSize = m_fHeight / 25.0f;
    const float fSmallFontSize = m_fHeight / 40.0f;

    //m_pd2dWriteFactory->CreateTextFormat(L"궁서체", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", &m_pdwTextFormat);
    m_pd2dWriteFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fSmallFontSize, L"en-us", &m_pdwTextFormat);

    m_pdwTextFormat->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(TextAlignment));
    m_pdwTextFormat->SetParagraphAlignment(static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(ParagraphAlignment));
}

void UILayer::setAlpha(float Layout_a, float Text_a)
{
    m_pBrush->SetOpacity(Layout_a);
    m_pd2dTextBrush->SetOpacity(Text_a);
}


//-----------------------------------------
UIBar::UIBar(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor) : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{

}

UIBar::~UIBar() {}

void UIBar::UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y)
{
    m_vTextBlocks[0] = { strUIText, D2D1::RectF(Basic_LeftTop_x, Basic_LeftTop_y, Basic_RightBottom_x, Basic_RightBottom_y), m_pdwTextFormat };
    Color_Bar = D2D1::RectF(LeftTop_x, LeftTop_y, RightBottom_x, RightBottom_y);
}

void UIBar::Render(UINT nFrame)
{
    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));
    //--------
    m_pd2dDeviceContext->BeginDraw();
    m_pd2dDeviceContext->FillRectangle(Behind_Bar, m_pBehindBrush);
    m_pd2dDeviceContext->DrawRectangle(Behind_Bar, m_pBehindBrush);
    m_pd2dDeviceContext->FillRectangle(Color_Bar, m_pColorBrush);
    m_pd2dDeviceContext->DrawRectangle(Color_Bar, m_pColorBrush);
    for (auto textBlock : m_vTextBlocks)
    {
        m_pd2dDeviceContext->FillRectangle(textBlock.d2dLayoutRect, m_pBrush);
        m_pd2dDeviceContext->DrawRectangle(textBlock.d2dLayoutRect, m_pBrush);
        m_pd2dDeviceContext->DrawText(textBlock.strText.c_str(), static_cast<UINT>(textBlock.strText.length()), textBlock.pdwFormat, textBlock.d2dLayoutRect, m_pd2dTextBrush);
    }
    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
}   

void UIBar::SetBehindBrush(D2D1::ColorF::Enum c, float a,UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y)
{
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(c), (ID2D1SolidColorBrush**)&m_pBehindBrush);
    m_pBehindBrush->SetColor(D2D1::ColorF(c));
    m_pBehindBrush->SetOpacity(a);
    Behind_Bar = D2D1::RectF(LeftTop_x, LeftTop_y, RightBottom_x, RightBottom_y);

    Basic_LeftTop_x = LeftTop_x;
    Basic_LeftTop_y = LeftTop_y;
    Basic_RightBottom_x = RightBottom_x;
    Basic_RightBottom_y = RightBottom_y;
}

void UIBar::SetColorBrush(D2D1::ColorF::Enum c, float a, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y)
{

    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(c), (ID2D1SolidColorBrush**)&m_pColorBrush);
    m_pColorBrush->SetColor(D2D1::ColorF(c));
    m_pColorBrush->SetOpacity(a);
    Color_Bar = D2D1::RectF(LeftTop_x, LeftTop_y, RightBottom_x, RightBottom_y);
}
//-----------------------------------------
UIBitmap::UIBitmap(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor) : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    Setup(NULL);
}
UIBitmap::~UIBitmap()
{

}

HRESULT UIBitmap::WICInit(IWICImagingFactory** factory)
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

HRESULT UIBitmap::D2DLoadBitmap(LPCWSTR fileName, ID2D1RenderTarget* target, IWICImagingFactory* factory, ID2D1Bitmap** bitmap)
{
    HRESULT hr;

    // 디코더 생성
    IWICBitmapDecoder* decoder = 0;
    if(fileName == NULL)
        hr = factory->CreateDecoderFromFilename(L"\Image/Element/none.png", 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
    else
        hr = factory->CreateDecoderFromFilename(fileName, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);

    if (FAILED(hr)) return hr;

    // 프레임 얻기
    IWICBitmapFrameDecode* frameDecode = 0;

    // 0번 프레임을 읽어들임.
    hr = decoder->GetFrame(0, &frameDecode);
    if (FAILED(hr)) {
        decoder->Release();
        return hr;
    }

    // 컨버터 생성
    IWICFormatConverter* converter = 0;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        decoder->Release();
        return hr;
    }

    // 컨버터 초기화
    hr = converter->Initialize(frameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone,
        0, 0.0, WICBitmapPaletteTypeCustom);

    if (FAILED(hr)) {
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

void UIBitmap::UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y)
{
    m_vTextBlocks[0] = { strUIText, D2D1::RectF(LeftTop_x, LeftTop_y, RightBottom_x, RightBottom_y), m_pdwTextFormat };
}

bool UIBitmap::Setup(LPCWSTR fileName)
{
    if (FAILED(WICInit(&imagingFactory)))
    {
        MessageBox(0, L"Imaging  Factory", 0, 0);
        return false;
    }
    if (FAILED(D2DLoadBitmap(fileName, m_pd2dDeviceContext, imagingFactory, &bitmap)))
        return false;

    return true;
}

void UIBitmap::Render(UINT nFrame)
{
    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

    m_pd2dDeviceContext->BeginDraw();
    m_pd2dDeviceContext->DrawBitmap(bitmap, m_vTextBlocks[0].d2dLayoutRect);
    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
}

void UIBitmap::Clean()
{
    SafeRelease(bitmap);
    SafeRelease(imagingFactory);
    //  SafeRelease(m_pd2dDeviceContext);
}

//-----------------------------------------
BuffUI::BuffUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor) : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    
}

BuffUI::~BuffUI() {}

HRESULT BuffUI::WICInit(IWICImagingFactory** factory)
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

HRESULT BuffUI::D2DLoadBitmap(LPCWSTR fileName, ID2D1RenderTarget* target, IWICImagingFactory* factory, ID2D1Bitmap** bitmap)
{
    HRESULT hr;

    // 디코더 생성
    IWICBitmapDecoder* decoder = 0;
    hr = factory->CreateDecoderFromFilename(fileName, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);

    if (FAILED(hr)) return hr;

    // 프레임 얻기
    IWICBitmapFrameDecode* frameDecode = 0;

    // 0번 프레임을 읽어들임.
    hr = decoder->GetFrame(0, &frameDecode);
    if (FAILED(hr)) {
        decoder->Release();
        return hr;
    }

    // 컨버터 생성
    IWICFormatConverter* converter = 0;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        decoder->Release();
        return hr;
    }

    // 컨버터 초기화
    hr = converter->Initialize(frameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone,
        0, 0.0, WICBitmapPaletteTypeCustom);

    if (FAILED(hr)) {
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

bool BuffUI::Setup()
{

    if (FAILED(WICInit(&imagingFactory[0])))
    {
        MessageBox(0, L"Imaging  Factory", 0, 0);
        return false;
    }
    if (FAILED(D2DLoadBitmap(L"\Image/마나2.png", m_pd2dDeviceContext, imagingFactory[0], &bitmap[0])))
        return false;


    if (FAILED(WICInit(&imagingFactory[1])))
    {
        MessageBox(0, L"Imaging  Factory", 0, 0);
        return false;
    }
    if (FAILED(D2DLoadBitmap(L"\Image/가호.png", m_pd2dDeviceContext, imagingFactory[1], &bitmap[1])))
        return false;


    if (FAILED(WICInit(&imagingFactory[2])))
    {
        MessageBox(0, L"Imaging  Factory", 0, 0);
        return false;
    }
    if (FAILED(D2DLoadBitmap(L"\Image/천사.png", m_pd2dDeviceContext, imagingFactory[2], &bitmap[2])))
        return false;


    if (FAILED(WICInit(&imagingFactory[3])))
    {
        MessageBox(0, L"Imaging  Factory", 0, 0);
        return false;
    }
    if (FAILED(D2DLoadBitmap(L"\Image/공격력.png", m_pd2dDeviceContext, imagingFactory[3], &bitmap[3])))
        return false;


    if (FAILED(WICInit(&imagingFactory[4])))
    {
        MessageBox(0, L"Imaging  Factory", 0, 0);
        return false;
    }
    if (FAILED(D2DLoadBitmap(L"\Image/전광석화.png", m_pd2dDeviceContext, imagingFactory[4], &bitmap[4])))
        return false;

    return true;
}

void BuffUI::Display()
{
    m_pd2dDeviceContext->BeginDraw();
    m_pd2dDeviceContext->Clear();
    m_pd2dDeviceContext->DrawBitmap(bitmap[0], D2D1::RectF(0.0f, 0.0f, 300.0f, 300.0f));
};

void BuffUI::Clean()
{
    SafeRelease(bitmap[0]);
    SafeRelease(imagingFactory[0]);
    //  SafeRelease(m_pd2dDeviceContext);
}

void BuffUI::UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y)
{
    m_vTextBlocks[0] = { strUIText, D2D1::RectF(LeftTop_x, LeftTop_y, RightBottom_x, RightBottom_y), m_pdwTextFormat };
}

void BuffUI::Render(UINT nFrame)
{
    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

    m_pd2dDeviceContext->BeginDraw();
    if (buff_ui_num[0] == 0) {
        m_pd2dDeviceContext->DrawBitmap(bitmap[0], buff_space0);
    }
    if (buff_ui_num[1] == 1) {
        m_pd2dDeviceContext->DrawBitmap(bitmap[1], buff_space1);
    }
    if (buff_ui_num[2] == 2) {
        m_pd2dDeviceContext->DrawBitmap(bitmap[2], buff_space2);
    }
    if (buff_ui_num[3] == 3) {
        m_pd2dDeviceContext->DrawBitmap(bitmap[3], buff_space3);
    }
    if (buff_ui_num[4] == 4) {
        m_pd2dDeviceContext->DrawBitmap(bitmap[4], buff_space4);
    }
    /*for (auto textBlock : m_vTextBlocks)
    {
        m_pd2dDeviceContext->FillRectangle(textBlock.d2dLayoutRect, m_pBrush);
        m_pd2dDeviceContext->DrawRectangle(textBlock.d2dLayoutRect, m_pBrush);
        m_pd2dDeviceContext->DrawText(textBlock.strText.c_str(), static_cast<UINT>(textBlock.strText.length()), textBlock.pdwFormat, textBlock.d2dLayoutRect, m_pd2dTextBrush);
    }*/

    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();

    end_buff_0 = clock();
    end_buff_1 = clock();
    end_buff_2 = clock();
    end_buff_3 = clock();
    end_buff_4 = clock();

    if ((end_buff_0 - start_buff_0) / CLOCKS_PER_SEC >= 3)
        buff_ui_num[0] = -1;
    if ((end_buff_1 - start_buff_1) / CLOCKS_PER_SEC >= 10)
        buff_ui_num[1] = -1;
    if ((end_buff_2 - start_buff_2) / CLOCKS_PER_SEC >= 3)
        buff_ui_num[2] = -1;
    if ((end_buff_3 - start_buff_3) / CLOCKS_PER_SEC >= 10)  //공격력 
        buff_ui_num[3] = -1;
    if ((end_buff_4 - start_buff_4) / CLOCKS_PER_SEC >= 5)  //공속 
        buff_ui_num[4] = -1;
}
//------------------------------
//skill_ui
SkillUI::SkillUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor) : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
        m_fWidth = 0.0f;
        m_fHeight = 0.0f;
        m_vWrappedRenderTargets.resize(nFrame);
        m_vd2dRenderTargets.resize(nFrame);
        m_vTextBlocks.resize(1);
        Initialize(pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor);
}
SkillUI::~SkillUI() {}

HRESULT SkillUI::WICInit(IWICImagingFactory** factory)
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

HRESULT SkillUI::D2DLoadBitmap(LPCWSTR fileName, ID2D1RenderTarget* target, IWICImagingFactory* factory, ID2D1Bitmap** bitmap)
{
    HRESULT hr;

    // 디코더 생성
    IWICBitmapDecoder* decoder = 0;
    hr = factory->CreateDecoderFromFilename(fileName, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);

    if (FAILED(hr)) return hr;

    // 프레임 얻기
    IWICBitmapFrameDecode* frameDecode = 0;

    // 0번 프레임을 읽어들임.
    hr = decoder->GetFrame(0, &frameDecode);
    if (FAILED(hr)) {
        decoder->Release();
        return hr;
    }

    // 컨버터 생성
    IWICFormatConverter* converter = 0;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        decoder->Release();
        return hr;
    }

    // 컨버터 초기화
    hr = converter->Initialize(frameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone,
        0, 0.0, WICBitmapPaletteTypeCustom);

    if (FAILED(hr)) {
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

bool SkillUI::Setup()
{
    //직업에 따라 다르게 셋업하는게 좋을듯??
   
    switch (my_job)
    {
    case J_DILLER:
        if (skill_ui_num[0] == 0) {
            if (FAILED(WICInit(&imagingFactory[0])))
            {
                MessageBox(0, L"Imaging  Factory", 0, 0);
                return false;
            }
            if (FAILED(D2DLoadBitmap(L"\Image/스킬ui/전사1.png", m_pd2dDeviceContext, imagingFactory[0], &bitmap[0])))
                return false;
        }
        if (skill_ui_num[1] == 1) {
            if (FAILED(WICInit(&imagingFactory[1])))
            {
                MessageBox(0, L"Imaging  Factory", 0, 0);
                return false;
            }
            if (FAILED(D2DLoadBitmap(L"\Image/스킬ui/전사2.png", m_pd2dDeviceContext, imagingFactory[1], &bitmap[1])))
                return false;
        }
        if (skill_ui_num[2] == 2) {

            if (FAILED(WICInit(&imagingFactory[2])))
            {
                MessageBox(0, L"Imaging  Factory", 0, 0);
                return false;
            }
            if (FAILED(D2DLoadBitmap(L"\Image/스킬ui/전사3.png", m_pd2dDeviceContext, imagingFactory[2], &bitmap[2])))
                return false;
        }
        break;
    case J_TANKER:
        if (skill_ui_num[0] == 0) {
            if (FAILED(WICInit(&imagingFactory[0])))
            {
                MessageBox(0, L"Imaging  Factory", 0, 0);
                return false;
            }
            if (FAILED(D2DLoadBitmap(L"\Image/스킬ui/탱커1.png", m_pd2dDeviceContext, imagingFactory[0], &bitmap[0])))
                return false;
        }

        if (skill_ui_num[1] == 1) {
            if (FAILED(WICInit(&imagingFactory[1])))
            {
                MessageBox(0, L"Imaging  Factory", 0, 0);
                return false;
            }
            if (FAILED(D2DLoadBitmap(L"\Image/스킬ui/탱커2.png", m_pd2dDeviceContext, imagingFactory[1], &bitmap[1])))
                return false;
        }
        if (skill_ui_num[2] == 2) {

            if (FAILED(WICInit(&imagingFactory[2])))
            {
                MessageBox(0, L"Imaging  Factory", 0, 0);
                return false;
            }
            if (FAILED(D2DLoadBitmap(L"\Image/스킬ui/탱커3.png", m_pd2dDeviceContext, imagingFactory[2], &bitmap[2])))
                return false;
        }
        break;
    case J_MAGICIAN:
        if (skill_ui_num[0] == 0) {
            if (FAILED(WICInit(&imagingFactory[0])))
            {
                MessageBox(0, L"Imaging  Factory", 0, 0);
                return false;
            }
            if (FAILED(D2DLoadBitmap(L"\Image/스킬ui/마법사1.png", m_pd2dDeviceContext, imagingFactory[0], &bitmap[0])))
                return false;
        }
        if (skill_ui_num[1] == 1) {
            if (FAILED(WICInit(&imagingFactory[1])))
            {
                MessageBox(0, L"Imaging  Factory", 0, 0);
                return false;
            }
            if (FAILED(D2DLoadBitmap(L"\Image/스킬ui/마법사2.png", m_pd2dDeviceContext, imagingFactory[1], &bitmap[1])))
                return false;
        }
    //3은아직 
        if (skill_ui_num[2] == 2) {

            if (FAILED(WICInit(&imagingFactory[2])))
            {
                MessageBox(0, L"Imaging  Factory", 0, 0);
                return false;
            }
            if (FAILED(D2DLoadBitmap(L"\Image/스킬ui/마법사3.png", m_pd2dDeviceContext, imagingFactory[2], &bitmap[2])))
                return false;
        }
        break;
    case J_SUPPORTER:
        if (skill_ui_num[0] == 0) {
            if (FAILED(WICInit(&imagingFactory[0])))
            {
                MessageBox(0, L"Imaging  Factory", 0, 0);
                return false;
            }
            if (FAILED(D2DLoadBitmap(L"\Image/스킬ui/서포터1.png", m_pd2dDeviceContext, imagingFactory[0], &bitmap[0])))
                return false;
        }
        if (skill_ui_num[1] == 1) {
            if (FAILED(WICInit(&imagingFactory[1])))
            {
                MessageBox(0, L"Imaging  Factory", 0, 0);
                return false;
            }
            if (FAILED(D2DLoadBitmap(L"\Image/스킬ui/서포터2.png", m_pd2dDeviceContext, imagingFactory[1], &bitmap[1])))
                return false;
        }
        if (skill_ui_num[2] == 2) {

            if (FAILED(WICInit(&imagingFactory[2])))
            {
                MessageBox(0, L"Imaging  Factory", 0, 0);
                return false;
            }
            if (FAILED(D2DLoadBitmap(L"\Image/스킬ui/서포터3.png", m_pd2dDeviceContext, imagingFactory[2], &bitmap[2])))
                return false;
        }
        break;
    }

    return true;
}

void SkillUI::Display()
{
    m_pd2dDeviceContext->BeginDraw();
    m_pd2dDeviceContext->Clear();
    m_pd2dDeviceContext->DrawBitmap(bitmap[0], D2D1::RectF(0.0f, 0.0f, 300.0f, 300.0f));
};

void SkillUI::Clean()
{
    SafeRelease(bitmap[0]);
    SafeRelease(imagingFactory[0]);
    //  SafeRelease(m_pd2dDeviceContext);
}

void SkillUI::UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y)
{
    m_vTextBlocks[0] = { strUIText, D2D1::RectF(LeftTop_x, LeftTop_y, RightBottom_x, RightBottom_y), m_pdwTextFormat };
}

void SkillUI::Render(UINT nFrame)
{
   // for (int i = 0; i < 3; i++)
     //   skill_ui_num[i] = i;


   // Setup();
    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

    m_pd2dDeviceContext->BeginDraw();

   // if (skill_ui_num[0] == 0) {
        m_pd2dDeviceContext->DrawBitmap(bitmap[0], skill_space0);
    //}
   // if (skill_ui_num[1] == 1) {
        m_pd2dDeviceContext->DrawBitmap(bitmap[1], skill_space1);
  //  }
  //  if (skill_ui_num[2] == 2) {
        m_pd2dDeviceContext->DrawBitmap(bitmap[2], skill_space2);
   // }

    if (first_skill_used == true) {
        end_skill[0] = clock();
        skill_cool_rect[0] = 60.0f - 6.0f * ((float)(end_skill[0] - start_skill[0]) / (float)CLOCKS_PER_SEC);

        if ((float)(end_skill[0] - start_skill[0]) / (float)CLOCKS_PER_SEC >= 10.0f) {
            skill_cool_rect[0] = (float)FRAME_BUFFER_WIDTH / 30.0f;
            first_skill_used = false;
        }
    }
    if (second_skill_used == true) {
        end_skill[1] = clock();
        skill_cool_rect[1] = 60.0f - 6.0f * ((float)(end_skill[1] - start_skill[1]) / (float)CLOCKS_PER_SEC);

        if ((float)(end_skill[1] - start_skill[1]) / (float)CLOCKS_PER_SEC >= 10.0f) {
            skill_cool_rect[1] = (float)FRAME_BUFFER_WIDTH / 30.0f;
            second_skill_used = false;
        }
    }
    if (third_skill_used == true) {
        end_skill[2] = clock();
        skill_cool_rect[2] = 60.0f - 6.0f * ((float)(end_skill[2] - start_skill[2]) / (float)CLOCKS_PER_SEC);

        if ((float)(end_skill[2] - start_skill[2]) / (float)CLOCKS_PER_SEC >= 10.0f) {
            skill_cool_rect[2] = (float)FRAME_BUFFER_WIDTH / 30.0f;
            third_skill_used = false;
        }
    }
    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
    
}
//-------------------------


Title_UI::Title_UI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor)   : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    m_vTextBlocks.resize(2);

    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), (ID2D1SolidColorBrush**)&m_pTextLayoutBrush);
    m_pTextLayoutBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
    m_pTextLayoutBrush->SetOpacity(1.0f);

}

Title_UI::~Title_UI()
{

}

void Title_UI::UpdateLabels_ID(const std::wstring& strUIText)
{
    m_vTextBlocks[0] = { L"ID", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - 250, FRAME_BUFFER_HEIGHT / 2 +200 ,
        FRAME_BUFFER_WIDTH / 2 - 50 , FRAME_BUFFER_HEIGHT / 2 + 245), m_pdwTextFormat };
    m_vTextBlocks[1] = { strUIText, D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - 50, FRAME_BUFFER_HEIGHT / 2 + 200,
        FRAME_BUFFER_WIDTH / 2 + 150, FRAME_BUFFER_HEIGHT / 2 + 245), m_pdwTextFormat2 };
}

void Title_UI::UpdateLabels_PASSWORD(const std::wstring& strUIText)
{
    m_vTextBlocks[0] = { L"PASSWORD", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - 250, FRAME_BUFFER_HEIGHT / 2  + 250,
        FRAME_BUFFER_WIDTH / 2 - 50 , FRAME_BUFFER_HEIGHT / 2 +  295), m_pdwTextFormat };
    m_vTextBlocks[1] = { strUIText, D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - 50, FRAME_BUFFER_HEIGHT / 2 + 250,
        FRAME_BUFFER_WIDTH / 2 + 150, FRAME_BUFFER_HEIGHT / 2 + 295), m_pdwTextFormat2 };
}

void Title_UI::UpdateLabels_JOIN_ID(const std::wstring& strUIText)
{
    m_vTextBlocks[0] = { L"ID", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - 200, FRAME_BUFFER_HEIGHT / 2 - 250 ,
        FRAME_BUFFER_WIDTH / 2  , FRAME_BUFFER_HEIGHT / 2 - 205), m_pdwTextFormat };
    m_vTextBlocks[1] = { strUIText, D2D1::RectF(FRAME_BUFFER_WIDTH / 2 , FRAME_BUFFER_HEIGHT / 2 -250,
        FRAME_BUFFER_WIDTH / 2 + 200, FRAME_BUFFER_HEIGHT / 2 -205), m_pdwTextFormat2 };
}

void Title_UI::UpdateLabels_JOIN_PASSWORD(const std::wstring& strUIText)
{
    m_vTextBlocks[0] = { L"PASSWORD", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - 200, FRAME_BUFFER_HEIGHT / 2 - 195,
        FRAME_BUFFER_WIDTH / 2 , FRAME_BUFFER_HEIGHT / 2 -150), m_pdwTextFormat };
    m_vTextBlocks[1] = { strUIText, D2D1::RectF(FRAME_BUFFER_WIDTH / 2 , FRAME_BUFFER_HEIGHT / 2 -195,
        FRAME_BUFFER_WIDTH / 2 + 200, FRAME_BUFFER_HEIGHT / 2 -150), m_pdwTextFormat2 };
}
void Title_UI::UpdateLabels_JOIN_NICKNAME(const std::wstring& strUIText)
{
    m_vTextBlocks[0] = { L"NICKNAME", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - 200, FRAME_BUFFER_HEIGHT / 2 -140 ,
        FRAME_BUFFER_WIDTH / 2  , FRAME_BUFFER_HEIGHT / 2 - 95), m_pdwTextFormat };
    m_vTextBlocks[1] = { strUIText, D2D1::RectF(FRAME_BUFFER_WIDTH / 2 , FRAME_BUFFER_HEIGHT / 2 -140,
        FRAME_BUFFER_WIDTH / 2 + 200, FRAME_BUFFER_HEIGHT / 2  -95), m_pdwTextFormat2 };
}

void Title_UI::Render(UINT nFrame)
{

    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

    m_pd2dDeviceContext->BeginDraw();

    m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[0].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[0].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawText(m_vTextBlocks[0].strText.c_str(), static_cast<UINT>(m_vTextBlocks[0].strText.length()),
        m_vTextBlocks[0].pdwFormat, m_vTextBlocks[0].d2dLayoutRect, m_pd2dTextBrush);

    m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[1].d2dLayoutRect, m_pTextLayoutBrush);
    m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[1].d2dLayoutRect, m_pTextLayoutBrush);
    m_pd2dDeviceContext->DrawText(m_vTextBlocks[1].strText.c_str(), static_cast<UINT>(m_vTextBlocks[1].strText.length()),
        m_vTextBlocks[1].pdwFormat, m_vTextBlocks[1].d2dLayoutRect, m_pd2dTextBrush);

    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
}

void Title_UI::Resize(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight, UINT TextAlignment, UINT ParagraphAlignment)
{
    m_fWidth = static_cast<float>(nWidth);
    m_fHeight = static_cast<float>(nHeight);

    D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

    for (UINT i = 0; i < GetRenderTargetsCount(); i++)
    {

        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        m_pd3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&m_vWrappedRenderTargets[i]));
        IDXGISurface* pdxgiSurface = NULL;
        m_vWrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);


        m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &m_vd2dRenderTargets[i]);
        pdxgiSurface->Release();
    }

    if (m_pd2dDeviceContext) m_pd2dDeviceContext->Release();
    m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2dDeviceContext);
    m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    if (m_pd2dTextBrush) m_pd2dTextBrush->Release();
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(m_TextColor), &m_pd2dTextBrush);

    const float fFontSize = m_fHeight / 25.0f;
    const float fSmallFontSize = m_fHeight / 40.0f;

    //m_pd2dWriteFactory->CreateTextFormat(L"궁서체", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", &m_pdwTextFormat);
    m_pd2dWriteFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fSmallFontSize, L"en-us", &m_pdwTextFormat);

    m_pdwTextFormat->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(TextAlignment));
    m_pdwTextFormat->SetParagraphAlignment(static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(ParagraphAlignment));

    m_pd2dWriteFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fSmallFontSize, L"en-us", &m_pdwTextFormat2);
    m_pdwTextFormat2->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_pdwTextFormat2->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}
///----
JOIN_ELEMENT_UI::JOIN_ELEMENT_UI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor)
    : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    m_vTextBlocks.resize(9);
    BG_Rect = D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 10, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 + 10,
        FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 10, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20+ 130);

    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), (ID2D1SolidColorBrush**)&m_pButtonBrush);
    m_pButtonBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Gray));
    m_pButtonBrush->SetOpacity(1.0f);
}

JOIN_ELEMENT_UI::~JOIN_ELEMENT_UI()
{

}
void JOIN_ELEMENT_UI::UpdateLabels_JOIN_ELEMENT()  //아래 와이는 100 , 130 
{
    m_vTextBlocks[0] = { L"물", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - 175, FRAME_BUFFER_HEIGHT / 2  +  100,
        FRAME_BUFFER_WIDTH / 2 - 175 + 80, FRAME_BUFFER_HEIGHT / 2 + 130), m_pdwTextFormat };

    m_vTextBlocks[1] = { L"강철", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 360 - FRAME_BUFFER_WIDTH / 22.5, FRAME_BUFFER_HEIGHT / 2 + 100,
        FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + 130), m_pdwTextFormat };

    m_vTextBlocks[2] = { L"바람", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + 100,
        FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 360 + FRAME_BUFFER_WIDTH / 22.5, FRAME_BUFFER_HEIGHT / 2 + 130), m_pdwTextFormat };

    m_vTextBlocks[3] = { L"불", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 10 - FRAME_BUFFER_WIDTH / 360 - FRAME_BUFFER_WIDTH / 22.5 ,FRAME_BUFFER_HEIGHT / 2 + 100,
        FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 10 - FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + 130), m_pdwTextFormat };

    m_vTextBlocks[4] = { L"나무", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - 175 , FRAME_BUFFER_HEIGHT / 2 + 140,

    FRAME_BUFFER_WIDTH / 2 - 175 + 80, FRAME_BUFFER_HEIGHT / 2 + 170), m_pdwTextFormat };

    m_vTextBlocks[5] = { L"땅", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 360 - FRAME_BUFFER_WIDTH / 22.5 , FRAME_BUFFER_HEIGHT / 2 + 140,
     FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + 170), m_pdwTextFormat };

    m_vTextBlocks[6] = { L"얼음", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + 140,
     FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 360 + FRAME_BUFFER_WIDTH / 22.5, FRAME_BUFFER_HEIGHT / 2 + 170), m_pdwTextFormat };

    m_vTextBlocks[7] = { L"확인", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 360 - 100, FRAME_BUFFER_HEIGHT / 2 + 220,
   FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 360 -20, FRAME_BUFFER_HEIGHT / 2 + 270), m_pdwTextFormat };
    m_vTextBlocks[8] = { L"취소", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 360 , FRAME_BUFFER_HEIGHT / 2 + 220,
   FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 360 +80 , FRAME_BUFFER_HEIGHT / 2 + 270), m_pdwTextFormat };

}
void JOIN_ELEMENT_UI::Render(UINT nFrame)
{
    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));
    //--------
    m_pd2dDeviceContext->BeginDraw();
    m_pd2dDeviceContext->FillRectangle(BG_Rect, m_pBrush);
    m_pd2dDeviceContext->DrawRectangle(BG_Rect, m_pBrush);
    for (int i = 0; i < 9; i++)
    {
        m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[i].d2dLayoutRect, m_pButtonBrush);
        m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[i].d2dLayoutRect, m_pButtonBrush);
        m_pd2dDeviceContext->DrawText(m_vTextBlocks[i].strText.c_str(), static_cast<UINT>(m_vTextBlocks[i].strText.length()),
            m_vTextBlocks[i].pdwFormat, m_vTextBlocks[i].d2dLayoutRect, m_pd2dTextBrush);

    }
    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
}

//----
Fail_UI::Fail_UI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor) : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    m_vTextBlocks.resize(2);

    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), (ID2D1SolidColorBrush**)&m_pTextLayoutBrush);
    m_pTextLayoutBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
    m_pTextLayoutBrush->SetOpacity(1.0f);

}

Fail_UI::~Fail_UI()
{

}

void Fail_UI::UpdateLabels_Fail_Select()
{
    m_vTextBlocks[0] = { L"확인", D2D1::RectF(FRAME_BUFFER_WIDTH / 2  -50, FRAME_BUFFER_HEIGHT / 2 + 200 ,
        FRAME_BUFFER_WIDTH / 2 + 50 , FRAME_BUFFER_HEIGHT / 2 + 245), m_pdwTextFormat };
}
void Fail_UI::Render(UINT nFrame)
{

    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

    m_pd2dDeviceContext->BeginDraw();

    m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[0].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[0].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawText(m_vTextBlocks[0].strText.c_str(), static_cast<UINT>(m_vTextBlocks[0].strText.length()),
        m_vTextBlocks[0].pdwFormat, m_vTextBlocks[0].d2dLayoutRect, m_pd2dTextBrush);

    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
}

void Fail_UI::Resize(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight, UINT TextAlignment, UINT ParagraphAlignment)
{
    m_fWidth = static_cast<float>(nWidth);
    m_fHeight = static_cast<float>(nHeight);

    D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

    for (UINT i = 0; i < GetRenderTargetsCount(); i++)
    {

        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        m_pd3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&m_vWrappedRenderTargets[i]));
        IDXGISurface* pdxgiSurface = NULL;
        m_vWrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);


        m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &m_vd2dRenderTargets[i]);
        pdxgiSurface->Release();
    }

    if (m_pd2dDeviceContext) m_pd2dDeviceContext->Release();
    m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2dDeviceContext);
    m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    if (m_pd2dTextBrush) m_pd2dTextBrush->Release();
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(m_TextColor), &m_pd2dTextBrush);

    const float fFontSize = m_fHeight / 25.0f;
    const float fSmallFontSize = m_fHeight / 40.0f;

    //m_pd2dWriteFactory->CreateTextFormat(L"궁서체", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", &m_pdwTextFormat);
    m_pd2dWriteFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fSmallFontSize, L"en-us", &m_pdwTextFormat);

    m_pdwTextFormat->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(TextAlignment));
    m_pdwTextFormat->SetParagraphAlignment(static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(ParagraphAlignment));

    m_pd2dWriteFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fSmallFontSize, L"en-us", &m_pdwTextFormat2);
    m_pdwTextFormat2->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_pdwTextFormat2->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

Skill_Name_UI::Skill_Name_UI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor) : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    m_vTextBlocks.resize(10);

    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), (ID2D1SolidColorBrush**)&m_pTextLayoutBrush);
    m_pTextLayoutBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
    m_pTextLayoutBrush->SetOpacity(1.0f);

    UpdateLabels(L"", L"", L"");
}

Skill_Name_UI::~Skill_Name_UI()
{

}

void Skill_Name_UI::UpdateLabels(const std::wstring& strUIText1, const std::wstring& strUIText2, const std::wstring& strUIText3)
{
    m_vTextBlocks[0] = { strUIText1, D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - 200, FRAME_BUFFER_HEIGHT / 2 + 360 ,
        FRAME_BUFFER_WIDTH / 2 - 120 , FRAME_BUFFER_HEIGHT / 2 + 380), m_pdwTextFormat };
    m_vTextBlocks[1] = { strUIText2, D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - 111, FRAME_BUFFER_HEIGHT / 2 + 360,
        FRAME_BUFFER_WIDTH / 2 - 36, FRAME_BUFFER_HEIGHT / 2 + 380), m_pdwTextFormat };
    m_vTextBlocks[2] = { strUIText3, D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - 25 , FRAME_BUFFER_HEIGHT / 2 + 360,
    FRAME_BUFFER_WIDTH / 2 + 60, FRAME_BUFFER_HEIGHT / 2 + 380), m_pdwTextFormat };
}

void Skill_Name_UI::Render(UINT nFrame)
{
    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

    m_pd2dDeviceContext->BeginDraw();

    m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[0].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[0].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawText(m_vTextBlocks[0].strText.c_str(), static_cast<UINT>(m_vTextBlocks[0].strText.length()),
        m_vTextBlocks[0].pdwFormat, m_vTextBlocks[0].d2dLayoutRect, m_pd2dTextBrush);

    m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[1].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[1].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawText(m_vTextBlocks[1].strText.c_str(), static_cast<UINT>(m_vTextBlocks[1].strText.length()),
        m_vTextBlocks[1].pdwFormat, m_vTextBlocks[1].d2dLayoutRect, m_pd2dTextBrush);

    m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[2].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[2].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawText(m_vTextBlocks[2].strText.c_str(), static_cast<UINT>(m_vTextBlocks[2].strText.length()),
        m_vTextBlocks[2].pdwFormat, m_vTextBlocks[2].d2dLayoutRect, m_pd2dTextBrush);

    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
}

void Skill_Name_UI::Resize(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight, UINT TextAlignment, UINT ParagraphAlignment)
{
    m_fWidth = static_cast<float>(nWidth);
    m_fHeight = static_cast<float>(nHeight);

    D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

    for (UINT i = 0; i < GetRenderTargetsCount(); i++)
    {

        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        m_pd3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&m_vWrappedRenderTargets[i]));
        IDXGISurface* pdxgiSurface = NULL;
        m_vWrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);


        m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &m_vd2dRenderTargets[i]);
        pdxgiSurface->Release();
    }

    if (m_pd2dDeviceContext) m_pd2dDeviceContext->Release();
    m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2dDeviceContext);
    m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    if (m_pd2dTextBrush) m_pd2dTextBrush->Release();
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(m_TextColor), &m_pd2dTextBrush);

    const float fFontSize = m_fHeight / 25.0f;
    const float fSmallFontSize = m_fHeight / 75.0f;

    //m_pd2dWriteFactory->CreateTextFormat(L"궁서체", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", &m_pdwTextFormat);
    m_pd2dWriteFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fSmallFontSize, L"en-us", &m_pdwTextFormat);
    m_pdwTextFormat->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(TextAlignment));
    m_pdwTextFormat->SetParagraphAlignment(static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(ParagraphAlignment));






}

//-----------------
BossSkillUI::BossSkillUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor) 
    : UIBitmap(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    Setup(L"\Image/BossSkill.png");
    m_vTextBlocks.resize(2);
}
BossSkillUI::~BossSkillUI()
{

}

void BossSkillUI::UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y)
{
    m_vTextBlocks[0] = { strUIText, D2D1::RectF(LeftTop_x, LeftTop_y, RightBottom_x, RightBottom_y), m_pdwTextFormat };
}

void BossSkillUI::Render(UINT nFrame)
{
    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

    m_pd2dDeviceContext->BeginDraw();
    m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[0].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[0].d2dLayoutRect, m_pBrush);
    
    m_pd2dDeviceContext->DrawText(m_vTextBlocks[0].strText.c_str(), static_cast<UINT>(m_vTextBlocks[0].strText.length()),
        m_vTextBlocks[0].pdwFormat, m_vTextBlocks[0].d2dLayoutRect, m_pd2dTextBrush);
    m_pd2dDeviceContext->DrawBitmap(bitmap, m_vTextBlocks[1].d2dLayoutRect);

    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
}

void BossSkillUI::Resize(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight, UINT TextAlignment, UINT ParagraphAlignment)
{
    m_fWidth = static_cast<float>(nWidth);
    m_fHeight = static_cast<float>(nHeight);

    D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

    for (UINT i = 0; i < GetRenderTargetsCount(); i++)
    {

        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        m_pd3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&m_vWrappedRenderTargets[i]));
        IDXGISurface* pdxgiSurface = NULL;
        m_vWrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);


        m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &m_vd2dRenderTargets[i]);
        pdxgiSurface->Release();
    }

    if (m_pd2dDeviceContext) m_pd2dDeviceContext->Release();
    m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2dDeviceContext);
    m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    if (m_pd2dTextBrush) m_pd2dTextBrush->Release();
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(m_TextColor), &m_pd2dTextBrush);

    const float fFontSize = m_fHeight / 10.0f;  //25
    const float fSmallFontSize = m_fHeight / 16.0f; //40
    
    m_pd2dWriteFactory->CreateTextFormat(L"한컴 말랑말랑", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fSmallFontSize, L"en-us", &m_pdwTextFormat);

    m_pdwTextFormat->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(TextAlignment));
    m_pdwTextFormat->SetParagraphAlignment(static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(ParagraphAlignment));

    m_vTextBlocks[1] = { L"", D2D1::RectF(FRAME_BUFFER_WIDTH - 300, FRAME_BUFFER_HEIGHT - 400, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT), m_pdwTextFormat };
}

///-----------------------------------------------------------------------
Damage_UI::Damage_UI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor) : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    m_vTextBlocks.resize(nFrame);

    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(LayoutColor), (ID2D1SolidColorBrush**)&m_pTextLayoutBrush);
    m_pTextLayoutBrush->SetColor(D2D1::ColorF(TextColor));
    m_pTextLayoutBrush->SetOpacity(1.0f);
}

Damage_UI::~Damage_UI()
{

}

void Damage_UI::Resize(UINT nFrame)
{
    m_vTextBlocks.resize(nFrame);
}

void Damage_UI::UpdateLabels(CCamera* camera, vector<int> vector, int time, int damageIndex)
{
    int n = -1;

    for (auto& vec : vector) {
        if (!mPlayer[vec]->GetUse())
            continue;

        int raidY = 0;
        float uiUp = 180.0f;
        if (vec == GAIA_ID) {
            raidY = 1300;
            uiUp = 380.0f;
        }
        if (mPlayer[0]->m_job == J_MAGICIAN) {
            uiUp = 250.0f;
        }
        
        XMFLOAT3 xmf3ObjectPos = XMFLOAT3(mPlayer[vec]->GetPosition().x, mPlayer[vec]->GetPosition().y + raidY, mPlayer[vec]->GetPosition().z);
        XMFLOAT3 xmf3ViewProj = Vector3::TransformCoord(Vector3::TransformCoord(xmf3ObjectPos, camera->GetViewMatrix()), camera->GetProjectionMatrix());

        float fScreenX = xmf3ViewProj.x * (FRAME_BUFFER_WIDTH / 2) + FRAME_BUFFER_WIDTH / 2;
        float fScreenY = -xmf3ViewProj.y * (FRAME_BUFFER_HEIGHT / 2) + FRAME_BUFFER_HEIGHT / 2;

        int damage = 0;
        switch (damageIndex) {
        case 0:
            damage = mPlayer[vec]->m_nDamage1;
            break;
        case 1:
            damage = mPlayer[vec]->m_nDamage2;
            break;
        case 2:
            damage = mPlayer[vec]->m_nDamage3;
            break;
        }

        m_vTextBlocks[++n] = { to_wstring(damage),
            D2D1::RectF(fScreenX - 100.0f, fScreenY - uiUp + 20.0f - time,
                        fScreenX  + 100.0f, fScreenY - uiUp - time), m_pdwTextFormat };
    }
}