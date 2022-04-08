#ifndef MYGAME_H
#define MYGAME_H

#include "image.h"
#include "framework.h"
#include "world.h"
#include "synth.h"


class Stage {
public:
	enum TYPE {
		INTRO,
		PLAY,
		END
	};

	enum POTION_STATE {
		// 20 corresponds to the width of the bar that shows the potion level
		FULL = 20,
		EMPTY = 0,
		ACTIVE = 1,
		CHARGING = 2
	};

	TYPE type;

	bool completed;

	Stage();
	virtual void render(Image* framebuffer, float time, Color* bgcolor, World* world) {};
	virtual void update(double dt, World* world) {};
};

class IntroStage : public Stage {
public:
	enum OPTION {
		PLAY,
		EXIT
	};
	OPTION currOption = OPTION::PLAY;
	Image fontOption;
	Image font;
	IntroStage() :Stage() {};
	virtual void render(Image* framebuffer, float time, Color* bgcolor, World* world);
	virtual void update(double dt, World* world);
};

class PlayStage : public Stage {
public:
	Image minifont;
	Image potion;
	float potionLevel;
	float potionVelocity;
	POTION_STATE potionState;

	PlayStage():Stage() {};
	virtual void render(Image* framebuffer, float time, Color* bgcolor, World* world);
	virtual void update(double dt, World* world);
	Enemy* findEnemy(World* world);
	void updatePotionState(float dt, World* world);
	bool isValid(float posx, float posy, World* world);
	bool isValid_Enemy(float posx, float posy, World* world, int num_enemy);
	void handlePlayerCollision(Cell cell, int celx, int cely, World* world);
	void handleEnemyCollision(Cell cell, int celx, int cely, World* world, int num_enemy);
	Vector2 detectColisions(float posx, float posy, float targetx, float targety, World* world);
	virtual void updateEnemyPosition(World* world, float dt);
};

class EndStage : public Stage {
public:
	enum OPTION {
		MENU,
		EXIT
	};
	OPTION currOption = OPTION::MENU;
	Image fontOption;
	Image font;

	EndStage() :Stage() {};
	virtual void render(Image* framebuffer, float time, Color* bgcolor, World* world);
	virtual void update(double dt, World* world);
};

#endif 