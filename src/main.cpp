#include "pch.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "core/core.hpp"
#include "engine/engine.hpp"

struct Player {
    int number{};
};

struct Enemy {
    int number{};
};

struct MeshComponent {
    int mesh{};
};

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Test", 800, 600, SDL_WINDOW_OPENGL);
    if (!window) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    std::cout << SDL_GetBasePath() << std::endl;

    Model model = load_gltf("/../assets/models/Barbarian.glb");

    World world{};

    Entity e1 = spawn(world, Player{1});
    Entity e2 = spawn(world, Enemy{2});

    add_component(world, e1, MeshComponent{ 6 });
    add_component(world, e2, MeshComponent{ 7 });

    bool quit = false;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }
        
        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}