#pragma once
#include "stdafx.h"
class EXP_OVER
{
public:
    WSAOVERLAPPED   _wsa_over;
    COMP_OP         _comp_op;
    WSABUF          _wsa_buf;
    unsigned char   _net_buf[BUFSIZE];
    int             _target;
public:
    EXP_OVER(COMP_OP comp_op, char num_bytes, void* mess);

    EXP_OVER(COMP_OP comp_op) : _comp_op(comp_op) {}

    EXP_OVER();

    ~EXP_OVER();
};

