#include "native_window.h"

#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>
#include <stdio.h>

#include "SDL.h"
#include "SDL_syswm.h"

static int32_t g_menu_user_event_id;

@interface MenuHandler : NSObject
- (void)requestOpenFile;
@end

@implementation MenuHandler
- (void)requestOpenFile {
    SDL_Event event;
    SDL_memset(&event, 0, sizeof(event));
    event.type = g_menu_user_event_id;
    event.user.code = MENU_OPEN_FILE_ACTION;
    event.user.windowID = SDL_GetWindowID(SDL_GetMouseFocus());
    SDL_PushEvent(&event);
}
- (void)requestFlipHorizontal {
    SDL_Event event;
    SDL_memset(&event, 0, sizeof(event));
    event.type = g_menu_user_event_id;
    event.user.code = MENU_EDIT_FLIP_HORIZONTAL_ACTION;
    event.user.windowID = SDL_GetWindowID(SDL_GetMouseFocus());
    SDL_PushEvent(&event);
}
- (void)requestFlipVertical {
    SDL_Event event;
    SDL_memset(&event, 0, sizeof(event));
    event.type = g_menu_user_event_id;
    event.user.code = MENU_EDIT_FLIP_VERTICAL_ACTION;
    event.user.windowID = SDL_GetWindowID(SDL_GetMouseFocus());
    SDL_PushEvent(&event);
}
@end

void NativeWindow_maximize(SDL_SysWMinfo* window_info) {
    NSWindow* native_window = window_info->info.cocoa.window;
    [native_window performZoom:nil];
}

void NativeWindow_customizeTitleBar(SDL_SysWMinfo* window_info) {
    NSWindow* native_window = window_info->info.cocoa.window;
    NSWindowStyleMask style_mask = NSWindowStyleMaskFullSizeContentView | [native_window styleMask];

    [[native_window contentView] setWantsLayer:true];
    [[native_window standardWindowButton:(NSWindowZoomButton)] setHidden:true];
    [native_window setShowsToolbarButton:false];
    [native_window setStyleMask:style_mask];
    [native_window setTitlebarAppearsTransparent:true];
    [native_window setTitleVisibility:true];
}

void NativeWindow_customizeApplicationMenu(uint32_t const menu_user_event_id) {
    g_menu_user_event_id = menu_user_event_id;

    NSMenu* main_menu = [[NSApplication sharedApplication] menu];

    NSMenuItem* file_submenu_item = [main_menu insertItemWithTitle:@"File Submenu" action:nil keyEquivalent:@"" atIndex:1];
    NSMenu*     file_submenu = [[NSMenu alloc] initWithTitle:@"File"];
    [main_menu setSubmenu:file_submenu forItem:file_submenu_item];

    NSMenuItem* open_file_item = [file_submenu addItemWithTitle:@"Open File" action:@selector(requestOpenFile) keyEquivalent:@"o"];

    NSMenuItem* edit_submenu_item = [main_menu insertItemWithTitle:@"Edit Submenu" action:nil keyEquivalent:@"" atIndex:2];
    NSMenu*     edit_submenu = [[NSMenu alloc] initWithTitle:@"Edit"];
    [main_menu setSubmenu:edit_submenu forItem:edit_submenu_item];

    NSMenuItem* flip_horizontal_item = [edit_submenu addItemWithTitle:@"Flip Horizontal" action:@selector(requestFlipHorizontal) keyEquivalent:@"f"];
    NSMenuItem* flip_vertical_item = [edit_submenu addItemWithTitle:@"Flip Vertical" action:@selector(requestFlipVertical) keyEquivalent:@"F"];

    MenuHandler* menu_handler = [[MenuHandler alloc] init];
    [open_file_item setTarget:menu_handler];
    [flip_horizontal_item setTarget:menu_handler];
    [flip_vertical_item setTarget:menu_handler];

    // [menu_handler release]; // Release will disable the menu
    [file_submenu release];
}

void NativeWindow_customizeWindowMenu(struct SDL_SysWMinfo* window_info, uint32_t const menu_user_event_id) {
    // NOP
}