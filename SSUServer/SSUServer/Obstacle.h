#pragma once
#include "stdafx.h"
class Obstacle
{
private:

protected:
    int _sector_id;
    int _id;
    TRIBE _tribe;
    float _x;
    float _y;
    float _z;
public:

    Obstacle();
    Obstacle(int id);

    void set_x(float x);
    void set_y(float y);
    void set_z(float z);
    void set_id(int id);
    void set_sector_id(int id);

    float get_x();
    float get_y();
    float get_z();

    int get_id();
    int get_sector_id();
    TRIBE get_tribe();


};
