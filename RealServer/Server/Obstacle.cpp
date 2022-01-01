#include "Obstacle.h"


int Obstacle::get_x()
{
	return _x;
}


int Obstacle::get_y()
{
	return _y;
}

int Obstacle::get_id()
{
	return _id;
}

TRIBE Obstacle::get_tribe()
{
	return _tribe;
}

void Obstacle::set_x(int x)
{
	_x = x;
}

void Obstacle::set_y(int y)
{
	_y = y;
}

void Obstacle::set_id(int id)
{
	_id = id;
}
