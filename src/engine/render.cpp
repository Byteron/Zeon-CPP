struct Vertex {
    Vec3 position{};
    Vec3 normal{};
    Vec4 uv{};
    Vec3 color{};
    Vec3 tangent{};
    Vec3 bitangent{};
    float weights[4]{};
    unsigned int joints[4]{};
};

struct Material {
    Vec4 albedo{};

    SDL_GPUTexture* diffuse_texture{};
    SDL_GPUTexture* normal_texture{};
    SDL_GPUTexture* specular_texture{};
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
        Array<size_t> indices{};

        SDL_GPUBuffer* vertexBuffer{};
        SDL_GPUBuffer* indexBuffer{};

        AABB aabb{};

        Material material{};
    };

    MeshType type{};

    AABB aabb{};
    Array<Primitive> primitives{};
};

struct Skeleton {
    struct Joint {
        std::string name{};
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
    std::string name{};
    float duration{};
    Array<JointAnimation> joint_animations{};
};

struct Model {
    Transform transform{};
    Array<Mesh> meshes{};
    AABB aabb{};
    Skeleton skeleton{};
    std::unordered_map<std::string, Animation> animations{};
    string path{};
};

SDL_GPUTexture* load_texture(void* data, size_t size) {
    return nullptr;
}

SDL_GPUTexture* load_texture(const std::string& path) {
    return nullptr;
}

void setup_mesh(Mesh* mesh) {
    // TODO: initialize mesh SDL GPU Style. If possible. We will see.
}

Mesh process_mesh(std::string path, const cgltf_mesh* raw_mesh, cgltf_node* node) {
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
                primitive.material.albedo = make_vec4(raw_material->pbr_metallic_roughness.base_color_factor);

                if (raw_material->pbr_metallic_roughness.base_color_texture.texture) {
                    if (auto diffuse_tex_uri = raw_material->pbr_metallic_roughness.base_color_texture.texture->image->uri) {
                        std::string diffuse_tex_path = path + diffuse_tex_uri;
                        primitive.material.diffuse_texture = load_texture(diffuse_tex_path);
                    } else {
                        auto buffer_view = raw_material->pbr_metallic_roughness.base_color_texture.texture->image->buffer_view;
                        auto diffuse_tex_data = static_cast<uint8_t*>(buffer_view->buffer->data) + buffer_view->offset;
                        auto diffuse_tex_size = buffer_view->buffer->size;
                        primitive.material.diffuse_texture = load_texture(diffuse_tex_data, diffuse_tex_size);
                    }
                } else {
                    // TODO: replace with white texture
                    primitive.material.diffuse_texture = nullptr;
                }
            }
        }
    }

    setup_mesh(&mesh);

    for (auto& primitive : mesh.primitives) {
        for (auto& vertex : primitive.vertices) {
            primitive.aabb.min = vec_min(primitive.aabb.min, vertex.position);
            primitive.aabb.max = vec_max(primitive.aabb.max, vertex.position);
        }

        mesh.aabb.min = vec_min(mesh.aabb.min, primitive.aabb.min);
        mesh.aabb.max = vec_max(mesh.aabb.max, primitive.aabb.max);
    }

    return mesh;
}

void process_node(std::string path, cgltf_node* node, Array<Mesh>& meshes) {
    if (node->mesh) {
        add(meshes, process_mesh(path, node->mesh, node));
    }

    if (node->children_count > 0) {
        for (int i = 0; i < node->children_count; ++i) {
            process_node(path, node->children[i], meshes);
        }
    }
}

Array<Mesh> process_meshes(std::string path, cgltf_data* data) {
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
        name = node->name;
    }

    return skeleton;
}

std::unordered_map<std::string, Animation> process_animations(const cgltf_data* data, Skeleton& skeleton) {
    if (data->skins_count == 0 || data->animations_count == 0)
        return {};

    const cgltf_skin skin = data->skins[0];

    std::unordered_map<std::string, Animation> animations{};

    animations.reserve(data->animations_count);

    Span all_joints = { skin.joints, skin.joints_count };

    for (int i = 0; i < data->animations_count; ++i) {
        cgltf_animation animation_data = data->animations[i];
        std::string name = animation_data.name;
        std::cout << "name: " << name << std::endl;

        Animation& animation = animations[name];
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

        for (auto& joint_animation : animation.joint_animations) {
            for (auto& position : joint_animation.positions) {
                animation.duration = MAX(animation.duration, position.timestamp);
            }
            for (auto& rotation : joint_animation.rotations) {
                animation.duration = MAX(animation.duration, rotation.timestamp);
            }
            for (auto& scale : joint_animation.scales) {
                animation.duration = MAX(animation.duration, scale.timestamp);
            }
        }
    }

    return animations;
}

Model load_gltf(const char* path) {
    string exe_path = get_executable_directory();
    string full_path = exe_path + path;

    string file_contents = read_entire_file(full_path);

    cgltf_options options = {};
    cgltf_data* data = {};
    if (cgltf_result parse_result = cgltf_parse(&options, file_contents.data, file_contents.length, &data)) {
        std::cout << "Failed to parse gltf file: " << parse_result << std::endl;
    }
    if (cgltf_result load_buffers_result = cgltf_load_buffers(&options, data, full_path.data)) {
        std::cout << "Failed to load buffers: " << load_buffers_result << std::endl;
    }

    Array<Mesh> meshes = process_meshes(path, data);
    Skeleton skeleton = process_skeleton(data);
    std::unordered_map<std::string, Animation> animations = process_animations(data, skeleton);

    Model model{};
    model.meshes = meshes;
    model.skeleton = skeleton;
    model.animations = animations;
    model.path = full_path;

    for (auto& mesh : model.meshes) {
        model.aabb.min = vec_min(model.aabb.min, mesh.aabb.min);
        model.aabb.max = vec_max(model.aabb.max, mesh.aabb.max);
    }

    MeshType mesh_type = is_empty(model.skeleton.joints) ? MeshType::Static : MeshType::Skinned;

    for (auto& mesh : model.meshes) {
        mesh.type = mesh_type;

        for (auto& primitive : mesh.primitives) {
            primitive.material.shader = mesh_type == MeshType::Static ? nullptr : nullptr;
        }
    }

    return model;
}