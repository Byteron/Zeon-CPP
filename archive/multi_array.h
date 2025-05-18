template<typename... Types>
struct MultiArray {
    byte** columns{};
    
    size_t count{};
    size_t capacity{};

    static constexpr size_t column_count = sizeof...(Types);
    static constexpr size_t type_sizes[] = { sizeof(Types)... };

    MultiArray() {
        columns = (byte**)malloc(column_count * sizeof(byte*));
        for (size_t i = 0; i < column_count; i++) {
            columns[i] = nullptr;
        }
    }

    ~MultiArray() {
        if (columns) {
            for (size_t i = 0; i < column_count; i++) {
                free(columns[i]);
            }
            free(columns);
        }
    }
};

// Helper to get the index of a type in the type list
template<typename T, typename... Types>
struct type_index;

template<typename T, typename... Types>
struct type_index<T, T, Types...> {
    static constexpr size_t value = 0;
};

template<typename T, typename U, typename... Types>
struct type_index<T, U, Types...> {
    static constexpr size_t value = 1 + type_index<T, Types...>::value;
};

template<typename T, typename... Types>
constexpr size_t get_type_index() {
    return type_index<T, Types...>::value;
}

// Add a row of values (one for each type)
template<typename... Types>
void add(MultiArray<Types...>& array, Types... values) {
    if (array.count >= array.capacity) {
        array.capacity = array.capacity == 0 ? 8 : array.capacity * 2;
        for (size_t i = 0; i < array.column_count; i++) {
            array.columns[i] = (byte*)realloc(array.columns[i], array.capacity * array.type_sizes[i]);
        }
    }

    size_t col = 0;
    ((memcpy(array.columns[col] + (array.count * array.type_sizes[col]), &values, array.type_sizes[col]), col++), ...);
    array.count++;
}

// Get a value of type T at index
template<typename T, typename... Types>
T& get(MultiArray<Types...>& array, size_t index) {
    assert(index < array.count);
    const size_t type_index = get_type_index<T, Types...>();
    return *(T*)(array.columns[type_index] + (index * array.type_sizes[type_index]));
}

template<typename T, typename... Types>
T* get(MultiArray<Types...>& array) {
    const size_t type_index = get_type_index<T, Types...>();
    return (T*)array.columns[type_index];
}

// Remove a row at index
template<typename... Types>
void remove(MultiArray<Types...>& array, size_t index) {
    assert(index < array.count);
    
    const size_t last_index = array.count - 1;
    if (index != last_index) {
        for (size_t i = 0; i < array.column_count; i++) {
            memmove(array.columns[i] + (index * array.type_sizes[i]),
                    array.columns[i] + (last_index * array.type_sizes[i]),
                    array.type_sizes[i]);
        }
    }
    
    array.count--;
}

// Clear all rows
template<typename... Types>
void clear(MultiArray<Types...>& array) {
    array.count = 0;
}

// Reserve space for n rows
template<typename... Types>
void reserve(MultiArray<Types...>& array, size_t n) {
    if (n <= array.capacity) return;
    
    array.capacity = n;
    for (size_t i = 0; i < array.column_count; i++) {
        array.columns[i] = (byte*)realloc(array.columns[i], array.capacity * array.type_sizes[i]);
    }
}
