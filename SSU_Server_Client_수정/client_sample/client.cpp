#define SFML_STATIC 1
#define WIN32_LEAN_AND_MEAN //추가 
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <chrono>
#include "protocol.h"
#include <Windows.h>  //추가 


using namespace std;

#ifdef _DEBUG
#pragma comment (lib, "lib/sfml-graphics-s-d.lib")
#pragma comment (lib, "lib/sfml-window-s-d.lib")
#pragma comment (lib, "lib/sfml-system-s-d.lib")
#pragma comment (lib, "lib/sfml-network-s-d.lib")
#else
#pragma comment (lib, "lib/sfml-graphics-s.lib")
#pragma comment (lib, "lib/sfml-window-s.lib")
#pragma comment (lib, "lib/sfml-system-s.lib")
#pragma comment (lib, "lib/sfml-network-s.lib")
#endif
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")

sf::TcpSocket socket;

constexpr auto SCREEN_WIDTH = 8;
constexpr auto SCREEN_HEIGHT = 8;

constexpr auto TILE_WIDTH = 65;
constexpr auto WINDOW_WIDTH = TILE_WIDTH * SCREEN_WIDTH + 10;   // size of window
constexpr auto WINDOW_HEIGHT = TILE_WIDTH * SCREEN_WIDTH + 10;
// constexpr auto BUF_SIZE = MAX_BUFFER;

int g_myid;

sf::RenderWindow* g_window;
sf::Font g_font;


class OBJECT {
private:
	bool m_showing;
	sf::Sprite m_sprite;
	sf::Text m_name;
	sf::Text m_chat;
	chrono::system_clock::time_point m_mess_end_time;
public:
	int m_x, m_y;
	int hp, mp;
	int physical_attack, magical_attack;
	int physical_defense, magical_defense;
	ELEMENT element;
	short level;
	int exp;
	short attack_factor;
	float defense_factor;
	TRIBE tribe;
	bool died = false;

	OBJECT(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
		set_name("NONAME");
		m_mess_end_time = chrono::system_clock::now();
	}
	OBJECT() {
		m_showing = false;
	}
	void show()
	{
		m_showing = true;
	}
	void hide()
	{
		m_showing = false;
	}

	void a_move(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);
	}

	void a_draw() {
		g_window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}
	void draw() {
		if (false == m_showing) return;
		float rx = m_x * 65.0f + 8;
		float ry = m_y * 65.0f + 8;
		m_sprite.setPosition(rx, ry);
		g_window->draw(m_sprite);
		if (m_mess_end_time < chrono::system_clock::now()) {
			m_name.setPosition(rx - 10, ry - 20);
			g_window->draw(m_name);
		}
		else {
			m_chat.setPosition(rx - 10, ry - 20);
			g_window->draw(m_chat);
		}
	}
	void set_name(const char str[]) {
		m_name.setFont(g_font);
		m_name.setString(str);
		m_name.setFillColor(sf::Color(255, 255, 0));
		m_name.setStyle(sf::Text::Bold);
	}
	void set_chat(const char str[]) {
		m_chat.setFont(g_font);
		m_chat.setString(str);
		m_chat.setFillColor(sf::Color(255, 255, 255));
		m_chat.setStyle(sf::Text::Bold);
		m_mess_end_time = chrono::system_clock::now() + chrono::seconds(3);
	}
};

OBJECT avatar;
OBJECT players[MAX_USER];
OBJECT monsters;

OBJECT white_tile;
OBJECT black_tile;

sf::Texture* board;
sf::Texture* pieces;

bool start_attack = false;

void clientInfo_display()
{
	cout << "나의 정보" << endl;
	cout << "Level : " << avatar.level << ", 속성 : ";
	switch (avatar.element)
	{
	case E_WATER: cout << "WATER" << endl;break;
	case E_FULLMETAL: cout << "FULLMETAL" << endl; break;
	case E_WIND: cout << "WIND" << endl; break;
	case E_FIRE: cout << "FIRE" << endl; break;
	case E_TREE: cout << "TREE" << endl; break;
	case E_EARTH: cout << "EARTH" << endl; break;
	case E_ICE: cout << "ICE" << endl; break;
	default:
		break;
	}
	cout << "Hp : " << avatar.hp << ", Mp : " << avatar.mp << endl;
	cout << "물리 공격력 : " << avatar.physical_attack << ", 마법 공격력 : " << avatar.magical_attack << endl;
	cout << "물리 방어력 : " << avatar.physical_defense << ", 마법 방어력 : " << avatar.magical_defense << endl;

	cout << "공격 계수 : " << avatar.attack_factor << ", 방어 계수 : " << avatar.defense_factor << endl;
}

// 클라의 기본 맵 실행
void client_initialize()
{
	board = new sf::Texture;
	pieces = new sf::Texture;
	if (false == g_font.loadFromFile("cour.ttf")) {
		cout << "Font Loading Error!\n";
		while (true);
	}
	board->loadFromFile("chessmap.bmp");
	pieces->loadFromFile("chess2.png");
	white_tile = OBJECT{ *board, 5, 5, TILE_WIDTH, TILE_WIDTH };
	black_tile = OBJECT{ *board, 69, 5, TILE_WIDTH, TILE_WIDTH };
	avatar = OBJECT{ *pieces, 128, 0, 64, 64 };
	for (auto& pl : players) {
		pl = OBJECT{ *pieces, 0, 0, 64, 64 };
	}
	monsters = OBJECT{ *pieces, 64, 0, 64, 64 };
}

void client_finish()
{
	delete board;
	delete pieces;
}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_PACKET_LOGIN:
	{
		sc_packet_login* packet = reinterpret_cast<sc_packet_login*>(ptr);
		g_myid = packet->id;
		avatar.m_x = packet->x;
		avatar.m_y = packet->y;
		avatar.hp = packet->hp;
		avatar.mp = packet->mp;
		avatar.physical_attack = packet->physical_attack;
		avatar.magical_attack = packet->magical_attack;
		avatar.physical_defense = packet->physical_defense;
		avatar.magical_defense = packet->magical_defense;
		avatar.element = packet->element;
		avatar.level = packet->level;
		avatar.exp = packet->exp;
		avatar.attack_factor = packet->attack_factor;
		avatar.defense_factor = packet->defense_factor;
		avatar.tribe = packet->tribe;
		clientInfo_display();
		avatar.move(packet->x, packet->y);
		avatar.show();
	}
	break;
	case SC_PACKET_PUT_OBJECT:
	{
		sc_packet_put_object* my_packet = reinterpret_cast<sc_packet_put_object*>(ptr);
		if (my_packet->tribe == T_HUMAN) {
			int id = my_packet->id;
			if (id < MAX_USER) {
				players[id].set_name(my_packet->name);
				players[id].move(my_packet->x, my_packet->y);
				players[id].show();
			}
		}

		if(my_packet->tribe == T_MONSTER){
			monsters.set_name(my_packet->name);
			monsters.move(my_packet->x, my_packet->y);
			monsters.show();
		}
		break;
	}
	case SC_PACKET_MOVE:
	{
		sc_packet_move * my_packet = reinterpret_cast<sc_packet_move *>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.move(my_packet->x, my_packet->y);
		}
		else if (other_id < MAX_USER) {
			players[other_id].move(my_packet->x, my_packet->y);
		}
		else {
			//npc[other_id - NPC_START].x = my_packet->x;
			//npc[other_id - NPC_START].y = my_packet->y;
		}
		break;
	}

	case SC_PACKET_ATTACK: {
		sc_packet_attack* my_packet = reinterpret_cast<sc_packet_attack*>(ptr);
		avatar.hp = my_packet->p_hp;
		monsters.hp = my_packet->m_hp;
		cout << "플레이어 -> 몬스터 데미지 : " << my_packet->damage_size << endl;
		cout << "플레이어 Hp : " << avatar.hp << endl;
		cout << "몬스터 Hp : " << monsters.hp << endl;
		if (monsters.hp < 0) start_attack = false;

		if (avatar.hp <= 0) {   //죽으면 내꺼에선 사라짐 일단  //추가 
			avatar.died = true;
			avatar.hide();
		}
			

		break;
	}


	case SC_PACKET_LOGOUT:
	{
		sc_packet_logout* my_packet = reinterpret_cast<sc_packet_logout*>(ptr);
		int other_id = my_packet->id;
		if (my_packet->tribe == T_HUMAN) {
			if (other_id == g_myid) {
				avatar.hide();
			}
			else
				players[other_id].hide();
		}
		else{
			monsters.hide();
		}
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUFSIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void client_main()
{
	char net_buf[BUFSIZE];
	size_t	received;

	auto recv_result = socket.receive(net_buf, BUFSIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	for (int i = 0; i < SCREEN_WIDTH; ++i)
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i;
			int tile_y = j;
			if (((tile_x  + tile_y) % 2) == 1) {
				white_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				white_tile.a_draw();
			}
			else
			{
				black_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				black_tile.a_draw();
			}
		}
	avatar.draw();
	for (auto& pl : players) pl.draw();
	monsters.draw();
	sf::Text text;
	text.setFont(g_font);
	char buf[100];
	sprintf_s(buf, "(%d, %d)", avatar.m_x, avatar.m_y);
	text.setString(buf);
	g_window->draw(text);
}

void send_attack_packet()
{
	cs_packet_attack packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_ATTACK;
	packet.skill = 0;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}

void send_move_packet(char dr)
{
	cs_packet_move packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_MOVE;
	packet.direction = dr;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}

void send_login_packet(string &name)
{
	cs_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOGIN;
	strcpy_s(packet.name, name.c_str());
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}

/*
void keep_attacking() //추가 
{
	if (start_attack == true) {

		if (monsters.hp > 0 && avatar.died == false) {
			send_attack_packet();
			Sleep(1000);
			keep_attacking();
		}
		else if (monsters.hp <= 0 || avatar.died == true) {
			start_attack == false;
			return;
		}

	}

		
}*/

int main()
{
	wcout.imbue(locale("korean"));
	sf::Socket::Status status = socket.connect("127.0.0.1", SERVERPORT);	// connect


	socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}

	client_initialize();
	string name{ "PL" };
	auto tt = chrono::duration_cast<chrono::milliseconds>
		(chrono::system_clock::now().
			time_since_epoch()).count();
	name += to_string(tt % 1000);
	send_login_packet(name);	
	avatar.set_name(name.c_str());
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
	g_window = &window;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (start_attack) {
				send_attack_packet();
				break;
			}
			if (event.type == sf::Event::KeyPressed) {
				char p_type = NULL;
				switch (event.key.code) {
				case sf::Keyboard::Left:
					p_type = 2;
					send_move_packet(p_type);
					break;
				case sf::Keyboard::Right:
					p_type = 3;
					send_move_packet(p_type);
					break;
				case sf::Keyboard::Up:
					p_type = 0;
					send_move_packet(p_type);
					break;
				case sf::Keyboard::Down:
					p_type = 1;
					send_move_packet(p_type);
					break;
				case sf::Keyboard::A:  //추가 자동공격: 이 부분 수정 필요 1초마다 공격은 하는데 어느순간 터짐  
					 start_attack = true;
					 if (start_attack == true) {
						 while (avatar.hp > 0 && monsters.hp > 0) {
							 send_attack_packet();
							 if (avatar.hp <= 0 && monsters.hp <= 0) {
								 start_attack == false;
								 break;
							 }
							 Sleep(1000);
						 }
						 start_attack == false;
					 }
					// keep_attacking();
				
					break;
				case sf::Keyboard::Escape:
					window.close();
					break;
				}
			}
		}

		window.clear();
		client_main();
		window.display();
	}
	client_finish();

	return 0;
}
