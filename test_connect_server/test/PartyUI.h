#pragma once
#include "UILayer.h"

class PartyUI : public UILayer
{
private:
	D2D1_RECT_F                 BG_Rect;
	D2D1_RECT_F					AI_Rect;
	ID2D1SolidColorBrush		*m_pButtonBrush;

	TextBlock					m_Invite_Text;
	TextBlock					m_AI_Text;

public:
	PartyUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor);
	~PartyUI();

	virtual void UpdateLabels(const std::wstring& strUIText, UINT LeftTop_x, UINT LeftTop_y, UINT RightBottom_x, UINT RightBottom_y);
	virtual void Render(UINT nFrame);
};

