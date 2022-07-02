#include "Obstacle.h"
Obstacle::Obstacle()
{
	_tribe = OBSTACLE;
	_sector_id = -1;
}

Obstacle::Obstacle(int id)
{
	_id = id;
	_tribe = OBSTACLE;
	_sector_id = -1;
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

void Obstacle::set_sector_id(int id)
{
	_sector_id = id;
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

int Obstacle::get_sector_id()
{
	return _sector_id;
}

TRIBE Obstacle::get_tribe()
{
	return _tribe;
}