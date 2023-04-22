#include "main.hpp"
#include "image_viewer.hpp"
#include "native_window.h"

static int eventMonitor(void *userdata, SDL_Event *event);

static int SDL_SetRenderDrawColorEx(SDL_Renderer* renderer, SDL_Color color) {
    return SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

static bool switchImage(
    std::unique_ptr<SDL_Window, SDLit::SDL_Deleter> const& window,
    std::unique_ptr<SDL_Renderer, SDLit::SDL_Deleter> const& renderer,
    std::unique_ptr<SDL_Texture, SDLit::SDL_Deleter>& image_texture,
    std::filesystem::path const& image_filepath);

static void updateWindowDecoration(SDL_Window* window);

static void maximizeWindow(SDL_Window* window);

static void populateMenu(SDL_Window* window, std::uint32_t open_file_event_id);

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [IMAGE]\n";
        return EXIT_FAILURE;
    }

    RET_FAIL_IF_FALSE(preamble());

    auto const window = SDLit::make_unique(
        SDL_CreateWindow,
        "Image Viewer V2",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    RET_FAIL_IF_NULL(window);

    std::uint32_t const open_file_event_id{SDL_RegisterEvents(1)};
    if (open_file_event_id == 0xFFFFFFFF) {
        std::cerr << "There is no space for user events in sdl";
        return EXIT_FAILURE;
    }

    populateMenu(window.get(), open_file_event_id);

    auto const renderer = SDLit::make_unique(
        SDL_CreateRenderer, window.get(), -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    RET_FAIL_IF_NULL(renderer);

    std::filesystem::path image_filepath = (argc == 2) ? argv[1] : pickImageDialog();
    RET_FAIL_IF_EMPTY(image_filepath);

    std::unique_ptr<SDL_Texture, SDLit::SDL_Deleter> image_texture{nullptr};
    RET_FAIL_IF_FALSE(switchImage(window, renderer, image_texture, image_filepath));

    std::function<void()> repaint = [&]() {
        SDL_Rect window_rect{};
        SDL_GetWindowSizeInPixels(window.get(), &window_rect.w, &window_rect.h);

        SDL_Rect image_rect{};
        WARN_IF_FAIL(SDL_QueryTexture(image_texture.get(), nullptr, nullptr, &image_rect.w, &image_rect.h));
        SDL_FRect viewport_rect_f = resizeToFit(image_rect, window_rect);

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
                if (event.window.event == SDL_WINDOWEVENT_MOVED) {
                    updateWindowDecoration(window.get());
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
                    if (event.button.clicks == 2U) {
                        maximizeWindow(window.get());
                    }
                    break;
                default:
                    break;
                }
                break;
            default:
                if (event.type == open_file_event_id) {
                    image_filepath =  pickImageDialog();
                    if (not image_filepath.empty()) {
                        RET_FAIL_IF_FALSE(switchImage(window, renderer, image_texture, image_filepath));
                        repaint();
                    }
                }
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

bool preamble() noexcept {
    if (not pfd::settings::available())
    {
        SDL_SetError("this platform has no portable-file-dialogs backend.");
        return false;
    }

    SDLit::init(SDL_INIT_VIDEO, IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_WEBP);

    std::vector<std::pair<std::string, std::string>> const hints{
        {SDL_HINT_IME_SHOW_UI, "1"},
        {SDL_HINT_RENDER_SCALE_QUALITY, "best"},
        {SDL_HINT_RENDER_VSYNC, "1"},
        {SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1"}
    };

    for (auto const& hint : hints) {
        if (not SDL_SetHint(hint.first.c_str(), hint.second.c_str())) {
            SDL_SetError("failed to set hint %s=%s: %s", hint.first.c_str(), hint.second.c_str(), std::string{SDL_GetError()}.c_str());
            return false;
        }
    }

    return true;
}

static bool switchImage(
    std::unique_ptr<SDL_Window, SDLit::SDL_Deleter> const& window,
    std::unique_ptr<SDL_Renderer, SDLit::SDL_Deleter> const& renderer,
    std::unique_ptr<SDL_Texture, SDLit::SDL_Deleter>& image_texture,
    std::filesystem::path const& image_filepath) {

    image_texture = SDLit::make_unique(IMG_LoadTexture, renderer.get(), image_filepath.c_str());
    if (not image_texture) {
        return false;
    }

    SDL_Rect image_rect{};
    if (SDL_QueryTexture(image_texture.get(), nullptr, nullptr, &image_rect.w, &image_rect.h)) {
        return false;
    }

    SDL_Rect desktop_rect{};
    if (SDL_GetDisplayUsableBounds(SDL_GetWindowDisplayIndex(window.get()), &desktop_rect)) {
        return false;
    }

    desktop_rect.w -= desktop_rect.x;
    desktop_rect.h -= desktop_rect.y;
    desktop_rect.x = 0;
    desktop_rect.y = 0;

    SDL_Rect window_rect{};
    if (image_rect.w > desktop_rect.w || image_rect.h > desktop_rect.h) {
        SDL_FRect window_rect_f = resizeToFit(image_rect, desktop_rect);
        window_rect.x = static_cast<int>(window_rect_f.x);
        window_rect.y = static_cast<int>(window_rect_f.y);
        window_rect.w = static_cast<int>(window_rect_f.w);
        window_rect.h = static_cast<int>(window_rect_f.h);
    } else {
        window_rect = image_rect;
    }

    SDL_SetWindowTitle(window.get(), image_filepath.filename().c_str());
    SDL_SetWindowSize(window.get(), window_rect.w, window_rect.h);
    SDL_SetWindowPosition(window.get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    return true;
}

void populateMenu(SDL_Window* window, std::uint32_t open_file_event_id) {
    SDL_SysWMinfo info{};
    SDL_GetWindowWMInfo(window, &info);
    NativeWindow_populateMenu(&info, open_file_event_id);
}

void updateWindowDecoration(SDL_Window* window) {
    SDL_SysWMinfo info{};
    SDL_GetWindowWMInfo(window, &info);
    NativeWindow_showFullScreenButton(&info, false);
    NativeWindow_transparentTitleBar(&info);
}

void maximizeWindow(SDL_Window* window) {
    SDL_SysWMinfo info{};
    SDL_GetWindowWMInfo(window, &info);
    NativeWindow_maximize(&info);
}
