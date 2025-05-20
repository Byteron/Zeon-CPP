struct GraphicsPipeline {
    SDL_GPUShader* vertex_shader{};
    SDL_GPUShader* fragment_shader{};

    SDL_GPUSampler* sampler{};

    SDL_GPUGraphicsPipeline* pipeline{};
};

struct Vertex {
    Vec3 position{};
    Vec3 normal{};
    Vec4 uv{};
    Vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
    Vec3 tangent{};
    Vec3 bitangent{};
    float weights[4]{};
    unsigned int joints[4]{};
};

struct Image {
    int width{}, height{};
    int channel_count{};
    byte* data{};
};

struct Texture {
    Image image;
    SDL_GPUTexture* gpu_texture{};
};

struct Material {
    Vec4 albedo{};

    Texture* diffuse_texture{};
    Texture* normal_texture{};
    Texture* specular_texture{};

    int alpha_mode{};

    SDL_GPUShader* shader{};
};

enum MeshType {
    Static,
    Skinned,
};

struct Mesh {
    struct Primitive {
        Array<Vertex> vertices{};
        Array<uint> indices{};

        SDL_GPUBuffer* vertex_buffer{};
        SDL_GPUBuffer* index_buffer{};

        AABB aabb{};

        Material material{};
    };

    MeshType type{};

    AABB aabb{};
    Array<Primitive> primitives{};
};

struct OpaqueStaticRenderData {
    Mat4 transform;
    Mesh::Primitive* primitive;
    AABB aabb;

    float distance_to_camera;
};

struct OpaqueSkinnedRenderData {
    Mat4 transform;
    Mesh::Primitive* primitive;
    AABB aabb;
    Array<Mat4> skinning_matrices;

    float distance_to_camera;
};

struct Engine {
    string name{};

    float time{};
    float delta_time{};

    Array<float> delta_samples{};

    uint frame_count{};

    string root_path{};
    string assets_path{};

    bool quit{};

    SDL_Window* window{};
    SDL_GPUDevice* gpu{};

    SDL_GPUTexture* swapchain_texture{};
    SDL_GPUTexture* depth_texture{};

    GraphicsPipeline solid_skinned_pipeline{};

    Array<Mesh> meshes_to_upload{};
    Array<Texture*> textures_to_upload{};

    Array<OpaqueStaticRenderData> opaque_static_pass{};
    Array<OpaqueSkinnedRenderData> opaque_skinned_pass{};

    int window_width{};
    int window_height{};

    int render_width{};
    int render_height{};

    HashMap<string, Texture*> path_textures{};
    HashMap<byte*, Texture*> data_textures{};

    TemporaryAllocator temp;
};

Engine* _engine{};

#include "temp.h"
#include "entity.h"
#include "filesystem.h"
#include "render.h"
#include "input.h"

void init_engine(const char* assets_path) {
    _engine = new Engine();
    _engine->root_path = to_string(SDL_GetBasePath());
    _engine->assets_path = _engine->root_path + assets_path;
}

void init_window(const char* name, int width, int height) {
    _engine->name = to_string(name);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);

    SDL_GamepadEventsEnabled();

    _engine->window = SDL_CreateWindow(name, width, height, 0);
    assert(_engine->window);

    init_graphics();
}

bool should_window_close() {
    _engine->frame_count += 1;

    upload_meshes_to_gpu();
    upload_textures_to_gpu();
    
    render();

    // swap buffers
    reset_temp();

    reset_input();
    process_input();

    // update time
    
    // update sound
    // update animation / physics?

    // update file watcher?

    return _engine->quit;
}

void deinit_engine() {
    SDL_DestroyWindow(_engine->window);
    SDL_Quit();
}
