struct Storage {
    int id{};
    std::size_t count{};
    std::vector<Entity> entities{};
    std::vector<uint8_t> data{};

    std::size_t component_byte_size{};
};

void reserve(Storage& storage, std::size_t desired_items) {
    if (storage.entities.size() >= desired_items) return;

    desired_items *= 2;
    if (desired_items < 8) desired_items = 8;

    storage.entities.resize(desired_items);
    storage.data.resize(desired_items * storage.component_byte_size);
}

void set(Storage& storage, const int index, const uint8_t* value) {
    const std::size_t i = storage.component_byte_size * index;
    std::memcpy(storage.data.data() + i, value, storage.component_byte_size);
}

uint8_t* get(Storage& storage, const int index) {
    const std::size_t i = storage.component_byte_size * index;
    return storage.data.data() + i;
}

Entity remove(Storage& storage, const int index) {
    const std::size_t last_index = storage.count - 1;

    if (index == last_index) {
        storage.count -= 1;
        return {};
    }

    const Entity last_entity = storage.entities[last_index];

    const uint8_t* value = get(storage, last_entity.id);
    set(storage, index, value);

    storage.entities[index] = last_entity;

    storage.count -= 1;

    return last_entity;
}

void add(Storage& storage, const Entity entity, const uint8_t* value) {
    const std::size_t index = storage.count;
    storage.count++;
    reserve(storage, storage.count);
    set(storage, index, value);
    storage.entities[index] = entity;
}

