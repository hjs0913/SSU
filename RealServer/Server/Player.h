#pragma once
#include "EXP_OVER.h"
#include "Npc.h"
#include "SkillBuf.h"
class Player : public Npc
{
protected:
    int                 _login_id;
    int		            _exp;
    JOB                 _job;
    int                 _mp;
    int                 _max_mp;

    atomic_bool	        _attack_active;		// NPC가 가만히 안있고 움직일때
    atomic_bool         _skill_active[3] = { false };
public:
    atomic_bool         _auto_hp = false;
	SOCKET				_socket;
	EXP_OVER			_recv_over;
	int					_prev_size;

	mutex		        vl;
	unordered_set<int>	viewlist;

    mutex               ob_vl;
    unordered_set<int>  ob_viewlist;

    int                 last_move_time;
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
                wcout << lpMsgBuf << endl;
                //while (true);
                LocalFree(lpMsgBuf);
            }
        }
    }

    void do_send(int num_bytes, void* mess)
    {
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
                wcout << lpMsgBuf << endl;
                LocalFree(lpMsgBuf);
            }
        }
    }

    // --------------------------
    void set_login_id(int login_id)
    {
        _login_id = login_id;
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

    int get_login_id() {
        return _login_id;
    }

    bool get_attack_active() {
        return _attack_active;
    }

    bool get_skill_active(int skill_type)
    {
        return _skill_active[skill_type];
    }

    int get_exp();

    JOB get_job();

    int get_Pmp();
    void set_Pmp(int mp);
    int get_Pmaxmp();
    void set_Pmaxmp(int mp);
 
};
