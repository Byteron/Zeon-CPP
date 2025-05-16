template <typename T>
struct Span {
    T* data{};
    size_t count{};

    T& operator[](size_t index) {
        return data[index];
    }

    const T& operator[](size_t index) const {
        return data[index];
    }  
};

template <typename T>
size_t find(Span<T> span, T value) {
    for (size_t i = 0; i < span.count; i++) {
        if (span.data[i] == value) {
            return i;
        }
    }

    return SIZE_MAX;
}

template<typename T>
Span<T> new_span(int count) {
    Span<T> span{};
    span.data = reinterpret_cast<T*>(malloc(sizeof(T) * count));
    span.count = count;
    return span;
}
