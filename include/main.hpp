#pragma once
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <utility>

#include "SDL.h"
#include "SDL_image.h"
#include "SDLit.hpp"
#include "portable-file-dialogs.h"

#define RET_FAIL_IF_TRUE(result) \
    if (result) { \
        std::cerr << #result << " failed: " << SDL_GetError() << '\n'; \
        return EXIT_FAILURE; \
    }

#define RET_FAIL_IF_FALSE(result) \
    if (not result) { \
        std::cerr << #result << " failed: " << SDL_GetError() << '\n'; \
        return EXIT_FAILURE; \
    }

#define RET_FAIL_IF_NULL(ptr) \
    if (ptr == nullptr) { \
        std::cerr << #ptr << " is null: " << SDL_GetError() << '\n'; \
        return EXIT_FAILURE; \
    }

#define RET_FAIL_IF_EMPTY(storage) \
    if (storage.empty()) { \
        std::cerr << #storage << " is empty: " << SDL_GetError() << '\n'; \
        return EXIT_FAILURE; \
    }

#define EXIT_IF_FAIL(result) \
    if (result != 0) { \
        std::cerr << #result << " failed: " << SDL_GetError() << '\n'; \
        return EXIT_FAILURE; \
    }

#define WARN_IF_FAIL(result) \
    if (result != 0) { \
        std::cerr << #result << " failed: " << SDL_GetError() << '\n'; \
    }

bool preamble() noexcept;