#ifndef GRAPHICS_IS_DEF
#define GRAPHICS_IS_DEF

#include <SDL.h>


#define WIN_WIDTH   1024
#define WIN_HEIGHT  768
#define TILE        64


typedef struct {
  SDL_Texture *texture;
  SDL_Texture **textures;
  int xframes, frames;
  int native_width, native_height;
  int display_width, display_height;
} sprite_t;


#define GFX_NONE      0
#define GFX_BLINK     1
#define GFX_FADE      2
#define GFX_BLEND     4
#define GFX_CENTER    8
#define GFX_SCALE    16


typedef struct {
  int type;
  int state;
  int x, y; // position
  int xs, ys; // speed
  int direction; // 0 = right, 1 = left
  int wounded;
  int gfx;
  Uint32 alpha;
  double scale;
  int current_sprite;
  int tempo;
  int sound_channel;
  sprite_t *sprite;
  unsigned refcount;
} mobile_object_t;


extern sprite_t mario_sprite,
  missile_sprite,
  explosion_sprite,
  edit_sprite,
  bomb_sprite,
  mine_sprite,
  ouch_sprite,
  yeeha_sprite;


void graphics_init (Uint32 render_flags);
void graphics_map_begin (unsigned map_objects);
void graphics_map_add (char *png_file, int frames);
void graphics_map_end (void);
void graphics_intro (void);
void graphics_render (void);
void renderObject (mobile_object_t *obj);
void graphics_clean (void);

#endif
