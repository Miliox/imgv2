#include "image_viewer.hpp"

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
