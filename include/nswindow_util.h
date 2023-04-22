#pragma once

extern "C" {
typedef struct _NSWindow NSWindow;
void NSWindowUtil_transparentTitle(NSWindow* window);
void NSWindowUtil_performZoom(NSWindow* window);
}