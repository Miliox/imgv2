#import <AppKit/NSWindow.h>

void NSWindowUtil_transparentTitle(NSWindow* window) {
    [[window contentView] setWantsLayer:true];
    [[window standardWindowButton:(NSWindowZoomButton)] setHidden:true];
    [window setTitlebarAppearsTransparent:true];
    [window setShowsToolbarButton:false];
    [window setTitleVisibility:true];
    [window setStyleMask:([window styleMask] | NSWindowStyleMaskFullSizeContentView)];
}

void NSWindowUtil_performZoom(NSWindow* window) {
    [window performZoom:nil];
}
