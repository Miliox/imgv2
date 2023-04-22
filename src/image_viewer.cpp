#include "image_viewer.hpp"

#include <utility>

#include "native_window.h"

#include "portable-file-dialogs.h"

std::filesystem::path pickImageDialog() {
    auto const images = pfd::open_file{"Select an image to view", pfd::path::home(),
        {"All files", "*",
         "All images", "*.avif *.bmp *.gif *.iff *.ilbm *.lbm *.jpg, *.jpeg, *.jpe, *.jif, *.jfif *.jxl *.xv *.cur *.ico *.pcx *.pcc, *.dcx *.pnm *.png *.svg *.tif *.tiff *.qoi *.tga *.xpm, *.pm *.xcf *.webp",
         "AV1 Image Format", "*.avif",
         "Bitmap Format", "*.bmp",
         "Graphics Interchange Format", "*.gif",
         "InterLeaved BitMap Format", "*.iff *.ilbm *.lbm",
         "Joint Photographic Experts Group Image Format", "*.jpg, *.jpeg, *.jpe, *.jif, *.jfif *.jxl",
         "Khoros Visualization Image File Format (VIFF)", "*.xv",
         "Microsoft Icon Format", "*.cur *.ico",
         "Paintbrush Format", "*.pcx *.pcc, *.dcx",
         "Portable Anymap File Format", "*.pnm",
         "Portable Network Graphics", "*.png",
         "Scalable Vector Graphics ", "*.svg",
         "Tagged Image File Format", "*.tif *.tiff",
         "The Quite OK Image Format", "*.qoi",
         "Truevision Graphics Adapter File Format", "*.tga",
         "X-Pixmap Format", "*.xpm, *.pm",
         "eXperimental Computing Facility Format", "*.xcf",
         "WebP image format", "*.webp"}
    }.result();

    if (images.empty()) {
        return {};
    }

    return std::filesystem::path{images.front()};
}

SDL_FRect resizeToFit(SDL_Rect const& src, SDL_Rect const& dst) {
    float const src_ratio = static_cast<float>(src.w) / static_cast<float>(src.h);

    float const adjusted_width  = static_cast<float>(dst.h) * src_ratio;
    float const adjusted_height = static_cast<float>(dst.w) / src_ratio;

    SDL_FRect inner_rect{};
    if (adjusted_width <= dst.w) {
        inner_rect.x = (dst.w - adjusted_width) / 2;
        inner_rect.y = 0.0f;
        inner_rect.w = adjusted_width;
        inner_rect.h = dst.h;
    } else {
        inner_rect.x = 0.0f;
        inner_rect.y = (dst.h - adjusted_height) / 2;
        inner_rect.w = dst.w;
        inner_rect.h = adjusted_height;
    }

    return inner_rect;
}

std::unique_ptr<ImageViewer> ImageViewer::open(std::filesystem::path const image_path) noexcept {
    auto image_surface = SDLit::make_unique(IMG_Load, image_path.c_str());
    if (not image_surface) {
        return {nullptr};
    }

    auto image_window = SDLit::make_unique(
        SDL_CreateWindow,
        image_path.filename().c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        image_surface->w,
        image_surface->h,
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_ALLOW_HIGHDPI);

    SDL_SysWMinfo image_window_manager_info{};
    if (not SDL_GetWindowWMInfo(image_window.get(), &image_window_manager_info)) {
        return {nullptr};
    }

    NativeWindow_customizeTitleBar(&image_window_manager_info);

    auto image_renderer = SDLit::make_unique(
        SDL_CreateRenderer,
        image_window.get(),
        -1,
        SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC);
    if (not image_renderer) {
        return {nullptr};
    }

    auto image_texture = SDLit::make_unique(
        SDL_CreateTextureFromSurface,
        image_renderer.get(),
        image_surface.get());
    if (not image_texture) {
        return {nullptr};
    }

    auto image_viewer = std::unique_ptr<ImageViewer>{new ImageViewer{
        std::move(image_path),
        std::move(image_window_manager_info),
        std::move(image_window),
        std::move(image_renderer),
        std::move(image_texture)}};

    image_viewer->resize();
    image_viewer->center();
    image_viewer->repaint();

    return image_viewer;
}

ImageViewer::ImageViewer(
    std::filesystem::path image_path,
    SDL_SysWMinfo window_info,
    std::unique_ptr<SDL_Window, SDLit::SDL_Deleter> window,
    std::unique_ptr<SDL_Renderer, SDLit::SDL_Deleter> renderer,
    std::unique_ptr<SDL_Texture, SDLit::SDL_Deleter> texture) noexcept
    : m_image_path{std::move(image_path)}
    ,  m_window_info{std::move(window_info)}
    ,  m_window{std::move(window)}
    ,  m_renderer{std::move(renderer)}
    ,  m_texture{std::move(texture)} {}

ImageViewer::~ImageViewer() noexcept {}

SDL_Window* ImageViewer::window() const noexcept { return m_window.get(); }

SDL_Renderer* ImageViewer::renderer() const noexcept { return m_renderer.get(); }

SDL_Texture* ImageViewer::texture() const noexcept { return m_texture.get(); }

bool ImageViewer::center() noexcept {
    SDL_SetWindowPosition(m_window.get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    return true;
}

bool ImageViewer::customizeTitlebar() noexcept {
    NativeWindow_customizeTitleBar(&m_window_info);
    return true;
}

bool ImageViewer::maximize() noexcept {
    NativeWindow_maximize(&m_window_info);
    return true;
}

bool ImageViewer::resize() noexcept {
    SDL_Rect image_rect{};
    if (SDL_QueryTexture(m_texture.get(), nullptr, nullptr, &image_rect.w, &image_rect.h)) {
        return false;
    }

    SDL_Rect desktop_rect{};
    if (SDL_GetDisplayUsableBounds(SDL_GetWindowDisplayIndex(m_window.get()), &desktop_rect)) {
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

    SDL_SetWindowSize(m_window.get(), window_rect.w, window_rect.h);
    return true;
}

bool ImageViewer::repaint() noexcept {
    SDL_Rect window_rect{};
    SDL_GetWindowSizeInPixels(m_window.get(), &window_rect.w, &window_rect.h);

    SDL_Rect image_rect{};
    if (SDL_QueryTexture(m_texture.get(), nullptr, nullptr, &image_rect.w, &image_rect.h)) {
        return false;
    }

    SDL_FRect viewport_frect = resizeToFit(image_rect, window_rect);

    if (SDL_SetRenderDrawColor(m_renderer.get(), 0xC0, 0xC0, 0xC0, 0xFF)) {
        return false;
    }

    if (SDL_RenderClear(m_renderer.get())) {
        return false;
    }

    if (SDL_RenderCopyF(m_renderer.get(), m_texture.get(), nullptr, &viewport_frect)) {
        return false;
    }

    SDL_RenderPresent(m_renderer.get());
    return true;
}
