#include "stdafx.h"
#include "SocketManager.h"
#include "ObjectManager.h"
#include "SectorManager.h"
#include "PacketManager.h"
#include "TimerManager.h"
#include "database.h"

int main()
{
   Initialize_DB();

    // 소켓, 네트워크 초기화
	setlocale(LC_ALL, "korean");
    wcout.imbue(locale("korean"));
    MainSocketManager s_socket;

    // DB연결
    // InitializeCriticalSection(&cs);

    SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
    char   accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
    EXP_OVER   accept_ex;
    *(reinterpret_cast<SOCKET*>(&accept_ex._net_buf)) = c_socket;
    ZeroMemory(&accept_ex._wsa_over, sizeof(accept_ex._wsa_over));
    accept_ex._comp_op = OP_ACCEPT;

    AcceptEx(*(s_socket.get_socket()), c_socket, accept_buf, 0, sizeof(SOCKADDR_IN) + 16,
        sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);

    // 초기화 실행
    SectorManager m_SectorManager;
    ObjectManager m_ObjectManager(&m_SectorManager, &s_socket);
    PacketManager m_PacketManager(&m_ObjectManager, &m_SectorManager, s_socket.get_iocp());
    m_ObjectManager.set_packetManager(&m_PacketManager);
    TimerManager m_TimerManager(s_socket.get_iocp());

    //static_ObjectManager::set_objManger(&m_ObjectManager);

    // 멀티 쓰레드 생성
    vector <thread> worker_threads;
    thread timer_thread{ &TimerManager::do_timer, m_TimerManager };
    for (int i = 0; i < 10; ++i)
        worker_threads.emplace_back(std::thread(&ObjectManager::worker, m_ObjectManager));

    for (auto& th : worker_threads)
        th.join();
    timer_thread.join();


    m_ObjectManager.CloseServer();

    // 종료
    s_socket.CloseSocket();
    //DeleteCriticalSection(&cs);
    WSACleanup();
}
