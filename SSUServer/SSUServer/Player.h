#pragma once
#include "EXP_OVER.h"
#include "Npc.h"
#include "SkillBuf.h"

class Gaia;

class Player : public Npc
{
private:
    SOCKET				_socket;
    char                _login_id[MAX_NAME_SIZE];
    float		        _exp;
	EXP_OVER			_recv_over;
	int					_prev_size;
protected:
    JOB                 _job;
    atomic_bool	        _attack_active;		// NPC가 가만히 안있고 움직일때
    atomic_bool         _skill_active[3] = { false };
public:
    atomic_bool         _auto_hp = false;

    mutex               ob_vl;
    unordered_set<int>  ob_viewlist;

    int                 last_move_time;
    bool                superposition;
    bool                join_dungeon_room;
    int                 indun_id;
    int                attack_speed_up;

private:
    void return_npc_position() = delete;
    void do_npc_move() = delete;

public:
    Player(int id);

    ~Player()
    {
        closesocket(_socket);
    }

    void do_recv()
    {
        DWORD recv_flag = 0;
        ZeroMemory(&_recv_over._wsa_over, sizeof(_recv_over._wsa_over));
        _recv_over._wsa_buf.buf = reinterpret_cast<char*>(_recv_over._net_buf + _prev_size);
        _recv_over._wsa_buf.len = sizeof(_recv_over._net_buf) - _prev_size;
        int ret = WSARecv(_socket, &_recv_over._wsa_buf, 1, 0, &recv_flag, &_recv_over._wsa_over, NULL);
        if (SOCKET_ERROR == ret) {
            int error_num = WSAGetLastError();
            if (ERROR_IO_PENDING != error_num) {
                WCHAR* lpMsgBuf;
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL, error_num,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&lpMsgBuf, 0, 0);
                cout << "받기 실패" << endl;
                wcout << lpMsgBuf << endl;
                //while (true);
                LocalFree(lpMsgBuf);
            }
        }
    }

    void do_send(int num_bytes, void* mess)
    {
        if (_tribe != HUMAN) return;
        EXP_OVER* ex_over = new EXP_OVER(OP_SEND, num_bytes, mess);
        int ret = WSASend(_socket, &ex_over->_wsa_buf, 1, 0, 0, &ex_over->_wsa_over, NULL);
        if (SOCKET_ERROR == ret) {
            int error_num = WSAGetLastError();
            if (ERROR_IO_PENDING != error_num) {
                WCHAR* lpMsgBuf;
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL, error_num,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&lpMsgBuf, 0, 0);
                cout << _id << "\t";
                wcout << lpMsgBuf << endl;
                LocalFree(lpMsgBuf);
            }
        }
    }

    // --------------------------
    void set_login_id(char* login_id)
    {
        strcpy_s(_login_id, login_id);
    }
    
    void set_attack_active(bool atk) {
        _attack_active = atk;
    }
    
    void set_skill_active(int skill_type, bool act)
    {
        _skill_active[skill_type] = act;
    }

    void set_exp(int exp);

    void set_job(JOB job);

    void set_indun_id(int id);

    char* get_login_id() {
        return _login_id;
    }

    bool get_attack_active() {
        return _attack_active;
    }

    bool get_skill_active(int skill_type)
    {
        return _skill_active[skill_type];
    }

    int get_indun_id();

    int get_exp();

    JOB get_job();

    int get_Pmp();
    void set_Pmp(int mp);
    int get_Pmaxmp();
    void set_Pmaxmp(int mp);

    int get_prev_size();
    void set_prev_size(int prev_size);

    void accept_initialize();
    void set_socket(SOCKET c_socket);
    void CloseSocketPlayer();

    // 공격
    virtual void attack_dead_judge(Npc* target, float fDamage);	// 죽었는지 아닌지 판정
    virtual void attack_element_judge(Npc* target);	// 공격에 대한 속성 판정
    virtual void basic_attack_success(Npc* target);	// 일반공격 데미지 계산
    virtual void phisical_skill_success(Npc* target, float skill_factor);	// 물리스킬 데미지 계산
    virtual void magical_skill_success(Npc* target, float skill_factor);	// 마법스킬 데미지 계산

    // 부활
    virtual void revive();
    void revive_indun(Gaia* g);
};
