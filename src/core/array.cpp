#include <typeinfo>
template <typename T>
struct Array {
    struct Iterator {
        T* ptr{};

        T& operator*() { return *ptr; }
        Iterator& operator++() { ++ptr; return *this; }
        bool operator!=(const Iterator& other) { return ptr != other.ptr; }
    };

    T* data;
    size_t count;
    size_t capacity;

    Iterator begin() { return Iterator(data); }
    Iterator end() { return Iterator(data + count); }

    T& operator[](size_t index) {
        assert(index >= 0 && index < count);
        return data[index];
    }

        const T& operator[](size_t index) const {
        assert(index >= 0 && index < count);
        return data[index];
    }
};

template <typename T>
bool is_empty(Array<T>& array) {
    return array.count == 0;
}

template <typename T>
void add(Array<T>& array, T value) {
    if (array.count >= array.capacity) {
        array.capacity *= 2;
        
        if (array.capacity < 8) {
            array.capacity = 8;
        }

        array.data = (T*)realloc(array.data, array.capacity * sizeof(T));
    }

    array.data[array.count++] = value;
}

template <typename T>
void remove(Array<T>& array, size_t index) {
    assert(index >= 0 && index < array.count);

    for (size_t i = index; i < array.count - 1; i++) {
        array.data[i] = array.data[i + 1];
    }

    array.count--;
}

template <typename T>
void unordered_remove(Array<T>& array, size_t index) {
    assert(index >= 0 && index < array.count);

    array.data[index] = array.data[array.count - 1];
    array.count--;
}

template <typename T>
T pop_back(Array<T>& array) {
    assert(array.count > 0);
    array.count--;
    return array.data[array.count];
}

template <typename T>
void pop_front(Array<T>& array) {
    assert(array.count > 0);

    for (size_t i = 0; i < array.count - 1; i++) {
        array.data[i] = array.data[i + 1];
    }

    array.count--;
}

template <typename T>
void reserve(Array<T>& array, size_t capacity) {
    if (capacity <= array.capacity) {
        return;
    }

    array.capacity = capacity;
    array.data = (T*)realloc(array.data, capacity * sizeof(T));
}

template <typename T>
void resize(Array<T>& array, size_t count) {
    if (count > array.count) {
        reserve(array, count);

        for (size_t i = array.count; i < count; i++) {
            array.data[i] = T();
        }
    }

    array.count = count;
}

template <typename T>
void clear(Array<T>& array) {
    array.count = 0;
}

template <typename T>
void free(Array<T>& array) {
    free(array.data);
}







