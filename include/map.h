
#ifndef MAP_IS_DEF
#define MAP_IS_DEF

#define MAP_OBJECT_NONE -1

#define MIN_HEIGHT 12
#define MAX_HEIGHT 20
#define MIN_WIDTH  16
#define MAX_WIDTH  1024

#define MAP_OBJECT_AIR           0
#define MAP_OBJECT_SEMI_SOLID    1
#define MAP_OBJECT_SOLID         2

#define MAP_OBJECT_DESTRUCTIBLE  4
#define MAP_OBJECT_COLLECTIBLE   8
#define MAP_OBJECT_GENERATOR     16


void map_init (char *filename);

unsigned map_width (void);
unsigned map_height (void);
unsigned map_objects (void);

int map_get (int x, int y);
void map_set (int x, int y, int object);
void map_set_used (int x, int y);
int map_is_used (int x, int y);

char *map_get_name (int obj);
unsigned map_get_frames (int obj);
int map_get_solidity (int obj);
int map_is_destructible (int obj);
int map_is_collectible (int obj);
int map_collectible (void);
int map_is_floor (int x, int y);
int map_is_tough (int x, int y);
int map_is_generator (int obj);

void map_mine_add (int x, int y);
void map_mine_clear (int x, int y);
int map_is_mine (int x, int y);

void map_object_begin (unsigned nb_objects);
void map_object_add (char *png_file, unsigned frames, unsigned obj_type);
void map_object_end (void);

void map_allocate (int w, int h);
void map_new (unsigned width, unsigned height);
void map_save (char *filename);
void map_load (char *filename);

#endif
