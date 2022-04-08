#include "world.h"
#include "game.h"
#include "gamemap.h"
#include "player.h"
#include "camera.h"

World::World() {
	player = new Player();
	tutorial = new Tutorial();

	title_screen = new Image();
	title_screen->loadTGA("data/title_screen.tga");
	num_enemies = 13;
	enemy = new Enemy[num_enemies];

	// Set the position of the player
	player->pos = Vector2(975, 270);
	player->original_pos = Vector2(975, 270);
	player->dir = Player::DIRECTION::DOWN;

	// Set the velocity
	player->playerVelocity = 50.0;

	camera = new Camera();
	map = map->loadGameMap("data/map_play_stage.map");

	// Pass the information charged in the data array when the map is loaded to the colisions_data array
	// By doing this, we can move the characters without losing any information of the original map
	// for example, when the player moves, we would loose the information about where the floor is
	for (int x = 0; x < map->width; x++) {
		for (int y = 0; y < map->height; y++) {
			Cell cell = map->getCell_data(x, y);
			map->colisions_data[x + y * map->width].type = cell.type;
		}
	}

	// Locate the objects in the collisions_data array
	for (int i = 0; i < 5; i++) {
		Object object = map->objects[i];
		Vector2* pos = object.pos;
		Vector2 cell = map->computeCell(int(pos->x), int(pos->y));
		map->colisions_data[(int(cell.x) + int(cell.y) * map->width)].type = Cell::eCellType(i + 10);
	}

	// Locate the player
	Vector2 pos_player = player->pos;
	Vector2 cell = map->computeCell(pos_player.x, pos_player.y);
	map->moveChar(cell.x, cell.y, Cell::eCellType::CHARACTER);

	player->isMoving = false;
	player->gameOver = false;

	// Set the shapes of the player (walking and idle)
	player->setWalkingShape("data/char_walking_down.tga", "data/char_walking_left.tga", "data/char_walking_right.tga", "data/char_walking_up.tga");
	player->setIdleShape("data/char_idle_down.tga", "data/char_idle_left.tga", "data/char_idle_right.tga", "data/char_idle_up.tga");

	initializeEnemies();
}

void World::initializeEnemies() {
	// Set the current position of the enemies in the map. The positions have been chosen strategically, 
	// so that they are displaced along the map and inside the walls
	enemy[0].pos = Vector2(712, 128);
	enemy[1].pos = Vector2(768, 32);
	enemy[2].pos = Vector2(504, 216);
	enemy[3].pos = Vector2(576, 480);
	enemy[4].pos = Vector2(144, 464);
	enemy[5].pos = Vector2(16, 56);
	enemy[6].pos = Vector2(112, 168);
	enemy[7].pos = Vector2(16, 120);
	enemy[8].pos = Vector2(672, 72);
	enemy[9].pos = Vector2(272, 240);
	enemy[10].pos = Vector2(240, 456);
	enemy[11].pos = Vector2(80, 56);
	enemy[12].pos = Vector2(16, 416);

	// Set the initial diretions of the enemies. They have been chosen strategically
	// The direction vector is useful to compute the new position in a better way without needing a specific line for each direction
	enemy[0].direction_vec = Vector2(1, 0);
	enemy[1].direction_vec = Vector2(0, 1);
	enemy[2].direction_vec = Vector2(0, -1);
	enemy[3].direction_vec = Vector2(1, 0);
	enemy[4].direction_vec = Vector2(0, 1);
	enemy[5].direction_vec = Vector2(0, -1);
	enemy[6].direction_vec = Vector2(1, 0);
	enemy[7].direction_vec = Vector2(0, 1);
	enemy[8].direction_vec = Vector2(0, 1);
	enemy[9].direction_vec = Vector2(-1, 0);
	enemy[10].direction_vec = Vector2(0, -1);
	enemy[11].direction_vec = Vector2(-1, 0);
	enemy[12].direction_vec = Vector2(0, 1);

	// The direction is useful to know which animation we have to show in each moment
	enemy[0].dir = Character::DIRECTION::RIGHT;
	enemy[1].dir = Character::DIRECTION::DOWN;
	enemy[2].dir = Character::DIRECTION::UP;
	enemy[3].dir = Character::DIRECTION::RIGHT;
	enemy[4].dir = Character::DIRECTION::DOWN;
	enemy[5].dir = Character::DIRECTION::UP;
	enemy[6].dir = Character::DIRECTION::RIGHT;
	enemy[7].dir = Character::DIRECTION::DOWN;
	enemy[8].dir = Character::DIRECTION::UP;
	enemy[9].dir = Character::DIRECTION::LEFT;
	enemy[10].dir = Character::DIRECTION::UP;
	enemy[11].dir = Character::DIRECTION::LEFT;
	enemy[12].dir = Character::DIRECTION::DOWN;

	enemy[0].setWalkingShape("data/enemy_walking_down.tga", "data/enemy_walking_left.tga", "data/enemy_walking_right.tga", "data/enemy_walking_up.tga");
	for (int i = 0; i < num_enemies; i++){
		// Set the original position of every enemy so that when the player dies, we can recover their initial position
		enemy[i].original_pos = enemy[i].pos;

		// Locate the enemies
		Vector2 pos_enemy = enemy[i].pos;
		Vector2 cell = map->computeCell(pos_enemy.x, pos_enemy.y);
		map->moveChar(cell.x, cell.y, Cell::eCellType::ENEMY);

		// Set the enemies velocity
		enemy[i].playerVelocity = 30.0;

		enemy[i].isAlive = true;

		// Set the shape
		// We have already charged the first enemy shape
		if (i != 0){
			enemy[i].walking_shape = enemy[0].walking_shape;
		}
	}
}

Tutorial::Tutorial() {
	isActive = true;
	num_text_intro = int(TYPE::INTRO);
	num_text_key = INT(TYPE::KEY);
	curr_text = 0;
	curr_type = TYPE::INTRO;

	text_intro = new string[num_text_intro];
	text_intro[0] = "Welcome to Tiny Bricks. If you want";
	text_intro[1] = "to skip this you can press the s key.";
	text_intro[2] = "As you may use the displacement";
	text_intro[3] = "buttons to move throughout the world.";
	text_intro[4] = "The goal is to collect 5 objects by";
	text_intro[5] = "exploring this place.";
	text_intro[6] = "Once you have them, you need to";
	text_intro[7] = "collect a key to complete the game.";
	text_intro[8] = "Every time you find one, it will show";
	text_intro[9] = "in the black rectangle above.";
	text_intro[10] = "But be careful! There are some";
	text_intro[11] = "monsters around here.";
	text_intro[12] = "If you touch one of them";
	text_intro[13] = "you will have to start over.";
	text_intro[14] = "Fortunately, you have a potion that";
	text_intro[15] = "allows you to kill them by touching.";
	text_intro[16] = "You can activate it by pressing";
	text_intro[17] = "the space button";
	text_intro[18] = "but its effect lasts for a limited";
	text_intro[19] = "amount of time.";
	text_intro[20] = "Once its effect is off, you will";
	text_intro[21] = "have to wait activate it again.";
	text_intro[22] = "To know how much time left you have,";
	text_intro[23] = "you can look at the purple bar above,";
	text_intro[24] = "next to the potion.";
	text_intro[25] = "";
	text_intro[26] = "Now you know all you need, so find";
	text_intro[27] = "the first object and leave this room";
	text_intro[28] = "Maybe you should start by going up.";
	text_intro[29] = "Good luck!";

	text_key = new string[num_text_key];
	text_key[0] = "You have some objects left";
	text_key[1] = "to collect.";
	text_key[2] = "Find them first and them come back";
	text_key[3] = "here to complete the game";
}