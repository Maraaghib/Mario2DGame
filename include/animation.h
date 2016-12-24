
#ifndef ANIMATION_IS_DEF
#define ANIMATION_IS_DEF

#include <SDL.h>

#include "graphics.h"

extern int x_offset_ground, y_offset_ground;
extern int edit_mode;


void animation_init (void);
void animation_keypressed (SDL_Keycode key);
void animation_one_step (int left, int right, int up, int down);
void animation_render_objects (void);
void animation_timer_expired (void *arg1, void *arg2);
void animation_clean (void);

void animation_objects_save (int fd);
void animation_objects_load (int fd);

#endif
