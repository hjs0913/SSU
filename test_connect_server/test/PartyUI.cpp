#include "stdafx.h"
#include "PartyUI.h"

PartyUI::PartyUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, D2D1::ColorF::Enum LayoutColor, D2D1::ColorF::Enum TextColor) : UILayer(nFrame, pd3dDevice, pd3dCommandQueue, LayoutColor, TextColor)
{

}

PartyUI::~PartyUI()
{

}



