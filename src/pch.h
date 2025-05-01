#include <iostream>
#include <iterator>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <span>
#include <string>
#include <cstddef>
#include <cstdint>
#include <SDL3/SDL.h>
#include <handmade_math.h>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#else
#error Unsupported platform
#endif