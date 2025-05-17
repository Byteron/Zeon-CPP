template<typename T>
size_t hash(T value) {
    static_assert("unsupported type");
}

size_t hash(uint key) {
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = (key >> 16) ^ key;
    return key;
}

size_t hash(int key) {
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = (key >> 16) ^ key;
    return key;
}

// Safer hash function for strings
size_t hash(char* str) {
    if (!str) return 0;
    
    size_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Hash function for custom string type
size_t hash(string str) {
    return hash(str.data);
}

// Hash function for pointers
template<typename T>
size_t hash(T* ptr) {
    if (!ptr) return 0;
    
    size_t value = reinterpret_cast<size_t>(ptr);
    value = ((value >> 16) ^ value) * 0x45d9f3b;
    value = ((value >> 16) ^ value) * 0x45d9f3b;
    value = (value >> 16) ^ value;
    return value;
}