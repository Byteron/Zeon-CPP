#include <SDL3/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "core/core.h"
#include "engine/engine.h"


struct Player : Entity {
    float speed{};
};

struct Enemy : Entity {
    float speed{};
};

struct Renderable : Entity {
    Model* model;
};

int main() {
    init_engine("../assets/");
    init_window("Zeon", 1920, 1080);

    Model model1 = load_gltf("models/Barbarian.glb");
    Model model2 = load_gltf("models/Rogue_Hooded.glb");

    World world{};

    spawn(world, Player{ .speed = 0.007f }, { .position = { 0, 0, -5 } }, Renderable{ .model = &model1 });
    spawn(world, Enemy{ .speed = -0.01f }, { .position = { 3, -1, -7 } }, Renderable{ .model = &model2 });
    spawn(world, Enemy{ .speed = 0.02f }, { .position = { -3, -1, -7 } }, Renderable{ .model = &model2 });

    Array<Mat4> skinning_matrices{};

    while (!should_window_close()) {
        for (auto& player : get<Player>(world)) {
            Quat y_rotation = quat_from_axis_angle({0.0f, 1.0f, 0.0f}, player.speed);
            player.transform.rotation = normalize(y_rotation * player.transform.rotation);
        }

        for (auto& enemy : get<Enemy>(world)) {
            Quat y_rotation = quat_from_axis_angle({0.0f, 1.0f, 0.0f}, enemy.speed);
            enemy.transform.rotation = normalize(y_rotation * enemy.transform.rotation);
        }

        for (auto& renderable : get<Renderable>(world)) {
            Entity& parent = get_parent(world, renderable);

            draw_model(renderable.model, to_mat4(parent.transform), skinning_matrices);
        }
    }

    deinit_engine();

    return 0;
}