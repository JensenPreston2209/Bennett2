#pragma once
#include <string>
#include "TileMap.h"
#include "Player.h"

class World {
public:
    World(SDL_Renderer* renderer);

    void loadTileset(const std::string& path, int tileW, int tileH);
    void loadSimpleMap();

    void update(float delta, const bool* keys);
    void render(int camX, int camY);

private:
    SDL_Renderer* renderer;
    TileMap* map = nullptr;
    Player* player = nullptr;
};
