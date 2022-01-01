#pragma once
#include "stdafx.h"
class Obstacle
{
private:

protected:
    int _id;
    TRIBE _tribe;
    short _x;
    short _y;
public:

    Obstacle() {
        _tribe = OBSTACLE;
    }

    int get_x();
    int get_y();
    int get_id();
    TRIBE get_tribe();


    void set_x(int x);
    void set_y(int y);
    void set_id(int id);
};
