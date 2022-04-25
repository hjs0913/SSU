#include "stdafx.h"
#include "PartyUI.h"

Party::Party()
{
    player_cnt = 0;
    dst = DUN_ST_FREE;
    for (int i = 0; i < GAIA_ROOM; i++) {
        player_name[i] = new char[MAX_NAME_SIZE];
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
    BG_Rect = D2D1::RectF(100, 50, 540, 430);

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
    m_vTextBlocks[0] = { L"방만들기", D2D1::RectF(140, 360, 205, 400), m_pdwTextFormat };
    m_vTextBlocks[1] = { L"방나가기", D2D1::RectF(215, 360, 280, 400), m_pdwTextFormat };
    m_vTextBlocks[2] = { L"초대하기", D2D1::RectF(360, 360, 425, 400), m_pdwTextFormat};
    m_vTextBlocks[3] = { L"AI넣기", D2D1::RectF(435, 360, 500, 400), m_pdwTextFormat };
   
    for (int i = 4; i < m_room_cnt; i++) {
        m_vTextBlocks[i] = { strUIText[i-4], D2D1::RectF(120, 60+50*(i-4), 300, 100 + 50 * (i - 4)), m_pdwTextFormat };
    }
}

void PartyUI::UpdateLabels_PartyInfo(const std::wstring* strUIText, Party* p)
{
    m_vTextBlocks[0] = { L"방만들기", D2D1::RectF(140, 360, 205, 400), m_pdwTextFormat };
    m_vTextBlocks[1] = { L"방나가기", D2D1::RectF(215, 360, 280, 400), m_pdwTextFormat };
    m_vTextBlocks[2] = { L"초대하기", D2D1::RectF(360, 360, 425, 400), m_pdwTextFormat };
    m_vTextBlocks[3] = { L"AI넣기", D2D1::RectF(435, 360, 500, 400), m_pdwTextFormat };

    for (int i = 4; i < m_room_cnt-GAIA_ROOM; i++) {
        m_vTextBlocks[i] = { strUIText[i - 4], D2D1::RectF(120, 60 + 50 * (i - 4), 300, 100 + 50 * (i - 4)), m_pdwTextFormat };
    }

    int tmp = 0;
    for (int i = m_room_cnt - GAIA_ROOM; i < m_room_cnt; i++) {
        wchar_t* temp;
        if (tmp > p->player_cnt) temp = L"";
        else {
            int len = 1 + strlen(p->player_name[tmp]);
            temp = new TCHAR[len];
            mbstowcs(temp, p->player_name[tmp], len);
        }
        m_vTextBlocks[i] = { temp, D2D1::RectF(320, 60 + 50 * (tmp), 520, 100 + 50 * (tmp)), m_pdwTextFormat };
        tmp++;
        delete temp;
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
