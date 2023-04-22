#include "native_window.h"

#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>
#include <stdio.h>

#include "SDL.h"
#include "SDL_syswm.h"

static int32_t g_file_open_clicked_event_id;

@interface MenuHandler : NSObject
- (void)requestOpenFile;
@end

@implementation MenuHandler
- (void)requestOpenFile {
    NSLog(@"User clicked on 'Open File'.");
    SDL_Event event;
    SDL_memset(&event, 0, sizeof(event));
    event.type = g_file_open_clicked_event_id;
    SDL_PushEvent(&event);
}
@end

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

void NativeWindow_populateMenu(struct SDL_SysWMinfo* window_info, uint32_t file_open_clicked_event_id) {
    g_file_open_clicked_event_id = file_open_clicked_event_id;

    NSWindow* native_window = window_info->info.cocoa.window;
    NSMenu* main_menu = [native_window menu];

    printf("menu: %p\n", main_menu);
    printf("menu_len: %ld\n", [main_menu numberOfItems]);

    NSMenuItem* file_submenu_item = [main_menu insertItemWithTitle:@"File" action:nil keyEquivalent:@"" atIndex:1];
    NSMenu*     file_submenu = [[NSMenu alloc] initWithTitle:@"File"];

    [main_menu setSubmenu:file_submenu forItem:file_submenu_item];

    NSMenuItem* open_file_item = [file_submenu addItemWithTitle:@"Open File" action:@selector(requestOpenFile) keyEquivalent:@"o"];

    MenuHandler* menu_handler = [[MenuHandler alloc] init];
    [open_file_item setTarget:menu_handler];

    if ([menu_handler respondsToSelector:@selector(requestOpenFile)]) {
        printf("respond to selector\n");
    } else {
        printf("NOT!! respond to selector\n");
    }


    printf("menu_item: %p\n", file_submenu_item);
    // [menu_handler release];
    [file_submenu release];
}