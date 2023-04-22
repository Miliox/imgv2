#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SDL_SysWMinfo;

/// Customize window title bar
/// - Fill content
/// - Hide title
/// - Make transparent
/// - Remove fullscreen button
/// @note: call it again when leaving fullscreen
void NativeWindow_customizeTitleBar(struct SDL_SysWMinfo* window_info);

/// Customize application menu bar
/// - Add 'File > Open File'
/// - Emit user event to be handle in sdl event loop
/// @note: call only once
void NativeWindow_customizeApplicationMenu(uint32_t const menu_user_event_id);

/// Customize window menu bar
/// - Add 'File > Open File'
/// - Emit user event to be handle in sdl event loop
/// @note: call only once
void NativeWindow_customizeWindowMenu(struct SDL_SysWMinfo* window_info, uint32_t const menu_user_event_id);

/// Maximize window
/// @note call it again to undo the operation
void NativeWindow_maximize(struct SDL_SysWMinfo* window_info);

#define MENU_OPEN_FILE_ACTION 1

#ifdef __cplusplus
}
#endif
