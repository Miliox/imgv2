#pragma once
#include <filesystem>

#include "SDL.h"

/// Pick an image to view using a dialog, returns the path to it if successful otherwise empty
std::filesystem::path pickImageDialog();

/// Recalculate the rect to fit src inside dst
SDL_FRect resizeToFit(SDL_Rect const& src, SDL_Rect const& dst);