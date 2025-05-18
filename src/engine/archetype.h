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

template<typename... Components>
struct Archetype {
    template<typename T>
    static uint get_index() {
        return get_type_index<T, Components...>();
    }
};

template<typename A, typename T>
uint index_of() {
    uint type_index = A::template get_index<T>();
    printf("Type Index: %d\n", type_index);
    return type_index;
}

inline uint component_id_counter{0};

template <typename T>
uint get_type_id() {
    static const uint id = component_id_counter++;
    return id;
}