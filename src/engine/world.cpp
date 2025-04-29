struct EntityMeta {
    int gen{};
    int type_id{};
    int index{};

    Entity parent{};
    Entity first_child{};
    Entity last_child{};
    Entity prev_sibling{};
    Entity next_sibling{};

    Transform local_transform{};
};

struct Operation {
    Entity entity;
};

inline std::atomic<std::size_t> component_id_counter{0};

template <typename T>
std::size_t get_type_id() noexcept {
    static const std::size_t id = component_id_counter++;
    return id;
}

struct World {
    std::vector<EntityMeta> entities{};

    std::vector<Storage> storages{};

    std::vector<int> unused_ids{};

    std::vector<Operation> ops{};
    int lock_count{};
    bool is_locked{};
};

bool is_alive(const World& world, const Entity entity) {
    return entity.id < world.entities.size() && entity.gen == world.entities[entity.id].gen;
}

int index_of(const World& world, const Entity entity) {
    return world.entities[entity.id].index;
}

int type_id_of(const World& world, const Entity entity) {
    return world.entities[entity.id].type_id;
}

template <typename T>
Storage& get_storage(World& world) {
    const std::size_t type_id = get_type_id<T>();

    if (type_id >= world.storages.size()) {
        world.storages.resize(type_id + 1);
    }

    if (world.storages[type_id].component_byte_size == 0) {
        world.storages[type_id].id = type_id;
        world.storages[type_id].component_byte_size = sizeof(T);
    }

    return world.storages[type_id];
}

Storage& get_storage(World& world, const std::size_t type_id) {
    assert(world.storages.size() > type_id);
    return world.storages[type_id];
}

template <typename T>
std::span<T> get_span(World& world) {
    Storage& storage = get_storage<T>(world);
    std::span<T> span = { reinterpret_cast<T*>(storage.data.data()), storage.count };
    return span;
}

Entity get_parent(World& world, const Entity entity) {
    EntityMeta& meta = world.entities[entity.id];
    assert(is_alive(world, meta.parent));
    return meta.parent;
}

template <typename T>
bool is_a(World& world, const Entity entity) {
    std::size_t type_id = get_type_id<T>();
    EntityMeta& meta = world.entities[entity.id];
    return meta.type_id == type_id;
}

HMM_Vec3& get_position(World& world, const Entity entity) {
    EntityMeta& meta = world.entities[entity.id];
    return meta.local_transform.position;
}

Entity get_entity(World& world, const int entity_id) {
    EntityMeta& meta = world.entities[entity_id];
    return Entity{ .id = entity_id, .gen = meta.gen };
}
template <typename T>
std::span<Entity> get_entities(World& world) {
    Storage& storage = get_storage<T>(world);
    return storage.entities;
}

template <typename T>
struct EntityComponentSpans {
    std::span<T> components{};
    std::span<Entity> entities{};
    size_t count{};

    std::tuple<Entity, T&> operator[](std::size_t i) noexcept {
        return { entities[i], components[i] };
    }
};

template <typename T>
EntityComponentSpans<T> get(World& world) {
    Storage& storage = get_storage<T>(world);
    std::span<T> span = { reinterpret_cast<T*>(storage.data.data()), storage.count };
    return EntityComponentSpans { .components = span, .entities = storage.entities, .count = storage.count };
}

template <typename T>
Entity spawn(World& world, T value) {
    int id;

    if (world.unused_ids.empty()) {
        id = world.entities.size();
        world.entities.emplace_back(EntityMeta{});
    } else {
        id = world.unused_ids.back();
        world.unused_ids.pop_back();
    }

    Storage& storage = get_storage<T>(world);

    const int index = storage.count;

    EntityMeta& meta = world.entities[id];

    meta = EntityMeta{
        .gen = -meta.gen + 1,
        .type_id = storage.id,
        .index = index,
    };

    Entity entity = Entity{ .id = id, .gen = meta.gen };

    add(storage, entity, reinterpret_cast<uint8_t*>(&value));

    return entity;
}

void add_child(World& world, const Entity parent, const Entity child) {
    assert(is_alive(world, parent) || is_alive(world, child));

    EntityMeta& child_meta = world.entities[child.id];
    EntityMeta& parent_meta = world.entities[parent.id];

    assert(!is_alive(world, child_meta.parent));

    if (is_alive(world, parent_meta.last_child)) {
        EntityMeta& last_child_meta = world.entities[parent_meta.last_child.id];
        last_child_meta.next_sibling = child;
        child_meta.prev_sibling = parent_meta.last_child;
    } else {
        parent_meta.first_child = child;
    }

    parent_meta.last_child = child;

    child_meta.parent = parent;
}

void remove_child(World& world, const Entity parent, const Entity child) {
    assert(is_alive(world, parent));

    EntityMeta& parent_meta = world.entities[parent.id];
    EntityMeta& child_meta = world.entities[child.id];

    if (is_alive(world, child_meta.next_sibling)) {
        EntityMeta& next_sibling_meta = world.entities[child_meta.next_sibling.id];

        if (is_alive(world, child_meta.prev_sibling)) {
            next_sibling_meta.prev_sibling = child_meta.prev_sibling;
        } else {
            next_sibling_meta.next_sibling = {};
        }
    }

    if (is_alive(world, child_meta.prev_sibling)) {
        EntityMeta& prev_sibling_meta = world.entities[child_meta.prev_sibling.id];

        if (is_alive(world, child_meta.next_sibling)) {
            prev_sibling_meta.next_sibling = child_meta.next_sibling;
        } else {
            prev_sibling_meta.next_sibling = {};
        }
    }

    if (parent_meta.first_child == child) {
        parent_meta.first_child = child_meta.next_sibling;
    }

    if (parent_meta.last_child == child) {
        parent_meta.last_child = child_meta.prev_sibling;
    }

    child_meta.parent = {};
}

void delete_entity(World& world, const Entity entity) {
    EntityMeta& meta = world.entities[entity.id];
    meta.gen = -meta.gen;

    Storage& storage = get_storage(world, meta.type_id);

    if (const Entity moved_entity = remove(storage, meta.index); moved_entity != ENTITY_NONE) {
        EntityMeta& last_entity_meta = world.entities[moved_entity.id];
        last_entity_meta.index = meta.index;
    }

    if (is_alive(world, meta.parent)) {
        remove_child(world, meta.parent, entity);
    }

    world.unused_ids.push_back(entity.id);
}

void despawn(World& world, const Entity entity) {
    if (!is_alive(world, entity)) {
        return;
    }

    if (world.is_locked) {
        world.ops.emplace_back(Operation{ .entity = entity });
        return;
    }

    EntityMeta& meta = world.entities[entity.id];

    Entity child = meta.first_child;
    // ReSharper disable once CppDFAEndlessLoop
    while (is_alive(world, child)) {
        // ReSharper disable once CppDFAUnreachableCode
        const Entity next = world.entities[child.id].next_sibling;
        despawn(world, child);
        child = next;
    }

    // ReSharper disable once CppDFAUnreachableCode
    meta.parent = {};
    delete_entity(world, entity);
}

template<typename T>
T& get(World& world, const Entity entity) {
    EntityMeta& meta = world.entities[entity.id];
    std::span<T> span = get_span<T>(world);
    return span[meta.index];
}

template <typename T>
void add_component(World&world, Entity entity, T component) {
    Entity child = spawn(world, component);
    add_child(world, entity, child);
}

void apply_operations(World& world) {
    for (int i = 0; i < world.ops.size(); i++) {
        if (is_alive(world, world.ops[i].entity)) {
            despawn(world, world.ops[i].entity);
        }
    }

    world.ops.clear();
}

void lock(World& world) {
    world.is_locked = true;
    world.lock_count++;
}

void unlock(World& world) {
    world.lock_count -= 1;
    if (world.lock_count != 0) {
        return;
    }
    world.is_locked = false;
    apply_operations(world);
}


