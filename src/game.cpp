#include "game.h"
#include "utils.h"
#include "input.h"
#include "image.h"
#include "mygame.h"
#include "world.h"

#include <typeinfo>
#include <cmath>


Game* Game::instance = NULL;

Color bgcolor(105, 102, 177);

IntroStage* intro_stage = NULL;
PlayStage* play_stage = NULL;
EndStage* end_stage = NULL;
Stage* current_stage = NULL;

World* world = NULL;

Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;

	intro_stage = new IntroStage();
	play_stage = new PlayStage();
	end_stage = new EndStage();

	// Initialize the variables for each stage
	intro_stage->fontOption.loadTGA("data/bitmap-font-black.tga");
	intro_stage->font.loadTGA("data/bitmap-font-white.tga");
	end_stage->fontOption = intro_stage->fontOption;
	end_stage->font = intro_stage->font;
	play_stage->minifont.loadTGA("data/mini-font-white-4x6.tga");
	play_stage->potion.loadTGA("data/potion.tga");

	// Start at potion level = 20 since it is equal to the width of the bar that shows it
	play_stage->potionLevel = 20.0;
	play_stage->potionVelocity = 2.0;
	play_stage->potionState = PlayStage::POTION_STATE::FULL;

	// By default we start at the intro stage
	current_stage = (Stage*)intro_stage;
	current_stage->type = IntroStage::TYPE::INTRO;

	world = new World();
	world->synth = &synth;

	enableAudio();
	sample = world->synth->playSample("data/main_music.wav", 1, true);
}

void Game::render(void)
{
	Image framebuffer(160, 120);

	// Render the frame according to the current stage render function
	current_stage->render(&framebuffer, time, &bgcolor, world);
	//send image to screen
	showFramebuffer(&framebuffer);
}

void Game::update(double seconds_elapsed)
{
	// Update the game according to the current stage update funciton
	current_stage->update(seconds_elapsed, world);

	// if the completed attribute is true, this means that the player has completed the game, so we show the end screen
	if (current_stage->completed) {
		current_stage->completed = false;
		current_stage = end_stage;
		current_stage->type = Stage::TYPE::END;
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_A))
	{
		// This is a debug key to show the end screen by pressing A
		current_stage = end_stage;
		current_stage->type = Stage::TYPE::END;
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_S))
	{
		// We press S to skip the tutorial shown at the beginning of the game
		world->tutorial->isActive = false;
		world->tutorial->curr_text = 0;
	}

	if (Input::gamepads[0].direction & PAD_UP) //left stick pointing up
	{
		bgcolor.set(0, 255, 0);
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_K)) { 
		// Go directly next to the key cell
		world->player->pos = Vector2(105,455);
		world->camera->pos = Vector2(-45,-395);
	}
}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_KP_ENTER:
			// If we are in the intro stage, we change the current stage according to the user input saved in the currOption variable
			if (current_stage->type == Stage::TYPE::INTRO) {
				if (intro_stage->currOption == IntroStage::OPTION::PLAY) {
					current_stage = play_stage;
					current_stage->type = Stage::TYPE::PLAY;
				}
				else if (intro_stage->currOption == IntroStage::OPTION::EXIT) {
					exit(0);
				}
			}
			// If we are in the end stage, we change the current stage according to the user input
			else if (current_stage->type == Stage::TYPE::END) {
				if (end_stage->currOption == EndStage::OPTION::MENU) {
					current_stage = intro_stage;
					current_stage->type = Stage::TYPE::INTRO;
				}
				else if (end_stage->currOption == EndStage::OPTION::EXIT) {
					exit(0);
				}
			}
			// If we are in the play stage, we need to know if the tutorial mode is on, since then
			// every time the user press the space button the text shown in the screen has to change
			else if (current_stage->type == Stage::TYPE::PLAY) {
				Tutorial* tutorial = world->tutorial;
				if (tutorial->isActive) {
					tutorial->curr_text += 2;
					if (tutorial->curr_text == int(tutorial->curr_type)){
						tutorial->curr_text = 0;
						tutorial->isActive = false;
					}
				}
			}
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Game::onMouseMove(SDL_MouseMotionEvent event)
{
}

void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
}

void Game::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	window_width = width;
	window_height = height;
}

//sends the image to the framebuffer of the GPU
void Game::showFramebuffer(Image* img)
{
	static GLuint texture_id = -1;
	static GLuint shader_id = -1;
	if (!texture_id)
		glGenTextures(1, &texture_id);

	//upload as texture
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);

	glDisable(GL_CULL_FACE); glDisable(GL_DEPTH_TEST); glEnable(GL_TEXTURE_2D);
	float startx = -1.0; float starty = -1.0;
	float width = 2.0; float height = 2.0;

	//center in window
	float real_aspect = window_width / (float)window_height;
	float desired_aspect = img->width / (float)img->height;
	float diff = desired_aspect / real_aspect;
	width *= diff;
	startx = -diff;

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(startx, starty + height);
	glTexCoord2f(1.0, 0.0); glVertex2f(startx + width, starty + height);
	glTexCoord2f(1.0, 1.0); glVertex2f(startx + width, starty);
	glTexCoord2f(0.0, 1.0); glVertex2f(startx, starty);
	glEnd();

	/* this version resizes the image which is slower
	Image resized = *img;
	//resized.quantize(1); //change this line to have a more retro look
	resized.scale(window_width, window_height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	if (1) //flip
	{
	glRasterPos2f(-1, 1);
	glPixelZoom(1, -1);
	}
	glDrawPixels( resized.width, resized.height, GL_RGBA, GL_UNSIGNED_BYTE, resized.pixels );
	*/
}

//AUDIO STUFF ********************

SDL_AudioSpec audio_spec;

void AudioCallback(void*  userdata,
	Uint8* stream,
	int    len)
{
	static double audio_time = 0;

	memset(stream, 0, len);//clear
	if (!Game::instance)
		return;

	Game::instance->onAudio((float*)stream, len / sizeof(float), audio_time, audio_spec);
	audio_time += len / (double)audio_spec.freq;
}

void Game::enableAudio()
{
	SDL_memset(&audio_spec, 0, sizeof(audio_spec)); /* or SDL_zero(want) */
	audio_spec.freq = 48000;
	audio_spec.format = AUDIO_F32;
	audio_spec.channels = 1;
	audio_spec.samples = 1024;
	audio_spec.callback = AudioCallback; /* you wrote this function elsewhere. */
	if (SDL_OpenAudio(&audio_spec, &audio_spec) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}
	SDL_PauseAudio(0);
}

void Game::onAudio(float *buffer, unsigned int len, double time, SDL_AudioSpec& audio_spec)
{
	//fill the audio buffer using our custom retro synth
	synth.generateAudio(buffer, len, audio_spec);
}
