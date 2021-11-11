#include "Network.h"
#include "Player.h"

int my_id = 0;
XMFLOAT3 my_position(-1.0f, 5.0f, -1.0f);
XMFLOAT3 my_camera(0.0f, 0.0f, 0.0f);
WSADATA wsa;
SOCKET sock;
SOCKADDR_IN serveraddr;
int retval = 0;

SOCKET g_s_socket;
char g_recv_buf[BUFSIZE];

WSABUF mybuf_recv;
WSABUF mybuf;

bool g_client_shutdown = false;

void CALLBACK send_callback(DWORD err, DWORD num_byte, LPWSAOVERLAPPED send_over, DWORD flag);
void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD flag);

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	cout << "[" << msg << "] " << (char*)lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	MessageBox(NULL, (LPTSTR)lpMsgBuf, (LPTSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void send_attack_packet(int skill)
{
	cs_packet_attack packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_ATTACK;
	packet.skill = (char)skill;
	do_send(sizeof(packet), &packet);
}

void send_move_packet(int direction)
{
	cs_packet_move packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_MOVE;
	packet.direction = (char)direction;
	do_send(sizeof(packet), &packet);
}

void do_send(int num_bytes, void* mess)
{
	char buf[BUFSIZE];
	ZeroMemory(&buf, sizeof(buf));
	memcpy(buf, mess, num_bytes);

	// send()
	DWORD sent_byte;
	mybuf.buf = buf;
	mybuf.len = num_bytes;

	// Overlapped 추가사항
	WSAOVERLAPPED* send_over = new WSAOVERLAPPED;
	ZeroMemory(send_over, sizeof(send_over));

	int ret = WSASend(g_s_socket, &mybuf, 1, &sent_byte, 0, send_over, send_callback);
	if (ret == SOCKET_ERROR) {
		int err_num = WSAGetLastError();
		if (WSA_IO_PENDING != err_num) {
			cout << " EROOR : SEND " << endl;
			err_display("send()");
		}
	}
}

void do_recv()
{
	cout << "recv" << endl;
	// recv()
	mybuf_recv.buf = g_recv_buf;
	mybuf_recv.len = BUFSIZE;
	DWORD recv_flag = 0;

	WSAOVERLAPPED* recv_over = new WSAOVERLAPPED;
	ZeroMemory(recv_over, sizeof(recv_over));

	int ret = WSARecv(g_s_socket, &mybuf_recv, 1, 0, &recv_flag, recv_over, recv_callback);
	if (ret == SOCKET_ERROR) {
		int err_num = WSAGetLastError();
		if (WSA_IO_PENDING != err_num) {
			cout << "에러??" << endl;
			cout << " EROOR : RECV " << endl;
			err_display("recv()");
		}
	}
}

unordered_map<int, CPlayer> mPlayer;
unordered_map<int, bool> check_createShader;

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD flag)
{
	delete recv_over;
	cout << "recvcallback" << endl;
	char* p = g_recv_buf;
	while (p < g_recv_buf + num_bytes) {
		unsigned char packet_size = *p;
		int type = *(p + 1);
		if (packet_size <= 0) break;
		switch (type) {
		case SC_PACKET_LOGIN:
			cout << "login" << endl;
			break;
		case SC_PACKET_MOVE:{
			sc_packet_move* packet = reinterpret_cast<sc_packet_move*>(p);
			my_position.x = packet->x;
			my_position.y = packet->y;
			my_position.z = packet->z;
			cout << packet->x << "." << packet->y << "." << packet->z << endl;
			break;
		}
		case SC_PACKET_LOGOUT:
			cout << "logout" << endl;
			break;
		case SC_PACKET_PUT_OBJECT:
			cout << "put" << endl;
			break;
		case SC_PACKET_ATTACK:
			cout << "attack" << endl;
			break;
		case SC_PACKET_DIED:
			cout << "died" << endl;
			break;
		}
		p = p + packet_size;
	}

	do_recv();
}

void CALLBACK send_callback(DWORD err, DWORD num_byte, LPWSAOVERLAPPED send_over, DWORD flag)
{
	delete send_over;
}

int netInit()
{
	const char* SERVERIP;
	char tempIP[16];
	SERVERIP = "127.0.0.1";

	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (g_s_socket == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVERPORT);
	inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr);
	int ret = connect(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	int err_num = WSAGetLastError();
	if (ret == SOCKET_ERROR) {
		int err_num = WSAGetLastError();
		if (WSA_IO_PENDING != err_num) {
			cout << " EROOR : Connect " << endl;
			err_quit("connect()");

		}
	}

	cs_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOGIN;
	strcpy_s(packet.name, "황천길");
	do_send(sizeof(packet), &packet);

	// Nodelay설정
	int tcp_option = 1;
	setsockopt(g_s_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&tcp_option), sizeof(tcp_option));
	do_recv();
}

int netclose()
{
	// close socket()
	closesocket(sock);

	// 윈속종료
	WSACleanup();
	return 0;
}

XMFLOAT3 return_myPosition() {
	return my_position;
}

void return_otherPlayer(CPlayer** m_otherPlayer, ID3D12Device* m_pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {
	for (auto& k : mPlayer) {
		if (check_createShader[k.first] == true) {
			m_otherPlayer[k.first]->SetPosition(k.second.GetPosition());
			m_otherPlayer[k.first]->Render(pd3dCommandList, pCamera);
		}
		if (check_createShader[k.first] == false) {
			/*check_createShader.erase(k.first);
			mPlayer.erase(k.first);*/
		}
	}
}

XMFLOAT3 return_myCamera() {
	return my_camera;
}