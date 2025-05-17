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

    Enemy* e = temp<Enemy>();
    Span<int> t = temp_span<int>(100);
    
    reset_temp();

    Model model = load_gltf("models/Barbarian.glb");
    model.transform.position = { 0, 0, -5 };

    World world{};

    Entity e1 = spawn(world, Player{1});
    Entity e2 = spawn(world, Enemy{2});

    add_component(world, e1, MeshComponent{ 6 });
    add_component(world, e2, MeshComponent{ 7 });

    Array<Mat4> skinning_matrices{};

    while (!should_window_close()) {
        // Create a rotation quaternion around Y axis
        Quat y_rotation = quat_from_axis_angle({0.0f, 1.0f, 0.0f}, 0.01f);
        model.transform.rotation = normalize(y_rotation * model.transform.rotation);
        
        draw_model(&model, skinning_matrices);
    }

    deinit_engine();

    return 0;
}