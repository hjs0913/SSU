#include "SocketManager.h"

SocketManager::SocketManager()
{

}

void SocketManager::CloseSocket()
{
    closesocket(socket);
}

SOCKET* SocketManager::get_socket()
{
    return &socket;
}

MainSocketManager::MainSocketManager()
{
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 2), &WSAData);
    socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
    SOCKADDR_IN server_addr;
    ZeroMemory(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    listen(socket, SOMAXCONN);

    h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), h_iocp, 0, 0);
}

HANDLE* MainSocketManager::get_iocp()
{
    return &h_iocp;
}