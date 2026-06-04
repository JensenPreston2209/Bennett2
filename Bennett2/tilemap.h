#pragma once
#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include "SpawnPoint.h"

struct SDL_Renderer;
struct SDL_Texture;

struct ExitTrigger {
    float x, y, w, h;
    std::string exitID;
};

class TileMap {
public:
    TileMap(SDL_Renderer* renderer);
    ~TileMap();

    bool loadJSON(const std::string& path);
    void render(int cameraX, int cameraY);

    bool collides(const SDL_FRect& box) const;
    bool isBlocked(int tileX, int tileY) const;

    ExitTrigger* checkExit(float px, float py, float pw, float ph);
    SpawnPoint* getSpawnByID(const std::string& id);
    SpawnPoint* getDefaultSpawn();

    int tileWidth = 32;
    int tileHeight = 32;

private:
    SDL_Renderer* renderer;
    SDL_Texture* tileset = nullptr;

    int mapWidth = 30*32;
    int mapHeight = 20*32;
    int tilesetCols = 2;

    std::vector<ExitTrigger> exits;
    std::vector<SpawnPoint> spawnPoints;
    std::vector<std::vector<int>> groundLayer;
    std::vector<std::vector<int>> collisionLayer;
};
