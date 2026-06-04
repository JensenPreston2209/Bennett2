#pragma once
#include <SDL3/SDL.h>

class TileMap;

class Player {
public:
    Player(SDL_Renderer* renderer);

    void update(float delta, const bool* keys, TileMap* map);
    void render(SDL_Renderer* renderer, float camX, float camY);

    SDL_FRect getRect() const;
    void setPosition(float newX, float newY);

    float x = 0, y = 0;
    float w = 32, h = 48;
    float speed = 500.0f;

    float moveX = 0;
    float moveY = 0;

private:
    SDL_Texture* texture = nullptr;
};
