#include "native_window.h"

#import <AppKit/NSWindow.h>
#include <stdio.h>

#include "SDL.h"
#include "SDL_syswm.h"

void NativeWindow_maximize(SDL_SysWMinfo* window_info) {
    NSWindow* native_window = window_info->info.cocoa.window;
    [native_window performZoom:nil];
}

void NativeWindow_showFullScreenButton(SDL_SysWMinfo* window_info, bool visible) {
    NSWindow* native_window = window_info->info.cocoa.window;
    [[native_window standardWindowButton:(NSWindowZoomButton)] setHidden:!visible];
}

void NativeWindow_transparentTitleBar(SDL_SysWMinfo* window_info) {
    NSWindow* native_window = window_info->info.cocoa.window;
    NSWindowStyleMask style_mask = NSWindowStyleMaskFullSizeContentView | [native_window styleMask];

    [[native_window contentView] setWantsLayer:true];
    [native_window setShowsToolbarButton:false];
    [native_window setStyleMask:style_mask];
    [native_window setTitlebarAppearsTransparent:true];
    [native_window setTitleVisibility:true];
}
