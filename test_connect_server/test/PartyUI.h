#pragma once
#include "UILayer.h"
#include "../../RealServer/Server/protocol.h"

class Party
{
private:
	int		party_id;
	char	room_name[MAX_NAME_SIZE];

	char*	player_name[GAIA_ROOM];

	bool	player_use[4];
public:
	int		player_id[GAIA_ROOM];
	int		player_lv[GAIA_ROOM];
	JOB		player_job[GAIA_ROOM];
	int		player_cnt;
	DUNGEON_STATE dst;
public:
	Party();
	Party(int r_id);
	~Party();

	void set_party_id(int id);
	void set_room_name(char* name);
	void set_player_name(char* name);

	void get_party_id(int id);
	void get_room_name(char* name);
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
	virtual void UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
	virtual void Render(UINT nFrame);
};
