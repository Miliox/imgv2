#include <algorithm>
#include <cmath>
#include <filesystem>
#include <functional>
#include <iostream>
#include <utility>

#include "SDL.h"
#include "SDL_image.h"
#include "SDLit.hpp"
#include "portable-file-dialogs.h"

#define EXIT_IF_NULL(ptr) \
    if (ptr == nullptr) { \
        std::cerr << #ptr << " is nullptr: " << SDL_GetError() << '\n'; \
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

static int eventMonitor(void *userdata, SDL_Event *event);

static SDL_FRect fitInside(SDL_Rect const& src, SDL_Rect const& dst);

int main(int argc, char** argv) {

    SDLit::init(SDL_INIT_VIDEO, IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_WEBP);

    SDL_assert_always(1 == SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1"));
    SDL_assert_always(1 == SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best"));
    SDL_assert_always(1 == SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1"));

    if (!pfd::settings::available())
    {
        std::cout << "Portable File Dialogs are not available on this platform.\n";
        return 1;
    }
    pfd::settings::verbose(true);

    auto open_file_dialog = pfd::open_file{"Choose an image file", pfd::path::home(),
                                           { "All images", "*.bmp *.jpg *.jpeg *.png *.qoi" }};

    auto open_file_result = open_file_dialog.result();
    if (open_file_result.size() != 1) {
        std::cout << "Failed to open file\n";
        return 1;
    }

    std::filesystem::path const image_filepath{open_file_result.front()};

    auto const window = SDLit::make_unique(
        SDL_CreateWindow,
        image_filepath.filename().c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    EXIT_IF_NULL(window);

    auto const renderer = SDLit::make_unique(
        SDL_CreateRenderer, window.get(), -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    EXIT_IF_NULL(renderer);

    auto const image_texture = SDLit::make_unique(IMG_LoadTexture, renderer.get(), image_filepath.c_str());
    EXIT_IF_NULL(image_texture);

    SDL_Rect image_rect{};
    EXIT_IF_FAIL(SDL_QueryTexture(image_texture.get(), nullptr, nullptr, &image_rect.w, &image_rect.h));

    auto const SDL_SetRenderDrawColorEx = [](SDL_Renderer* r, SDL_Color c) -> int {
        return SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    };

    // Resize window to image but fit on desktop
    std::function<void()> resize_window = [&]() -> void {
        SDL_Rect desktop_rect{};
        WARN_IF_FAIL(SDL_GetDisplayUsableBounds(SDL_GetWindowDisplayIndex(window.get()), &desktop_rect));

        desktop_rect.w -= desktop_rect.x;
        desktop_rect.h -= desktop_rect.y;
        desktop_rect.x = 0;
        desktop_rect.y = 0;

        SDL_Rect window_rect{};
        if (image_rect.w > desktop_rect.w || image_rect.h > desktop_rect.h) {
            SDL_FRect window_rect_f = fitInside(image_rect, desktop_rect);
            window_rect.x = static_cast<int>(window_rect_f.x);
            window_rect.y = static_cast<int>(window_rect_f.y);
            window_rect.w = static_cast<int>(window_rect_f.w);
            window_rect.h = static_cast<int>(window_rect_f.h);
        } else {
            window_rect = image_rect;
        }

        SDL_SetWindowSize(window.get(), window_rect.w, window_rect.h);
        SDL_SetWindowPosition(window.get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    };
    resize_window();

    std::function<void()> repaint = [&]() {
        SDL_Rect window_rect{};
        SDL_GetWindowSizeInPixels(window.get(), &window_rect.w, &window_rect.h);

        SDL_FRect viewport_rect_f = fitInside(image_rect, window_rect);

        WARN_IF_FAIL(SDL_SetRenderDrawColorEx(renderer.get(), SDL_Color{0xC0, 0xC0, 0xC0, 0xFF}));
        WARN_IF_FAIL(SDL_RenderClear(renderer.get()));
        WARN_IF_FAIL(SDL_RenderCopyF(renderer.get(), image_texture.get(), nullptr, &viewport_rect_f));
        SDL_RenderPresent(renderer.get());
    };
    repaint();

    // Repaint inside the eventMonitor because SDL_PollEvent only emits SDL_WINDOWEVENT_SIZE_CHANGED
    // at the end of resizing operation. This allows the image to be responsive during the resizing.
    SDL_AddEventWatch(eventMonitor, &repaint);

    bool running{true};
    SDL_Event event{};
    do {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
                    repaint();
                }
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                default:
                    break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch (event.button.button) {
                case SDL_BUTTON_LEFT:
                case SDL_BUTTON_RIGHT:
                    break;
                default:
                    break;
                }
                break;
                break;
            default:
                break;
            }
        }
        SDL_Delay(1'000U / 60U);
    } while (running);

    SDL_DelEventWatch(eventMonitor, &repaint);

    return 0;
}

int eventMonitor(void* repaint_callback, SDL_Event* event) {
    // Refresh while resizing window
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        if (repaint_callback != nullptr) {
            static_cast<std::function<void()>*>(repaint_callback)->operator()();
        }
    }
    return 1;
}

SDL_FRect fitInside(SDL_Rect const& src, SDL_Rect const& dst) {
    float const src_ratio = static_cast<float>(src.w) / static_cast<float>(src.h);

    float const adjusted_width  = static_cast<float>(dst.h) * src_ratio;
    float const adjusted_height = static_cast<float>(dst.w) / src_ratio;

    SDL_FRect result{};
    if (adjusted_width <= dst.w) {
        result.x = (dst.w - adjusted_width) / 2;
        result.y = 0.0f;
        result.w = adjusted_width;
        result.h = dst.h;
    } else {
        result.x = 0.0f;
        result.y = (dst.h - adjusted_height) / 2;
        result.w = dst.w;
        result.h = adjusted_height;
    }

    return result;
}

