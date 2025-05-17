
const size_t TEMP_ALLOCATOR_PAGE_SIZE = 4u * 1024u * 1024u; // 4MiB

struct TemporaryAllocator {
    Array<byte*> pages;
    size_t head{};
    size_t page{};
};

size_t __align_up(size_t value, size_t alignment) {
    return (value + (alignment - 1)) & ~(alignment - 1);
}

byte* temp_raw(TemporaryAllocator& ta, size_t size, size_t alignment = 8) {
    if (ta.pages.count == 0) {
        add(ta.pages, reinterpret_cast<byte*>(malloc(TEMP_ALLOCATOR_PAGE_SIZE)));
    }

    size_t aligned_head = __align_up(ta.head, alignment);
    size_t new_head = aligned_head + size;

    if (new_head > TEMP_ALLOCATOR_PAGE_SIZE) {
        ta.page += 1;

        if (ta.page == ta.pages.count) {
            add(ta.pages, reinterpret_cast<byte*>(malloc(TEMP_ALLOCATOR_PAGE_SIZE)));
        }
        
        ta.head = 0;
        aligned_head = 0;
        new_head = size;
    }

    byte* bytes = reinterpret_cast<byte*>(ta.pages[ta.page] + aligned_head);

    ta.head = new_head;

    return bytes;
}

template<typename T>
T* temp(TemporaryAllocator& ta, size_t alignment = alignof(T)) {
    return reinterpret_cast<T*>(temp_raw(ta, sizeof(T), alignment));
}

template<typename T>
Span<T> temp_span(TemporaryAllocator& ta, size_t count, size_t alignment = alignof(T)) {
    Span<T> span;
    span.data = reinterpret_cast<T*>(temp_raw(ta, sizeof(T) * count, alignment));
    span.count = count;
    return span;
}

void reset(TemporaryAllocator& ta) {
    ta.head = 0;
    ta.page = 0;
}

void free(TemporaryAllocator& ta) {
    for (int i = 0; i < ta.pages.count; ++i) {
        free(ta.pages[i]);
    }
}

