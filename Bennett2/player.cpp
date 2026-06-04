#include "Player.h"
#include "TileMap.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>

Player::Player(SDL_Renderer* renderer) {
    texture = IMG_LoadTexture(renderer, "assets/sprites/player.png");

    if (!texture)
        std::cout << "Failed to load player sprite: " << SDL_GetError() << "\n";
}

void Player::update(float delta, const bool* keys, TileMap* map) {
    moveX = moveY = 0;

    if (keys[SDL_SCANCODE_W]) moveY = -speed * delta;
    if (keys[SDL_SCANCODE_S]) moveY = speed * delta;
    if (keys[SDL_SCANCODE_A]) moveX = -speed * delta;
    if (keys[SDL_SCANCODE_D]) moveX = speed * delta;

    SDL_FRect newX = { x + moveX, y, w, h };
    SDL_FRect newY = { x, y + moveY, w, h };

    if (!map->collides(newX)) x += moveX;
    if (!map->collides(newY)) y += moveY;
}

void Player::render(SDL_Renderer* renderer, float camX, float camY) {
    SDL_FRect dst = { x - camX, y - camY, w, h };
    SDL_RenderTexture(renderer, texture, nullptr, &dst);
}

SDL_FRect Player::getRect() const {
    return SDL_FRect{ x, y, w, h };
}

void Player::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
}
