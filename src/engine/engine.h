
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
    SDL_GPUCommandBuffer* command_buffer{};

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

    init_graphics(_engine);
}

void reset_input() {

}

void process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            _engine->quit = true;
        }
    }
}

bool should_window_close() {
    reset_input();
    process_input();

    // bool ok = SDL_WaitAndAcquireGPUSwapchainTexture(_engine->command_buffer, _engine->window, &_engine->swapchain_texture, 0, 0);
    // assert(ok);

    // if (_engine->swapchain_texture != nullptr) {
    //     SDL_GPUColorTargetInfo color_target_info = {
    //         .texture = _engine->swapchain_texture,
    //         .clear_color = { 0.0f, 0.2f, 0.4f, 1.0f},
    //         .load_op = SDL_GPU_LOADOP_CLEAR,
    //         .store_op = SDL_GPU_STOREOP_STORE
    //     };

    //     // SDL_GPUDepthStencilTargetInfo depth_target_info = {
    //     //     .texture = _engine->depth_texture,
    //     //     .clear_depth = 1.0f,
    //     //     .load_op = SDL_GPU_LOADOP_CLEAR,
    //     //     .store_op = SDL_GPU_STOREOP_STORE
    //     // };

    //     SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(_engine->command_buffer, &color_target_info, 1, nullptr);
    //     assert(render_pass);

    //     SDL_EndGPURenderPass(render_pass);
    // }

    return _engine->quit;
}

void deinit_engine() {
    SDL_DestroyWindow(_engine->window);
    SDL_Quit();
}

