#pragma once
#include "UILayer.h"
#include "stdafx.h"
//#include "../RealServer/Server/protocol.h"
#include "../SSUServer/SSUServer/protocol.h"

class Party
{
private:
	int		party_id;
	char	room_name[MAX_NAME_SIZE];
	bool	player_use[4];

public:
	char*	player_name[GAIA_ROOM];
	int		player_id[GAIA_ROOM];
	int		player_lv[GAIA_ROOM];
	JOB		player_job[GAIA_ROOM];
	int		player_cnt;
	int		myId_in_partyIndex;
	DUNGEON_STATE dst;
public:
	Party();
	Party(int r_id);
	~Party();

	void set_party_id(int id);
	void set_room_name(char* name);
	void set_player_name(char* name);

	int get_party_id();
	char* get_room_name();
	void get_player_name(char* name);
};

class PartyUI : public UILayer
{
private:
	D2D1_RECT_F                 BG_Rect;
	D2D1_RECT_F					AI_Rect;
	ID2D1SolidColorBrush		*m_pButtonBrush;
	ID2D1SolidColorBrush		*m_pIndexBrush;

	TextBlock					m_Invite_Text;
	TextBlock					m_AI_Text;
	int							m_room_cnt;

public:
	PartyUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor);
	~PartyUI();

	virtual void ResizeTextBlock(int size);
	virtual void UpdateLabels(const std::wstring* strUIText);
	void UpdateLabels_PartyInfo(const std::wstring* strUIText, Party* p, bool party_enter);
	virtual void Render(UINT nFrame);
};

class PartyInviteUI : public UILayer
{
private:
	IDWriteTextFormat			*m_pdwTextFormat2 = NULL;
	ID2D1SolidColorBrush		*m_pTextLayoutBrush;

public:
	PartyInviteUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor);
	~PartyInviteUI();

	virtual void UpdateLabels(const std::wstring& strUIText);
	virtual void Render(UINT nFrame);
	virtual void Resize(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height, UINT TextAlignment, UINT ParagraphAlignment);
};

class InvitationCardUI : public UILayer
{
private:
	IDWriteTextFormat* m_pdwTextFormat2 = NULL;
	ID2D1SolidColorBrush* m_pTButtonBrush;
public:
	InvitationCardUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor);
	~InvitationCardUI();
	virtual void UpdateLabels(const std::wstring& strUIText);
	virtual void Resize(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height, UINT TextAlignment, UINT ParagraphAlignment);
	virtual void Render(UINT nFrame);
};

class AddAIUI : public UILayer
{
private:
	D2D1_RECT_F                 BG_Rect;
public:
	ID2D1SolidColorBrush* m_pButtonBrush;
	AddAIUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor);
	~AddAIUI();

	virtual void UpdateLabels();
	virtual void UpdateLabels_JOIN_JOB();
	virtual void Render(UINT nFrame);
};
