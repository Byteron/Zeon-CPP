template<typename T>
T* temp() {
    return temp<T>(_engine->temp);
}

string temp_concat(string a, string b) {
    uint size = a.length + b.length + 1;
    char* buffer = reinterpret_cast<char*>(temp_raw(_engine->temp, size));   
    memcpy(buffer, a.data, a.length);
    memcpy(buffer + a.length, b.data, b.length);
    buffer[size - 1] = '\0';
    return string { .data = buffer, .length = size };
}

string temp_concat(string a, const char* b) {
    uint size = a.length + strlen(b) + 1;
    char* buffer = reinterpret_cast<char*>(temp_raw(_engine->temp, size));   
    memcpy(buffer, a.data, a.length);
    memcpy(buffer + a.length, b, strlen(b));
    buffer[size - 1] = '\0';
    return string { .data = buffer, .length = size };
}

template<typename T>
Span<T> temp_span(int count) {
    return temp_span<T>(_engine->temp, count);
}

void reset_temp() {
    reset(_engine->temp);
}