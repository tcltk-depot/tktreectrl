/* 
 * tkWinTree.c --
 *
 *	Platform-specific parts of TkTreeCtrl for Microsoft Windows.
 *
 * Copyright (c) 2010 Tim Baker
 *
 * RCS: @(#) $Id$
 */

#define WINVER 0x0501 /* MingW32 */
#define _WIN32_WINNT 0x0501 /* ACTCTX stuff */

#include "tkTreeCtrl.h"
#include "tkWinInt.h"

/*
 *----------------------------------------------------------------------
 *
 * Tree_HDotLine --
 *
 *	Draws a horizontal 1-pixel-tall dotted line.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn.
 *
 *----------------------------------------------------------------------
 */

void
Tree_HDotLine(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    GC gc,			/* Graphics context. */
    int x1, int y1, int x2	/* Left, top and right coordinates. */
    )
{
    TkWinDCState state;
    HDC dc;
    HPEN pen, oldPen;
    int nw;
    int wx = x1 + tree->drawableXOrigin;
    int wy = y1 + tree->drawableYOrigin;

    dc = TkWinGetDrawableDC(tree->display, drawable, &state);
    SetROP2(dc, R2_COPYPEN);

    pen = CreatePen(PS_SOLID, 1, gc->foreground);
    oldPen = SelectObject(dc, pen);

    nw = !(wx & 1) == !(wy & 1);
    for (x1 += !nw; x1 < x2; x1 += 2) {
	MoveToEx(dc, x1, y1, NULL);
	LineTo(dc, x1 + 1, y1);
    }

    SelectObject(dc, oldPen);
    DeleteObject(pen);

    TkWinReleaseDrawableDC(drawable, dc, &state);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_VDotLine --
 *
 *	Draws a vertical 1-pixel-wide dotted line.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn.
 *
 *----------------------------------------------------------------------
 */

void
Tree_VDotLine(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    GC gc,			/* Graphics context. */
    int x1, int y1, int y2)	/* Left, top, and bottom coordinates. */
{
    TkWinDCState state;
    HDC dc;
    HPEN pen, oldPen;
    int nw;
    int wx = x1 + tree->drawableXOrigin;
    int wy = y1 + tree->drawableYOrigin;

    dc = TkWinGetDrawableDC(tree->display, drawable, &state);
    SetROP2(dc, R2_COPYPEN);

    pen = CreatePen(PS_SOLID, 1, gc->foreground);
    oldPen = SelectObject(dc, pen);

    nw = !(wx & 1) == !(wy & 1);
    for (y1 += !nw; y1 < y2; y1 += 2) {
	MoveToEx(dc, x1, y1, NULL);
	LineTo(dc, x1 + 1, y1);
    }

    SelectObject(dc, oldPen);
    DeleteObject(pen);

    TkWinReleaseDrawableDC(drawable, dc, &state);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_DrawActiveOutline --
 *
 *	Draws 0 or more sides of a rectangle, dot-on dot-off, XOR style.
 *	This is used by rectangle Elements to indicate the "active"
 *	item.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn.
 *
 *----------------------------------------------------------------------
 */

void
Tree_DrawActiveOutline(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    int x, int y,		/* Left and top coordinates. */
    int width, int height,	/* Size of rectangle. */
    int open			/* RECT_OPEN_x flags */
    )
{
    int wx = x + tree->drawableXOrigin;
    int wy = y + tree->drawableYOrigin;
    int w = !(open & RECT_OPEN_W);
    int n = !(open & RECT_OPEN_N);
    int e = !(open & RECT_OPEN_E);
    int s = !(open & RECT_OPEN_S);
    int nw, ne, sw, se;
    int i;
    TkWinDCState state;
    HDC dc;

    /* Dots on even pixels only */
    nw = !(wx & 1) == !(wy & 1);
    ne = !((wx + width - 1) & 1) == !(wy & 1);
    sw = !(wx & 1) == !((wy + height - 1) & 1);
    se = !((wx + width - 1) & 1) == !((wy + height - 1) & 1);

    dc = TkWinGetDrawableDC(tree->display, drawable, &state);
    SetROP2(dc, R2_NOT);

    if (w) /* left */
    {
	for (i = !nw; i < height; i += 2) {
	    MoveToEx(dc, x, y + i, NULL);
	    LineTo(dc, x + 1, y + i);
	}
    }
    if (n) /* top */
    {
	for (i = nw ? w * 2 : 1; i < width; i += 2) {
	    MoveToEx(dc, x + i, y, NULL);
	    LineTo(dc, x + i + 1, y);
	}
    }
    if (e) /* right */
    {
	for (i = ne ? n * 2 : 1; i < height; i += 2) {
	    MoveToEx(dc, x + width - 1, y + i, NULL);
	    LineTo(dc, x + width, y + i);
	}
    }
    if (s) /* bottom */
    {
	for (i = sw ? w * 2 : 1; i < width - (se && e); i += 2) {
	    MoveToEx(dc, x + i, y + height - 1, NULL);
	    LineTo(dc, x + i + 1, y + height - 1);
	}
    }

    TkWinReleaseDrawableDC(drawable, dc, &state);
}

/*
 * The following structure is used when drawing a number of dotted XOR
 * rectangles.
 */
struct DotStatePriv
{
    TreeCtrl *tree;
    Drawable drawable;
    HDC dc;
    TkWinDCState dcState;
    HRGN rgn;
};

/*
 *----------------------------------------------------------------------
 *
 * TreeDotRect_Setup --
 *
 *	Prepare a drawable for drawing a series of dotted XOR rectangles.
 *
 * Results:
 *	State info is returned to be used by the other TreeDotRect_xxx()
 *	procedures.
 *
 * Side effects:
 *	On Win32 and OSX the device context/graphics port is altered
 *	in preparation for drawing. On X11 a new graphics context is
 *	created.
 *
 *----------------------------------------------------------------------
 */

void
TreeDotRect_Setup(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    DotState *p			/* Where to save state info. */
    )
{
    struct DotStatePriv *dotState = (struct DotStatePriv *) p;

    if (sizeof(*dotState) > sizeof(*p))
	panic("TreeDotRect_Setup: DotState hack is too small");

    dotState->tree = tree;
    dotState->drawable = drawable;
    dotState->dc = TkWinGetDrawableDC(tree->display, drawable, &dotState->dcState);

    /* XOR drawing */
    SetROP2(dotState->dc, R2_NOT);

    /* Keep drawing inside the contentbox. */
    dotState->rgn = CreateRectRgn(
	Tree_ContentLeft(tree),
	Tree_ContentTop(tree),
	Tree_ContentRight(tree),
	Tree_ContentBottom(tree));
    SelectClipRgn(dotState->dc, dotState->rgn);
}

/*
 *----------------------------------------------------------------------
 *
 * TreeDotRect_Draw --
 *
 *	Draw a dotted XOR rectangle.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn.
 *
 *----------------------------------------------------------------------
 */

void
TreeDotRect_Draw(
    DotState *p,		/* Info returned by TreeDotRect_Setup(). */
    int x, int y,		/* Left and top coordinates. */
    int width, int height	/* Size of rectangle. */
    )
{
    struct DotStatePriv *dotState = (struct DotStatePriv *) p;
#if 1
    RECT rect;

    rect.left = x;
    rect.right = x + width;
    rect.top = y;
    rect.bottom = y + height;
    DrawFocusRect(dotState->dc, &rect);
#else
    HDC dc = dotState->dc; 
    int i;
    int wx = x + dotState->tree->drawableXOrigin;
    int wy = y + dotState->tree->drawableYOrigin;
    int nw, ne, sw, se;

    /* Dots on even pixels only */
    nw = !(wx & 1) == !(wy & 1);
    ne = !((wx + width - 1) & 1) == !(wy & 1);
    sw = !(wx & 1) == !((wy + height - 1) & 1);
    se = !((wx + width - 1) & 1) == !((wy + height - 1) & 1);

    for (i = !nw; i < height; i += 2) {
	MoveToEx(dc, x, y + i, NULL);
	LineTo(dc, x + 1, y + i);
    }
    for (i = nw ? 2 : 1; i < width; i += 2) {
	MoveToEx(dc, x + i, y, NULL);
	LineTo(dc, x + i + 1, y);
    }
    for (i = ne ? 2 : 1; i < height; i += 2) {
	MoveToEx(dc, x + width - 1, y + i, NULL);
	LineTo(dc, x + width, y + i);
    }
    for (i = sw ? 2 : 1; i < width - se; i += 2) {
	MoveToEx(dc, x + i, y + height - 1, NULL);
	LineTo(dc, x + i + 1, y + height - 1);
    }
#endif
}

/*
 *----------------------------------------------------------------------
 *
 * TreeDotRect_Restore --
 *
 *	Restore the drawing environment.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	On Win32 and OSX the device context/graphics port is restored.
 *	On X11 a new graphics context is freed.
 *
 *----------------------------------------------------------------------
 */

void
TreeDotRect_Restore(
    DotState *p			/* Info returned by TreeDotRect_Setup(). */
    )
{
    struct DotStatePriv *dotState = (struct DotStatePriv *) p;
    SelectClipRgn(dotState->dc, NULL);
    DeleteObject(dotState->rgn);
    TkWinReleaseDrawableDC(dotState->drawable, dotState->dc, &dotState->dcState);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_FillRegion --
 *
 *	Paint a region with the foreground color of a graphics context.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn.
 *
 *----------------------------------------------------------------------
 */

void
Tree_FillRegion(
    Display *display,		/* Display. */
    Drawable drawable,		/* Where to draw. */
    GC gc,			/* Foreground color. */
    TkRegion rgn		/* Region to paint. */
    )
{
    HDC dc;
    TkWinDCState dcState;
    HBRUSH brush;

    dc = TkWinGetDrawableDC(display, drawable, &dcState);
    SetROP2(dc, R2_COPYPEN);
    brush = CreateSolidBrush(gc->foreground);
    FillRgn(dc, (HRGN) rgn, brush);
    DeleteObject(brush);
    TkWinReleaseDrawableDC(drawable, dc, &dcState);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_OffsetRegion --
 *
 *	Offset a region.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
Tree_OffsetRegion(
    TkRegion region,		/* Region to modify. */
    int xOffset, int yOffset	/* Horizontal and vertical offsets. */
    )
{
    OffsetRgn((HRGN) region, xOffset, yOffset);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_UnionRegion --
 *
 *	Compute the union of 2 regions.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
Tree_UnionRegion(
    TkRegion rgnA,
    TkRegion rgnB,
    TkRegion rgnOut)
{
    CombineRgn((HRGN) rgnA, (HRGN) rgnB, (HRGN) rgnOut, RGN_OR);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_ScrollWindow --
 *
 *	Wrapper around TkScrollWindow() to fix an apparent bug with the
 *	Mac/OSX versions.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is scrolled in a drawable.
 *
 *----------------------------------------------------------------------
 */

int
Tree_ScrollWindow(
    TreeCtrl *tree,		/* Widget info. */
    GC gc,			/* Arg to TkScrollWindow(). */
    int x, int y,		/* Arg to TkScrollWindow(). */
    int width, int height,	/* Arg to TkScrollWindow(). */
    int dx, int dy,		/* Arg to TkScrollWindow(). */
    TkRegion damageRgn		/* Arg to TkScrollWindow(). */
    )
{
#if 0
    /* It would be best to call ScrollWindowEx with SW_SCROLLCHILDREN so
     * that windows in window elements scroll smoothly with a minimum of
     * redrawing. */
    HWND hwnd = TkWinGetHWND(Tk_WindowId(tree->tkwin));
    HWND hwndChild;
    RECT scrollRect, childRect;
    struct {
	int x;
	int y;
	TkWindow *winPtr;
    } winInfo[128], *winInfoPtr;
    TkWindow *winPtr = (TkWindow *) tree->tkwin;
    int winCount = 0;
    int result;

    winInfoPtr = winInfo;
    for (winPtr = winPtr->childList; winPtr != NULL; winPtr = winPtr->nextPtr) {
	if (winPtr->window != None) {
	    hwndChild = TkWinGetHWND(winPtr->window);
	    GetWindowRect(hwndChild, &childRect);
	    winInfoPtr->x = childRect.left;
	    winInfoPtr->y = childRect.top;
	    winInfoPtr->winPtr = winPtr;
	    winInfoPtr++;
	    winCount++;
	}
    }

    scrollRect.left = x;
    scrollRect.top = y;
    scrollRect.right = x + width;
    scrollRect.bottom = y + height;
    result = (ScrollWindowEx(hwnd, dx, dy, &scrollRect, NULL, (HRGN) damageRgn,
	    NULL, SW_SCROLLCHILDREN) == NULLREGION) ? 0 : 1;

    winInfoPtr = winInfo;
    while (winCount--) {
	winPtr = winInfoPtr->winPtr;
	hwndChild = TkWinGetHWND(winPtr->window);
	GetWindowRect(hwndChild, &childRect);
	if (childRect.left != winInfoPtr->x ||
		childRect.top != winInfoPtr->y) {
	    dbwin("moved window %s %d,%d\n", winPtr->pathName, childRect.left - winInfoPtr->x, childRect.top - winInfoPtr->y);
	    winPtr->changes.x += childRect.left - winInfoPtr->x;
	    winPtr->changes.y += childRect.top - winInfoPtr->y;
	    /* TkDoConfigureNotify(winPtr); */
	}
	winInfoPtr++;
    }
#else
    int result = TkScrollWindow(tree->tkwin, gc, x, y, width, height, dx, dy,
	damageRgn);
#endif

    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_UnsetClipMask --
 *
 *	Wrapper around XSetClipMask(). On Win32 Tk_DrawChars() does
 *	not clear the clipping region.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
Tree_UnsetClipMask(
    TreeCtrl *tree,		/* Widget info. */
    Drawable drawable,		/* Where to draw. */
    GC gc			/* Graphics context to modify. */
    )
{
    XSetClipMask(tree->display, gc, None);

    /* Tk_DrawChars does not clear the clip region */
    if (drawable == Tk_WindowId(tree->tkwin)) {
	HDC dc;
	TkWinDCState dcState;

	dc = TkWinGetDrawableDC(tree->display, drawable, &dcState);
	SelectClipRgn(dc, NULL);
	TkWinReleaseDrawableDC(drawable, dc, &dcState);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_DrawBitmapWithGC --
 *
 *	Draw part of a bitmap.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn.
 *
 *----------------------------------------------------------------------
 */

void
Tree_DrawBitmapWithGC(
    TreeCtrl *tree,		/* Widget info. */
    Pixmap bitmap,		/* Bitmap to draw. */
    Drawable drawable,		/* Where to draw. */
    GC gc,			/* Graphics context. */
    int src_x, int src_y,	/* Left and top of part of bitmap to copy. */
    int width, int height,	/* Width and height of part of bitmap to
				 * copy. */
    int dest_x, int dest_y	/* Left and top coordinates to copy part of
				 * the bitmap to. */
    )
{
    TkpClipMask *clipPtr = (TkpClipMask *) gc->clip_mask;

    XSetClipOrigin(tree->display, gc, dest_x, dest_y);

    /*
     * It seems as though the device context is not set up properly
     * when drawing a transparent bitmap into a window. Normally Tk draws
     * into an offscreen pixmap which gets a temporary device context.
     * This fixes a bug with -doublebuffer none in the demo "Bitmaps".
     */
    if (drawable == Tk_WindowId(tree->tkwin)) {
	if ((clipPtr != NULL) &&
	    (clipPtr->type == TKP_CLIP_PIXMAP) &&
	    (clipPtr->value.pixmap == bitmap)) {
	    HDC dc;
	    TkWinDCState dcState;

	    dc = TkWinGetDrawableDC(tree->display, drawable, &dcState);
	    SetTextColor(dc, RGB(0,0,0));
	    SetBkColor(dc, RGB(255,255,255));
	    TkWinReleaseDrawableDC(drawable, dc, &dcState);
	}
    }
    XCopyPlane(tree->display, bitmap, drawable, gc,
	src_x, src_y, (unsigned int) width, (unsigned int) height,
	dest_x, dest_y, 1);
    XSetClipOrigin(tree->display, gc, 0, 0);
}

/*
 * TIP #116 altered Tk_PhotoPutBlock API to add interp arg.
 * We need to remove that for compiling with 8.4.
 */
#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION < 5)
#define TK_PHOTOPUTBLOCK(interp, hdl, blk, x, y, w, h, cr) \
	Tk_PhotoPutBlock(hdl, blk, x, y, w, h, cr)
#define TK_PHOTOPUTZOOMEDBLOCK(interp, hdl, blk, x, y, w, h, \
		zx, zy, sx, sy, cr) \
	Tk_PhotoPutZoomedBlock(hdl, blk, x, y, w, h, \
		zx, zy, sx, sy, cr)
#else
#define TK_PHOTOPUTBLOCK	Tk_PhotoPutBlock
#define TK_PHOTOPUTZOOMEDBLOCK	Tk_PhotoPutZoomedBlock
#endif

/*
 *----------------------------------------------------------------------
 *
 * Tree_XImage2Photo --
 *
 *	Copy pixels from an XImage to a Tk photo image.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The given photo image is blanked and all the pixels from the
 *	XImage are put into the photo image.
 *
 *----------------------------------------------------------------------
 */

void
Tree_XImage2Photo(
    Tcl_Interp *interp,		/* Current interpreter. */
    Tk_PhotoHandle photoH,	/* Existing photo image. */
    XImage *ximage,		/* XImage to copy pixels from. */
    unsigned long trans,	/* Pixel value in ximage that should be
				 * considered transparent. */
    int alpha			/* Desired transparency of photo image.*/
    )
{
    Tk_PhotoImageBlock photoBlock;
    unsigned char *pixelPtr;
    int x, y, w = ximage->width, h = ximage->height;

    Tk_PhotoBlank(photoH);

    /* See TkPoscriptImage */

    pixelPtr = (unsigned char *) Tcl_Alloc(ximage->width * ximage->height * 4);
    photoBlock.pixelPtr  = pixelPtr;
    photoBlock.width     = ximage->width;
    photoBlock.height    = ximage->height;
    photoBlock.pitch     = ximage->width * 4;
    photoBlock.pixelSize = 4;
    photoBlock.offset[0] = 0;
    photoBlock.offset[1] = 1;
    photoBlock.offset[2] = 2;
    photoBlock.offset[3] = 3;

    for (y = 0; y < ximage->height; y++) {
	for (x = 0; x < ximage->width; x++) {
	    int r, g, b;
	    unsigned long pixel;

	    /* FIXME: I think this blows up on classic Mac??? */
	    pixel = XGetPixel(ximage, x, y);

	    /* Set alpha=0 for transparent pixel in the source XImage */
	    if (trans != 0 && pixel == trans) {
		pixelPtr[y * photoBlock.pitch + x * 4 + 3] = 0;
		continue;
	    }

	    r = GetRValue(pixel);
	    g = GetGValue(pixel);
	    b = GetBValue(pixel);

	    pixelPtr[y * photoBlock.pitch + x * 4 + 0] = r;
	    pixelPtr[y * photoBlock.pitch + x * 4 + 1] = g;
	    pixelPtr[y * photoBlock.pitch + x * 4 + 2] = b;
	    pixelPtr[y * photoBlock.pitch + x * 4 + 3] = alpha;
	}
    }

    TK_PHOTOPUTBLOCK(interp, photoH, &photoBlock, 0, 0, w, h,
	    TK_PHOTO_COMPOSITE_SET);

    Tcl_Free((char *) pixelPtr);
}

typedef struct {
    TreeCtrl *tree;
    TreeClip *clip;
    HDC dc;
    TkRegion region;
} TreeClipStateDC;

static void
TreeClip_ToDC(
    TreeCtrl *tree,		/* Widget info. */
    TreeClip *clip,		/* Clipping area or NULL. */
    HDC dc,			/* Windows device context. */
    TreeClipStateDC *state
    )
{
    state->tree = tree;
    state->clip = clip;
    state->dc = dc;
    state->region = None;

    if (clip && clip->type == TREE_CLIP_RECT) {
	state->region = Tree_GetRegion(tree);
	Tree_SetRectRegion(state->region, &clip->tr);
	SelectClipRgn(dc, (HRGN) state->region);
    }
    if (clip && clip->type == TREE_CLIP_AREA) {
	int x1, y1, x2, y2;
	XRectangle xr;
	if (Tree_AreaBbox(tree, clip->area, &x1, &y1, &x2, &y2) == 0)
	    return;
	xr.x = x1, xr.y = y1, xr.width = x2 - x1, xr.height = y2 - y1;
	state->region = Tree_GetRegion(tree);
	TkUnionRectWithRegion(&xr, state->region, state->region);
	SelectClipRgn(dc, (HRGN) state->region);
    }
    if (clip && clip->type == TREE_CLIP_REGION) {
	SelectClipRgn(dc, (HRGN) clip->region);
    }
}

static void
TreeClip_FinishDC(
    TreeClipStateDC *state
    )
{
    SelectClipRgn(state->dc, NULL);
    if (state->region != NULL)
	Tree_FreeRegion(state->tree, state->region);
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_FillRectangle --
 *
 *	Wrapper around XFillRectangle() because the clip region is
 *	ignored on Win32.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Draws onto the specified drawable.
 *
 *----------------------------------------------------------------------
 */

void
Tree_FillRectangle(
    TreeCtrl *tree,		/* Widget info. */
    TreeDrawable td,		/* Where to draw. */
    TreeClip *clip,		/* Clipping area or NULL. */
    GC gc,			/* Graphics context. */
    TreeRectangle tr		/* Rectangle to paint. */
    )
{
    HDC dc;
    TkWinDCState dcState;
    HBRUSH brush;
    RECT rect;
    TreeClipStateDC clipState;

    dc = TkWinGetDrawableDC(tree->display, td.drawable, &dcState);
    TreeClip_ToDC(tree, clip, dc, &clipState);

    brush = CreateSolidBrush(gc->foreground);
    rect.left = tr.x, rect.top = tr.y,
	rect.right = tr.x + tr.width, rect.bottom = tr.y + tr.height;
    FillRect(dc, &rect, brush);

    TreeClip_FinishDC(&clipState);
    DeleteObject(brush);
    TkWinReleaseDrawableDC(td.drawable, dc, &dcState);
}

/*** Themes ***/

#include <uxtheme.h>
#include <tmschema.h>
#include <shlwapi.h>
#include <basetyps.h> /* MingW32 */

#ifndef TMT_CONTENTMARGINS
#define TMT_CONTENTMARGINS 3602
#endif

typedef HTHEME (STDAPICALLTYPE OpenThemeDataProc)(HWND hwnd,
    LPCWSTR pszClassList);
typedef HRESULT (STDAPICALLTYPE CloseThemeDataProc)(HTHEME hTheme);
typedef HRESULT (STDAPICALLTYPE DrawThemeBackgroundProc)(HTHEME hTheme,
    HDC hdc, int iPartId, int iStateId, const RECT *pRect,
    OPTIONAL const RECT *pClipRect);
typedef HRESULT (STDAPICALLTYPE DrawThemeBackgroundExProc)(HTHEME hTheme,
    HDC hdc, int iPartId, int iStateId, const RECT *pRect,
    DTBGOPTS *PDTBGOPTS);
typedef HRESULT (STDAPICALLTYPE DrawThemeParentBackgroundProc)(HWND hwnd,
    HDC hdc, OPTIONAL const RECT *prc);
typedef HRESULT (STDAPICALLTYPE DrawThemeEdgeProc)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, const RECT *pDestRect,
    UINT uEdge, UINT uFlags, RECT *pContentRect);
typedef HRESULT (STDAPICALLTYPE DrawThemeTextProc)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount,
    DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect);
typedef HRESULT (STDAPICALLTYPE GetThemeBackgroundContentRectProc)(
    HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
    const RECT *pBoundingRect, RECT *pContentRect);
typedef HRESULT (STDAPICALLTYPE GetThemeBackgroundExtentProc)(HTHEME hTheme,
    HDC hdc, int iPartId, int iStateId, const RECT *pContentRect,
    RECT *pExtentRect);
typedef HRESULT (STDAPICALLTYPE GetThemeMarginsProc)(HTHEME, HDC,
    int iPartId, int iStateId, int iPropId, OPTIONAL RECT *prc,
    MARGINS *pMargins);
typedef HRESULT (STDAPICALLTYPE GetThemePartSizeProc)(HTHEME, HDC, int iPartId,
    int iStateId, RECT *prc, enum THEMESIZE eSize, SIZE *psz);
typedef HRESULT (STDAPICALLTYPE GetThemeTextExtentProc)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount,
    DWORD dwTextFlags, const RECT *pBoundingRect, RECT *pExtentRect);
typedef BOOL (STDAPICALLTYPE IsThemeActiveProc)(VOID);
typedef BOOL (STDAPICALLTYPE IsAppThemedProc)(VOID);
typedef BOOL (STDAPICALLTYPE IsThemePartDefinedProc)(HTHEME, int, int);
typedef HRESULT (STDAPICALLTYPE IsThemeBackgroundPartiallyTransparentProc)(
    HTHEME, int, int);

typedef struct
{
    OpenThemeDataProc				*OpenThemeData;
    CloseThemeDataProc				*CloseThemeData;
    DrawThemeBackgroundProc			*DrawThemeBackground;
    DrawThemeBackgroundExProc			*DrawThemeBackgroundEx;
    DrawThemeParentBackgroundProc		*DrawThemeParentBackground;
    DrawThemeEdgeProc				*DrawThemeEdge;
    DrawThemeTextProc				*DrawThemeText;
    GetThemeBackgroundContentRectProc		*GetThemeBackgroundContentRect;
    GetThemeBackgroundExtentProc		*GetThemeBackgroundExtent;
    GetThemeMarginsProc				*GetThemeMargins;
    GetThemePartSizeProc			*GetThemePartSize;
    GetThemeTextExtentProc			*GetThemeTextExtent;
    IsThemeActiveProc				*IsThemeActive;
    IsAppThemedProc				*IsAppThemed;
    IsThemePartDefinedProc			*IsThemePartDefined;
    IsThemeBackgroundPartiallyTransparentProc 	*IsThemeBackgroundPartiallyTransparent;
} XPThemeProcs;

typedef struct
{
    HINSTANCE hlibrary;
    XPThemeProcs *procs;
    int registered;
    int themeEnabled;
    SIZE buttonOpen;
    SIZE buttonClosed;
} XPThemeData;

typedef struct TreeThemeData_
{
    HTHEME hThemeHEADER;
    HTHEME hThemeTREEVIEW;
} TreeThemeData_;

static XPThemeProcs *procs = NULL;
static XPThemeData *appThemeData = NULL; 
TCL_DECLARE_MUTEX(themeMutex)

/* Functions imported from kernel32.dll requiring windows XP or greater. */
/* But I already link to GetVersionEx so is this importing needed? */
typedef HANDLE (STDAPICALLTYPE CreateActCtxAProc)(PCACTCTXA pActCtx);
typedef BOOL (STDAPICALLTYPE ActivateActCtxProc)(HANDLE hActCtx, ULONG_PTR *lpCookie);
typedef BOOL (STDAPICALLTYPE DeactivateActCtxProc)(DWORD dwFlags, ULONG_PTR ulCookie);
typedef VOID (STDAPICALLTYPE ReleaseActCtxProc)(HANDLE hActCtx);

typedef struct
{
    CreateActCtxAProc *CreateActCtxA;
    ActivateActCtxProc *ActivateActCtx;
    DeactivateActCtxProc *DeactivateActCtx;
    ReleaseActCtxProc *ReleaseActCtx;
} ActCtxProcs;

static ActCtxProcs *
GetActCtxProcs(void)
{
    HINSTANCE hInst;
    ActCtxProcs *procs = (ActCtxProcs *) ckalloc(sizeof(ActCtxProcs));

    hInst = LoadLibrary("kernel32.dll"); /* FIXME: leak? */
    if (hInst != 0)
    {
 #define LOADPROC(name) \
	(0 != (procs->name = (name ## Proc *)GetProcAddress(hInst, #name) ))

	if (LOADPROC(CreateActCtxA) &&
	    LOADPROC(ActivateActCtx) &&
	    LOADPROC(DeactivateActCtx) &&
	    LOADPROC(ReleaseActCtx))
	{
	    return procs;
	}

#undef LOADPROC    
    }

    ckfree((char*)procs);
    return NULL;
}

static HMODULE thisModule = NULL;

/* Return the HMODULE for this treectrl.dll. */
static HMODULE
GetMyHandle(void)
{
#if 1
    return thisModule;
#else
    HMODULE hModule = NULL;

    /* FIXME: Only >=NT so I shouldn't link to it? But I already linked to
     * GetVersionEx so will it run on 95/98? */
    GetModuleHandleEx(
	GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
	(LPCTSTR)&appThemeData,
	&hModule);
    return hModule;
#endif
}

BOOL WINAPI
DllMain(
    HINSTANCE hInst,	/* Library instance handle. */
    DWORD reason,	/* Reason this function is being called. */
    LPVOID reserved)	/* Not used. */
{
    if (reason == DLL_PROCESS_ATTACH) {
	thisModule = (HMODULE) hInst;
    }
    return TRUE;
}

static HANDLE
ActivateManifestContext(ActCtxProcs *procs, ULONG_PTR *ulpCookie)
{
    ACTCTXA actctx;
    HANDLE hCtx;
#if 1
    char myPath[1024];
    DWORD len;

    if (procs == NULL)
	return INVALID_HANDLE_VALUE;

    len = GetModuleFileName(GetMyHandle(),myPath,1024);
    myPath[len] = 0;

    ZeroMemory(&actctx, sizeof(actctx));
    actctx.cbSize = sizeof(actctx);
    actctx.lpSource = myPath;
    actctx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
#else

    if (procs == NULL)
	return INVALID_HANDLE_VALUE;

    ZeroMemory(&actctx, sizeof(actctx));
    actctx.cbSize = sizeof(actctx);
    actctx.dwFlags = ACTCTX_FLAG_HMODULE_VALID | ACTCTX_FLAG_RESOURCE_NAME_VALID;
    actctx.hModule = GetMyHandle();
#endif
    actctx.lpResourceName = MAKEINTRESOURCE(2);

    hCtx = procs->CreateActCtxA(&actctx);
    if (hCtx == INVALID_HANDLE_VALUE)
    {
	char msg[1024];
	DWORD err = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS|
		FORMAT_MESSAGE_MAX_WIDTH_MASK, 0L, err, 0, (LPVOID)msg,
		sizeof(msg), 0);
	return INVALID_HANDLE_VALUE;
    }

    if (procs->ActivateActCtx(hCtx, ulpCookie))
	return hCtx;

    return INVALID_HANDLE_VALUE;
}

static void
DeactivateManifestContext(ActCtxProcs *procs, HANDLE hCtx, ULONG_PTR ulpCookie)
{
    if (procs == NULL)
	return;

    if (hCtx != INVALID_HANDLE_VALUE)
    {
	procs->DeactivateActCtx(0, ulpCookie);
	procs->ReleaseActCtx(hCtx);
    }

    ckfree((char*)procs);
}

/* http://www.manbu.net/Lib/En/Class5/Sub16/1/29.asp */
static int
ComCtlVersionOK(void)
{
    HINSTANCE handle;
    typedef HRESULT (STDAPICALLTYPE DllGetVersionProc)(DLLVERSIONINFO *);
    DllGetVersionProc *pDllGetVersion;
    int result = FALSE;
    ActCtxProcs *procs;
    HANDLE hCtx;
    ULONG_PTR ulpCookie;

    procs = GetActCtxProcs();
    hCtx = ActivateManifestContext(procs, &ulpCookie);
    handle = LoadLibrary("comctl32.dll");
    DeactivateManifestContext(procs, hCtx, ulpCookie);
    if (handle == NULL)
	return FALSE;
    pDllGetVersion = (DllGetVersionProc *) GetProcAddress(handle,
	    "DllGetVersion");
    if (pDllGetVersion != NULL) {
	DLLVERSIONINFO dvi;

	memset(&dvi, '\0', sizeof(dvi));
	dvi.cbSize = sizeof(dvi);
	if ((*pDllGetVersion)(&dvi) == NOERROR)
	    result = dvi.dwMajorVersion >= 6;
    }
    FreeLibrary(handle);
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * LoadXPThemeProcs --
 *	Initialize XP theming support.
 *
 *	XP theme support is included in UXTHEME.DLL
 *	We dynamically load this DLL at runtime instead of linking
 *	to it at build-time.
 *
 * Returns:
 *	A pointer to an XPThemeProcs table if successful, NULL otherwise.
 *----------------------------------------------------------------------
 */

static XPThemeProcs *
LoadXPThemeProcs(HINSTANCE *phlib)
{
    OSVERSIONINFO os;

    /*
     * We have to check whether we are running at least on Windows XP.
     * In order to determine this we call GetVersionEx directly, although
     * it would be a good idea to wrap it inside a function similar to
     * TkWinGetPlatformId...
     */
    ZeroMemory(&os, sizeof(os));
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&os);
    if ((os.dwMajorVersion >= 5 && os.dwMinorVersion >= 1) ||
	    (os.dwMajorVersion > 5)) {
	/*
	 * We are running under Windows XP or a newer version.
	 * Load the library "uxtheme.dll", where the native widget
	 * drawing routines are implemented.
	 */
	HINSTANCE handle;
	*phlib = handle = LoadLibrary("uxtheme.dll");
	if (handle != 0) {
	    /*
	     * We have successfully loaded the library. Proceed in storing the
	     * addresses of the functions we want to use.
	     */
	    XPThemeProcs *procs = (XPThemeProcs*)ckalloc(sizeof(XPThemeProcs));
#define LOADPROC(name) \
	(0 != (procs->name = (name ## Proc *)GetProcAddress(handle, #name) ))

	    if (   LOADPROC(OpenThemeData)
		&& LOADPROC(CloseThemeData)
		&& LOADPROC(DrawThemeBackground)
		&& LOADPROC(DrawThemeBackgroundEx)
		&& LOADPROC(DrawThemeParentBackground)
		&& LOADPROC(DrawThemeEdge)
		&& LOADPROC(DrawThemeText)
		&& LOADPROC(GetThemeBackgroundContentRect)
		&& LOADPROC(GetThemeBackgroundExtent)
		&& LOADPROC(GetThemeMargins)
		&& LOADPROC(GetThemePartSize)
		&& LOADPROC(GetThemeTextExtent)
		&& LOADPROC(IsThemeActive)
		&& LOADPROC(IsAppThemed)
		&& LOADPROC(IsThemePartDefined)
		&& LOADPROC(IsThemeBackgroundPartiallyTransparent)
		&& ComCtlVersionOK()
	    ) {
		return procs;
	    }
#undef LOADPROC
	    ckfree((char*)procs);
	}
    }
    return 0;
}

int TreeTheme_DrawHeaderItem(TreeCtrl *tree, Drawable drawable, int state,
    int arrow, int visIndex, int x, int y, int width, int height)
{
    HTHEME hTheme;
    HDC hDC;
    TkWinDCState dcState;
    RECT rc;
    HRESULT hr;

    int iPartId = HP_HEADERITEM;
    int iStateId = HIS_NORMAL;

    switch (state) {
	case COLUMN_STATE_ACTIVE:  iStateId = HIS_HOT; break;
	case COLUMN_STATE_PRESSED: iStateId = HIS_PRESSED; break;
    }

    if (!appThemeData->themeEnabled || !procs)
	return TCL_ERROR;

    hTheme = tree->themeData->hThemeHEADER;
    if (!hTheme)
	return TCL_ERROR;

#if 0 /* Always returns FALSE */
    if (!procs->IsThemePartDefined(
	hTheme,
	iPartId,
	iStateId)) {
	return TCL_ERROR;
    }
#endif

    rc.left = x;
    rc.top = y;
    rc.right = x + width;
    rc.bottom = y + height;

    /* Is transparent for the default XP style. */
    if (procs->IsThemeBackgroundPartiallyTransparent(
	hTheme,
	iPartId,
	iStateId)) {
#if 1
	/* What color should I use? */
	Tk_Fill3DRectangle(tree->tkwin, drawable, tree->border, x, y, width, height, 0, TK_RELIEF_FLAT);
#else
	/* This draws nothing, maybe because the parent window is not
	 * themed */
	procs->DrawThemeParentBackground(
	    hwnd,
	    hDC,
	    &rc);
#endif
    }

    hDC = TkWinGetDrawableDC(tree->display, drawable, &dcState);

#if 0
    {
	/* Default XP theme gives rect 3 pixels narrower than rc */
	RECT contentRect, extentRect;
	hr = procs->GetThemeBackgroundContentRect(
	    hTheme,
	    hDC,
	    iPartId,
	    iStateId,
	    &rc,
	    &contentRect
	);
	dbwin("GetThemeBackgroundContentRect width=%d height=%d\n",
	    contentRect.right - contentRect.left,
	    contentRect.bottom - contentRect.top);

	/* Gives rc */
	hr = procs->GetThemeBackgroundExtent(
	    hTheme,
	    hDC,
	    iPartId,
	    iStateId,
	    &contentRect,
	    &extentRect
	);
	dbwin("GetThemeBackgroundExtent width=%d height=%d\n",
	    extentRect.right - extentRect.left,
	    extentRect.bottom - extentRect.top);
    }
#endif

    hr = procs->DrawThemeBackground(
	hTheme,
	hDC,
	iPartId,
	iStateId,
	&rc,
	NULL);

    TkWinReleaseDrawableDC(drawable, hDC, &dcState);

    if (hr != S_OK)
	return TCL_ERROR;

    return TCL_OK;
}

int TreeTheme_GetHeaderContentMargins(TreeCtrl *tree, int state, int arrow, int bounds[4])
{
    Window win = Tk_WindowId(tree->tkwin);
    HTHEME hTheme;
    HDC hDC;
    TkWinDCState dcState;
    HRESULT hr;
    MARGINS margins;

    int iPartId = HP_HEADERITEM;
    int iStateId = HIS_NORMAL;

    switch (state) {
	case COLUMN_STATE_ACTIVE:  iStateId = HIS_HOT; break;
	case COLUMN_STATE_PRESSED: iStateId = HIS_PRESSED; break;
    }

    if (!appThemeData->themeEnabled || !procs)
	return TCL_ERROR;

    hTheme = tree->themeData->hThemeHEADER;
    if (!hTheme)
	return TCL_ERROR;

    hDC = TkWinGetDrawableDC(tree->display, win, &dcState);

    /* The default XP themes give 3,0,0,0 which makes little sense since
     * it is the *right* side that should not be drawn over by text; the
     * 2-pixel wide header divider is on the right */
    hr = procs->GetThemeMargins(
	hTheme,
	hDC,
	iPartId,
	iStateId,
	TMT_CONTENTMARGINS,
	NULL,
	&margins);

    TkWinReleaseDrawableDC(win, hDC, &dcState);

    if (hr != S_OK)
	return TCL_ERROR;

    bounds[0] = margins.cxLeftWidth;
    bounds[1] = margins.cyTopHeight;
    bounds[2] = margins.cxRightWidth;
    bounds[3] = margins.cyBottomHeight;
/*
dbwin("margins %d %d %d %d\n", bounds[0], bounds[1], bounds[2], bounds[3]);
*/
    return TCL_OK;
}

int TreeTheme_DrawHeaderArrow(TreeCtrl *tree, Drawable drawable, int up,
    int x, int y, int width, int height)
{
#if 1
    XColor *color;
    GC gc;
    int i;

    if (!appThemeData->themeEnabled || !procs)
	return TCL_ERROR;

    color = Tk_GetColor(tree->interp, tree->tkwin, "#ACA899");
    gc = Tk_GCForColor(color, drawable);

    if (up) {
	for (i = 0; i < height; i++) {
	    XDrawLine(tree->display, drawable, gc,
		x + width / 2 - i, y + i,
		x + width / 2 + i + 1, y + i);
	}
    } else {
	for (i = 0; i < height; i++) {
	    XDrawLine(tree->display, drawable, gc,
		x + width / 2 - i, y + (height - 1) - i,
		x + width / 2 + i + 1, y + (height - 1) - i);
	}
    }

    Tk_FreeColor(color);
    return TCL_OK;
#else
    /* Doesn't seem that Microsoft actually implemented this */
    Window win = Tk_WindowId(tree->tkwin);
    HWND hwnd = Tk_GetHWND(win);
    HTHEME hTheme;
    HDC hDC;
    TkWinDCState dcState;
    RECT rc;
    HRESULT hr;

    int iPartId = HP_HEADERSORTARROW;
    int iStateId = up ? HSAS_SORTEDUP : HSAS_SORTEDDOWN;

    if (!appThemeData->themeEnabled || !procs)
	return TCL_ERROR;

    hTheme = tree->themeData->hThemeHEADER;
    if (!hTheme)
	return TCL_ERROR;

    if (!procs->IsThemePartDefined(
	hTheme,
	iPartId,
	iStateId)) {
	return TCL_ERROR;
    }

    hDC = TkWinGetDrawableDC(tree->display, drawable, &dcState);

    rc.left = x;
    rc.top = y;
    rc.right = x + width;
    rc.bottom = y + height;

    hr = procs->DrawThemeBackground(
	hTheme,
	hDC,
	iPartId,
	iStateId,
	&rc,
	NULL);

    TkWinReleaseDrawableDC(drawable, hDC, &dcState);
    return TCL_OK;
#endif /* 0 */
}

int TreeTheme_DrawButton(TreeCtrl *tree, Drawable drawable, int open,
    int x, int y, int width, int height)
{
    HTHEME hTheme;
    HDC hDC;
    TkWinDCState dcState;
    RECT rc;
    HRESULT hr;
    int iPartId, iStateId;

    if (!appThemeData->themeEnabled || !procs)
	return TCL_ERROR;

    iPartId  = TVP_GLYPH;
    iStateId = open ? GLPS_OPENED : GLPS_CLOSED;

    hTheme = tree->themeData->hThemeTREEVIEW;
    if (!hTheme)
	return TCL_ERROR;

#if 0 /* Always returns FALSE */
    if (!procs->IsThemePartDefined(
	hTheme,
	iPartId,
	iStateId)) {
	return TCL_ERROR;
    }
#endif

    hDC = TkWinGetDrawableDC(tree->display, drawable, &dcState);

    rc.left = x;
    rc.top = y;
    rc.right = x + width;
    rc.bottom = y + height;
    hr = procs->DrawThemeBackground(
	hTheme,
	hDC,
	iPartId,
	iStateId,
	&rc,
	NULL);

    TkWinReleaseDrawableDC(drawable, hDC, &dcState);

    if (hr != S_OK)
	return TCL_ERROR;

    return TCL_OK;
}

int TreeTheme_GetButtonSize(TreeCtrl *tree, Drawable drawable, int open,
    int *widthPtr, int *heightPtr)
{
    HTHEME hTheme;
    HDC hDC;
    TkWinDCState dcState;
    HRESULT hr;
    SIZE size;
    int iPartId, iStateId;

    if (!appThemeData->themeEnabled || !procs)
	return TCL_ERROR;

    /* Use cached values */
    size = open ? appThemeData->buttonOpen : appThemeData->buttonClosed;
    if (size.cx > 1) {
	*widthPtr = size.cx;
	*heightPtr = size.cy;
	return TCL_OK;
    }

    iPartId  = TVP_GLYPH;
    iStateId = open ? GLPS_OPENED : GLPS_CLOSED;

    hTheme = tree->themeData->hThemeTREEVIEW;
    if (!hTheme)
	return TCL_ERROR;

#if 0 /* Always returns FALSE */
    if (!procs->IsThemePartDefined(
	hTheme,
	iPartId,
	iStateId)) {
	return TCL_ERROR;
    }
#endif

    hDC = TkWinGetDrawableDC(tree->display, drawable, &dcState);

    /* Returns 9x9 for default XP style */
    hr = procs->GetThemePartSize(
	hTheme,
	hDC,
	iPartId,
	iStateId,
	NULL,
	TS_DRAW,
	&size
    );

    TkWinReleaseDrawableDC(drawable, hDC, &dcState);

    /* With RandomN of 10000, I eventually get hr=E_HANDLE, invalid handle */
    /* Not any longer since I don't call OpenThemeData/CloseThemeData for
     * every call. */
    if (hr != S_OK)
	return TCL_ERROR;

    /* Gave me 0,0 for a non-default theme, even though glyph existed */
    if ((size.cx <= 1) && (size.cy <= 1))
	return TCL_ERROR;

    /* Cache the values */
    if (open)
	appThemeData->buttonOpen = size;
    else
	appThemeData->buttonClosed = size;

    *widthPtr = size.cx;
    *heightPtr = size.cy;
    return TCL_OK;
}

int TreeTheme_GetArrowSize(TreeCtrl *tree, Drawable drawable, int up, int *widthPtr, int *heightPtr)
{
    if (!appThemeData->themeEnabled || !procs)
	return TCL_ERROR;

    *widthPtr = 9;
    *heightPtr = 5;
    return TCL_OK;
}

int TreeTheme_SetBorders(TreeCtrl *tree)
{
    return TCL_ERROR;
}

int
TreeTheme_DrawBorders(
    TreeCtrl *tree,
    Drawable drawable
    )
{
    return TCL_ERROR;
}

void
TreeTheme_Relayout(
    TreeCtrl *tree
    )
{
}

int
TreeTheme_IsDesktopComposited(
    TreeCtrl *tree
    )
{
    /* TODO:
	Detect Vista/Win7 use of Desktop Window Manager using
	  DwmIsCompositionEnabled().
	WndProc should listen for WM_DWMCOMPOSITIONCHANGED.
    */
#if 1
    /* On Win7 I see lots of flickering with the dragimage in the demo
     * "Explorer (Large Icons)", so Composition must not work quite how I
     * expected. */
    return FALSE;
#elif 0
    HMODULE library = LoadLibrary("dwmapi.dll");
    int result = FALSE;

    if (0 != library) {
	typedef BOOL (STDAPICALLTYPE DwmIsCompositionEnabledProc)(BOOL *pfEnabled);
	DwmIsCompositionEnabledProc *proc;

	if (0 != (proc = GetProcAddress(library, "DwmIsCompositionEnabled"))) {
	    BOOL enabled = FALSE;
	    result = SUCCEEDED(proc(&enabled)) && enabled;
	}

	FreeLibrary(library);
    }

    return result;
#else
/* http://weblogs.asp.net/kennykerr/archive/2006/08/10/Windows-Vista-for-Developers-_1320_-Part-3-_1320_-The-Desktop-Window-Manager.aspx */
bool IsCompositionEnabled()
{
    HMODULE library = ::LoadLibrary(L"dwmapi.dll");
    bool result = false;

    if (0 != library)
    {
        if (0 != ::GetProcAddress(library,
                                  "DwmIsCompositionEnabled"))
        {
            BOOL enabled = FALSE;
            result = SUCCEEDED(::DwmIsCompositionEnabled(&enabled)) && enabled;
        }

        VERIFY(::FreeLibrary(library));
    }

    return result;
}
#endif
    return FALSE;
}

#if !defined(WM_THEMECHANGED)
#define WM_THEMECHANGED 0x031A
#endif

static LRESULT WINAPI
WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    Tcl_Interp *interp = (Tcl_Interp *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg) {
	case WM_THEMECHANGED:
	    Tcl_MutexLock(&themeMutex);
	    appThemeData->themeEnabled = procs->IsThemeActive() &&
		    procs->IsAppThemed();
	    appThemeData->buttonClosed.cx = appThemeData->buttonOpen.cx = -1;
	    Tcl_MutexUnlock(&themeMutex);
	    Tree_TheWorldHasChanged(interp);
	    break;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

static CHAR windowClassName[32] = "TreeCtrlMonitorClass";

static BOOL
RegisterThemeMonitorWindowClass(HINSTANCE hinst)
{
    WNDCLASSEX wc;
    
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (WNDPROC)WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hinst;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszMenuName  = windowClassName;
    wc.lpszClassName = windowClassName;
    
    return RegisterClassEx(&wc);
}

static HWND
CreateThemeMonitorWindow(HINSTANCE hinst, Tcl_Interp *interp)
{
    CHAR title[32] = "TreeCtrlMonitorWindow";
    HWND hwnd;

    hwnd = CreateWindow(windowClassName, title, WS_OVERLAPPEDWINDOW,
	CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	NULL, NULL, hinst, NULL);
    if (!hwnd)
	return NULL;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG)interp);
    ShowWindow(hwnd, SW_HIDE);
    UpdateWindow(hwnd);

    return hwnd;
}

typedef struct PerInterpData PerInterpData;
struct PerInterpData
{
    HWND hwnd;
};

static void FreeAssocData(ClientData clientData, Tcl_Interp *interp)
{
    PerInterpData *data = (PerInterpData *) clientData;

    DestroyWindow(data->hwnd);
    ckfree((char *) data);
}

void TreeTheme_ThemeChanged(TreeCtrl *tree)
{
    Window win = Tk_WindowId(tree->tkwin);
    HWND hwnd = Tk_GetHWND(win);

    if (tree->themeData != NULL) {
	if (tree->themeData->hThemeHEADER != NULL) {
	    procs->CloseThemeData(tree->themeData->hThemeHEADER);
	    tree->themeData->hThemeHEADER = NULL;
	}
	if (tree->themeData->hThemeTREEVIEW != NULL) {
	    procs->CloseThemeData(tree->themeData->hThemeTREEVIEW);
	    tree->themeData->hThemeTREEVIEW = NULL;
	}
    }

    if (!appThemeData->themeEnabled || !procs)
	return;

    if (tree->themeData == NULL)
	tree->themeData = (TreeThemeData) ckalloc(sizeof(TreeThemeData_));

    tree->themeData->hThemeHEADER = procs->OpenThemeData(hwnd, L"HEADER");
    tree->themeData->hThemeTREEVIEW = procs->OpenThemeData(hwnd, L"TREEVIEW");
}

int TreeTheme_Init(TreeCtrl *tree)
{
    Window win = Tk_WindowId(tree->tkwin);
    HWND hwnd = Tk_GetHWND(win);

    if (!appThemeData->themeEnabled || !procs)
	return TCL_ERROR;

    tree->themeData = (TreeThemeData) ckalloc(sizeof(TreeThemeData_));

    /* http://www.codeproject.com/cs/miscctrl/themedtabpage.asp?msg=1445385#xx1445385xx */
    /* http://msdn2.microsoft.com/en-us/library/ms649781.aspx */

    tree->themeData->hThemeHEADER = procs->OpenThemeData(hwnd, L"HEADER");
    tree->themeData->hThemeTREEVIEW = procs->OpenThemeData(hwnd, L"TREEVIEW");
    return TCL_OK;
}

int TreeTheme_Free(TreeCtrl *tree)
{
    if (tree->themeData != NULL) {
	if (tree->themeData->hThemeHEADER != NULL)
	    procs->CloseThemeData(tree->themeData->hThemeHEADER);
	if (tree->themeData->hThemeTREEVIEW != NULL)
	    procs->CloseThemeData(tree->themeData->hThemeTREEVIEW);
	ckfree((char *) tree->themeData);
    }
    return TCL_OK;
}

int TreeTheme_InitInterp(Tcl_Interp *interp)
{
    HWND hwnd;
    PerInterpData *data;

    Tcl_MutexLock(&themeMutex);

    /* This is done once per-application */
    if (appThemeData == NULL) {
	appThemeData = (XPThemeData *) ckalloc(sizeof(XPThemeData));
	appThemeData->procs = LoadXPThemeProcs(&appThemeData->hlibrary);
	appThemeData->registered = FALSE;
	appThemeData->themeEnabled = FALSE;
	appThemeData->buttonClosed.cx = appThemeData->buttonOpen.cx = -1;

	procs = appThemeData->procs;

	if (appThemeData->procs) {
	    /* Check this again if WM_THEMECHANGED arrives */
	    appThemeData->themeEnabled = procs->IsThemeActive() &&
		    procs->IsAppThemed();

	    appThemeData->registered =
		RegisterThemeMonitorWindowClass(Tk_GetHINSTANCE());
	}
    }

    Tcl_MutexUnlock(&themeMutex);

    if (!procs || !appThemeData->registered)
	return TCL_ERROR;

    /* Per-interp */
    hwnd = CreateThemeMonitorWindow(Tk_GetHINSTANCE(), interp);
    if (!hwnd)
	return TCL_ERROR;

    data = (PerInterpData *) ckalloc(sizeof(PerInterpData));
    data->hwnd = hwnd;
    Tcl_SetAssocData(interp, "TreeCtrlTheme", FreeAssocData, (ClientData) data);

    return TCL_OK;
}

/*** Gradients ***/

#ifdef __MINGW32__
#include <gdiplus.h> /* Works in C, MicroSoft's does not */
#endif

/*
 * GDI+ flat api
 */

#ifndef __MINGW32__ /* With MingW32 we can just #include <gdiplus.h> */
#define WINGDIPAPI __stdcall
#define GDIPCONST const
#define VOID void
typedef enum CombineMode {
    CombineModeReplace = 0
} CombineMode;
typedef enum GpFillMode
{
    FillModeAlternate,
    FillModeWinding
} GpFillMode;
typedef enum GpUnit {
    UnitWorld = 0,
    UnitDisplay = 1,
    UnitPixel = 2,
    UnitPoint = 3,
    UnitInch = 4,
    UnitDocument = 5,
    UnitMillimeter = 6
} GpUnit;
typedef enum GpStatus {
    Ok = 0
} GpStatus;
typedef enum GpWrapMode
{
    WrapModeTile
} GpWrapMode;
typedef enum LinearGradientMode
{
    LinearGradientModeHorizontal,
    LinearGradientModeVertical
} LinearGradientMode;
typedef enum SmoothingMode {
    SmoothingModeHighQuality = 2,
    SmoothingModeAntiAlias = 4
} SmoothingMode;
typedef struct GdiplusStartupInput
{
    UINT32 GdiplusVersion;
    /*DebugEventProc*/VOID* DebugEventCallback;
    BOOL SuppressBackgroundThread;
    BOOL SuppressExternalCodecs;
} GdiplusStartupInput;
typedef struct GdiplusStartupOutput
{
    /*NotificationHookProc*/VOID* NotificationHook;
    /*NotificationUnhookProc*/VOID* NotificationUnhook;
} GdiplusStartupOutput;
typedef struct GpPoint {
    INT X;
    INT Y;
} GpPoint;
typedef struct GpRect {
    INT X;
    INT Y;
    INT Width;
    INT Height;
} GpRect;
typedef DWORD ARGB;
typedef float REAL;
typedef void GpBrush;
typedef void GpGraphics;
typedef void GpLineGradient;
typedef void GpPath;
typedef void GpPen;
typedef void GpSolidFill;
#endif /* not __MINGW32__ */

/* After gdiplus.dll is dynamically loaded, this structure is
 * filled in with pointers to functions that are used below. */
static struct
{
    HMODULE handle; /* gdiplus.dll */

    VOID* (WINGDIPAPI *_GdipAlloc)(size_t);
    VOID (WINGDIPAPI *_GdipFree)(VOID*);

    GpStatus (WINGDIPAPI *_GdiplusStartup)(ULONG_PTR*,GDIPCONST GdiplusStartupInput*,GdiplusStartupOutput*);
    VOID (WINGDIPAPI *_GdiplusShutdown)(ULONG_PTR);

    /* Graphics */
    GpStatus (WINGDIPAPI *_GdipCreateFromHDC)(HDC,GpGraphics**);
    GpStatus (WINGDIPAPI *_GdipFillRectangleI)(GpGraphics*,GpBrush*,INT,INT,INT,INT);
    GpStatus (WINGDIPAPI *_GdipDeleteGraphics)(GpGraphics*);
    GpStatus (WINGDIPAPI *_GdipDrawPath)(GpGraphics*,GpPen*,GpPath*);
    GpStatus (WINGDIPAPI *_GdipFillPath)(GpGraphics*,GpBrush*,GpPath*);
    GpStatus (WINGDIPAPI *_GdipSetClipRectI)(GpGraphics*,INT,INT,INT,INT,CombineMode);
    GpStatus (WINGDIPAPI *_GdipSetSmoothingMode)(GpGraphics*,SmoothingMode);

    /* GraphicsPath */
    GpStatus (WINGDIPAPI *_GdipCreatePath)(GpFillMode,GpPath**);
    GpStatus (WINGDIPAPI *_GdipDeletePath)(GpPath*);
    GpStatus (WINGDIPAPI *_GdipResetPath)(GpPath*);
    GpStatus (WINGDIPAPI *_GdipAddPathArcI)(GpPath*,INT,INT,INT,INT,REAL,REAL);
    GpStatus (WINGDIPAPI *_GdipAddPathLineI)(GpPath*,INT,INT,INT,INT);
    GpStatus (WINGDIPAPI *_GdipStartPathFigure)(GpPath*);
    GpStatus (WINGDIPAPI *_GdipClosePathFigure)(GpPath*);

    /* Linear Gradient brush */
    GpStatus (WINGDIPAPI *_GdipCreateLineBrushFromRectI)(GDIPCONST GpRect*,ARGB,ARGB,LinearGradientMode,GpWrapMode,GpLineGradient**);
    GpStatus (WINGDIPAPI *_GdipSetLinePresetBlend)(GpLineGradient*,GDIPCONST ARGB*,GDIPCONST REAL*,INT);
    GpStatus (WINGDIPAPI *_GdipDeleteBrush)(GpBrush*);

    /* Pen */
    GpStatus (WINGDIPAPI *_GdipCreatePen1)(ARGB,REAL,GpUnit,GpPen**);
    GpStatus (WINGDIPAPI *_GdipDeletePen)(GpPen*);

    /* SolidFill brush */
    GpStatus (WINGDIPAPI *_GdipCreateSolidFill)(ARGB,GpSolidFill**);

} DllExports = {0};

/* Per-application global data */
typedef struct
{
    ULONG_PTR token;			/* Result of GdiplusStartup() */
#if 0
    GdiplusStartupOutput output;	/* Result of GdiplusStartup() */
#endif
} TreeDrawAppData;

static TreeDrawAppData *appDrawData = NULL;

/* Tcl_CreateExitHandler() callback that shuts down GDI+ */
static void
TreeDraw_ExitHandler(
    ClientData clientData
    )
{
    if (appDrawData != NULL) {
	if (DllExports.handle != NULL)
	    DllExports._GdiplusShutdown(appDrawData->token);
    }
}

/* Load gdiplus.dll (if it exists) and fill in the DllExports global */
/* If gdiplus.dll can't be loaded DllExports.handle is set to NULL which
 * should be checked to test whether GDI+ can be used. */
static int
LoadGdiplus(void)
{
    DllExports.handle = LoadLibrary("gdiplus.dll");
    if (DllExports.handle != NULL) {
#define LOADPROC(name) \
	(0 != (DllExports._ ## name = (VOID *)GetProcAddress(DllExports.handle, #name) ))
	if (   LOADPROC(GdipAlloc)
	    && LOADPROC(GdipFree)
	    && LOADPROC(GdiplusStartup)
	    && LOADPROC(GdiplusShutdown)
	    && LOADPROC(GdipCreateFromHDC)
	    && LOADPROC(GdipFillRectangleI)
	    && LOADPROC(GdipDeleteGraphics)
	    && LOADPROC(GdipDrawPath)
	    && LOADPROC(GdipFillPath)
	    && LOADPROC(GdipSetClipRectI)
	    && LOADPROC(GdipSetSmoothingMode)
	    && LOADPROC(GdipCreatePath)
	    && LOADPROC(GdipDeletePath)
	    && LOADPROC(GdipResetPath)
	    && LOADPROC(GdipAddPathArcI)
	    && LOADPROC(GdipAddPathLineI)
	    && LOADPROC(GdipStartPathFigure)
	    && LOADPROC(GdipClosePathFigure)
	    && LOADPROC(GdipCreateLineBrushFromRectI)
	    && LOADPROC(GdipSetLinePresetBlend)
	    && LOADPROC(GdipDeleteBrush)
	    && LOADPROC(GdipCreatePen1)
	    && LOADPROC(GdipDeletePen)
	    && LOADPROC(GdipCreateSolidFill)
	) {
	    return 1;
	}
#undef LOADPROC
    }
    DllExports.handle = NULL;
    return 0;
}

/* Per-interp init */
int
TreeDraw_InitInterp(
    Tcl_Interp *interp
    )
{
    /* This is done once per-application */
    if (appDrawData == NULL) {
	appDrawData = (TreeDrawAppData *) ckalloc(sizeof(TreeDrawAppData));
	memset(appDrawData, '\0', sizeof(TreeDrawAppData));
	if (LoadGdiplus()) {
	    GdiplusStartupInput input;
	    GpStatus status;
	    input.GdiplusVersion = 1;
	    input.DebugEventCallback = NULL;
	    input.SuppressBackgroundThread = FALSE;
	    input.SuppressExternalCodecs = FALSE;
	    /* Not sure what happens when the main application or other
	     * DLLs also call this, probably it is okay. */
	    status = DllExports._GdiplusStartup(&appDrawData->token, &input,
#if 1
		NULL);
#else
		&appDrawData->output);
#endif
	    if (status != Ok) {
		DllExports.handle = NULL;
	    }
	}
	Tcl_CreateExitHandler(TreeDraw_ExitHandler, NULL);
    }

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_HasNativeGradients --
 *
 *	Determine if this platform supports gradients natively.
 *
 * Results:
 *	1 if GDI+ is available,
 *	0 otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Tree_HasNativeGradients(
    TreeCtrl *tree)
{
    return (DllExports.handle != NULL);
}

/* ARGB is a DWORD color used by GDI+ */
static ARGB MakeARGB(BYTE a, BYTE r, BYTE g, BYTE b)
{
    return (ARGB) ((((DWORD) a) << 24) | (((DWORD) r) << 16)
		 | (((DWORD) g) << 8) | ((DWORD) b));
}

static ARGB MakeGDIPlusColor(XColor *xc, double opacity)
{
    return MakeARGB(
	(BYTE)(opacity*255),
	(BYTE)((xc)->pixel & 0xFF),
	(BYTE)(((xc)->pixel >> 8) & 0xFF),
	(BYTE)(((xc)->pixel >> 16) & 0xFF));
}

typedef struct {
    TreeCtrl *tree;
    TreeClip *clip;
    GpGraphics *graphics;
} TreeClipStateGraphics;

static GpStatus
TreeClip_ToGraphics(
    TreeCtrl *tree,
    TreeClip *clip,
    GpGraphics *graphics,
    TreeClipStateGraphics *state
    )
{
    GpStatus status = Ok;

    state->tree = tree;
    state->clip = clip;
    state->graphics = graphics;

    if (clip && clip->type == TREE_CLIP_RECT) {
	status = DllExports._GdipSetClipRectI(graphics,
	    clip->tr.x, clip->tr.y, clip->tr.width, clip->tr.height,
	    CombineModeReplace);
    }
    if (clip && clip->type == TREE_CLIP_AREA) {
	int x1, y1, x2, y2;
	if (Tree_AreaBbox(tree, clip->area, &x1, &y1, &x2, &y2) == 0) {
	    x1 = y1 = x2 = y2 = 0;
	}
	status = DllExports._GdipSetClipRectI(graphics,
	    x1, y1, x2 - x1, y2 - y1, CombineModeReplace);
    }
    if (clip && clip->type == TREE_CLIP_REGION) {
	panic("TREE_CLIP_REGION unimplemented @ %s:%s", __FILE__, __LINE__);
    }

    return status;
}

static GpStatus
MakeLinearGradientBrush(
    TreeGradient gradient,	/* Gradient token. */
    TreeRectangle trBrush,	/* Brush bounds. */
    GpLineGradient **lgPtr	/* Result. */
    )
{
    GpLineGradient *lineGradient;
    GpStatus status;
    GpRect rect;
    GradientStop *stop;
    int i, nstops;
    ARGB color1, color2;

    (*lgPtr) = NULL;

    nstops = gradient->stopArrPtr->nstops;

    rect.X = trBrush.x, rect.Y = trBrush.y,
	rect.Width = trBrush.width, rect.Height = trBrush.height;

    /* BUG BUG BUG: A linear gradient brush will *sometimes* wrap when it
     * shouldn't due to rounding errors or something, resulting in a line
     * of color2 where color1 starts. The recommended solution is to
     * make the brush 1-pixel larger on all sides than the area being
     * painted.  The downside of this is you will lose a bit of the gradient. */
    if (gradient->vertical)
	rect.Y -= 1, rect.Height += 1;
    else
	rect.X -= 1, rect.Width += 1;

    stop = gradient->stopArrPtr->stops[0];
    color1 = MakeGDIPlusColor(stop->color, stop->opacity);
    stop = gradient->stopArrPtr->stops[nstops-1];
    color2 = MakeGDIPlusColor(stop->color, stop->opacity);

    status = DllExports._GdipCreateLineBrushFromRectI(
	&rect, color1, color2,
	gradient->vertical ? LinearGradientModeVertical : LinearGradientModeHorizontal,
	WrapModeTile, &lineGradient);
    if (status != Ok)
	return status;

    if (nstops > 2) {
	ARGB *col = DllExports._GdipAlloc(nstops * sizeof(ARGB));
	if (col != NULL) {
	    REAL *pos = DllExports._GdipAlloc(nstops * sizeof(REAL));
	    if (pos != NULL) {
		for (i = 0; i < nstops; i++) {
		    stop = gradient->stopArrPtr->stops[i];
		    col[i] = MakeGDIPlusColor(stop->color, stop->opacity);
		    pos[i] = (REAL)stop->offset;
		}
		status = DllExports._GdipSetLinePresetBlend(lineGradient,
		    col, pos, nstops);
		DllExports._GdipFree((void*) pos);
	    }
	    DllExports._GdipFree((void*) col);
	}
    }

    (*lgPtr) = lineGradient;
    return Ok;
}
    
/*
 *----------------------------------------------------------------------
 *
 * TreeGradient_FillRect --
 *
 *	Paint a rectangle with a gradient using GDI+.
 *
 * Results:
 *	If GDI+ isn't available then fall back to X11.  If the gradient
 *	has <2 stops then nothing is drawn.
 *
 * Side effects:
 *	Drawing.
 *
 *----------------------------------------------------------------------
 */

void
TreeGradient_FillRect(
    TreeCtrl *tree,		/* Widget info. */
    TreeDrawable td,		/* Where to draw. */
    TreeClip *clip,		/* Clipping area or NULL. */
    TreeGradient gradient,	/* Gradient token. */
    TreeRectangle trBrush,	/* Brush bounds. */
    TreeRectangle tr		/* Rectangle to paint. */
    )
{
    HDC hDC;
    TkWinDCState dcState;
    TreeClipStateGraphics clipState;
    GpGraphics *graphics;
    GpLineGradient *lineGradient = NULL;
    GpStatus status;
    GpRect rect;
    int nstops;

    if (!tree->nativeGradients || (DllExports.handle == NULL)) {
	TreeGradient_FillRectX11(tree, td, clip, gradient, trBrush, tr);
	return;
    }

    nstops = gradient->stopArrPtr->nstops;
    if (nstops < 2) /* can be 0, but < 2 isn't allowed */
	return;

    hDC = TkWinGetDrawableDC(tree->display, td.drawable, &dcState);

    status = DllExports._GdipCreateFromHDC(hDC, &graphics);
    if (status != Ok)
	goto error1;

    status = TreeClip_ToGraphics(tree, clip, graphics, &clipState);
    if (status != Ok)
	goto error2;

    status = MakeLinearGradientBrush(gradient, trBrush, &lineGradient);
    if (status != Ok)
	goto error2;

    rect.X = tr.x, rect.Y = tr.y, rect.Width = tr.width, rect.Height = tr.height;

    DllExports._GdipFillRectangleI(graphics, lineGradient,
	rect.X, rect.Y, rect.Width, rect.Height);

    DllExports._GdipDeleteBrush(lineGradient);

error2:
    DllExports._GdipDeleteGraphics(graphics);

error1:
    TkWinReleaseDrawableDC(td.drawable, hDC, &dcState);

#ifdef TREECTRL_DEBUG
    if (status != Ok) dbwin("TreeGradient_FillRect gdiplus status != Ok");
#endif
}

static void
GetRoundRectPath_Outline(
    GpPath *path,
    TreeRectangle tr,		/* Where to draw. */
    int rx, int ry,		/* Corner radius. */
    int open,			/* RECT_OPEN_x flags. */
    int fudgeX,			/* Fix for "open" edge endpoints when */
    int fudgeY			/* outlineWidth>1. */
    )
{
    int x = tr.x, y = tr.y, width = tr.width, height = tr.height;
    int drawW = (open & RECT_OPEN_W) == 0;
    int drawN = (open & RECT_OPEN_N) == 0;
    int drawE = (open & RECT_OPEN_E) == 0;
    int drawS = (open & RECT_OPEN_S) == 0;

    /* Simple case: draw all 4 corners and 4 edges */
    if (drawW && drawN && drawE && drawS) {
	width -= 1, height -= 1;
	DllExports._GdipAddPathArcI(path, x, y, rx*2, ry*2, 180, 90); /* top-left */
	DllExports._GdipAddPathArcI(path, x + width - rx*2, y, rx*2, ry*2, 270, 90); /* top-right */
	DllExports._GdipAddPathArcI(path, x + width - rx*2, y + height - ry*2, rx*2, ry*2, 0, 90); /* bottom-right */
	DllExports._GdipAddPathArcI(path, x, y + height - ry*2, rx*2, ry*2, 90, 90); /* bottom-left */
	DllExports._GdipClosePathFigure(path);

    /* Complicated case: some edges are "open" */
    } else {
	GpPoint start[4], end[4]; /* start and end points of line segments*/
	width -= 1, height -= 1;
	start[0].X = x, start[0].Y = y;
	end[3] = start[0];
	if (drawW && drawN) {
	    start[0].X += rx;
	    end[3].Y += ry;
	} else {
	    start[0].X -= fudgeX;
	    end[3].Y -= fudgeY;
	}
	end[0].X = x + width, end[0].Y = y;
	start[1]= end[0];
	if (drawE && drawN) {
	    end[0].X -= rx;
	    start[1].Y += ry;
	} else {
	    end[0].X += fudgeX;
	    start[1].Y -= fudgeY;
	}
	end[1].X = x + width, end[1].Y = y + height;
	start[2] = end[1];
	if (drawE && drawS) {
	    end[1].Y -= ry;
	    start[2].X -= rx;
	} else {
	    end[1].Y += fudgeY;
	    start[2].X += fudgeX;
	}
	end[2].X = x, end[2].Y = y + height;
	start[3] = end[2];
	if (drawW && drawS) {
	    end[2].X += rx;
	    start[3].Y -= ry;
	} else {
	    end[2].X -= fudgeX;
	    start[3].Y += fudgeY;
	}

	if (drawW && drawN)
	    DllExports._GdipAddPathArcI(path, x, y, rx*2, ry*2, 180, 90); /* top-left */
	if (drawN)
	    DllExports._GdipAddPathLineI(path, start[0].X, start[0].Y, end[0].X, end[0].Y);
	if (drawE && drawN)
	    DllExports._GdipAddPathArcI(path, x + width - rx*2, y, rx*2, ry*2, 270, 90); /* top-right */
	if (drawE)
	    DllExports._GdipAddPathLineI(path, start[1].X, start[1].Y, end[1].X, end[1].Y);
	else if (drawN)
	    DllExports._GdipStartPathFigure(path);
	if (drawE && drawS)
	    DllExports._GdipAddPathArcI(path, x + width - rx*2, y + height - ry*2, rx*2, ry*2, 0, 90); /* bottom-right */
	if (drawS)
	    DllExports._GdipAddPathLineI(path, start[2].X, start[2].Y, end[2].X, end[2].Y);
	else if (drawE)
	    DllExports._GdipStartPathFigure(path);
	if (drawW && drawS)
	    DllExports._GdipAddPathArcI(path, x, y + height - ry*2, rx*2, ry*2, 90, 90); /* bottom-left */
	if (drawW)
	    DllExports._GdipAddPathLineI(path, start[3].X, start[3].Y, end[3].X, end[3].Y);
    }
}

void
Tree_DrawRoundRect(
    TreeCtrl *tree,		/* Widget info. */
    TreeDrawable td,		/* Where to draw. */
    XColor *xcolor,		/* Color. */
    TreeRectangle tr,		/* Rectangle to draw. */
    int outlineWidth,		/* Width of outline. */
    int rx, int ry,		/* Corner radius */
    int open			/* RECT_OPEN_x flags */
    )
{
    HDC hDC;
    TkWinDCState dcState;
    GpGraphics *graphics;
    GpPath *path;
    ARGB color;
    GpPen *pen;
    GpStatus status;
    int i;

    if (!tree->nativeGradients || (DllExports.handle == NULL)) {
	GC gc = Tk_GCForColor(xcolor, td.drawable);
	Tree_DrawRoundRectX11(tree, td, gc, tr, outlineWidth, rx, ry, open);
	return;
    }

    hDC = TkWinGetDrawableDC(tree->display, td.drawable, &dcState);

    status = DllExports._GdipCreateFromHDC(hDC, &graphics);
    if (status != Ok)
	goto error1;

    status = DllExports._GdipCreatePath(FillModeAlternate, &path);
    if (status != Ok)
	goto error2;

    color = MakeGDIPlusColor(xcolor,1.0f);
    status = DllExports._GdipCreatePen1(color, 1, UnitPixel, &pen);
    if (status != Ok)
	goto error3;

    GetRoundRectPath_Outline(path, tr, rx, ry, open, 0, 0);
    DllExports._GdipDrawPath(graphics, pen, path);

    /* http://www.codeproject.com/KB/GDI-plus/RoundRect.aspx */
    for (i = 1; i < outlineWidth; i++) {
	tr.x += 1, tr.width -= 2;
	DllExports._GdipResetPath(path);
	GetRoundRectPath_Outline(path, tr, rx, ry, open, i, i-1);
	DllExports._GdipDrawPath(graphics, pen, path);
    
	tr.y += 1, tr.height -= 2;
	DllExports._GdipResetPath(path);
	GetRoundRectPath_Outline(path, tr, rx, ry, open, i, i);
	DllExports._GdipDrawPath(graphics, pen, path);
    }

    DllExports._GdipDeletePen(pen);

error3:
    DllExports._GdipDeletePath(path);

error2:
    DllExports._GdipDeleteGraphics(graphics);

error1:
    TkWinReleaseDrawableDC(td.drawable, hDC, &dcState);
}

#define ROUND_RECT_SYMMETRY_HACK

/* This returns a path 1-pixel smaller on the right and bottom edges than
 * it should be.
 * For some reason GdipFillPath produces different (and asymmetric) results
 * than GdipDrawPath.  So after filling the round rectangle with this path
 * GetRoundRectPath_Outline should be called to paint the right and bottom
 * edges. */
/* http://www.codeproject.com/KB/GDI-plus/RoundRect.aspx */
static void
GetRoundRectPath_Fill(
    GpPath *path,
    TreeRectangle tr,		/* Where to draw. */
    int rx, int ry,		/* Corner radius. */
    int open			/* RECT_OPEN_x flags. */
#ifdef ROUND_RECT_SYMMETRY_HACK
    , int rrhack
#endif
    )
{
    int x = tr.x, y = tr.y, width = tr.width, height = tr.height;
    int drawW = (open & RECT_OPEN_W) == 0;
    int drawN = (open & RECT_OPEN_N) == 0;
    int drawE = (open & RECT_OPEN_E) == 0;
    int drawS = (open & RECT_OPEN_S) == 0;

    /* Simple case: draw all 4 corners and 4 edges */
    if (drawW && drawN && drawE && drawS) {
#ifdef ROUND_RECT_SYMMETRY_HACK
	if (rrhack)
	    width -= 1, height -= 1;
#endif
	DllExports._GdipAddPathArcI(path, x, y, rx*2, ry*2, 180, 90); /* top-left */
	DllExports._GdipAddPathArcI(path, x + width - rx*2, y, rx*2, ry*2, 270, 90); /* top-right */
	DllExports._GdipAddPathArcI(path, x + width - rx*2, y + height - ry*2, rx*2, ry*2, 0, 90); /* bottom-right */
	DllExports._GdipAddPathArcI(path, x, y + height - ry*2, rx*2, ry*2, 90, 90); /* bottom-left */
	DllExports._GdipClosePathFigure(path);

    /* Complicated case: some edges are "open" */
    } else {
	GpPoint start[4], end[4]; /* start and end points of line segments*/
#ifdef ROUND_RECT_SYMMETRY_HACK
	if (rrhack) {
	    if (drawE)
		width -= 1;
	    if (drawS)
		height -= 1;
	}
#endif
	start[0].X = x, start[0].Y = y;
	end[3] = start[0];
	if (drawW && drawN) {
	    start[0].X += rx;
	    end[3].Y += ry;
	}
	end[0].X = x + width, end[0].Y = y;
	start[1]= end[0];
	if (drawE && drawN) {
	    end[0].X -= rx;
	    start[1].Y += ry;
	}
	end[1].X = x + width, end[1].Y = y + height;
	start[2] = end[1];
	if (drawE && drawS) {
	    end[1].Y -= ry;
	    start[2].X -= rx;
	}
	end[2].X = x, end[2].Y = y + height;
	start[3] = end[2];
	if (drawW && drawS) {
	    end[2].X += rx;
	    start[3].Y -= ry;
	}

	if (drawW && drawN)
	    DllExports._GdipAddPathArcI(path, x, y, rx*2, ry*2, 180, 90); /* top-left */
	DllExports._GdipAddPathLineI(path, start[0].X, start[0].Y, end[0].X, end[0].Y);
	if (drawE && drawN)
	    DllExports._GdipAddPathArcI(path, x + width - rx*2, y, rx*2, ry*2, 270, 90); /* top-right */
	DllExports._GdipAddPathLineI(path, start[1].X, start[1].Y, end[1].X, end[1].Y);
	if (drawE && drawS)
	    DllExports._GdipAddPathArcI(path, x + width - rx*2, y + height - ry*2, rx*2, ry*2, 0, 90); /* bottom-right */
	DllExports._GdipAddPathLineI(path, start[2].X, start[2].Y, end[2].X, end[2].Y);
	if (drawW && drawS)
	    DllExports._GdipAddPathArcI(path, x, y + height - ry*2, rx*2, ry*2, 90, 90); /* bottom-left */
	DllExports._GdipAddPathLineI(path, start[3].X, start[3].Y, end[3].X, end[3].Y);
    }
}

void
Tree_FillRoundRect(
    TreeCtrl *tree,		/* Widget info. */
    TreeDrawable td,		/* Where to draw. */
    XColor *xcolor,		/* Color. */
    TreeRectangle tr,		/* Where to draw. */
    int rx, int ry,		/* Corner radius */
    int open			/* RECT_OPEN_x flags */
    )
{
    HDC hDC;
    TkWinDCState dcState;
    GpGraphics *graphics;
    GpPath *path;
    ARGB color;
    GpSolidFill *brush;
#ifdef ROUND_RECT_SYMMETRY_HACK
    GpPen *pen;
#endif
    GpStatus status;

    if (!tree->nativeGradients || (DllExports.handle == NULL)) {
	GC gc = Tk_GCForColor(xcolor, td.drawable);
	Tree_FillRoundRectX11(tree, td, gc, tr, rx, ry, open);
	return;
    }

    hDC = TkWinGetDrawableDC(tree->display, td.drawable, &dcState);

    status = DllExports._GdipCreateFromHDC(hDC, &graphics);
    if (status != Ok)
	goto error1;

    status = DllExports._GdipCreatePath(FillModeAlternate, &path);
    if (status != Ok)
	goto error2;

    color = MakeGDIPlusColor(xcolor,1.0f);
    status = DllExports._GdipCreateSolidFill(color, &brush);
    if (status != Ok)
	goto error3;

#if !defined(ROUND_RECT_SYMMETRY_HACK) && 0
    /* SmoothingModeHighQuality and SmoothingModeAntiAlias seem the same. */
    DllExports._GdipSetSmoothingMode(graphics, SmoothingModeHighQuality);

    /* Antialiasing paints outside the rectangle.  If I clip drawing to the
     * rectangle I still get artifacts on the "open" edges. */
//    status = DllExports._GdipSetClipRectI(graphics,
//	tr.x, tr.y, tr.width, tr.height, CombineModeReplace);
#endif

    GetRoundRectPath_Fill(path, tr, rx, ry, open
#ifdef ROUND_RECT_SYMMETRY_HACK
	, 1
#endif
    );
    DllExports._GdipFillPath(graphics, brush, path);

#ifdef ROUND_RECT_SYMMETRY_HACK
    status = DllExports._GdipCreatePen1(color, 1, UnitPixel, &pen);
    if (status != Ok)
	goto error4;

    /* See comments above for why this is done */
    DllExports._GdipResetPath(path);
    GetRoundRectPath_Outline(path, tr, rx, ry, open, 0, 0);
    DllExports._GdipDrawPath(graphics, pen, path);

    DllExports._GdipDeletePen(pen);

error4:
#endif /* ROUND_RECT_SYMMETRY_HACK */
    DllExports._GdipDeleteBrush(brush);

error3:
    DllExports._GdipDeletePath(path);

error2:
    DllExports._GdipDeleteGraphics(graphics);

error1:
    TkWinReleaseDrawableDC(td.drawable, hDC, &dcState);
}

void
TreeGradient_FillRoundRect(
    TreeCtrl *tree,		/* Widget info. */
    TreeDrawable td,		/* Where to draw. */
    TreeGradient gradient,	/* Gradient token. */
    TreeRectangle trBrush,	/* Brush bounds. */
    TreeRectangle tr,		/* Where to draw. */
    int rx, int ry,		/* Corner radius */
    int open			/* RECT_OPEN_x flags */
    )
{
    HDC hDC;
    TkWinDCState dcState;
    GpGraphics *graphics;
    GpPath *path;
    GpLineGradient *lineGradient;
    GpStatus status;

    if (!tree->nativeGradients || (DllExports.handle == NULL)) {
	TreeGradient_FillRoundRectX11(tree, td, NULL, gradient, trBrush, tr,
	    rx, ry, open);
	return;
    }

    hDC = TkWinGetDrawableDC(tree->display, td.drawable, &dcState);

    status = DllExports._GdipCreateFromHDC(hDC, &graphics);
    if (status != Ok)
	goto error1;

    status = DllExports._GdipCreatePath(FillModeAlternate, &path);
    if (status != Ok)
	goto error2;

    status = MakeLinearGradientBrush(gradient, trBrush, &lineGradient);
    if (status != Ok)
	goto error3;

    GetRoundRectPath_Fill(path, tr, rx, ry, open
#ifdef ROUND_RECT_SYMMETRY_HACK
	, 0
#endif
    );
    DllExports._GdipFillPath(graphics, lineGradient, path);

    DllExports._GdipDeleteBrush(lineGradient);

error3:
    DllExports._GdipDeletePath(path);

error2:
    DllExports._GdipDeleteGraphics(graphics);

error1:
    TkWinReleaseDrawableDC(td.drawable, hDC, &dcState);
}
