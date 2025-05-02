#include <SDL3/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "core/defines.h"
#include "core/math.h"
#include "core/string.h"
#include "core/array.h"
#include "core/span.h"

#include "engine/filesystem.h"
#include "engine/entity.h"
#include "engine/render.h"

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
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Test", 800, 600, SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    fprintf(stderr, "SDL_GetBasePath: %s\n", SDL_GetBasePath());

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