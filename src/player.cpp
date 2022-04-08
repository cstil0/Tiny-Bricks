#include "player.h"

Character::Character() {
	// We have 6 possible animation frames
	animSize = 6;

	// We always have 4 images for each player (one for each direction)
	walking_shape = new Image[4];
	idle_shape = new Image[4];
	playerSize = Vector2(16,17);
}

void Character::setWalkingShape(const char* filename_down, const char* filename_left, const char* filename_right, const char* filename_up)
{
	walking_shape[0].loadTGA(filename_down);
	walking_shape[1].loadTGA(filename_left);
	walking_shape[2].loadTGA(filename_right);
	walking_shape[3].loadTGA(filename_up);
}

void Character::setIdleShape(const char* filename_down, const char* filename_left, const char* filename_right, const char* filename_up)
{
	idle_shape[0].loadTGA(filename_down);
	idle_shape[1].loadTGA(filename_left);
	idle_shape[2].loadTGA(filename_right);
	idle_shape[3].loadTGA(filename_up);
}


