string read_entire_file(const char* path) {
    size_t size = 0;
    void* data = SDL_LoadFile(path, &size);
    return string { .data = static_cast<char*>(data), .length = size };
}

string read_entire_file(string path) {
    return read_entire_file(path.data);
}
