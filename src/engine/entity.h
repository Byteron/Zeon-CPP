struct EntityId {
    int id{};
    int gen{};

    int type_id{};

    bool operator==(const EntityId& rhs) const noexcept {
        return id == rhs.id && gen == rhs.gen;
    }

    bool operator!=(const EntityId& rhs) const noexcept {
        return !(*this == rhs);
    }
};

constexpr EntityId ENTITY_NONE{};

struct Storage {
    int id{};
    size_t count{};
    Array<EntityId> entities{};
    Array<byte> data{};

    size_t component_byte_size{};
};

void free(Storage& storage) {
    free(storage.entities);
    free(storage.data);
}

void reserve(Storage& storage, size_t desired_items) {
    if (storage.entities.count >= desired_items) return;

    desired_items *= 2;
    if (desired_items < 8) desired_items = 8;

    resize(storage.entities, desired_items);
    resize(storage.data, desired_items * storage.component_byte_size);
}

void set(Storage& storage, const int index, const byte* value) {
    const size_t i = storage.component_byte_size * index;
    memcpy(storage.data.data + i, value, storage.component_byte_size);
}

byte* get(Storage& storage, const int index) {
    const size_t i = storage.component_byte_size * index;
    return storage.data.data + i;
}

EntityId remove(Storage& storage, const int index) {
    const size_t last_index = storage.count - 1;

    if (index == last_index) {
        storage.count -= 1;
        return {};
    }

    const EntityId last_entity = storage.entities[last_index];

    const byte* value = get(storage, last_entity.id);
    set(storage, index, value);

    storage.entities[index] = last_entity;

    storage.count -= 1;

    return last_entity;
}

void add(Storage& storage, const EntityId entity, const byte* value) {
    const size_t index = storage.count;
    storage.count++;
    reserve(storage, storage.count);
    set(storage, index, value);
    storage.entities[index] = entity;
}

struct EntityMeta {
    int gen{};
    int type_id{};
    int index{};
};

struct Entity {
    EntityId id{};

    Transform transform{};

    EntityId parent{};
    EntityId first_child{};
    EntityId last_child{};
    EntityId prev_sibling{};
    EntityId next_sibling{};
};

struct Operation {
    EntityId entity;
};

inline size_t component_id_counter{0};

template <typename T>
size_t get_type_id() noexcept {
    static const size_t id = component_id_counter++;
    return id;
}

struct World {
    Array<EntityMeta> entities{};

    Array<Storage> storages{};

    Array<int> unused_ids{};

    Array<Operation> ops{};
    int lock_count{};
    bool is_locked{};
};

void free(World& world) {
    for (int i = 0; i < world.storages.count; i++) {
        free(world.storages[i]);
    }

    free(world.entities);
    free(world.storages);
    free(world.unused_ids);
    free(world.ops);
}

bool is_alive(const World& world, const EntityId entity) {
    return entity.id < world.entities.count && entity.gen == world.entities[entity.id].gen;
}

int index_of(const World& world, const EntityId entity) {
    return world.entities[entity.id].index;
}

int type_id_of(const World& world, const EntityId entity) {
    return world.entities[entity.id].type_id;
}

template <typename T>
Storage& get_storage(World& world) {
    const size_t type_id = get_type_id<T>();

    if (type_id >= world.storages.count) {
        resize(world.storages, type_id + 1);
    }

    if (world.storages[type_id].component_byte_size == 0) {
        world.storages[type_id].id = type_id;
        world.storages[type_id].component_byte_size = sizeof(T);
    }

    return world.storages[type_id];
}

Storage& get_storage(World& world, const size_t type_id) {
    assert(world.storages.count > type_id);
    return world.storages[type_id];
}

template <typename T>
Span<T> get(World& world) {
    Storage& storage = get_storage<T>(world);
    Span<T> span{};
    span.data = reinterpret_cast<T*>(storage.data.data);
    span.count = storage.count;
    return span;
}

template <typename T>
bool is_a(World& world, const EntityId entity) {
    size_t type_id = get_type_id<T>();
    EntityMeta& meta = world.entities[entity.id];
    return meta.type_id == type_id;
}

Entity& get_entity(World& world, const EntityId entity) {
    EntityMeta& meta = world.entities[entity.id];
    Storage& storage = get_storage(world, meta.type_id);
    uint offset = meta.index * storage.component_byte_size;
    Entity* ptr = reinterpret_cast<Entity*>(storage.data.data + offset);
    return *ptr;
}

Entity& get_parent(World& world, Entity& entity) {
    return get_entity(world, entity.parent);
}

Entity* get_entity_ptr(Storage& storage, int index) {
    uint offset = index * storage.component_byte_size;
    return reinterpret_cast<Entity*>(storage.data.data + offset);
}

Entity& get_entity(Storage& storage, int index) {
    uint offset = index * storage.component_byte_size;
    return *reinterpret_cast<Entity*>(storage.data.data + offset);
}

void add_child(World& world, EntityId parent, EntityId child) {
    assert(is_alive(world, parent) || is_alive(world, child));

    Entity& child_entity = get_entity(world, child);
    Entity& parent_entity = get_entity(world, parent);

    assert(!is_alive(world, child_entity.parent));

    if (is_alive(world, parent_entity.last_child)) {
        Entity& last_child_entity = get_entity(world, parent_entity.last_child);
        last_child_entity.next_sibling = child;
        child_entity.prev_sibling = parent_entity.last_child;
    } else {
        parent_entity.first_child = child;
    }

    parent_entity.last_child = child;

    child_entity.parent = parent;
}

void remove_child(World& world, const EntityId parent, const EntityId child) {
    assert(is_alive(world, parent));

    Entity& parent_entity = get_entity(world, parent);
    Entity& child_entity = get_entity(world, child);

    if (is_alive(world, child_entity.next_sibling)) {
        Entity& next_sibling_entity = get_entity(world, child_entity.next_sibling);

        if (is_alive(world, child_entity.prev_sibling)) {
            next_sibling_entity.prev_sibling = child_entity.prev_sibling;
        } else {
            next_sibling_entity.next_sibling = {};
        }
    }

    if (is_alive(world, child_entity.prev_sibling)) {
        Entity& prev_sibling_entity = get_entity(world, child_entity.prev_sibling);

        if (is_alive(world, child_entity.next_sibling)) {
            prev_sibling_entity.next_sibling = child_entity.next_sibling;
        } else {
            prev_sibling_entity.next_sibling = {};
        }
    }

    if (parent_entity.first_child == child) {
        parent_entity.first_child = child_entity.next_sibling;
    }

    if (parent_entity.last_child == child) {
        parent_entity.last_child = child_entity.prev_sibling;
    }

    child_entity.parent = {};
}

template <typename T, typename... Components>
T& spawn(World& world, T value, Transform transform, Components... components) {
    value.transform = transform;

    T& entity = spawn(world, value);
    (add_child(world, entity.id, spawn(world, components).id), ...);
    return entity;
};

template <typename T>
T& spawn(World& world, T value) {
    int id;

    if (is_empty(world.unused_ids)) {
        id = world.entities.count;
        add(world.entities, {});
    } else {
        id = pop_back(world.unused_ids);
    }

    Storage& storage = get_storage<T>(world);

    const int index = storage.count;

    EntityMeta& meta = world.entities[id];

    assert(meta.gen <= 0);

    meta = EntityMeta{
        .gen = -meta.gen + 1,
        .type_id = storage.id,
        .index = index,
    };

    EntityId entity_id = EntityId{ .id = id, .gen = meta.gen, .type_id = storage.id };

    add(storage, entity_id, reinterpret_cast<byte*>(&value));

    T& entity = reinterpret_cast<T&>(get_entity(storage, index));

    entity.id = entity_id;

    return entity;
}

void delete_entity(World& world, const EntityId entity_id) {
    EntityMeta& meta = world.entities[entity_id.id];
    Entity& entity = get_entity(world, entity_id);
    
    Storage& storage = get_storage(world, meta.type_id);

    if (const EntityId moved_entity_id = remove(storage, meta.index); moved_entity_id != ENTITY_NONE) {
        EntityMeta& last_entity_meta = world.entities[moved_entity_id.id];
        last_entity_meta.index = meta.index;
    }

    if (is_alive(world, entity.parent)) {
        remove_child(world, entity.parent, entity_id);
    }

    meta.gen = -meta.gen;
    entity.id.gen = meta.gen;

    add(world.unused_ids, entity_id.id);
}

void despawn(World& world, const EntityId entity_id) {
    if (!is_alive(world, entity_id)) {
        return;
    }

    if (world.is_locked) {
        add(world.ops, Operation{ .entity = entity_id });
        return;
    }

    EntityMeta& meta = world.entities[entity_id.id];
    Entity& entity = get_entity(world, entity_id);

    EntityId child_id = entity.first_child;
    Entity& child = get_entity(world, child_id);

    while (is_alive(world, child_id)) {
        const EntityId next_id = child.next_sibling;
        despawn(world, child_id);
        child_id = next_id;
    }

    entity.parent = ENTITY_NONE;
    delete_entity(world, entity_id);
}

template<typename T>
T* get(World& world, const EntityId entity) {
    EntityMeta& meta = world.entities[entity.id];
    Storage& storage = get_storage(world, meta.type_id);
    uint offset = meta.index * storage.component_byte_size;
    return reinterpret_cast<T*>(storage.data.data + offset);
}

void apply_operations(World& world) {
    for (int i = 0; i < world.ops.count; i++) {
        if (is_alive(world, world.ops[i].entity)) {
            despawn(world, world.ops[i].entity);
        }
    }

    clear(world.ops);
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

