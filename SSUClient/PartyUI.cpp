#include "stdafx.h"
#include "PartyUI.h"
#include <string>
#include <iostream>

Party::Party()
{
    player_cnt = 0;
    dst = DUN_ST_FREE;
    for (int i = 0; i < GAIA_ROOM; i++) {
        player_name[i] = new char[MAX_NAME_SIZE];
        strcpy_s(player_name[i], MAX_NAME_SIZE, "");
        player_id[i] = -1;
        player_use[i] = false;
    }

}

Party::Party(int r_id)
{
    player_cnt = 0;
    party_id = r_id;
    for (int i = 0; i < GAIA_ROOM; i++) {
        player_name[i] = new char[MAX_NAME_SIZE];
        player_id[i] = -1;
        player_use[i] = false;
    }

}

Party::~Party()
{
    delete player_name;
}

void Party::set_party_id(int id)
{
    party_id = id;
}

void Party::set_room_name(char* name)
{
    strcpy_s(room_name, name);
}

void Party::set_player_name(char* name)
{
    strcpy_s(player_name[player_cnt], MAX_NAME_SIZE, name);
}

int Party::get_party_id()
{
    return party_id;
}

char* Party::get_room_name()
{
    return room_name;
}

//--------------------------------------------
PartyUI::PartyUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor) : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    m_vTextBlocks.resize(4);
    BG_Rect = D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 4, FRAME_BUFFER_HEIGHT / 2 - FRAME_BUFFER_HEIGHT / 3, FRAME_BUFFER_WIDTH/2 + FRAME_BUFFER_WIDTH / 4, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3);

    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue), (ID2D1SolidColorBrush**)&m_pButtonBrush);
    m_pButtonBrush->SetColor(D2D1::ColorF(D2D1::ColorF::DarkBlue));
    m_pButtonBrush->SetOpacity(1.0f);

    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), (ID2D1SolidColorBrush**)&m_pIndexBrush);
    m_pIndexBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Gray));
    m_pIndexBrush->SetOpacity(1.0f);

    m_room_cnt = 4;
}

PartyUI::~PartyUI()
{

}

void PartyUI::ResizeTextBlock(int size)
{
    m_vTextBlocks.resize(size);
    m_room_cnt = size;
}

void PartyUI::UpdateLabels(const std::wstring* strUIText)
{
    m_vTextBlocks[0] = { L"방만들기", D2D1::RectF(FRAME_BUFFER_WIDTH / 4 + FRAME_BUFFER_WIDTH/90, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - FRAME_BUFFER_HEIGHT / 10,
        FRAME_BUFFER_WIDTH / 4 + FRAME_BUFFER_WIDTH / 9 + FRAME_BUFFER_WIDTH / 90, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - 10), m_pdwTextFormat };
    m_vTextBlocks[1] = { L"방들어가기", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 9 - FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - FRAME_BUFFER_HEIGHT / 10,
        FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - 10), m_pdwTextFormat };
    m_vTextBlocks[2] = { L"초대하기", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - FRAME_BUFFER_HEIGHT / 10,
        FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 9 + FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - 10), m_pdwTextFormat};
    m_vTextBlocks[3] = { L"AI넣기", D2D1::RectF(FRAME_BUFFER_WIDTH - FRAME_BUFFER_WIDTH / 4 - FRAME_BUFFER_WIDTH / 90 - FRAME_BUFFER_WIDTH / 9, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - FRAME_BUFFER_HEIGHT / 10,
        FRAME_BUFFER_WIDTH - FRAME_BUFFER_WIDTH / 4 - FRAME_BUFFER_WIDTH / 90, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - 10), m_pdwTextFormat };
   
    for (int i = 4; i < m_room_cnt; i++) {
        m_vTextBlocks[i] = { strUIText[i - 4], D2D1::RectF(FRAME_BUFFER_WIDTH / 4 + FRAME_BUFFER_WIDTH / 90,  FRAME_BUFFER_HEIGHT / 2 - FRAME_BUFFER_HEIGHT / 3 + FRAME_BUFFER_HEIGHT / 90 + (i - 4) * FRAME_BUFFER_HEIGHT / 10,
            FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 - FRAME_BUFFER_HEIGHT / 3 + (i - 3) * FRAME_BUFFER_HEIGHT / 10), m_pdwTextFormat };
    }
}

void PartyUI::UpdateLabels_PartyInfo(const std::wstring* strUIText, Party* p, bool party_enter)
{
    m_vTextBlocks[0] = { L"방만들기", D2D1::RectF(FRAME_BUFFER_WIDTH / 4 + FRAME_BUFFER_WIDTH / 90, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - FRAME_BUFFER_HEIGHT / 10,
        FRAME_BUFFER_WIDTH / 4 + FRAME_BUFFER_WIDTH / 9 + FRAME_BUFFER_WIDTH / 90, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - 10), m_pdwTextFormat };
    if(party_enter) m_vTextBlocks[1] = { L"방나가기", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 9 - FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - FRAME_BUFFER_HEIGHT / 10,
        FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - 10), m_pdwTextFormat };
    else m_vTextBlocks[1] = { L"방들어가기", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 9 - FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - FRAME_BUFFER_HEIGHT / 10,
        FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - 10), m_pdwTextFormat };
    m_vTextBlocks[2] = { L"초대하기", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - FRAME_BUFFER_HEIGHT / 10,
       FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 9 + FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - 10), m_pdwTextFormat };
    m_vTextBlocks[3] = { L"AI넣기", D2D1::RectF(FRAME_BUFFER_WIDTH - FRAME_BUFFER_WIDTH / 4 - FRAME_BUFFER_WIDTH / 90 - FRAME_BUFFER_WIDTH / 9, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - FRAME_BUFFER_HEIGHT / 10,
        FRAME_BUFFER_WIDTH - FRAME_BUFFER_WIDTH / 4 - FRAME_BUFFER_WIDTH / 90, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 3 - 10), m_pdwTextFormat };

    for (int i = 4; i < m_room_cnt-GAIA_ROOM; i++) {
        m_vTextBlocks[i] = { strUIText[i - 4], D2D1::RectF(FRAME_BUFFER_WIDTH / 4 + FRAME_BUFFER_WIDTH / 90,  FRAME_BUFFER_HEIGHT/2 - FRAME_BUFFER_HEIGHT/3 + FRAME_BUFFER_HEIGHT/90 + (i-4)* FRAME_BUFFER_HEIGHT / 10,
            FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 - FRAME_BUFFER_HEIGHT / 3 + (i - 3) * FRAME_BUFFER_HEIGHT / 10), m_pdwTextFormat };
    }

    int tmp = 0;
    for (int i = m_room_cnt - GAIA_ROOM; i < m_room_cnt; i++) {
        std::wstring temp2 = L"";
        wchar_t* temp;
        if (tmp >= p->player_cnt) {
            m_vTextBlocks[i] = { temp2, D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 - FRAME_BUFFER_HEIGHT / 3 + FRAME_BUFFER_HEIGHT / 90 + tmp * FRAME_BUFFER_HEIGHT / 7.5,
                FRAME_BUFFER_WIDTH - FRAME_BUFFER_WIDTH / 4 - FRAME_BUFFER_WIDTH / 90, FRAME_BUFFER_HEIGHT / 2 - FRAME_BUFFER_HEIGHT / 3 + (tmp+1) * FRAME_BUFFER_HEIGHT / 7.5), m_pdwTextFormat };
        }
        else {
            temp2 = L"";
            temp2.append(L"LV : ");
            temp2.append(std::to_wstring(p->player_lv[tmp]));
            temp2.append(L"\tJOB : ");
            switch (p->player_job[tmp])
            {
            case J_DILLER:  temp2.append(L"전사\n"); break;
            case J_TANKER:  temp2.append(L"탱커\n"); break;
            case J_MAGICIAN:  temp2.append(L"마법사\n"); break;
            case J_SUPPORTER:  temp2.append(L"서포터\n"); break;
            default:
                break;
            }
            temp2.append(L"이름 : ");
            int len = 1 + strlen(p->player_name[tmp]);
            temp = new TCHAR[len];
            mbstowcs(temp, p->player_name[tmp], len);
            temp2.append(temp);
            m_vTextBlocks[i] = { temp2, D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 180, FRAME_BUFFER_HEIGHT / 2 - FRAME_BUFFER_HEIGHT / 3 + FRAME_BUFFER_HEIGHT / 90 + tmp * FRAME_BUFFER_HEIGHT / 7.5,
                FRAME_BUFFER_WIDTH - FRAME_BUFFER_WIDTH / 4 - FRAME_BUFFER_WIDTH / 90, FRAME_BUFFER_HEIGHT / 2 - FRAME_BUFFER_HEIGHT / 3 + (tmp + 1) * FRAME_BUFFER_HEIGHT / 7.5), m_pdwTextFormat };
            delete []temp;
        }
        tmp++;
    }
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
    for (int i = 0; i < m_room_cnt; i++)
    {
        if (i >= 0 && i < 4) {
            m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[i].d2dLayoutRect, m_pButtonBrush);
            m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[i].d2dLayoutRect, m_pButtonBrush);
            m_pd2dDeviceContext->DrawText(m_vTextBlocks[i].strText.c_str(), static_cast<UINT>(m_vTextBlocks[i].strText.length()), 
                m_vTextBlocks[i].pdwFormat, m_vTextBlocks[i].d2dLayoutRect, m_pd2dTextBrush);
        }
        else {
            m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[i].d2dLayoutRect, m_pIndexBrush);
            m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[i].d2dLayoutRect, m_pIndexBrush);
            m_pd2dDeviceContext->DrawText(m_vTextBlocks[i].strText.c_str(), static_cast<UINT>(m_vTextBlocks[i].strText.length()),
                m_vTextBlocks[i].pdwFormat, m_vTextBlocks[i].d2dLayoutRect, m_pd2dTextBrush);
        }
    }
    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
}


//--------------------------------------------
PartyInviteUI::PartyInviteUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor)
    : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    m_vTextBlocks.resize(2);

    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), (ID2D1SolidColorBrush**)&m_pTextLayoutBrush);
    m_pTextLayoutBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
    m_pTextLayoutBrush->SetOpacity(1.0f);

}

PartyInviteUI::~PartyInviteUI()
{

}

void PartyInviteUI::UpdateLabels(const std::wstring& strUIText)
{
    m_vTextBlocks[0] = { L"초대하기", D2D1::RectF(FRAME_BUFFER_WIDTH/2 - FRAME_BUFFER_WIDTH/10, FRAME_BUFFER_HEIGHT/2 - FRAME_BUFFER_HEIGHT/20, 
        FRAME_BUFFER_WIDTH/2 + FRAME_BUFFER_WIDTH/10, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20), m_pdwTextFormat };
    m_vTextBlocks[1] = { strUIText, D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 10 + FRAME_BUFFER_WIDTH / 90, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - FRAME_BUFFER_HEIGHT/22.5,
        FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 10 - FRAME_BUFFER_WIDTH / 90, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - 10), m_pdwTextFormat2 };
}

void PartyInviteUI::Render(UINT nFrame)
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

void PartyInviteUI::Resize(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight, UINT TextAlignment, UINT ParagraphAlignment)
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


//--------------------------------------------
InvitationCardUI::InvitationCardUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor)
    : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    m_vTextBlocks.resize(3);
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue), (ID2D1SolidColorBrush**)&m_pTButtonBrush);
    m_pTButtonBrush->SetColor(D2D1::ColorF(D2D1::ColorF::DarkBlue));
    m_pTButtonBrush->SetOpacity(1.0f);
}

InvitationCardUI::~InvitationCardUI()
{

}

void InvitationCardUI::UpdateLabels(const std::wstring& strUIText)
{
    m_vTextBlocks[0] = { strUIText, D2D1::RectF(FRAME_BUFFER_WIDTH - FRAME_BUFFER_WIDTH/3, FRAME_BUFFER_HEIGHT - FRAME_BUFFER_HEIGHT/4,
        FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT), m_pdwTextFormat };
    m_vTextBlocks[1] = { L"수락", D2D1::RectF(FRAME_BUFFER_WIDTH - FRAME_BUFFER_WIDTH / 6 -FRAME_BUFFER_WIDTH / 60 - FRAME_BUFFER_WIDTH / 9, FRAME_BUFFER_HEIGHT - FRAME_BUFFER_HEIGHT/10,
        FRAME_BUFFER_WIDTH - FRAME_BUFFER_WIDTH / 6 - FRAME_BUFFER_WIDTH / 60, FRAME_BUFFER_HEIGHT - 10), m_pdwTextFormat2 };
    m_vTextBlocks[2] = { L"거절", D2D1::RectF(FRAME_BUFFER_WIDTH - FRAME_BUFFER_WIDTH / 6 + FRAME_BUFFER_WIDTH / 60, FRAME_BUFFER_HEIGHT - FRAME_BUFFER_HEIGHT / 10,
        FRAME_BUFFER_WIDTH - FRAME_BUFFER_WIDTH / 6 + FRAME_BUFFER_WIDTH / 60 + FRAME_BUFFER_WIDTH / 9, FRAME_BUFFER_HEIGHT - 10), m_pdwTextFormat2 };
}

void InvitationCardUI::Resize(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight, UINT TextAlignment, UINT ParagraphAlignment)
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
    m_pdwTextFormat2->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_pdwTextFormat2->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

void InvitationCardUI::Render(UINT nFrame)
{

    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

    m_pd2dDeviceContext->BeginDraw();

    m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[0].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[0].d2dLayoutRect, m_pBrush);
    m_pd2dDeviceContext->DrawText(m_vTextBlocks[0].strText.c_str(), static_cast<UINT>(m_vTextBlocks[0].strText.length()),
        m_vTextBlocks[0].pdwFormat, m_vTextBlocks[0].d2dLayoutRect, m_pd2dTextBrush);

    m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[1].d2dLayoutRect, m_pTButtonBrush);
    m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[1].d2dLayoutRect, m_pTButtonBrush);
    m_pd2dDeviceContext->DrawText(m_vTextBlocks[1].strText.c_str(), static_cast<UINT>(m_vTextBlocks[1].strText.length()),
        m_vTextBlocks[1].pdwFormat, m_vTextBlocks[1].d2dLayoutRect, m_pd2dTextBrush);

    m_pd2dDeviceContext->FillRectangle(m_vTextBlocks[2].d2dLayoutRect, m_pTButtonBrush);
    m_pd2dDeviceContext->DrawRectangle(m_vTextBlocks[2].d2dLayoutRect, m_pTButtonBrush);
    m_pd2dDeviceContext->DrawText(m_vTextBlocks[2].strText.c_str(), static_cast<UINT>(m_vTextBlocks[2].strText.length()),
        m_vTextBlocks[2].pdwFormat, m_vTextBlocks[2].d2dLayoutRect, m_pd2dTextBrush);

    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
}

//--------------------------------------------
AddAIUI::AddAIUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor)
    : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{
    m_vTextBlocks.resize(4);
    BG_Rect = D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 10, FRAME_BUFFER_HEIGHT / 2 - FRAME_BUFFER_HEIGHT / 20,
        FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 10, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20);

    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue), (ID2D1SolidColorBrush**)&m_pButtonBrush);
    m_pButtonBrush->SetColor(D2D1::ColorF(D2D1::ColorF::DarkBlue));
    m_pButtonBrush->SetOpacity(1.0f);
}

AddAIUI::~AddAIUI()
{

}

void AddAIUI::UpdateLabels()
{
    m_vTextBlocks[0] = { L"전사", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 10 + FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - FRAME_BUFFER_HEIGHT / 22.5,
        FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 10 + FRAME_BUFFER_WIDTH / 360 + FRAME_BUFFER_WIDTH / 22.5, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - 10), m_pdwTextFormat };
    m_vTextBlocks[1] = { L"탱커", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 360 - FRAME_BUFFER_WIDTH / 22.5, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - FRAME_BUFFER_HEIGHT / 22.5,
        FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - 10), m_pdwTextFormat };
    m_vTextBlocks[2] = { L"마법사", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - FRAME_BUFFER_HEIGHT / 22.5,
        FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 360 + FRAME_BUFFER_WIDTH / 22.5, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - 10), m_pdwTextFormat };
    m_vTextBlocks[3] = { L"서포터", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 10 - FRAME_BUFFER_WIDTH / 360 - FRAME_BUFFER_WIDTH / 22.5 , FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - FRAME_BUFFER_HEIGHT / 22.5,
        FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 10 - FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - 10), m_pdwTextFormat };
}
void AddAIUI::UpdateLabels_JOIN_JOB()
{
    m_vTextBlocks[0] = { L"전사", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 10 + FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - FRAME_BUFFER_HEIGHT / 22.5,
        FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 10 + FRAME_BUFFER_WIDTH / 360 + FRAME_BUFFER_WIDTH / 22.5, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - 10), m_pdwTextFormat };
    m_vTextBlocks[1] = { L"탱커", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 360 - FRAME_BUFFER_WIDTH / 22.5, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - FRAME_BUFFER_HEIGHT / 22.5,
        FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - 10), m_pdwTextFormat };
    m_vTextBlocks[2] = { L"마법사", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - FRAME_BUFFER_HEIGHT / 22.5,
        FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 360 + FRAME_BUFFER_WIDTH / 22.5, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - 10), m_pdwTextFormat };
    m_vTextBlocks[3] = { L"서포터", D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 10 - FRAME_BUFFER_WIDTH / 360 - FRAME_BUFFER_WIDTH / 22.5 , FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - FRAME_BUFFER_HEIGHT / 22.5,
        FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 10 - FRAME_BUFFER_WIDTH / 360, FRAME_BUFFER_HEIGHT / 2 + FRAME_BUFFER_HEIGHT / 20 - 10), m_pdwTextFormat };
}

void AddAIUI::Render(UINT nFrame)
{
    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));
    //--------
    m_pd2dDeviceContext->BeginDraw();
    m_pd2dDeviceContext->FillRectangle(BG_Rect, m_pBrush);
    m_pd2dDeviceContext->DrawRectangle(BG_Rect, m_pBrush);
    for (int i = 0; i < 4; i++)
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
