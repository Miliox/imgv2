#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SDL_SysWMinfo;

/// Maximize window, calling it again undo the operation
void NativeWindow_maximize(struct SDL_SysWMinfo* window_info);

/// Show or hide the full screen button
void NativeWindow_showFullScreenButton(struct SDL_SysWMinfo* window_info, bool visible);

/// Set title bar transparent
void NativeWindow_transparentTitleBar(struct SDL_SysWMinfo* window_info);

#ifdef __cplusplus
}
#endif
