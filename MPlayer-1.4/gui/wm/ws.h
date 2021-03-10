/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * based on: AutoSpace Window System for Linux/Win32,
 *           written by pontscho/fresh!mindworkz
 */

#ifndef MPLAYER_GUI_WS_H
#define MPLAYER_GUI_WS_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "gui/dialog/dialog.h"

#include "config.h"

#ifdef HAVE_SHM
#include <X11/extensions/XShm.h>
#endif

#define  wsKeyReleased   0
#define  wsKeyPressed    1

#define  wsPLMouseButton 1
#define  wsPMMouseButton 2
#define  wsPRMouseButton 3
#define  wsP4MouseButton 4
#define  wsP5MouseButton 5
#define  wsRLMouseButton (1 + 128)
#define  wsRMMouseButton (2 + 128)
#define  wsRRMouseButton (3 + 128)
#define  wsR4MouseButton (4 + 128)
#define  wsR5MouseButton (5 + 128)
#define  wsEnterWindow   253
#define  wsLeaveWindow   254
#define  wsMoveMouse     255

#define  wsShowMouseCursor   1
#define  wsHideMouseCursor   0
#define  wsHandleMouseButton 2
#define  wsHandleMouseMove   4

#define  wsHideFrame    0
#define  wsShowFrame    1
#define  wsMaxSize      2
#define  wsMinSize      4
#define  wsShowWindow   8
#define  wsHideWindow   16
#define  wsOverredirect 32
#define  wsAspect       128

#define wsNone                   0
#define wsWindowVisible          1
#define wsWindowPartialVisible   2
#define wsWindowNotVisible       4
#define wsWindowMapped           8
#define wsWindowUnmapped        16
#define wsWindowFocusIn         32
#define wsWindowFocusOut        64
#define wsWindowExpose         128
#define wsWindowRolled         256
#define wsWindowClosed         512

#define wsNo         0
#define wsMapped     1
#define wsFocused    2
#define wsVisible    3
#define wsNotVisible 4
#define wsPVisible   5
#define wsRolled     6

typedef   void (*wsTDrawHandler)(void);
typedef   void (*wsTMouseHandler)(int Button, int X, int Y, int RX, int RY);
typedef   void (*wsTKeyHandler)(int KeyCode, int Type, int Key);
typedef   void (*wsTDNDHandler)(int num, char **str);

typedef struct {
    Window WindowID;
    Window Parent;
    int X, Y, Width, Height;
    int OldX, OldY, OldWidth, OldHeight;
    Bool isFullScreen;
    int Property;
    unsigned char *bImage;
    XImage *xImage;
    Pixmap Mask;
    Bool Decoration;

    int State;
    int Visible;
    int Mapped;
    int Focused;
    int Rolled;

    wsTDrawHandler DrawHandler;
    wsTMouseHandler MouseHandler;
    wsTKeyHandler KeyHandler;
    wsTDNDHandler DNDHandler;

    int Alt;
    int Shift;
    int Control;
    int NumLock;
    int CapsLock;
/* Misc ------------------------------------------------------------------------------------- */

    Atom AtomDeleteWindow;
    Atom AtomTakeFocus;
    Atom AtomRolle;
    Atom AtomProtocols;
    Atom AtomsProtocols[3];
    Atom AtomLeaderClient;
    Atom AtomWMSizeHint;
    Atom AtomWMNormalHint;

#ifdef HAVE_SHM
    XShmSegmentInfo Shminfo;
#endif
    unsigned char *ImageData;
    unsigned short int *ImageDataw;
    unsigned int *ImageDatadw;
    GC wGC;
    unsigned long WindowMask;
    XVisualInfo VisualInfo;
    XSetWindowAttributes WindowAttrib;
    XWMHints WMHints;

    Cursor wsCursor;
    char wsCursorData[1];
    Pixmap wsCursorPixmap;
    int wsMouseEventType;
    XColor wsColor;
} wsWindow;

extern int wsMaxX;
extern int wsMaxY;
extern int wsOrgX;
extern int wsOrgY;

extern Display *wsDisplay;

// ----------------------------------------------------------------------------------------------
void wsDone(void);
void wsInit(Display *display);
void wsSetErrorHandler(void);

void wsMouseAutohide(void);
void wsEvent(XEvent *event);
void wsEvents(void);

void wsWindowCreate(wsWindow *win, int x, int y, int w, int h, int p, int c, char *label);
void wsWindowDestroy(wsWindow *win);
void wsWindowMove(wsWindow *win, Bool abs, int x, int y);
void wsWindowMoveWithin(wsWindow *win, Bool abs, int x, int y);
void wsWindowResize(wsWindow *win, int w, int h);
void wsWindowIconify(wsWindow *win);
void wsWindowRaiseTop(Display *display, Window Win);
void wsWindowBackground(wsWindow *win, int r, int g, int b);
void wsWindowVisibility(wsWindow *win, int vis);
void wsWindowLayer(Display *display, Window Win, Bool fullscreen);
void wsWindowFullscreen(wsWindow *win);
void wsWindowRedraw(wsWindow *win);
void wsWindowShape(wsWindow *win, char *data);
void wsWindowIcon(Display *display, Window Win, guiIcon_t *icon);

void wsMouseVisibility(wsWindow *win, int visibility);

// ----------------------------------------------------------------------------------------------
// Image handling
// ----------------------------------------------------------------------------------------------
void wsImageCreate(wsWindow *win, int w, int h);
void wsImageRender(wsWindow *win, unsigned char *img);
void wsImageDraw(wsWindow *win);
void wsImageResize(wsWindow *win, int w, int h);
void wsImageDestroy(wsWindow *win);

#endif /* MPLAYER_GUI_WS_H */
