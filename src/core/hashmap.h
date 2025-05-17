const uint NEVER_USED_HASH = 0;
const uint REMOVED_HASH = 1;
const uint FIRST_VALID_HASH = 2;

const uint MINIMUM_CAPACITY = 32;

template<typename K, typename V>
struct HashMap {
    struct Entry {
        uint hash{};
        K key{};
        V value{};
    };

    Span<Entry> entries{};
    uint count{};

    float max_load_factor{0.75f};
};

template<typename K, typename V>
void add(HashMap<K, V>& map, const K& key, const V& value);

template<typename K, typename V>
void expand(HashMap<K, V>& map) {
    auto old_entries = map.entries;
    uint new_capacity = max(MINIMUM_CAPACITY, old_entries.count * 2);

    map.entries = new_span<typename HashMap<K, V>::Entry>(new_capacity);

    map.count = 0;
    
    for (uint i = 0; i < old_entries.count; i++) {
        auto entry = old_entries[i];

        if (entry.hash >= FIRST_VALID_HASH) {
            add(map, entry.key, entry.value);
        }
    }

    free(old_entries);
}

template<typename K, typename V>
void add(HashMap<K, V>& map, const K& key, const V& value) {
    if (map.count + 1 > map.entries.count * map.max_load_factor) {
        expand(map);
    }

    uint h = hash(key);
    
    if (h < FIRST_VALID_HASH) {
        h += FIRST_VALID_HASH;
    }

    uint index = h % map.entries.count;
    uint start_index = index;

    while (true) {
        if (map.entries[index].hash < FIRST_VALID_HASH) {
            break;
        }

        index = (index + 1) % map.entries.count;
        assert(index == start_index); // we should have found an empty slot
    }

    map.entries[index] = {h, key, value};
    map.count++;
}

template<typename K, typename V>
int find(const HashMap<K, V>& map, const K& key) {
    if (map.entries.count == 0) {
        return -1;
    }

    uint h = hash(key);
    
    if (h < FIRST_VALID_HASH) {
        h += FIRST_VALID_HASH;
    }

    uint index = h % map.entries.count;
    uint start_index = index;

    while (true) {
        // If we hit an empty slot, the key isn't here
        if (map.entries[index].hash == NEVER_USED_HASH) {
            return -1;
        }

        // If we found our key, return it
        if (map.entries[index].hash == h && map.entries[index].key == key) {
            return index;
        }

        index = (index + 1) % map.entries.count;
        if (index == start_index) {
            break;
        }
    }

    return -1;
}

template<typename K, typename V>
void set(HashMap<K, V>& map, const K& key, const V& value) {
    int index = find(map, key);

    if (index == -1) {
        add(map, key, value);
    } else {
        map.entries[index].value = value;
    }
}

template<typename K, typename V>
bool remove(HashMap<K, V>& map, const K& key) {
    int index = find(map, key);

    if (index == -1) {
        return false;
    }

    map.entries[index].hash = REMOVED_HASH;
    map.count--;
    return true;
}

template<typename K, typename V>
bool get(const HashMap<K, V>& map, const K& key, V& out_value) {
    int index = find(map, key);

    if (index == -1) {
        return false;
    }

    out_value = map.entries[index].value;
    return true;
}

template<typename K, typename V>
void free(HashMap<K, V>& map) {
    free(map.entries);
    map.count = 0;
    map.slots_filled = 0;
} 