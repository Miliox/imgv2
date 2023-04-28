#pragma once
#include <filesystem>
#include <memory>
#include <vector>

#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_image.h"
#include "SDLit.hpp"

/// Pick an image to view using a dialog, returns the path to it if successful otherwise empty
std::vector<std::filesystem::path> pickImageDialog();

/// Recalculate the rect to fit src inside dst
SDL_FRect resizeToFit(SDL_Rect const& src, SDL_Rect const& dst);

class ImageViewer final {
public:
    static std::unique_ptr<ImageViewer> open(std::filesystem::path const image_path) noexcept;

    ImageViewer(ImageViewer const&) = delete;
    ImageViewer(ImageViewer&&) = delete;
    ImageViewer& operator=(const ImageViewer& ) = delete;
    ImageViewer& operator=(ImageViewer&&) = delete;

    ~ImageViewer() noexcept;

    SDL_Window* window() const noexcept;
    SDL_Renderer* renderer() const noexcept;
    SDL_Texture* texture() const noexcept;

    bool center() noexcept;
    bool customizeTitlebar() noexcept;
    bool focus() noexcept;
    bool maximize() noexcept;
    bool repaint() noexcept;
    bool resize() noexcept;

    void processMouseButtonEvent(SDL_MouseButtonEvent const& event);
    void processMouseMotionEvent(SDL_MouseMotionEvent const& event);
    void processMouseWheelEvent(SDL_MouseWheelEvent const& event);

private:
    explicit ImageViewer(
        std::filesystem::path image_path,
        SDL_SysWMinfo window_info,
        std::unique_ptr<SDL_Window, SDLit::SDL_Deleter> window,
        std::unique_ptr<SDL_Renderer, SDLit::SDL_Deleter> renderer,
        std::unique_ptr<SDL_Texture, SDLit::SDL_Deleter> texture) noexcept;

    std::filesystem::path m_image_path;
    SDL_SysWMinfo m_window_info;
    std::unique_ptr<SDL_Window, SDLit::SDL_Deleter> m_window;
    std::unique_ptr<SDL_Renderer, SDLit::SDL_Deleter> m_renderer;
    std::unique_ptr<SDL_Texture, SDLit::SDL_Deleter> m_texture;
};