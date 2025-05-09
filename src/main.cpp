#include <SDL3/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "core/core.h"
#include "engine/engine.h"

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
    init_engine("../assets/");
    init_window("Zeon", 800, 600);

    Model model = load_gltf("models/Barbarian.glb");

    World world{};

    Entity e1 = spawn(world, Player{1});
    Entity e2 = spawn(world, Enemy{2});

    add_component(world, e1, MeshComponent{ 6 });
    add_component(world, e2, MeshComponent{ 7 });

    while (!should_window_close()) {
        
    }

    deinit_engine();

    return 0;
}