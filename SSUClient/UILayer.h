#pragma once
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include "stdafx.h"
#pragma comment(lib, "D2D1.lib")
using namespace D2D1;



#define SafeRelease(p) { if(p) { (p)->Release(); (p)=NULL; } }

extern int buff_ui_num[5];

extern clock_t start_buff_0;
extern clock_t start_buff_1;
extern clock_t start_buff_2;
extern clock_t start_buff_3;
extern clock_t start_buff_4;

extern clock_t end_buff_0;
extern clock_t end_buff_1;
extern clock_t end_buff_2;
extern clock_t end_buff_3;
extern clock_t end_buff_4;

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
    virtual void Resize(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height, UINT TextAlignment, UINT ParagraphAlignment);
    void ReleaseResources();
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

public:
    UIBar(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor);
    ~UIBar();
    virtual void UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
    virtual void Render(UINT nFrame);
    void SetBehindBrush(D2D1::ColorF::Enum c, float a, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
    void SetColorBrush(D2D1::ColorF::Enum c, float a, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
};

class UIBitmap : public UILayer
{
private:
    IWICImagingFactory* imagingFactory = {};
public:
    ID2D1Bitmap* bitmap = {};
    UIBitmap(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor);
    ~UIBitmap();
    HRESULT WICInit(IWICImagingFactory** factory);
    HRESULT D2DLoadBitmap(LPCWSTR fileName, ID2D1RenderTarget* target, IWICImagingFactory* factory, ID2D1Bitmap** bitmap);
    virtual void UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
    virtual void Render(UINT nFrame);
    bool Setup(LPCWSTR fileName);
    void Display();
    void Clean();
};

class BuffUI : public UILayer
{
private:
    IWICImagingFactory* imagingFactory[5] = {};

    ID2D1Bitmap* bitmap[5] = {};

    D2D1_RECT_F buff_space0 = { 0.0f, FRAME_BUFFER_HEIGHT/5, 
        FRAME_BUFFER_WIDTH/30, FRAME_BUFFER_HEIGHT / 5 + FRAME_BUFFER_WIDTH / 30 };
    D2D1_RECT_F buff_space1 = { FRAME_BUFFER_WIDTH / 30 + 5, FRAME_BUFFER_HEIGHT / 5,
        (FRAME_BUFFER_WIDTH / 30 + 5) * 1 + FRAME_BUFFER_WIDTH / 30, FRAME_BUFFER_HEIGHT / 5 + FRAME_BUFFER_WIDTH / 30 };
    D2D1_RECT_F buff_space2 = { (FRAME_BUFFER_WIDTH / 30 + 5) * 2, FRAME_BUFFER_HEIGHT / 5,
        (FRAME_BUFFER_WIDTH / 30 + 5) * 2 + FRAME_BUFFER_WIDTH / 30, FRAME_BUFFER_HEIGHT / 5 + FRAME_BUFFER_WIDTH / 30 };
    D2D1_RECT_F buff_space3 = { (FRAME_BUFFER_WIDTH / 30 + 5) * 3, FRAME_BUFFER_HEIGHT / 5,
        (FRAME_BUFFER_WIDTH / 30 + 5) * 3 + FRAME_BUFFER_WIDTH / 30, FRAME_BUFFER_HEIGHT / 5 + FRAME_BUFFER_WIDTH / 30 };
    D2D1_RECT_F buff_space4 = { (FRAME_BUFFER_WIDTH / 30 + 5) * 4, FRAME_BUFFER_HEIGHT / 5,
        (FRAME_BUFFER_WIDTH / 30 + 5) * 4 + FRAME_BUFFER_WIDTH / 30, FRAME_BUFFER_HEIGHT / 5 + FRAME_BUFFER_WIDTH / 30 };
    int buff_space_used0 = -1;
    int buff_space_used1 = -1;
    int buff_space_used2 = -1;

public:
    BuffUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor);
    ~BuffUI();
    HRESULT WICInit(IWICImagingFactory** factory);
    HRESULT D2DLoadBitmap(LPCWSTR fileName, ID2D1RenderTarget* target, IWICImagingFactory* factory, ID2D1Bitmap** bitmap);
    virtual void UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
    virtual void Render(UINT nFrame);
    bool Setup();
    void Display();
    void Clean();
};
