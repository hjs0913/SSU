#pragma once
#include "stdafx.h"
#include "EXP_OVER.h"
class TimerManager
{
private:
	HANDLE* h_iocp;
public:
	TimerManager(HANDLE* iocp);
	~TimerManager() {};

	static concurrency::concurrent_priority_queue<timer_event> timer_queue;

	COMP_OP EVtoOP(EVENT_TYPE ev);

	void do_timer();
};

