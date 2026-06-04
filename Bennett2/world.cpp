#include "world.h"
#include <iostream>

World::World(SDL_Renderer* renderer)
    : renderer(renderer)
{
    map = new TileMap(renderer);
    player = new Player(renderer);
}

void World::loadTileset(const std::string& path, int tileW, int tileH) {
    map->tileWidth = tileW;
    map->tileHeight = tileH;
}

void World::loadSimpleMap() {
    map->loadJSON("assets/maps/simple_map.json");

    SpawnPoint* sp = map->getDefaultSpawn();
    if (sp)
        player->setPosition(sp->x, sp->y);
}

void World::update(float delta, const bool* keys) {
    player->update(delta, keys, map);
}

void World::render(int camX, int camY) {
    map->render(camX, camY);
    player->render(renderer, camX, camY);
}
