#import <AppKit/NSWindow.h>

void NSWindowUtil_transparentTitle(NSWindow* window) {
    [[window contentView] setWantsLayer:true];
    [window setTitlebarAppearsTransparent:true];
    [window setTitleVisibility:true];
    [window setStyleMask:([window styleMask] | NSWindowStyleMaskFullSizeContentView)];
}