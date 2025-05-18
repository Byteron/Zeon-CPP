struct Entity {
    int id{};
    int gen{};

    bool operator==(const Entity& rhs) const noexcept {
        return id == rhs.id && gen == rhs.gen;
    }

    bool operator!=(const Entity& rhs) const noexcept {
        return !(*this == rhs);
    }
};

constexpr Entity ENTITY_NONE{};

struct TypeInfo {
    uint id{};
    uint size{};
};

// template<typename T>
// TypeInfo get_type_info() {
//     return TypeInfo{ .id = get_type_id<T>(), .size = sizeof(T) };
// }

struct Storage {
    int id{-1};
    size_t count{};
    Array<Entity> entities{};
    Span<Array<byte>> data{};

    Span<TypeInfo> types{};
};

void reserve(Storage& storage, size_t desired_items) {
    if (storage.entities.count >= desired_items) return;

    desired_items *= 2;
    if (desired_items < 8) desired_items = 8;

    resize(storage.entities, desired_items);

    for (uint i = 0; i < storage.types.count; i++) {
        resize(storage.data[i], desired_items * storage.types[i].size);
    }
}

void set(Storage& storage, const int type_index, const int entity_index, const byte* value) {
    const size_t i = storage.types[type_index].size * entity_index;
    memcpy(storage.data[type_index].data + i, value, storage.types[type_index].size);
}

byte* get(Storage& storage, const int type_index, const int entity_index) {
    const size_t i = storage.types[type_index].size * entity_index;
    return storage.data[type_index].data + i;
}

Entity remove(Storage& storage, const int index) {
    const size_t last_index = storage.count - 1;

    if (index == last_index) {
        storage.count -= 1;
        return {};
    }

    const Entity last_entity = storage.entities[last_index];

    for (uint i = 0; i < storage.types.count; i++) {
        const byte* value = get(storage, i, last_index);
        set(storage, i, index, value);
    }

    storage.entities[index] = last_entity;

    storage.count -= 1;

    return last_entity;
}

void add(Storage& storage, const Entity entity) {
    const size_t index = storage.count;
    storage.count++;
    reserve(storage, storage.count);
    storage.entities[index] = entity;
}

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

struct World {
    Array<EntityMeta> entities{};

    Array<Storage> storages{};

    Array<int> unused_ids{};

    Array<Operation> ops{};
    int lock_count{};
    bool is_locked{};
};

bool is_alive(const World& world, const Entity entity) {
    return entity.id < world.entities.count && entity.gen == world.entities[entity.id].gen;
}

int index_of(const World& world, const Entity entity) {
    return world.entities[entity.id].index;
}

int type_id_of(const World& world, const Entity entity) {
    return world.entities[entity.id].type_id;
}

template <typename T>
bool has_storage(World& world) {
    const size_t type_id = get_type_id<T>();

    if (type_id >= world.storages.count) {
        return false;
    }

    return world.storages[type_id].id == type_id;
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
Span<T> get_span(World& world) {
    Storage& storage = get_storage<T>(world);
    Span<T> span = { reinterpret_cast<T*>(storage.data.data()), storage.count };
    return span;
}

Entity get_parent(World& world, const Entity entity) {
    EntityMeta& meta = world.entities[entity.id];
    assert(is_alive(world, meta.parent));
    return meta.parent;
}

template <typename T>
bool is_a(World& world, const Entity entity) {
    size_t type_id = get_type_id<T>();
    EntityMeta& meta = world.entities[entity.id];
    return meta.type_id == type_id;
}

Vec3& get_position(World& world, const Entity entity) {
    EntityMeta& meta = world.entities[entity.id];
    return meta.local_transform.position;
}

Entity get_entity(World& world, const int entity_id) {
    EntityMeta& meta = world.entities[entity_id];
    return Entity{ .id = entity_id, .gen = meta.gen };
}
template <typename T>
Span<Entity> get_entities(World& world) {
    Storage& storage = get_storage<T>(world);
    return storage.entities;
}

template <typename T>
struct EntityPair {
    Entity entity;
    T& component;
};
template <typename T>
struct EntityComponentSpans {
    Span<T> components{};
    Span<Entity> entities{};
    size_t count{};

    EntityPair<T> operator[](size_t i) noexcept {
        return { entities[i], components[i] };
    }
};

template <typename T>
EntityComponentSpans<T> get(World& world) {
    Storage& storage = get_storage<T>(world);
    Span<T> span = { reinterpret_cast<T*>(storage.data.data()), storage.count };
    return EntityComponentSpans { .components = span, .entities = storage.entities, .count = storage.count };
}

template<typename A>
struct archetype_components;

template<typename... Components>
struct archetype_components<Archetype<Components...>> {
    static void init_storage(World& world) {
        const size_t type_id = get_type_id<Archetype<Components...>>();
        if (type_id >= world.storages.count) {
            resize(world.storages, type_id + 1);
        }
        
        Storage& storage = world.storages[type_id];
        if (storage.id == -1) {
            storage.id = type_id;
            storage.types = { TypeInfo{get_type_id<Components>(), sizeof(Components)}... };
            storage.data = { Array<byte>{}... };
        }
    }
};

template <typename A, typename ...T>
Entity spawn(World& world, T... value) {
    int id;

    if (is_empty(world.unused_ids)) {
        id = world.entities.count;
        add(world.entities, {});
    } else {
        id = pop_back(world.unused_ids);
    }

    archetype_components<A>::init_storage(world);
    Storage& storage = get_storage<A>(world);

    const int index = storage.count;

    EntityMeta& meta = world.entities[id];

    meta = EntityMeta{
        .gen = -meta.gen + 1,
        .type_id = storage.id,
        .index = index,
    };

    Entity entity = Entity{ .id = id, .gen = meta.gen };

    add(storage, entity, reinterpret_cast<byte*>(&value));

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

    add(world.unused_ids, entity.id);
}

void despawn(World& world, const Entity entity) {
    if (!is_alive(world, entity)) {
        return;
    }

    if (world.is_locked) {
        add(world.ops, Operation{ .entity = entity });
        return;
    }

    EntityMeta& meta = world.entities[entity.id];

    Entity child = meta.first_child;
    while (is_alive(world, child)) {
        const Entity next = world.entities[child.id].next_sibling;
        despawn(world, child);
        child = next;
    }

    meta.parent = {};
    delete_entity(world, entity);
}

template<typename T>
T& get(World& world, const Entity entity) {
    EntityMeta& meta = world.entities[entity.id];
    Span<T> span = get_span<T>(world);
    return span[meta.index];
}

template <typename T>
void add_component(World&world, Entity entity, T component) {
    Entity child = spawn(world, component);
    add_child(world, entity, child);
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

