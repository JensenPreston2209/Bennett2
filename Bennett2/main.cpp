#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "world.h"
#include <iostream>

int main() {
    // In SDL3, SDL_Init returns true on success, false on failure
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL Init Error: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Pocket Animals", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cout << "Window Creation Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cout << "Renderer Creation Error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderLogicalPresentation(renderer, 800, 600, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Optional standalone image load test (automatic initialization in SDL3)
    //SDL_Surface* surface = IMG_Load("image.png");
    //if (!surface) {
    //    SDL_Log("IMG_Load test failed: %s", SDL_GetError());
    //}
    //else {
    //    SDL_DestroySurface(surface); // Clean up the test surface if it succeeded
    //}

    // Initialize your world object
    World world(renderer);
    world.loadTileset("assets/tiles/tileset.png", 32, 32);
    world.loadSimpleMap();

    Uint64 last = SDL_GetTicks();
    bool running = true;

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT)
                running = false;
        }

        Uint64 now = SDL_GetTicks();
        // Convert to seconds
        float delta = (now - last) / 1000.0f;
        last = now;

        // Get keyboard state (SDL3 uses const bool*)
        const bool* keys = SDL_GetKeyboardState(nullptr);
        world.update(delta, keys);

        // Render loop
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);

        world.render(0, 0);

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}