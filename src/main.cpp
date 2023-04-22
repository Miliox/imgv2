#include "main.hpp"
#include "image_viewer.hpp"
#include "native_window.h"

static int eventMonitor(void* repaint_callback, SDL_Event* event) {
    // Refresh while resizing window
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        if (repaint_callback != nullptr) {
            static_cast<std::unique_ptr<ImageViewer>*>(repaint_callback)->get()->repaint();
        }
    }
    return 1;
}

static bool preamble() noexcept {
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

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [IMAGE]\n";
        return EXIT_FAILURE;
    }
    RET_FAIL_IF_FALSE(preamble());

    std::filesystem::path image_filepath = (argc == 2) ? argv[1] : pickImageDialog();
    RET_FAIL_IF_EMPTY(image_filepath);

    std::uint32_t const menu_user_event_id{SDL_RegisterEvents(1)};
    if (menu_user_event_id == 0xFFFFFFFF) {
        std::cerr << "There is no space for user events in sdl";
        return EXIT_FAILURE;
    }
    NativeWindow_customizeApplicationMenu(menu_user_event_id);

    auto image_viewer = ImageViewer::open(image_filepath);
    RET_FAIL_IF_NULL(image_viewer);

    // Repaint inside the eventMonitor because SDL_PollEvent only emits SDL_WINDOWEVENT_SIZE_CHANGED
    // at the end of resizing operation. This allows the image to be responsive during the resizing.
    SDL_AddEventWatch(eventMonitor, &image_viewer);

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
                    image_viewer->repaint();
                }
                if (event.window.event == SDL_WINDOWEVENT_MOVED) {
                    image_viewer->customizeTitlebar();
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
                        image_viewer->maximize();
                    }
                    break;
                default:
                    break;
                }
                break;
            default:
                if (event.type == menu_user_event_id && event.user.code == MENU_OPEN_FILE_ACTION) {
                    // image_filepath =  pickImageDialog();
                    // if (not image_filepath.empty()) {
                    //     RET_FAIL_IF_FALSE(switchImage());
                    //     repaint();
                    // }
                }
                break;
            }
        }
        SDL_Delay(1'000U / 60U);
    } while (running);

    SDL_DelEventWatch(eventMonitor, &image_viewer);

    return 0;
}

