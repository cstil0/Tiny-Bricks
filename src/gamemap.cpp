#include "image.h"
#include "gamemap.h"
#include "world.h"
#include "camera.h"

Cell::Cell(){}

bool Cell::isEmpty(void) {
	if (this->type == eCellType::EMPTY || this->type == eCellType::FLOOR) {
		return true;
	}
	return false;
}

bool Cell::isObject(void) {
	if (this->type >= int(eCellType::KEY) && this->type <= int(eCellType::COIN)) {
		return true;
	}
	return false;
}

Object::Object() {
	shape = new Image[5];
	animSize = 4;
	collected = false;
}

GameMap::GameMap(int width, int height) {
	this->width = width;
	this->height = height;

	// We create two data arrays (data and colisions_data), so that the estatic elements of the map 
	// never appear or disappear no matter what the moving objects do
	data = new Cell[width * height];
	colisions_data = new Cell[width * height];
	tileset.loadTGA("data/tileset_def.tga");
	objects = new Object[5];

	// Set the position for each object
	objects[0].pos = new Vector2(120, 455);
	objects[1].pos = new Vector2(905, 200);
	objects[2].pos = new Vector2(650, 465);
	objects[3].pos = new Vector2(15, 25);
	objects[4].pos = new Vector2(64, 168);

	// Load the shape of every object
	objects[0].shape[0].loadTGA("data/key1.tga");
	objects[0].shape[1].loadTGA("data/key2.tga");
	objects[0].shape[2].loadTGA("data/key3.tga");
	objects[0].shape[3].loadTGA("data/key4.tga");

	objects[1].shape[0].loadTGA("data/box1.tga");
	objects[1].shape[1].loadTGA("data/box2.tga");
	objects[1].shape[2].loadTGA("data/box3.tga");
	objects[1].shape[3].loadTGA("data/box4.tga");

	objects[2].shape[0].loadTGA("data/flag1.tga");
	objects[2].shape[1].loadTGA("data/flag2.tga");
	objects[2].shape[2].loadTGA("data/flag3.tga");
	objects[2].shape[3].loadTGA("data/flag4.tga");

	objects[3].shape[0].loadTGA("data/torch1.tga");
	objects[3].shape[1].loadTGA("data/torch2.tga");
	objects[3].shape[2].loadTGA("data/torch3.tga");
	objects[3].shape[3].loadTGA("data/torch4.tga");

	objects[4].shape[0].loadTGA("data/coin1.tga");
	objects[4].shape[1].loadTGA("data/coin2.tga");
	objects[4].shape[2].loadTGA("data/coin3.tga");
	objects[4].shape[3].loadTGA("data/coin4.tga");
}

Cell& GameMap::getCell_data(int x, int y) {
	return this->data[x + y * width];
}

Cell& GameMap::getCell_colisions(int x, int y) {
	return this->colisions_data[x + y * width];
}

void GameMap::removeChar(int x, int y) {
	// Set to empty the cells where a character is placed in the colisions_data array
	// We consider the current cell and the next onw (below) since characters always occupy two cells
	this->colisions_data[x + y * width].type = Cell::eCellType::EMPTY;
	this->colisions_data[x + (y + 1) * width].type = Cell::eCellType::EMPTY;
}

void GameMap::moveChar(int x, int y, Cell::eCellType type) {
	// Set a character in a specified cell
	this->colisions_data[x + y * width].type = type;
	this->colisions_data[x + (y + 1) * width].type = type;
}

void GameMap::removeObject(int x, int y) {
	// Set to empty a cell where an object is placed in the colisions_data array
	// Here we only consider the current cell, since objects only occupy one
	this->colisions_data[x + y * width].type = Cell::eCellType::EMPTY;
}

Vector2 GameMap::computeCell(float posx, float posy) {
	// Compute the corresponding cell for a given position in the map
	int cs = tileset.width / 16;
	int celx = posx / cs;
	int cely = posy / cs;
	return Vector2(celx, cely);
}

GameMap* GameMap::loadGameMap(const char* filename){
	struct sMapHeader {
		int w; //width of map
		int h; //height of map
		unsigned char bytes; //num bytes per cell
		unsigned char extra[7]; //filling bytes, not used
	};
	FILE* file = fopen(filename, "rb");
	if (file == NULL) //file not found
		return NULL;
	sMapHeader header; //read header and store it in the struct
	fread(&header, sizeof(sMapHeader), 1, file);
	assert(header.bytes == 1); //always control bad cases!!
	//allocate memory for the cells data and read it
	unsigned char* cells = new unsigned char[header.w * header.h];
	fread(cells, header.bytes, header.w * header.h, file);
	fclose(file); //always close open files
	//create the map where we will store it
	GameMap* map = new GameMap(header.w, header.h);
	for (int x = 0; x < map->width; x++)
		for (int y = 0; y < map->height; y++)
			map->getCell_data(x, y).type = (Cell::eCellType)cells[x + y * map->width];
	delete[] cells; //always free any memory allocated!
	return map;
}

void GameMap::renderMap(Image* framebuffer, Camera* camera) {
	int cs = this->tileset.width / 16;
	//for every cell
	for (int x = 0; x < this->width; ++x)
		for (int y = 0; y < this->height; ++y)
		{
			//get cell info from both maps
			Cell& cell = this->getCell_data(x, y);
			Cell& cell_mov = this->getCell_colisions(x, y);
			if (cell.type == 0) //skip empty
				continue;
			int type = (int)cell.type;

			//compute tile pos in tileset image
			int tilex = (type % 16) * cs; //x pos in tileset
			int tiley = floor(type / 16) * cs; //y pos in tileset
			Area area(tilex, tiley, cs, cs); //tile area

			int screenx = ((x * cs) + camera->pos.x);//place offset here if you want
			int screeny = (y * cs) + camera->pos.y;
			//avoid rendering out of screen stuff
			if (screenx < -cs || screenx > framebuffer->width ||
				screeny < -cs || screeny > framebuffer->height)
				continue;
			//draw region of tileset inside framebuffer

			framebuffer->drawImage(tileset, //image
				screenx, screeny, //pos in screen
				area); //area
		}
}