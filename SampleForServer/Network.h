#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <iostream>
#include <stdlib.h>
#include <DirectXMath.h>
#include <array>

#include "../RealServer/Server/protocol.h"
#include "stdafx.h"

#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

using namespace std;


extern int my_id;
extern int m_prev_recv;



void err_quit(const char* msg);

void err_display(const char* msg);

void send_attack_packet(int skill);

void send_move_packet(int direction);

int netInit();

void process_packet(unsigned char* p);

// void network_check();	// ���߿� iocp�ٲ� �� ���

int netclose();

void worker();

void do_send(int num_bytes, void* mess);

void do_recv();

// float my_position2 = -1.0f;
XMFLOAT3 return_myPosition();
XMFLOAT3 return_myCamera();


#include "Camera.h"
#include "Object.h"
void return_otherPlayer(CPlayer** m_otherPlayer, ID3D12Device* m_pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);