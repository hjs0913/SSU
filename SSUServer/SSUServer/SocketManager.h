#pragma once
#include "EXP_OVER.h"
class SocketManager
{
protected:
	SOCKET socket;
public:
	SocketManager();
	void CloseSocket();
	SOCKET* get_socket();
};

class MainSocketManager : public SocketManager
{
private:
	HANDLE h_iocp;
public:
	MainSocketManager();

	HANDLE* get_iocp();
	HANDLE accept_player(SocketManager c_socket, int id);
};
