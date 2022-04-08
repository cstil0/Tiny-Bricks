#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "image.h"
#include "utils.h"
#include "camera.h"

class Cell {
public:
	enum eCellType : uint8 {
		EMPTY,
		KEY = 10,
		BOX = 11,
		FLAG = 12,
		TORCH = 13,
		COIN = 14,
		CHARACTER = 15,
		ENEMY = 16,

		FLOOR = 69,

		WALL_TOP_LEFT_CORNER = 1,
		WALL_TOP_RIGHT_CORNER = 3,
		WALL_BOTTOM_LEFT_CORNER = 33,
		WALL_BOTTOM_RIGHT_CORNER = 35,
		WALL_TOP_EDGE = 2,
		WALL_LEFT_EDGE = 17,
		WALL_RIGHT_EDGE = 19,
		WALL_BOTTOM_EDGE = 34,

		WALL_BRICKS_BOTTOM_RIGHT_CORNER = 51,
		WALL_BRICKS_BOTTOM_MID = 50,
		WALL_BRICKS_BOTTOM_LEFT_CORNER = 49,
		WALL_BRICKS_TOP_MID = 18
	};

	eCellType type;
	Cell();
	bool isEmpty(void);
	bool isObject(void);
};

class Object {
public:
	bool collected;
	Vector2* pos;
	Image* shape;
	int animSize;
	Object();
};

class GameMap {
public:
	Image tileset;
	int width;
	int height;
	Vector2 map_origin;
	Cell* data;
	Cell* colisions_data;
	Object* objects;

	GameMap(int width, int height);
	Cell& getCell_data(int x, int y);
	Cell& getCell_colisions(int x, int y);
	void removeChar(int x, int y);
	void moveChar(int x, int y, Cell::eCellType type);
	void removeObject(int x, int y);
	Vector2 computeCell(float posx, float posy);
	GameMap* loadGameMap(const char* filename);
	void renderMap(Image* framebuffer, Camera* camera);
};

#endif 