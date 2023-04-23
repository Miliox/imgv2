#include "main.hpp"
#include "image_viewer.hpp"
#include "native_window.h"

#include <unordered_map>

using ImageViewerMap = std::unordered_map<std::uint32_t, std::unique_ptr<ImageViewer>>;

static int eventMonitor(void* repaint_callback, SDL_Event* event) {
    // Refresh while resizing window
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        if (repaint_callback != nullptr) {
            ImageViewerMap* image_viewer_map = static_cast<ImageViewerMap*>(repaint_callback);
            auto it = image_viewer_map->find(event->window.windowID);
            if (it != image_viewer_map->end()) {
                it->second->repaint();
            }
        }
    }
    return 1;
}

static void openImages(ImageViewerMap& image_viewer_map, std::vector<std::filesystem::path> const& image_paths) {
    for (auto const& image_path  : image_paths) {
        auto image_viewer = ImageViewer::open(image_path);
        if (image_viewer) {
            image_viewer_map[SDL_GetWindowID(image_viewer->window())] = std::move(image_viewer);
        } else {
            std::cerr << "Failed to open '" << image_path << "': " << SDL_GetError() << '\n';
        }
    }
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
    for (int i = 1; i < argc; ++i) {
        auto arg_view = std::string_view{argv[i]};
        if (arg_view.starts_with("-")) {
            std::cerr << "ImageViewer V2\n\n";
            std::cerr << "imgv2 is a simple and minimalist cross platform image viewer.\n\n";
            std::cerr << "USAGE: \n";
            std::cerr << "    " << argv[0] << " [IMAGE ...]\n";
            std::cerr << "    " << argv[0] << " -h\n";
            std::cerr << "    " << argv[0] << " --help\n";

            return (arg_view == "--help" || arg_view == "-h") ? EXIT_SUCCESS : EXIT_FAILURE;
        }
    }
    RET_FAIL_IF_FALSE(preamble());

    std::uint32_t const menu_user_event_id{SDL_RegisterEvents(1)};
    if (menu_user_event_id == 0xFFFFFFFF) {
        std::cerr << "There is no space for user events in sdl";
        return EXIT_FAILURE;
    }
    NativeWindow_customizeApplicationMenu(menu_user_event_id);

    ImageViewerMap image_viewer_map{};
    {
        std::vector<std::filesystem::path> image_paths{};
        for (int i = 1; i < argc; ++i) {
            image_paths.emplace_back(argv[i]);
        }
        if (image_paths.empty()) {
            image_paths = pickImageDialog();
        }

        openImages(image_viewer_map, image_paths);
        RET_FAIL_IF_EMPTY(image_viewer_map);
    }

    // Repaint inside the eventMonitor because SDL_PollEvent only emits SDL_WINDOWEVENT_SIZE_CHANGED
    // at the end of resizing operation. This allows the image to be responsive during the resizing.
    SDL_AddEventWatch(eventMonitor, &image_viewer_map);

    bool running{true};
    SDL_Event event{};
    do {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running &= false;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
                    auto it = image_viewer_map.find(event.window.windowID);
                    if (it != image_viewer_map.end()) {
                        it->second->repaint();
                    }
                }
                if (event.window.event == SDL_WINDOWEVENT_MOVED) {
                    auto it = image_viewer_map.find(event.window.windowID);
                    if (it != image_viewer_map.end()) {
                        it->second->customizeTitlebar();
                    }
                }
                if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    auto it = image_viewer_map.find(event.window.windowID);
                    if (it != image_viewer_map.end()) {
                        image_viewer_map.erase(it);
                    }
                    running &= !image_viewer_map.empty();
                }
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    auto it = image_viewer_map.find(event.key.windowID);
                    if (it != image_viewer_map.end()) {
                        image_viewer_map.erase(it);
                    }
                    running &= !image_viewer_map.empty();
                    break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                case SDL_BUTTON_RIGHT:
                    if (event.button.clicks == 2U) {
                        auto it = image_viewer_map.find(event.button.windowID);
                        if (it != image_viewer_map.end()) {
                            it->second->maximize();
                        }
                    }
                    break;
                break;
            default:
                if (event.type == menu_user_event_id && event.user.code == MENU_OPEN_FILE_ACTION) {
                    openImages(image_viewer_map, pickImageDialog());
                }
                break;
            }
        }
        SDL_Delay(1'000U / 60U);
    } while (running);

    SDL_DelEventWatch(eventMonitor, &image_viewer_map);

    return 0;
}

