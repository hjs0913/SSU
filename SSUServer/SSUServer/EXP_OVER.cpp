#include "EXP_OVER.h"


EXP_OVER::EXP_OVER(COMP_OP comp_op, char num_bytes, void* mess) : _comp_op(comp_op)
{
    ZeroMemory(&_wsa_over, sizeof(_wsa_over));
    _wsa_buf.buf = reinterpret_cast<char*>(_net_buf);
    _wsa_buf.len = num_bytes;
    memcpy(_net_buf, mess, num_bytes);
}

EXP_OVER::EXP_OVER()
{
    _comp_op = OP_RECV;
}

EXP_OVER::~EXP_OVER() {}