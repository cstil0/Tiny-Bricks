#ifndef PLAYER_H
#define PLAYER_H

#include "utils.h"
#include "image.h"

class Character {
public: 
	enum DIRECTION {
		DOWN,
		LEFT,
		RIGHT,
		UP
	};

	DIRECTION dir;
	Vector2 pos;
	Image* walking_shape;
	Image* idle_shape;
	float playerVelocity;
	int animSize;
	Vector2 original_pos;
	Vector2 playerSize;


	Character();
	void setWalkingShape(const char* filename_down, const char* filename_left, const char* filename_right, const char* filename_up);
	void setIdleShape(const char* filename_down, const char* filename_left, const char* filename_right, const char* filename_up);
};

class Player : public Character{
public:
	bool isMoving;
	bool gameOver;

	Player() : Character() {};
};

class Enemy : public Character {
public:
	Vector2 rand_dir;
	Vector2 direction_vec;
	bool isAlive;

	Enemy() : Character() {};
};

#endif