#include "mygame.h"
#include "image.h"
#include "game.h"
#include "utils.h"
#include "world.h"
#include "input.h"
#include "gamemap.h"
#include "player.h"
#include<math.h>

Stage::Stage() {
	completed = false;
}

Enemy* PlayStage::findEnemy(World* world) {
	Player* player = world->player;
	// Set a big distance to make sure that the first one we compute is lower
	float dist = 80000;
	Enemy* enemy;
	for (int i = 0; i < world->num_enemies; i++) {
		Enemy* curr_enemy = &world->enemy[i];

		// We need to find which is the enemy with which we have colided, so we compute de distance to all of them and find the minimum one
		Vector2 en_pos = curr_enemy->pos;
		Vector2 pl_pos = player->pos;
		float curr_dist = pow((en_pos.x - pl_pos.x), 2) + pow((en_pos.y - pl_pos.y), 2);
		if (dist > curr_dist) {
			dist = curr_dist;
			enemy = curr_enemy;
		}
	}
	return enemy;
}

void PlayStage::handlePlayerCollision(Cell cell, int celx, int cely, World* world) {
	// Function to handle the player collisions
	GameMap* map = world->map;
	Player* player = world->player;
	if (cell.type == Cell::eCellType::ENEMY) {
		// First we find the enemy with which we have collided
		Enemy* enemy = findEnemy(world);
		if (potionState == POTION_STATE::ACTIVE) {
			// If the potion is active, the enemy dies
			// The enemy can be at the current cell or the below one, since they occupy 2 cells 
			if (map->getCell_colisions(celx, cely - 1).type == Cell::ENEMY) {
				map->removeChar(celx, cely - 1);
				enemy->isAlive = false;
				world->synth->playSample("data/enemy_dead.wav", 1, false);
			}
			else {
				map->removeChar(celx, cely);
				enemy->isAlive = false;
				world->synth->playSample("data/enemy_dead.wav", 1, false);
			}
		}
		else {
			// If the potion is not active the character dies
			player->gameOver = true;
			world->synth->playSample("data/lose.wav", 0.3, false);
		}
	}
	// If the cell is not occupied by an enemy, it can be one of the objects we need to collect
	else if (cell.isObject()) {
		if (cell.type == Cell::eCellType::KEY) {
			// If the object is the key, we check if all of the objects have been collected
			bool allObjects = true;
			for (int i = 1; i < 5; i++) {
				allObjects = allObjects && map->objects[i].collected;
			}
			// If they are collected, then the player has completed the game
			if (allObjects) {
				this->completed = true;
			}
			else {
				// If they are not collected, we show a message
				world->tutorial->isActive = true;
				world->tutorial->curr_type = Tutorial::TYPE::KEY;
			}
		}
		else {
			//If we have colided with a normal object we remove it from the map and label it as collected
			// Substract 10 because objects go from 10 to 13
			Object* object = &map->objects[(cell.type) - 10];
			object->collected = true;
			object->pos = NULL;
			map->removeObject(celx, cely);
			world->synth->playSample("data/collect.wav", 1.5, false);
		}
	}
	else {
		// If it is none of the options above, we play a sound to notify a simple collision
		world->synth->playSample("data/colision.wav", 0.03, false);
	}
}

void PlayStage::handleEnemyCollision(Cell cell, int celx, int cely, World* world, int num_enemy) {
	// Function to handle the enemies collisions
	GameMap* map = world->map;
	Player* player = world->player;
	Enemy* enemy = &world->enemy[num_enemy];
	if (cell.type == Cell::eCellType::CHARACTER) {
		if (potionState == POTION_STATE::ACTIVE) {
			// If the enemy collided with the player and the potion is active, then the enemy dies
			if (map->getCell_colisions(celx, cely - 1).type == Cell::ENEMY) {
				map->removeChar(celx, cely - 1);
				enemy->isAlive = false;
				world->synth->playSample("data/enemy_dead.wav", 1, false);
			}
			else {
				map->removeChar(celx, cely);
				enemy->isAlive = false;
				world->synth->playSample("data/enemy_dead.wav", 1, false);
			}
		}
		else {
			// Else the player dies
			player->gameOver = true;
			world->synth->playSample("data/lose.wav", 0.3, false);
		}
	}
}

bool PlayStage::isValid(float posx, float posy, World* world) {
	// Check if a given position is valid, this is, if the corresponding cell is empty
	GameMap* map = world->map;

	// convert from screen coordinates to map
	bool empty = true;
	bool currEmpty;
	Cell cell;
	int celx; int cely;

	int cs = map->tileset.width / 16;
	int playerw = world->player->playerSize.x;
	int playerh = world->player->playerSize.y;

	// Check the four borders of the player to see if there is a collision
	for (int x = 0; x < playerw; x++) {
		// Check the left border
		celx = (posx + x) / cs;
		cely = posy / cs;
		cell = map->getCell_colisions(celx, cely);

		currEmpty = cell.isEmpty();
		if (!currEmpty) {
			handlePlayerCollision(cell, celx, cely, world);
		}
		// If one of the 4 borders is false (non valid), then we have a collision
		empty = empty && cell.isEmpty();

		// Check the right border
		int y = playerh - 1;
		celx = (posx + x) / cs;
		cely = (posy + y) / cs;

		cell = map->getCell_colisions(celx, cely);
		currEmpty = cell.isEmpty();

		if (!cell.isEmpty()) {
			handlePlayerCollision(cell, celx, cely, world);
		}
		empty = empty && currEmpty;
	}

	for (int y = 0; y < playerh; y++) {
		// Check the upper border
		celx = posx / cs;
		cely = (posy + y) / cs;
		cell = map->getCell_colisions(celx, cely);

		currEmpty = cell.isEmpty();

		if (!cell.isEmpty()) {
			handlePlayerCollision(cell, celx, cely, world);
		}
		empty = empty && cell.isEmpty();

		// Check the bottom border
		int x = playerw - 1;
		celx = (posx + x) / cs;
		cely = (posy + y) / cs;
		cell = map->getCell_colisions(celx, cely);

		currEmpty = cell.isEmpty();

		if (!cell.isEmpty()) {
			handlePlayerCollision(cell, celx, cely, world);
		}
		empty = empty && cell.isEmpty();
	}

	return empty;
}

bool PlayStage::isValid_Enemy(float posx, float posy, World* world, int num_enemy) {
	// Same function as the above one, but now we check if a given position is valid for an enemy
	// Since the way to handle enemy collisions is different than for player ones
	GameMap* map = world->map;
	// convert from screen coordinates to map
	bool empty = true;
	bool currEmpty;
	Cell cell;
	int celx; int cely;

	int cs = map->tileset.width / 16;
	int playerw = world->player->playerSize.x;
	int playerh = world->player->playerSize.y;

	for (int x = 0; x < playerw; x++) {
		celx = (posx + x) / cs;
		cely = posy / cs;
		cell = map->getCell_colisions(celx, cely);

		currEmpty = cell.isEmpty();
		if (!cell.isEmpty()) {
			handleEnemyCollision(cell, celx, cely, world, num_enemy);
		}

		empty = empty && cell.isEmpty();

		int y = playerh - 1;
		celx = (posx + x) / cs;
		cely = (posy + y) / cs;

		cell = map->getCell_colisions(celx, cely);
		currEmpty = cell.isEmpty();

		if (!cell.isEmpty()) {
			handleEnemyCollision(cell, celx, cely, world, num_enemy);
		}

		empty = empty && currEmpty;
	}

	for (int y = 0; y < playerh; y++) {
		celx = posx / cs;
		cely = (posy + y) / cs;
		cell = map->getCell_colisions(celx, cely);

		currEmpty = cell.isEmpty();
		if (!cell.isEmpty()) {
			handleEnemyCollision(cell, celx, cely, world, num_enemy);
		}

		empty = empty && cell.isEmpty();

		int x = playerw - 1;
		celx = (posx + x) / cs;
		cely = (posy + y) / cs;
		cell = map->getCell_colisions(celx, cely);

		currEmpty = cell.isEmpty();
		if (!cell.isEmpty()) {
			handleEnemyCollision(cell, celx, cely, world, num_enemy);
		}

		empty = empty && cell.isEmpty();
	}

	return empty;
}

Vector2 PlayStage::detectColisions(float posx, float posy, float targetx, float targety, World* world) {
	// Return the new position of the character depending on whether there is a collision or not in the target position
	if (isValid(targetx, targety, world)) {
		return Vector2(targetx, targety);
	}
	else if (isValid(targetx, posy, world)) {
		return Vector2(targetx, posy);
	}
	else if (isValid(posx, targety, world)) {
		return Vector2(posx, targety);
	}
	return Vector2(posx, posy);
}

void PlayStage::updateEnemyPosition(World* world, float dt) {
	// Function for updating the enemies position in every frame
	GameMap* map = world->map;

	// Iterate over each enemy
	for (int i = 0; i < world->num_enemies; i++) {
		Enemy* enemy = &world->enemy[i];
		// If the enemy is not alive we do not need to compute its new position
		if (enemy->isAlive) {
			float en_posx = enemy->pos.x;
			float en_posy = enemy->pos.y;
			float en_vel = enemy->playerVelocity;

			int en_dirx = enemy->direction_vec.x;
			int en_diry = enemy->direction_vec.y;

			// Compute the new position considering the current direction of the enemy
			float en_targetx = en_posx + en_dirx * en_vel * dt;
			float en_targety = en_posy + en_diry * en_vel * dt;
			Vector2 cell_en = map->computeCell(en_posx, en_posy);

			// Remove the enemy from its current position
			map->removeChar(int(cell_en.x), int(cell_en.y));

			// If the new position is valid we keep walking without changing the position
			if (isValid_Enemy(en_targetx, en_targety, world, i)) {
				enemy->pos = Vector2(en_targetx, en_targety);
				cell_en = map->computeCell(en_targetx, en_targety);
				map->moveChar(int(cell_en.x), int(cell_en.y), Cell::eCellType::ENEMY);
			}
			// If it is not valid we try to move in the other directions and see if the adjacent cell is empty
			else {
				en_targetx = en_posx;
				en_targety = en_posy + en_vel * dt;
				if (isValid_Enemy(en_targetx, en_targety, world, i)) {
					cell_en = map->computeCell(en_targetx, en_targety);
					enemy->pos = Vector2(en_targetx, en_targety);
					enemy->dir = Character::DIRECTION::DOWN;
					enemy->direction_vec = Vector2(-1,0);
					map->moveChar(int(cell_en.x), int(cell_en.y), Cell::eCellType::ENEMY);
				}
				else {
					en_targetx = en_posx + en_vel * dt;
					en_targety = en_posy;
					if (isValid_Enemy(en_targetx, en_targety, world, i)) {
						cell_en = map->computeCell(en_targetx, en_targety);
						enemy->pos = Vector2(en_targetx, en_targety);
						enemy->dir = Character::DIRECTION::RIGHT;
						enemy->direction_vec = Vector2(1, 0);
						map->moveChar(int(cell_en.x), int(cell_en.y), Cell::eCellType::ENEMY);
					}
					else {
						en_targetx = en_posx;
						en_targety = en_posy - en_vel * dt;
						if (isValid_Enemy(en_targetx, en_targety, world, i)) {
							cell_en = map->computeCell(en_targetx, en_targety);
							enemy->pos = Vector2(en_targetx, en_targety);
							enemy->dir = Character::DIRECTION::UP;
							enemy->direction_vec = Vector2(0, -1);
							map->moveChar(int(cell_en.x), int(cell_en.y), Cell::eCellType::ENEMY);
						}
						else {
							en_targetx = en_posx - en_vel * dt;
							en_targety = en_posy;
							if (isValid_Enemy(en_targetx, en_targety, world, i)) {
								cell_en = map->computeCell(en_targetx, en_targety);
								enemy->pos = Vector2(en_targetx, en_targety);
								enemy->dir = Character::DIRECTION::LEFT;
								enemy->direction_vec = Vector2(0, 1);
								map->moveChar(int(cell_en.x), int(cell_en.y), Cell::eCellType::ENEMY);
							}
						}
					}
				}
			}
		}
	}
}

void IntroStage::render(Image* framebuffer, float time, Color* bgcolor, World* world) {
	// Paint the letters in one color or another if it corresponds to de current option selected
	framebuffer->fill(*bgcolor);
	framebuffer->drawImage(*world->title_screen,0 ,0);
	framebuffer->drawText("TINY BRICKS", 40, 35, font);
	framebuffer->drawText("Play", 60, 50, currOption == 0 ? fontOption : font);
	framebuffer->drawText("Exit", 60, 65, currOption == 1 ? fontOption : font);
}

void IntroStage::update(double dt, World* world) {
	if (Input::isKeyPressed(SDL_SCANCODE_UP))
	{
		// If we are in the first option we cannot keep going up
		if (currOption != OPTION::PLAY) {
			currOption = OPTION(currOption - 1);
			world->synth->playSample("data/menu_navigate.wav", 1, false);
		}
	}
	if (Input::isKeyPressed(SDL_SCANCODE_DOWN))
	{
		// If we are in the last option we cannot keep going up
		if (currOption != OPTION::EXIT) {
			currOption = OPTION(currOption + 1);
			world->synth->playSample("data/menu_navigate.wav", 1, false);
		}
	}
}

void PlayStage::render(Image* framebuffer, float time, Color* bgcolor, World* world) {
	Player* player = world->player;
	Camera* camera = world->camera;
	GameMap* map = world->map;

	float cam_posx = camera->pos.x;
	float cam_posy = camera->pos.y;
	float pl_posx = player->pos.x;
	float pl_posy = player->pos.y;
	int player_sizex = player->playerSize.x;
	int player_sizey = player->playerSize.y;

	framebuffer->fill(*bgcolor);

	// If true we find wich animation to use depending on the time elapsed, else it set to 0
	int currAnim = int(time * 5.0f) % player->animSize; 
	map->renderMap(framebuffer, camera);
	if (player->isMoving) {
		// To draw the character we substract the camera position to find the position in the screen
		framebuffer->drawImage(player->walking_shape[player->dir], pl_posx + cam_posx, pl_posy + cam_posy, Area(currAnim * player_sizex, 0, player_sizex, player_sizey));
	}
	else {
		framebuffer->drawImage(player->idle_shape[player->dir], pl_posx + cam_posx, pl_posy + cam_posy, Area(currAnim * player_sizex, 0, player_sizex, player_sizey));
	}

	Tutorial* tutorial = world->tutorial;
	// If the tutorial is active, we draw the corresponding text
	if (tutorial->isActive) {
		framebuffer->drawRectangle(0, 100, 160, 50, Color::BLACK);
		if (tutorial->curr_type == Tutorial::TYPE::INTRO) {
			framebuffer->drawText(tutorial->text_intro[tutorial->curr_text], 5, 105, minifont,4,6);
			framebuffer->drawText(tutorial->text_intro[tutorial->curr_text + 1], 5, 112, minifont, 4, 6);
		}
		else {
			framebuffer->drawText(tutorial->text_key[tutorial->curr_text], 5, 105, minifont,4,6);
			framebuffer->drawText(tutorial->text_key[tutorial->curr_text + 1], 5, 112, minifont, 4, 6);
		}
	}
	else {
		for (int i = 0; i < world->num_enemies; i++) {
			Enemy enemy = world->enemy[i];
			float en_posx = enemy.pos.x + cam_posx;
			float en_posy = enemy.pos.y + cam_posy;
			// We only draw the enemy if it is inside the screen and alive
			if (enemy.isAlive && en_posx <= framebuffer->width && en_posy <= framebuffer->height) {
				framebuffer->drawImage(enemy.walking_shape[enemy.dir], en_posx, en_posy, Area(currAnim * player_sizex, 0, player_sizex, player_sizey));
			}
		}
	}
	
	// Render potion rectangle
	// The positions and sizes have been chosen strategically
	framebuffer->drawRectangle(0, 0, 115, 17, Color::BLACK);
	framebuffer->drawLine(0, 0, 0, 17, Color::WHITE);
	framebuffer->drawLine(115, 0, 115, 17, Color::WHITE);
	framebuffer->drawLine(0, 0, 115, 0, Color::WHITE);
	framebuffer->drawLine(0, 17, 115, 17, Color::WHITE);

	framebuffer->drawRectangle(15, 6, int(potionLevel), 6, Color(124, 71, 88));
	framebuffer->drawLine(17, 5, 32, 5, Color::WHITE);
	framebuffer->drawLine(17, 11, 32, 11, Color::WHITE);
	framebuffer->drawCircleRight(32, 8, 3, Color::WHITE);
	framebuffer->drawCircleLeft(17, 8, 3, Color::WHITE);
	framebuffer->drawImage(potion, 0, 0);

	int currAnimOb = int(time * 5.0f) % map->objects[0].animSize;
	for (int i = 0; i < 5; i++) {
		Object object = map->objects[i];
		// If the object position is null, this means that we have already collected it, so we do not want to render it
		if (object.pos) {
			float obPosx = object.pos->x + cam_posx;
			float obPosy = object.pos->y + cam_posy;
			// Draw only if the object is inside the screen
			if (obPosx <= framebuffer->width && obPosy <= framebuffer->height) {
				framebuffer->drawImage(object.shape[currAnimOb], obPosx, obPosy);
			}
		}
		else {
			framebuffer->drawImage(object.shape[0], 25 + (i * 17), 0);
		}
	}
}

void PlayStage::updatePotionState(float dt, World* world) {
	// If the potion is active we have to substract its level considering the velocity
	if (potionState == POTION_STATE::ACTIVE) {
		potionLevel -= potionVelocity * dt;
		// Now, if the current potion level is less than 0, this means it is empty, so now the potion bar has to increase again
		if (potionLevel <= POTION_STATE::EMPTY) {
			potionState = POTION_STATE::CHARGING;
			potionLevel = POTION_STATE::EMPTY;
			world->synth->playSample("data/potion_empty.wav", 1, false);
		}
	}
	// If the potion is already charging we add its level considering the velocity
	else if (potionState == POTION_STATE::CHARGING) {
		potionLevel += potionVelocity * dt;
		if (potionLevel >= POTION_STATE::FULL) {
			potionState = POTION_STATE::FULL;
			potionLevel = POTION_STATE::FULL;
		}
	}
}

void PlayStage::update(double dt, World* world) {
	// If we have the tutorial mode active, we do not want to update anything
	if (!world->tutorial->isActive) {
		Player* player = world->player;
		GameMap* map = world->map;
		Camera* camera = world->camera;
		player->isMoving = false;

		if (Input::isKeyPressed(SDL_SCANCODE_UP)) //if key up
		{
			float posx = player->pos.x;
			float posy = player->pos.y;

			Vector2 cell = map->computeCell(posx, posy);
			// Remove temporally the player from the collision_data array so that it does not detect a collision with itself
			map->removeChar(int(cell.x), int(cell.y));

			// Compute the new position where the player should be placed
			float targety = posy - player->playerVelocity * dt;

			// Detect if there is a collision in the new position
			Vector2 new_pos = detectColisions(posx, posy, posx, targety, world);
			player->dir = player->DIRECTION::UP;
			player->isMoving = true;

			// Compute the camera pos considering the difference between the new player position and the old one
			// So we move the camera the same distance we move the player
			camera->pos -= new_pos - player->pos;
			player->pos = new_pos;

			// Add the player to the map again
			cell = map->computeCell(new_pos.x, new_pos.y);
			map->moveChar(int(cell.x), int(cell.y), Cell::eCellType::CHARACTER);
		}

		if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) //if key down
		{
			float posx = player->pos.x;
			float posy = player->pos.y;

			Vector2 cell = map->computeCell(posx, posy);
			map->removeChar(int(cell.x), int(cell.y));

			float targety = posy + player->playerVelocity * dt;

			Vector2 new_pos = detectColisions(posx, posy, posx, targety, world);
			player->dir = player->DIRECTION::DOWN;
			player->isMoving = true;
			camera->pos -= new_pos - player->pos;
			player->pos = new_pos;

			cell = map->computeCell(new_pos.x, new_pos.y);
			map->moveChar(int(cell.x), int(cell.y), Cell::eCellType::CHARACTER);
		}

		if (Input::isKeyPressed(SDL_SCANCODE_RIGHT)) //if key up
		{
			float posx = player->pos.x;
			float posy = player->pos.y;

			Vector2 cell = map->computeCell(posx, posy);
			map->removeChar(int(cell.x), int(cell.y));

			float targetx = posx + player->playerVelocity * dt;

			Vector2 new_pos = detectColisions(posx, posy, targetx, posy, world);
			player->dir = player->DIRECTION::RIGHT;
			player->isMoving = true;
			camera->pos -= new_pos - player->pos;
			player->pos = new_pos;

			cell = map->computeCell(new_pos.x, new_pos.y);
			map->moveChar(int(cell.x), int(cell.y), Cell::eCellType::CHARACTER);
		}

		if (Input::isKeyPressed(SDL_SCANCODE_LEFT)) //if key down
		{
			float posx = player->pos.x;
			float posy = player->pos.y;

			Vector2 cell = map->computeCell(posx, posy);
			map->removeChar(int(cell.x), int(cell.y));

			float targetx = posx - player->playerVelocity * dt;
			Vector2 new_pos = detectColisions(posx, posy, targetx, posy, world);
			player->dir = player->DIRECTION::LEFT;
			player->isMoving = true;
			camera->pos -= new_pos - player->pos;
			player->pos = new_pos;

			cell = map->computeCell(new_pos.x, new_pos.y);
			map->moveChar(int(cell.x), int(cell.y), Cell::eCellType::CHARACTER);
		}

		if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) {
			if (potionState == POTION_STATE::FULL) {
				potionState = POTION_STATE::ACTIVE;
				world->synth->playSample("data/potion_active.wav", 1, false);
			}
		}

		updateEnemyPosition(world, dt);

		updatePotionState(dt, world);

		// Restart positions if the player died
		if (player->gameOver) {
			// Remove the player from the old position
			Vector2 cell_pl = map->computeCell(player->pos.x, player->pos.y);
			map->removeChar(int(cell_pl.x), int(cell_pl.y));
			player->pos = player->original_pos;

			// Restart the player position
			cell_pl = map->computeCell(player->pos.x, player->pos.y);
			map->moveChar(int(cell_pl.x), int(cell_pl.y), Cell::eCellType::CHARACTER);

			// Restart camera position
			camera->pos = camera->original_pos;
			player->gameOver = false;

			// Restart potion state
			potionState = POTION_STATE::FULL;
			potionLevel = POTION_STATE::FULL;

			// Restart enemies
			for (int i = 0; i < world->num_enemies; i++) {
				Enemy* enemy = &world->enemy[i];
				enemy->isAlive = true;
				// Remove every enemy from the old position
				Vector2 cell_en = map->computeCell(enemy->pos.x, enemy->pos.y);
				map->removeChar(int(cell_en.x), int(cell_en.y));

				// Restart their position
				enemy->pos = enemy->original_pos;
				cell_en = map->computeCell(enemy->pos.x, enemy->pos.y);
				map->moveChar(int(cell_en.x), int(cell_en.y), Cell::eCellType::ENEMY);
			}
		}
	}
}

void EndStage::render(Image* framebuffer, float time, Color* bgcolor, World* world)
{
	framebuffer->fill(*bgcolor);
	framebuffer->drawImage(*world->title_screen, 0, 0);
	framebuffer->drawText("CONGRATULATIONS!", 25, 30, font);
	framebuffer->drawText("You have completed", 15, 45, font);
	framebuffer->drawText("the game", 50, 60, font);
	framebuffer->drawText("Back to menu", 35, 80, currOption == 0 ? fontOption : font);
	framebuffer->drawText("Exit", 65, 95, currOption == 1 ? fontOption : font);
}

void EndStage::update(double dt, World* world)
{
	if (Input::isKeyPressed(SDL_SCANCODE_UP))
	{
		// If we are in the first option we cannot keep going up
		if (currOption != OPTION::MENU) {
			currOption = OPTION(currOption - 1);
			world->synth->playSample("data/menu_navigate.wav", 1, false);
		}
	}
	if (Input::isKeyPressed(SDL_SCANCODE_DOWN))
	{
		// If we are in the last option we cannot keep going up
		if (currOption != OPTION::EXIT) {
			currOption = OPTION(currOption + 1);
			world->synth->playSample("data/menu_navigate.wav", 1, false);
		}
	}
}
