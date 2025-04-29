std::filesystem::path get_path_of_running_executable()
{
    std::error_code ec;
    std::filesystem::path exe_path;

#if defined(_WIN32)
    wchar_t buf[MAX_PATH];
    DWORD len = ::GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (len == 0 || len == MAX_PATH)
        throw std::system_error(::GetLastError(), std::system_category(), "GetModuleFileNameW");
    exe_path = std::filesystem::path(buf, buf + len);

#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);                // get buffer size
    std::string buf(size, '\0');
    if (_NSGetExecutablePath(buf.data(), &size) != 0)
        throw std::runtime_error("_NSGetExecutablePath failed");
    exePath = std::filesystem::canonical(std::filesystem::path(buf.c_str()), ec);

#elif defined(__linux__)
    char buf[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", buf, sizeof(buf)-1);
    if (len < 0)
        throw std::system_error(errno, std::generic_category(), "readlink");
    buf[len] = '\0';
    exePath = std::filesystem::path(buf);

#endif

    if (ec)
        throw std::system_error(ec, "canonicalizing executable path");
    return exe_path;
}

std::string read_entire_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::string file_contents = { std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
    file_contents.push_back('\0');
    return file_contents;
}
