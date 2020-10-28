#include "level.h"

#include "filesystem.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define TILE_EMPTY UINT8_MAX

Level* level_create(Renderer* renderer, Camera* camera) {
    Level* level = malloc(sizeof(Level));

    level->_renderer = renderer;
    level->_camera = camera;
    level->name = "";

    level->tile_set.sprite = NULL;
    level->tile_set.palette_size = 0;
    level->tile_set.palette = NULL;
    level->gravity = 22.0f;

    return level;
}

Tile* level_get_tile(Level* level, int x, int y) {
    uint8_t tile_index = level->_tile_map[(level->height - 1 - y) * level->width + x];

    if (tile_index != TILE_EMPTY)
        return &level->tile_set.palette[tile_index];
    else
        return NULL;
}

static void bound_vector(Level* level, Vec2* vec) {
    if (vec->x < 0.0f)
        vec->x = 0.0f;

    if (vec->x > level->width)
        vec->x = level->width;

    if (vec->y < 0.0f)
        vec->y = 0.0f;

    if (vec->y > level->height)
        vec->y = level->height;
}

void level_draw(Level* level) {
    Point screen_size;
    renderer_get_screen_size(level->_renderer, &screen_size);

    // screen position of the tile in the top left corner of the screen.  May be negative if the tile is not fully on screen.
    Point screen_origin = {0,0};

    // The x,y position of the tile occupying the top left corner of the screen.
    Point top_left_tile, bottom_left_tile;

    // world space position of the top left tile.
    Vec2 top_right_world, bottom_left_world;
    camera_screen_pos_to_world_pos(level->_camera, &screen_origin, &top_right_world);
    camera_screen_pos_to_world_pos(level->_camera, &screen_size, &bottom_left_world);

    // get grid coordinates that are seen by camera
    top_right_world.x = floor(top_right_world.x);
    top_right_world.y = ceil(top_right_world.y);
    bottom_left_world.x = ceil(bottom_left_world.x);
    bottom_left_world.y = floor(bottom_left_world.y);

    // ensure that we will not try to draw cells outside the bounds of the level
    bound_vector(level, &top_right_world);
    bound_vector(level, &bottom_left_world);

    point_set_from_vec2(&top_left_tile, &top_right_world);
    point_set_from_vec2(&bottom_left_tile, &bottom_left_world);

    camera_world_pos_to_screen_pos(level->_camera, &top_right_world, &screen_origin);

    int tile_width = sprite_horizontal_frame_size(level->tile_set.sprite);
    int tile_height = sprite_vertical_frame_size(level->tile_set.sprite);

    // curent screen position to draw the tile
    Point screen_pos = screen_origin;

    renderer_begin_tile_drawing(level->_renderer, level->tile_set.sprite);

    for (int y = top_left_tile.y - 1; y >= bottom_left_tile.y; y--) {
        for (int x = top_left_tile.x; x < bottom_left_tile.x; x++) {
            Tile* tile = level_get_tile(level, x, y);

            if (tile) {
                renderer_draw_tile(level->_renderer, tile->sprite_index, screen_pos.x, screen_pos.y);
            }

            screen_pos.x += tile_width;
        }

        screen_pos.x = screen_origin.x;
        screen_pos.y += tile_height;
    }

    renderer_end_tile_drawing(level->_renderer);
}

static int load_tile_set(Level* level, const char* path) {
    uint32_t tilemap_file = filesystem_open(path);
    if (tilemap_file < 0) return 0;

    uint32_t sprite_name_size;
    filesystem_read(&sprite_name_size, sizeof(uint32_t), 1, tilemap_file);
    char* sprite_name = malloc(sprite_name_size + 1);
    filesystem_read(sprite_name, 1, sprite_name_size, tilemap_file);
    sprite_name[sprite_name_size] = '\0';

    level->tile_set.sprite = renderer_load_sprite(level->_renderer, sprite_name);
    free(sprite_name);

    if (level->tile_set.sprite == NULL)
        return 0;

    filesystem_read(&level->tile_set.palette_size, sizeof(uint32_t), 1, tilemap_file);
    level->tile_set.palette = malloc(sizeof(Tile) * level->tile_set.palette_size);
    filesystem_read(level->tile_set.palette, sizeof(Tile), level->tile_set.palette_size, tilemap_file);

    filesystem_close(tilemap_file);

    camera_set_tile_size(level->_camera, sprite_horizontal_frame_size(level->tile_set.sprite), sprite_vertical_frame_size(level->tile_set.sprite));
    return 1;
}

int level_load(Level* level, const char* path) {
    uint32_t level_file = filesystem_open(path);
    if (level_file < 0) return 0;

    // Read level name
    uint32_t size;
    filesystem_read(&size, sizeof(uint32_t), 1, level_file);
    level->name = malloc(size + 1);
    filesystem_read(level->name, 1, size, level_file);
    level->name[size] = '\0';

    // Read tile map
    filesystem_read(&size, sizeof(uint32_t), 1, level_file);
    char* tile_set_name = malloc(size + 1);
    filesystem_read(tile_set_name, 1, size, level_file);
    tile_set_name[size] = '\0';

    // Read Tiles
    filesystem_read(&level->width, sizeof(uint32_t), 1, level_file);
    filesystem_read(&level->height, sizeof(uint32_t), 1, level_file);

    level->_tile_map = malloc(level->width * level->height);
    filesystem_read(level->_tile_map, 1, level->width * level->height, level_file);

    filesystem_close(level_file);

    int loaded_tile_set = load_tile_set(level, tile_set_name);
    free(tile_set_name);

    return loaded_tile_set;
}