#include "Obstacle.h"
Obstacle::Obstacle()
{
	_tribe = OBSTACLE;
}

Obstacle::Obstacle(int id)
{
	_id = id;
	_tribe = OBSTACLE;
}

void Obstacle::set_x(float x)
{
	_x = x;
}

void Obstacle::set_y(float y)
{
	_y = y;
}

void Obstacle::set_z(float z)
{
	_z = z;
}

void Obstacle::set_id(int id)
{
	_id = id;
}


float Obstacle::get_x()
{
	return _x;
}


float Obstacle::get_y()
{
	return _y;
}

float Obstacle::get_z()
{
	return _z;
}

int Obstacle::get_id()
{
	return _id;
}

TRIBE Obstacle::get_tribe()
{
	return _tribe;
}