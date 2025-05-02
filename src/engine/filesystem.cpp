string get_executable_directory()
{
    const char* path = SDL_GetBasePath();
    size_t length = strlen(path);

    return string { .data = const_cast<char*>(path), .length = length };
}

string read_entire_file(const char* path) {
    size_t size = 0;
    void* data = SDL_LoadFile(path, &size);
    return string { .data = static_cast<char*>(data), .length = size };
}

string read_entire_file(string path) {
    return read_entire_file(path.data);
}

