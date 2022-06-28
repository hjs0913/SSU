#pragma once
#include "ObjectManager.h"
class PacketManager
{
private:
	ObjectManager* m_ObjectManger;
	HANDLE* h_iocp;

public:
	PacketManager(ObjectManager* objectManager, HANDLE* iocp);
	void process_packet(int client_id, unsigned char* p);
};

