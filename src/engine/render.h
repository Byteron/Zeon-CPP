struct Skeleton {
    struct Joint {
        string name{};
        Transform transform{};
        Mat4 inverse_bind_matrix{};
        Array<size_t> children{};
    };

    Array<Joint> joints{};
};

struct SkeletonPose {
    struct Joint {
        size_t parent{};
        Transform local_transform{};
        Mat4 model_space_matrix{};
    };

    Skeleton* skeleton{};
    Array<Joint> joints{};
    Array<size_t> joints_in_traverse_order{};
    Array<Mat4> skinning_matrices{};
};

struct KeyPosition {
    Vec3 position{};
    float timestamp{};
};

struct KeyRotation {
    Quat rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
    float timestamp{};
};

struct KeyScale {
    Vec3 scale{};
    float timestamp{};
};

struct JointAnimation {
    int id{};
    Array<KeyPosition> positions{};
    Array<KeyRotation> rotations{};
    Array<KeyScale> scales{};
};

struct Animation {
    string name{};
    float duration{};
    Array<JointAnimation> joint_animations{};
};

const uint MAX_POSES = 6;

struct AnimationState {
    string name;
    string animations[6];
    int count;

    float blend_time;
    
    bool loop;
};

struct AnimationPlayer {
    SkeletonPose pose{};
    Array<Mat4> skinning_matrices{};

    string curring_animation;

    float current_time;
    // TODO: continue here with animation code
};

struct Model {
    Transform transform{};
    Array<Mesh> meshes{};
    AABB aabb{};
    Skeleton skeleton{};
    Array<Animation> animations{};
    string path{};
};

struct UBO {
    Mat4 projection;
    Mat4 model;
};

struct CameraUBO {
    Mat4 view_projection;
    Mat4 view;
    float near;
    float far;
    Mat4 model;
    Mat4 joint_matrices[100];
};

struct LightUBO {
    Vec3 light_direction;
    Vec3 light_color;
    Vec3 camera_position;

    Vec4 albedo;
};

SDL_GPUShader* load_shader(const string& path, SDL_GPUShaderStage stage, uint num_samplers, uint num_uniform_buffers) {
    string full_path = _engine->root_path + path;
    string file_contents = read_entire_file(full_path);

    SDL_GPUShaderCreateInfo create_info = {
        .code_size = file_contents.length,
        .code = reinterpret_cast<const byte*>(file_contents.data),
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage,
        .num_samplers = num_samplers,
        .num_uniform_buffers = num_uniform_buffers,
    };

    return SDL_CreateGPUShader(_engine->gpu, &create_info);
}

GraphicsPipeline create_solid_skinned_pipeline() {
    string vertex_shader_path = to_string("shaders/solid_skinned.vert");
    string fragment_shader_path = to_string("shaders/solid_skinned.frag");

    SDL_GPUShader* vertex_shader = load_shader(vertex_shader_path, SDL_GPU_SHADERSTAGE_VERTEX, 0, 1);
    SDL_GPUShader* fragment_shader = load_shader(fragment_shader_path, SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0);

    SDL_GPUColorTargetDescription color_target_description = {
        .format = SDL_GetGPUSwapchainTextureFormat(_engine->gpu, _engine->window),
    };

    SDL_GPUVertexBufferDescription vertex_buffer_description = {
        .slot = 0,
        .pitch = sizeof(Vertex),
    };

    SDL_GPUVertexAttribute vertex_attributes[] = {
        { .location = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0 * sizeof(float) },
        { .location = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 3 * sizeof(float) },
        { .location = 2, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 6 * sizeof(float) },
        { .location = 3, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 10 * sizeof(float) },
        { .location = 4, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 14 * sizeof(float) },
        { .location = 5, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 17 * sizeof(float) },
        { .location = 6, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 20 * sizeof(float) },
        { .location = 7, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = 23 * sizeof(float) },
        { .location = 8, .format = SDL_GPU_VERTEXELEMENTFORMAT_UINT4, .offset = 27 * sizeof(float) },
    };

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,
        .vertex_input_state = {
            .vertex_buffer_descriptions = &vertex_buffer_description,
            .num_vertex_buffers = 1,
            .vertex_attributes = &vertex_attributes[0],
            .num_vertex_attributes = sizeof(vertex_attributes) / sizeof(vertex_attributes[0]),
            
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state =  {
            .fill_mode = SDL_GPU_FILLMODE_FILL,
            .cull_mode = SDL_GPU_CULLMODE_BACK,
        },
        .depth_stencil_state = {
            .compare_op = SDL_GPU_COMPAREOP_LESS,
            .enable_depth_test = true,
            .enable_depth_write = true,
        },
        .target_info = {
            .color_target_descriptions = &color_target_description,
            .num_color_targets = 1,
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
            .has_depth_stencil_target = true,
        },
    };

    SDL_GPUSamplerCreateInfo sampler_create_info{};

    SDL_GPUSampler *sampler = SDL_CreateGPUSampler(_engine->gpu, &sampler_create_info);

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(_engine->gpu, &pipeline_info);

    return { vertex_shader, fragment_shader, sampler, pipeline };
}

void init_graphics() {
    SDL_GetWindowSize(_engine->window, &_engine->window_width, &_engine->window_height);
    SDL_GetWindowSize(_engine->window, &_engine->render_width, &_engine->render_height);

    _engine->gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
    assert(_engine->gpu);

    bool ok = SDL_ClaimWindowForGPUDevice(_engine->gpu, _engine->window);
    assert(ok);

    SDL_GPUTextureCreateInfo depth_texture_info = {
        .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = static_cast<uint>(_engine->render_width),
        .height = static_cast<uint>(_engine->render_height),
        .layer_count_or_depth = 1,
        .num_levels = 1,
    };

    _engine->depth_texture = SDL_CreateGPUTexture(_engine->gpu, &depth_texture_info);

    _engine->solid_skinned_pipeline = create_solid_skinned_pipeline();
}

void upload_textures_to_gpu() {
    if (_engine->textures_to_upload.count == 0)
        return;
    
    uint total_buffer_size{};

    for (int i = 0; i < _engine->textures_to_upload.count; ++i) {
        Texture* texture = _engine->textures_to_upload[i];

        SDL_GPUTextureCreateInfo texture_create_info {
            .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
            .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
            .width = static_cast<uint>(texture->image.width),
            .height = static_cast<uint>(texture->image.height),
            .layer_count_or_depth = 1,
            .num_levels = 1,
        };

        texture->gpu_texture = SDL_CreateGPUTexture(_engine->gpu, &texture_create_info);

        total_buffer_size += texture->image.width * texture->image.height * 4;
    }

    printf("Textures Uploaded: %d (%d KB)\n", _engine->textures_to_upload.count, total_buffer_size / 1024);

    SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = total_buffer_size,
    };

    SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(_engine->gpu, &transfer_buffer_create_info);
    byte* transfer_memory = reinterpret_cast<byte*>(SDL_MapGPUTransferBuffer(_engine->gpu, transfer_buffer, false));

    uint offset = 0;

    for (int i = 0; i < _engine->textures_to_upload.count; ++i) {
        Texture* texture = _engine->textures_to_upload[i];

        uint texture_size = texture->image.width * texture->image.height * 4;

        memcpy(transfer_memory + offset, texture->image.data, texture->image.width * texture->image.height * 4);

        offset += texture_size;
    }

    SDL_UnmapGPUTransferBuffer(_engine->gpu, transfer_buffer);

    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(_engine->gpu);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    offset = 0;

    for (int i = 0; i < _engine->textures_to_upload.count; ++i) {
        Texture* texture = _engine->textures_to_upload[i];

        uint texture_size = texture->image.width * texture->image.height * 4;

        SDL_GPUTextureTransferInfo texture_transfer_info {
            .transfer_buffer = transfer_buffer,
            .offset = offset,
        };

        SDL_GPUTextureRegion texture_region {
            .texture = texture->gpu_texture,
            .w = static_cast<uint>(texture->image.width),
            .h = static_cast<uint>(texture->image.height),
            .d = 1,
        };

        SDL_UploadToGPUTexture(copy_pass, &texture_transfer_info, &texture_region, false);

        offset += texture_size;
    }

    SDL_EndGPUCopyPass(copy_pass);

    bool ok = SDL_SubmitGPUCommandBuffer(command_buffer);
    assert(ok);

    SDL_ReleaseGPUTransferBuffer(_engine->gpu, transfer_buffer);

    clear(_engine->textures_to_upload);
}

void upload_meshes_to_gpu() {
    if (_engine->meshes_to_upload.count == 0)
        return;
    
    uint total_buffer_size{};

    for (int i = 0; i < _engine->meshes_to_upload.count; ++i) {
        Mesh& mesh = _engine->meshes_to_upload[i];
        for (int j = 0; j < mesh.primitives.count; ++j) {
            Mesh::Primitive& primitive = mesh.primitives[j];

            SDL_GPUBufferCreateInfo vertex_buffer_info {
                .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                .size = sizeof(Vertex) * primitive.vertices.count,
            };

            SDL_GPUBufferCreateInfo index_buffer_info {
                .usage = SDL_GPU_BUFFERUSAGE_INDEX,
                .size = sizeof(uint) * primitive.indices.count,
            };

            primitive.vertex_buffer = SDL_CreateGPUBuffer(_engine->gpu, &vertex_buffer_info);
            primitive.index_buffer = SDL_CreateGPUBuffer(_engine->gpu, &index_buffer_info);

            total_buffer_size += vertex_buffer_info.size + index_buffer_info.size;
        }
    }

    printf("Meshes Uploaded: %d (%d KB)\n", _engine->meshes_to_upload.count, total_buffer_size / 1024);

    SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = total_buffer_size,
    };

    SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(_engine->gpu, &transfer_buffer_create_info);
    byte* transfer_memory = reinterpret_cast<byte*>(SDL_MapGPUTransferBuffer(_engine->gpu, transfer_buffer, false));

    uint offset = 0;

    for (int i = 0; i < _engine->meshes_to_upload.count; ++i) {
        Mesh& mesh = _engine->meshes_to_upload[i];
        for (int j = 0; j < mesh.primitives.count; ++j) {
            Mesh::Primitive& primitive = mesh.primitives[j];

            uint vertex_buffer_size = sizeof(Vertex) * primitive.vertices.count;
            uint index_buffer_size = sizeof(uint) * primitive.indices.count;

            memcpy(transfer_memory + offset, primitive.vertices.data, vertex_buffer_size);
            offset += vertex_buffer_size;
            memcpy(transfer_memory + offset, primitive.indices.data, index_buffer_size);
            offset += index_buffer_size;
        }
    }

    SDL_UnmapGPUTransferBuffer(_engine->gpu, transfer_buffer);

    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(_engine->gpu);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    offset = 0;

    for (int i = 0; i < _engine->meshes_to_upload.count; ++i) {
        Mesh& mesh = _engine->meshes_to_upload[i];
        for (int j = 0; j < mesh.primitives.count; ++j) {
            Mesh::Primitive& primitive = mesh.primitives[j];

            uint vertex_buffer_size = sizeof(Vertex) * primitive.vertices.count;
            uint index_buffer_size = sizeof(uint) * primitive.indices.count;

            SDL_GPUTransferBufferLocation vertex_transfer_buffer_location{
                .transfer_buffer = transfer_buffer,
                .offset = offset,
            };

            SDL_GPUBufferRegion vertex_buffer_region {
                .buffer = primitive.vertex_buffer,
                .size = vertex_buffer_size,
            };

            offset += vertex_buffer_size;

            SDL_GPUTransferBufferLocation index_transfer_buffer_location{
                .transfer_buffer = transfer_buffer,
                .offset = offset,
            };

            SDL_GPUBufferRegion index_buffer_region {
                .buffer = primitive.index_buffer,
                .size = index_buffer_size,
            };

            offset += index_buffer_size;

            SDL_UploadToGPUBuffer(copy_pass, &vertex_transfer_buffer_location, &vertex_buffer_region, false);
            SDL_UploadToGPUBuffer(copy_pass, &index_transfer_buffer_location, &index_buffer_region, false);
        }
    }

    SDL_EndGPUCopyPass(copy_pass);

    bool ok = SDL_SubmitGPUCommandBuffer(command_buffer);
    assert(ok);

    SDL_ReleaseGPUTransferBuffer(_engine->gpu, transfer_buffer);

    clear(_engine->meshes_to_upload);
}

void draw_model(Model* model, Array<Mat4> skinning_matrices) {
    Mat4 matrix = to_mat4(model->transform);

    for (int i = 0; i < model->meshes.count; ++i) {
        Mesh& mesh = model->meshes[i];
        
        for (int j = 0; j < mesh.primitives.count; ++j) {
            Mesh::Primitive* primitive = &mesh.primitives[j];

            if (primitive->material.alpha_mode == 0) {
                if (mesh.type == MeshType::Static) {
                    add(_engine->opaque_static_pass, OpaqueStaticRenderData{
                        .transform = matrix,
                        .primitive = primitive,
                        .aabb = {},
                    });
                } else {
                    add(_engine->opaque_skinned_pass, OpaqueSkinnedRenderData{
                        .transform = matrix,
                        .primitive = primitive,
                        .aabb = {},
                        .skinning_matrices = skinning_matrices,
                    });
                }
            } else {

            }
        }
    }
}

void render() {
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(_engine->gpu);
    assert(command_buffer);

    bool ok = SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, _engine->window, &_engine->swapchain_texture, 0, 0);
    assert(ok);

    float aspect_ratio = static_cast<float>(_engine->window_width) / static_cast<float>(_engine->window_height);
    Mat4 camera_projection = perspective_projection(to_radians(60.0), aspect_ratio, 0.1, 100);

    if (_engine->swapchain_texture != nullptr) {
        SDL_GPUColorTargetInfo color_target_info = {
            .texture = _engine->swapchain_texture,
            .clear_color = { 0.2f, 0.22f, 0.24f, 1.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE
        };

        SDL_GPUDepthStencilTargetInfo depth_target_info = {
            .texture = _engine->depth_texture,
            .clear_depth = 1.0f,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE
        };

        SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, &depth_target_info);
        assert(render_pass);

        SDL_BindGPUGraphicsPipeline(render_pass, _engine->solid_skinned_pipeline.pipeline);

        // SDL_PushGPUVertexUniformData(command_buffer, 0, &vertex_ubo, sizeof(CameraUBO));
        // SDL_PushGPUFragmentUniformData(command_buffer, 0, &fragment_ubo, sizeof(LightUBO));

        for (int i = 0; i < _engine->opaque_skinned_pass.count; ++i) {
            OpaqueSkinnedRenderData& data = _engine->opaque_skinned_pass[i];

            SDL_GPUBufferBinding vertex_buffer_binding {
                .buffer = data.primitive->vertex_buffer,
                .offset = 0,
            };

            SDL_GPUBufferBinding index_buffer_binding {
                .buffer = data.primitive->index_buffer,
                .offset = 0,
            };

            SDL_GPUTextureSamplerBinding sampler_binding {
                .texture = data.primitive->material.diffuse_texture->gpu_texture,
                .sampler = _engine->solid_skinned_pipeline.sampler,
            };

            UBO ubo{ camera_projection, data.transform };

            SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_buffer_binding, 1);
            SDL_BindGPUIndexBuffer(render_pass, &index_buffer_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

            SDL_BindGPUFragmentSamplers(render_pass, 0, &sampler_binding, 1);

            SDL_PushGPUVertexUniformData(command_buffer, 0, &ubo, sizeof(UBO));

            SDL_DrawGPUIndexedPrimitives(render_pass, data.primitive->indices.count, 1, 0, 0, 0);
        }

        SDL_EndGPURenderPass(render_pass);
        
    }

    ok = SDL_SubmitGPUCommandBuffer(command_buffer);
    assert(ok);

    clear(_engine->opaque_static_pass);
    clear(_engine->opaque_skinned_pass);
}

// TODO: Texture as Resources with Index / Handle for stable pointers
Texture* load_texture(byte* data, size_t size) {
    Texture* texture{};

    if (get(_engine->data_textures, data, texture)) {
        return texture;
    }

    Image image{};
    image.data = stbi_load_from_memory(data, size, &image.width, &image.height, &image.channel_count, 0);
    texture = new Texture { .image = image };
    set(_engine->data_textures, data, texture);
    printf("Texture Loaded: W: %d, H: %d, Data: %p\n", image.width, image.height, image.data);
    add(_engine->textures_to_upload, texture);
    return texture;
}

Texture* load_texture(const string& path) {
    string full_path = temp_concat(_engine->assets_path, path);

    Texture* texture{};

    if (get(_engine->path_textures, full_path, texture)) {
        return texture;
    }
    
    Image image{};
    image.data = stbi_load(full_path.data, &image.width, &image.height, &image.channel_count, 0);
    texture = new Texture { .image = image };
    set(_engine->path_textures, copy(full_path), texture);
    printf("Texture Loaded: W: %d, H: %d, Data: %p\n", image.width, image.height, image.data);
    add(_engine->textures_to_upload, texture);
    return texture;
}

Mesh process_mesh(string path, const cgltf_mesh* raw_mesh, cgltf_node* node) {
    Mesh mesh{};

    if (raw_mesh->primitives_count == 1) {
        resize(mesh.primitives, raw_mesh->primitives_count);
    }

    resize(mesh.primitives, raw_mesh->primitives_count);

    for (int i = 0; i < raw_mesh->primitives_count; ++i) {
        cgltf_primitive raw_primitive = raw_mesh->primitives[i];
        Mesh::Primitive& primitive = mesh.primitives[i];

        int vertex_count = raw_primitive.attributes[0].data->count;
        resize(primitive.vertices, vertex_count);

        for (int j = 0; j < raw_primitive.attributes_count; ++j) {
            cgltf_attribute attribute = raw_primitive.attributes[j];

            switch (attribute.type) {
                case cgltf_attribute_type_position: {
                    for (int k = 0; k < vertex_count; ++k) {
                        cgltf_accessor_read_float(attribute.data, k, &primitive.vertices[k].position.x, 3);
                    }
                } break;
                case cgltf_attribute_type_normal: {
                    for (int k = 0; k < vertex_count; ++k) {
                        cgltf_accessor_read_float(attribute.data, k, &primitive.vertices[k].normal.x, 3);
                    }
                } break;
                case cgltf_attribute_type_texcoord: {
                    for (int k = 0; k < vertex_count; ++k) {
                        cgltf_accessor_read_float(attribute.data, k, &primitive.vertices[k].uv.x, 2);
                    }
                } break;
                case cgltf_attribute_type_color: {
                    for (int k = 0; k < vertex_count; ++k) {
                        cgltf_accessor_read_float(attribute.data, k, &primitive.vertices[k].color.x, 4);
                    }
                } break;
                case cgltf_attribute_type_joints: {
                    for (int k = 0; k < vertex_count; ++k) {
                        cgltf_accessor_read_uint(attribute.data, k, &primitive.vertices[k].joints[0], 4);
                    }
                } break;
                case cgltf_attribute_type_weights: {
                    for (int k = 0; k < vertex_count; ++k) {
                        cgltf_accessor_read_float(attribute.data, k, &primitive.vertices[k].weights[0], 4);
                    }
                }
                default: ;
            }
        }

        resize(primitive.indices, raw_primitive.indices->count);

        for (int j = 0; j < raw_primitive.indices->count; ++j) {
            primitive.indices[j] = cgltf_accessor_read_index(raw_primitive.indices, j);
        }

        if (raw_primitive.material) {
            auto raw_material = raw_primitive.material;

            primitive.material.alpha_mode = raw_material->alpha_mode;

            if (raw_material->has_pbr_metallic_roughness) {
                primitive.material.albedo = to_vec4(raw_material->pbr_metallic_roughness.base_color_factor);

                if (raw_material->pbr_metallic_roughness.base_color_texture.texture) {
                    if (auto diffuse_tex_uri = raw_material->pbr_metallic_roughness.base_color_texture.texture->image->uri) {
                        string diffuse_tex_path = path + diffuse_tex_uri;
                        primitive.material.diffuse_texture = load_texture(diffuse_tex_path);
                    } else {
                        auto buffer_view = raw_material->pbr_metallic_roughness.base_color_texture.texture->image->buffer_view;
                        auto diffuse_tex_data = static_cast<byte*>(buffer_view->buffer->data) + buffer_view->offset;
                        auto diffuse_tex_size = buffer_view->buffer->size;
                        primitive.material.diffuse_texture = load_texture(diffuse_tex_data, diffuse_tex_size);
                    }
                } else {
                    // TODO: replace with white texture
                    primitive.material.diffuse_texture = {};
                }
            }
        }
    }

    for (int i = 0; i < mesh.primitives.count; ++i) {
        auto& primitive = mesh.primitives[i];
        for (int j = 0; j < primitive.vertices.count; ++j) {
            auto& vertex = primitive.vertices[j];
            primitive.aabb.min = min(primitive.aabb.min, vertex.position);
            primitive.aabb.max = max(primitive.aabb.max, vertex.position);
        }

        mesh.aabb.min = min(mesh.aabb.min, primitive.aabb.min);
        mesh.aabb.max = max(mesh.aabb.max, primitive.aabb.max);
    }

    add(_engine->meshes_to_upload, mesh);

    return mesh;
}

void process_node(string path, cgltf_node* node, Array<Mesh>& meshes) {
    if (node->mesh) {
        add(meshes, process_mesh(path, node->mesh, node));
    }

    if (node->children_count > 0) {
        for (int i = 0; i < node->children_count; ++i) {
            process_node(path, node->children[i], meshes);
        }
    }
}

Array<Mesh> process_meshes(string path, cgltf_data* data) {
    Array<Mesh> meshes{};

    for (int i = 0; i < data->scenes_count; ++i) {
        auto& scene = data->scenes[i];

        for (int j = 0; j < scene.nodes_count; ++j) {
            process_node(path, scene.nodes[j], meshes);
        }
    }

    return meshes;
}

Skeleton process_skeleton(const cgltf_data* data) {
    Skeleton skeleton{};

    if (data->skins == nullptr)
        return skeleton;

    const cgltf_skin skin = data->skins[0];

    Span all_joints = { skin.joints, skin.joints_count };
    resize(skeleton.joints, skin.joints_count);

    for (int i = 0; i < skin.joints_count; ++i) {
        auto&[name, transform, inverse_bind_matrix, children] = skeleton.joints[i];
        cgltf_node* node = skin.joints[i];

        transform.position = { node->translation[0], node->translation[1], node->translation[2] };
        transform.rotation = { node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3] };
        transform.scale = { node->scale[0], node->scale[1], node->scale[2] };

        if (node->children_count > 0) {
            for (int j = 0; j < node->children_count; ++j) {
                cgltf_node* child = node->children[j];
                size_t index = find(all_joints, child);
                if (index != SIZE_MAX) {
                    add(children, index);
                }
            }
        }

        cgltf_accessor_read_float(skin.inverse_bind_matrices, i, &inverse_bind_matrix[0][0], 16);
        name = to_string(node->name);
    }

    return skeleton;
}

Array<Animation> process_animations(const cgltf_data* data, Skeleton& skeleton) {
    if (data->skins_count == 0 || data->animations_count == 0)
        return {};

    const cgltf_skin skin = data->skins[0];

    Array<Animation> animations{};

    resize(animations, data->animations_count);

    Span all_joints = { skin.joints, skin.joints_count };

    for (int i = 0; i < data->animations_count; ++i) {
        cgltf_animation animation_data = data->animations[i];
        string name = to_string(animation_data.name);
        // fprintf(stderr,"name: %s\n", name.data);

        Animation& animation = animations[i];
        animation.name = name;

        resize(animation.joint_animations, skeleton.joints.count);

        for (int j = 0; j < animation.joint_animations.count; ++j) {
            animation.joint_animations[j].id = j;
        }

        for (int j = 0; j < animation_data.channels_count; ++j) {
            const cgltf_animation_channel channel = animation_data.channels[j];
            int num_keyframes = channel.sampler->input->count;
            size_t index = find(all_joints, channel.target_node);
            if (index != SIZE_MAX) {
                JointAnimation& joint_animation = animation.joint_animations[index];

                switch (channel.target_path) {
                    case cgltf_animation_path_type_translation: {
                        resize(joint_animation.positions, num_keyframes);
                        for (int k = 0; k < num_keyframes; ++k) {
                            auto&[position, timestamp] = joint_animation.positions[k];
                            cgltf_accessor_read_float(channel.sampler->input, k, &timestamp, 1);
                            cgltf_accessor_read_float(channel.sampler->output, k, &position[0], 3);
                        }
                    } break;
                    case cgltf_animation_path_type_rotation: {
                        resize(joint_animation.rotations, num_keyframes);
                        for (int k = 0; k < num_keyframes; ++k) {
                            auto&[rotation, timestamp] = joint_animation.rotations[k];
                            cgltf_accessor_read_float(channel.sampler->input, k, &timestamp, 1);
                            cgltf_accessor_read_float(channel.sampler->output, k, &rotation[0], 4);
                        }
                    } break;
                    case cgltf_animation_path_type_scale: {
                        resize(joint_animation.scales, num_keyframes);
                        for (int k = 0; k < num_keyframes; ++k) {
                            auto&[scale, timestamp] = joint_animation.scales[k];
                            cgltf_accessor_read_float(channel.sampler->input, k, &timestamp, 1);
                            cgltf_accessor_read_float(channel.sampler->output, k, &scale[0], 3);
                        }
                    } break;
                    default: ;
                }
            }
        }

        for (int j = 0; j < animation.joint_animations.count; ++j) {
            auto& joint_animation = animation.joint_animations[j];
            for (int k = 0; k < joint_animation.positions.count; ++k) {
                animation.duration = max(animation.duration, joint_animation.positions[k].timestamp);
            }
            for (int k = 0; k < joint_animation.rotations.count; ++k) {
                animation.duration = max(animation.duration, joint_animation.rotations[k].timestamp);
            }
            for (int k = 0; k < joint_animation.scales.count; ++k) {
                animation.duration = max(animation.duration, joint_animation.scales[k].timestamp);
            }
        }
    }

    return animations;
}

Model load_gltf(const char* path) {
    string full_path = _engine->assets_path + path;

    fprintf(stderr, "paths: %s, %s, %s\n", path, _engine->assets_path.data, full_path.data);

    string file_contents = read_entire_file(full_path);

    cgltf_options options = {};
    cgltf_data* data = {};
    if (cgltf_result parse_result = cgltf_parse(&options, file_contents.data, file_contents.length, &data)) {
        fprintf(stderr, "Failed to parse gltf file: %d\n", parse_result);
    }
    if (cgltf_result load_buffers_result = cgltf_load_buffers(&options, data, full_path.data)) {
        fprintf(stderr, "Failed to load gltf buffers: %d\n", load_buffers_result);
    }

    Array<Mesh> meshes = process_meshes(to_string(path), data);
    Skeleton skeleton = process_skeleton(data);
    Array<Animation> animations = process_animations(data, skeleton);

    Model model{};
    model.meshes = meshes;
    model.skeleton = skeleton;
    model.animations = animations;
    model.path = full_path;

    for (int i = 0; i < model.meshes.count; ++i) {
        auto& mesh = model.meshes[i];
        model.aabb.min = min(model.aabb.min, mesh.aabb.min);
        model.aabb.max = max(model.aabb.max, mesh.aabb.max);
    }

    MeshType mesh_type = is_empty(model.skeleton.joints) ? MeshType::Static : MeshType::Skinned;

    for (int i = 0; i < model.meshes.count; ++i) {
        auto& mesh = model.meshes[i];

        mesh.type = mesh_type;

        for (int j = 0; j < mesh.primitives.count; ++j) {
            auto& primitive = mesh.primitives[j];
            primitive.material.shader = mesh_type == MeshType::Static ? nullptr : nullptr;
        }
    }

    return model;
}
