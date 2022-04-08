#ifndef WORLD_H
#define WORLD_H

#include "utils.h"
#include "gamemap.h"
#include "player.h"
#include "camera.h"
#include "synth.h"

#include <string>
using namespace std;

class Tutorial {
public:

	enum TYPE {
		INTRO = 30,
		KEY = 4
	};

	bool isActive;
	int num_text_intro;
	int num_text_key;
	int curr_text;
	TYPE curr_type;

	string* text_intro;
	string* text_key;

	Tutorial();
};


class World {
public:
	Player* player;
	Enemy* enemy;
	int num_enemies;
	GameMap* map;
	Camera* camera;
	Tutorial* tutorial;
	Synth* synth;
	Image* title_screen;
	World();
	void initializeEnemies();
};


#endif 
