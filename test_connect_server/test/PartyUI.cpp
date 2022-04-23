#include "stdafx.h"
#include "PartyUI.h"

PartyUI::PartyUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor) : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    m_vTextBlocks.resize(4);
    BG_Rect = D2D1::RectF(100, 50, 540, 430);

    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue), (ID2D1SolidColorBrush**)&m_pButtonBrush);
    m_pButtonBrush->SetColor(D2D1::ColorF(D2D1::ColorF::DarkBlue));
    m_pButtonBrush->SetOpacity(1.0f);
}

PartyUI::~PartyUI()
{

}


void PartyUI::UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y)
{
    m_vTextBlocks[0] = { L"방만들기", D2D1::RectF(140, 360, 205, 400), m_pdwTextFormat };
    m_vTextBlocks[1] = { L"방나가기", D2D1::RectF(215, 360, 280, 400), m_pdwTextFormat };
    m_vTextBlocks[2] = { L"초대하기", D2D1::RectF(360, 360, 425, 400), m_pdwTextFormat};
    m_vTextBlocks[3] = { L"AI넣기", D2D1::RectF(435, 360, 500, 400), m_pdwTextFormat };
}

void PartyUI::Render(UINT nFrame)
{
    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));
    //--------
    m_pd2dDeviceContext->BeginDraw();
    m_pd2dDeviceContext->FillRectangle(BG_Rect, m_pBrush);
    m_pd2dDeviceContext->DrawRectangle(BG_Rect, m_pBrush);
    for (auto textBlock : m_vTextBlocks)
    {
        m_pd2dDeviceContext->FillRectangle(textBlock.d2dLayoutRect, m_pButtonBrush);
        m_pd2dDeviceContext->DrawRectangle(textBlock.d2dLayoutRect, m_pButtonBrush);
        m_pd2dDeviceContext->DrawText(textBlock.strText.c_str(), static_cast<UINT>(textBlock.strText.length()), textBlock.pdwFormat, textBlock.d2dLayoutRect, m_pd2dTextBrush);
    }
    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
}
