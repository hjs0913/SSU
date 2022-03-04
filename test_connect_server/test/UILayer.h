#pragma once

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
    ID2D1SolidColorBrush*           m_pColorBrush;
    ID2D1SolidColorBrush*           m_pBehindBrush;
public:
    UIBar(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor);
    ~UIBar();
    void UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
    void Render(UINT nFrame);
    void SetBehindBrush(D2D1::ColorF::Enum c, float a, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
    void SetColorBrush(D2D1::ColorF::Enum c, float a, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
};