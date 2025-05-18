#include <SDL3/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "core/core.h"
#include "engine/engine.h"



struct Terrain {
    int height{};
    int width{};
};

struct Player {
    int number{};
};

struct Health {
    int health{};
};

struct Position  { float x, y; };
struct Velocity  { float dx, dy; };

struct Renderable {
    int mesh{};
};

struct ATile : Archetype<Terrain, Position, Renderable> {};

struct APlayer : Archetype<Player, Position, Velocity, Renderable> {};
struct APlayer2 : Archetype<Player, Position, Velocity, Renderable> {};

template<typename A, typename... Components>
void spawn_t(World&world, Components... components) {
    uint type_id = get_type_id<A>();
    printf("Type ID: %d\n", type_id);
}

template<typename A, typename T>
void get(World& world, Entity entity) {
    EntityMeta& meta = world.entities[entity.id];
    uint type_id = get_type_id<A>();
    Storage& storage = world.storages[type_id];
    uint type_index = get_archetype_index<A, T>();
    T* data = (T*)storage.data[type_index];
    return data[meta.index];
}

int main() {
    init_engine("../assets/");
    init_window("Zeon", 1920, 1080);

    World world{};

    printf("%d\n", get_type_id<ATile>());
    printf("%d\n", get_type_id<APlayer>());
    printf("%d\n", get_type_id<APlayer2>());

    spawn_t<ATile>(world, Terrain{10, 10}, Position{0, 0}, Renderable{1});
    spawn_t<APlayer>(world, Player{1}, Position{0, 0}, Velocity{1, 1}, Renderable{2});

    uint index1 = index_of<ATile, Renderable>();
    uint index2 = index_of<APlayer, Renderable>();

    Model model = load_gltf("models/Barbarian.glb");
    Transform t1{}, t2{};
    t1.position = { 0, -2, -7 };
    t2.position = { 2, 0, -5 };

    Array<Mat4> skinning_matrices{}; // not currently used

    while (!should_window_close()) {
        Quat y_rotation1 = quat_from_axis_angle({0.0f, 1.0f, 0.0f}, 0.01f);
        Quat y_rotation2 = quat_from_axis_angle({0.0f, 1.0f, 0.0f}, -0.01f);
        t1.rotation = normalize(y_rotation1 * t1.rotation);
        t2.rotation = normalize(y_rotation2 * t2.rotation);
        
        draw_model(&model, to_mat4(t1), skinning_matrices);
        draw_model(&model, to_mat4(t2), skinning_matrices);
    }

    deinit_engine();

    return 0;
}
