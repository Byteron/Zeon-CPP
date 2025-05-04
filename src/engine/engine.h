struct GraphicsPipeline {
    SDL_GPUShader* vertex_shader{};
    SDL_GPUShader* fragment_shader{};
    SDL_GPUGraphicsPipeline* pipeline{};
};

struct Engine {
    string name{};

    float time{};
    float delta_time{};
    
    Array<float> delta_samples{};

    string root_path{};
    string assets_path{};

    bool quit{};

    SDL_Window* window{};
    SDL_GPUDevice* gpu{};
    SDL_GPUTexture* swapchain_texture{};
    SDL_GPUTexture* depth_texture{};

    GraphicsPipeline solid_skinned_pipeline{};

    int window_width{};
    int window_height{};

    int render_width{};
    int render_height{};
};

Engine* _engine{};

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
    render();

    // swap buffers
    // reset temporary storage

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

