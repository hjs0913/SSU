#include "TimerManager.h"

concurrency::concurrent_priority_queue<timer_event> TimerManager::timer_queue;

TimerManager::TimerManager(HANDLE* iocp)
{
    h_iocp = iocp;
}

COMP_OP TimerManager::EVtoOP(EVENT_TYPE ev) {
    switch (ev)
    {
    case EVENT_NPC_MOVE:
        return OP_NPC_MOVE;
        break;
    case EVENT_NPC_ATTACK: {
        return OP_NPC_ATTACK;
        break;
    }
    case EVENT_AUTO_PLAYER_HP:
        return OP_AUTO_PLAYER_HP;
        break;
    case EVENT_PLAYER_REVIVE:
        return OP_PLAYER_REVIVE;
        break;
    case EVENT_NPC_REVIVE:
        return OP_NPC_REVIVE;
        break;
    case EVENT_BOSS_MOVE:
        return OP_BOSS_MOVE;
        break;
    case EVENT_BOSS_ATTACK:
        return OP_BOSS_ATTACK;
        break;

    case EVENT_ELEMENT_COOLTIME:
        return OP_ELEMENT_COOLTIME;
        break;
    case EVENT_GAIA_PATTERN:
        return OP_GAIA_PATTERN;
        break;
    case EVENT_PARTNER_MOVE:
        return OP_PARTNER_MOVE;
        break;
    case EVENT_PARTNER_SKILL:
        return OP_PARTNER_SKILL;
        break;
    case EVENT_PARTNER_NORMAL_ATTACK:
        return OP_PARTNER_NORMAL_ATTACK;
        break;
    case EVENT_GAMESTART_TIMER:
        return OP_GAMESTART_TIMER;
        break;
    case EVENT_FINISH_RAID:
        return OP_FINISH_RAID;
        break;
    case EVENT_PLAYER_ATTACK:
        return OP_PLAYER_ATTACK;
        break;
    case EVENT_SKILL_COOLTIME:
        return OP_SKILL_COOLTIME;
        break;
    case EVENT_ELEMENT_FIRE_COOLTIME:
        return OP_ELEMENT_FIRE_COOLTIME;
        break;
    case EVENT_PARTNER_SKILL_STOP:
        return OP_PARTNER_SKILL_STOP;
        break;
    case EVENT_PARTNER_ATTACK_STOP:
        return OP_PARTNER_ATTACK_STOP;
        break;
    }

}

void TimerManager::do_timer()
{
    chrono::system_clock::duration dura;
    const chrono::milliseconds waittime = 10ms;
    timer_event temp;
    bool temp_bool = false;
    while (true) {
        if (temp_bool) {
            temp_bool = false;
            
            EXP_OVER* ex_over = new EXP_OVER;
            ex_over->_comp_op = EVtoOP(temp.ev);
            ex_over->_target = temp.target_id;
            PostQueuedCompletionStatus(*h_iocp, 1, temp.obj_id, &ex_over->_wsa_over);   //0은 소켓취급을 받음
            
        }

        while (true) {
            timer_event ev;
            if (timer_queue.size() == 0) continue;
            timer_queue.try_pop(ev);

            dura = ev.start_time - chrono::system_clock::now();
            if (dura <= 0ms) {
                EXP_OVER* ex_over = new EXP_OVER;
                ex_over->_comp_op = EVtoOP(ev.ev);
                ex_over->_target = ev.target_id;
                PostQueuedCompletionStatus(*h_iocp, 1, ev.obj_id, &ex_over->_wsa_over);   //0은 소켓취급을 받음
            }
            else if (dura <= waittime) {
                temp = ev;
                temp_bool = true;
                break;
            }
            else {
                timer_queue.push(ev);   // 타이머 큐에 넣지 않고 최적화 필요
            }
        }
        this_thread::sleep_for(dura);
        // 쭉 사여있어서 계속 처리를 하도록 해야함
    }
}
