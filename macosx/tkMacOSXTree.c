/* 
 * tkMacOSXTree.c --
 *
 *	Platform-specific parts of TkTreeCtrl for Mac OSX (Cocoa API).
 *
 * Copyright (c) 2010 Tim Baker
 *
 * RCS: @(#) $Id$
 */

#include "tkTreeCtrl.h"
#include "tkMacOSXInt.h"
#import <Carbon/Carbon.h>
#ifdef MAC_TK_COCOA
#import <Cocoa/Cocoa.h>
#endif

#if MAC_TK_COCOA /*(TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 6)*/
typedef struct {
    CGContextRef context;
} MacContextSetup;
#endif /* Tk 8.6 */

#if MAC_TK_CARBON /*(TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION < 6)*/
typedef struct {
    CGrafPtr port, savePort;
    Boolean portChanged;
    CGContextRef context;
} MacContextSetup;
#endif /* Tk 8.4 + 8.5 */

MODULE_SCOPE CGContextRef TreeMacOSX_GetContext(TreeCtrl *tree,
    Drawable pixmap, TreeRectangle tr, MacContextSetup *dc);
MODULE_SCOPE void TreeMacOSX_ReleaseContext(TreeCtrl *tree,
    MacContextSetup *dc);

#define BlueFloatFromXPixel(pixel)   (float) (((pixel >> 0)  & 0xFF)) / 255.0
#define GreenFloatFromXPixel(pixel)  (float) (((pixel >> 8)  & 0xFF)) / 255.0
#define RedFloatFromXPixel(pixel)    (float) (((pixel >> 16) & 0xFF)) / 255.0

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
    int nw;
    int wx = x1 + tree->drawableXOrigin;
    int wy = y1 + tree->drawableYOrigin;
#if 1
    MacDrawable *macDraw = (MacDrawable *) drawable;
    TreeRectangle tr;
    MacContextSetup dc;
    CGContextRef context;
    CGFloat lengths[1] = {1.0f};

    if (!(macDraw->flags & TK_IS_PIXMAP)) {
	return;
    }

    tr.x = x1, tr.y = y1, tr.width = x2-x1, tr.height = 1;
    context = TreeMacOSX_GetContext(tree, drawable, tr, &dc);
    if (context == NULL) {
	return;
    }

    CGContextBeginPath(context);
    CGContextMoveToPoint(context, x1 /*+ 0.5*/, y1 + 0.5);
    CGContextAddLineToPoint(context, x2 /*- 0.5*/, y1 + 0.5);

    CGContextSetRGBStrokeColor(context,
	RedFloatFromXPixel(gc->foreground),
	GreenFloatFromXPixel(gc->foreground),
	BlueFloatFromXPixel(gc->foreground), 1.0);
    CGContextSetLineWidth(context, 0.01);
    nw = !(wx & 1) == !(wy & 1);
    CGContextSetLineDash(context, nw ? 0 : 1, lengths, 1);
    CGContextSetShouldAntialias(context, 0);
    CGContextStrokePath(context);

    TreeMacOSX_ReleaseContext(tree, &dc);
#else
    nw = !(wx & 1) == !(wy & 1);
    for (x1 += !nw; x1 < x2; x1 += 2) {
	XDrawPoint(tree->display, drawable, gc, x1, y1);
    }
#endif
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
    int nw;
    int wx = x1 + tree->drawableXOrigin;
    int wy = y1 + tree->drawableYOrigin;
#if 1
    MacDrawable *macDraw = (MacDrawable *) drawable;
    TreeRectangle tr;
    MacContextSetup dc;
    CGContextRef context;
    CGFloat lengths[1] = {1.0f};

    if (!(macDraw->flags & TK_IS_PIXMAP)) {
	return;
    }

    tr.x = x1, tr.y = y1, tr.width = x1, tr.height = y2-y1;
    context = TreeMacOSX_GetContext(tree, drawable, tr, &dc);
    if (context == NULL) {
	return;
    }

    CGContextBeginPath(context);
    CGContextMoveToPoint(context, x1 + 0.5, y1 /*+ 0.5*/);
    CGContextAddLineToPoint(context, x1 + 0.5, y2 - 0.5);

    CGContextSetRGBStrokeColor(context,
	RedFloatFromXPixel(gc->foreground),
	GreenFloatFromXPixel(gc->foreground),
	BlueFloatFromXPixel(gc->foreground), 1.0);
    CGContextSetLineWidth(context, 0.01);
    nw = !(wx & 1) == !(wy & 1);
    CGContextSetLineDash(context, nw ? 0 : 1, lengths, 1);
    CGContextSetShouldAntialias(context, 0);
    CGContextStrokePath(context);

    TreeMacOSX_ReleaseContext(tree, &dc);
#else
    nw = !(wx & 1) == !(wy & 1);
    for (y1 += !nw; y1 < y2; y1 += 2) {
	XDrawPoint(tree->display, drawable, gc, x1, y1);
    }
#endif
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
#if 1
    int w = !(open & RECT_OPEN_W);
    int n = !(open & RECT_OPEN_N);
    int e = !(open & RECT_OPEN_E);
    int s = !(open & RECT_OPEN_S);
    XGCValues gcValues;
    unsigned long gcMask;
    GC gc;

    gcValues.function = GXcopy;
    gcMask = GCFunction;
    gc = Tree_GetGC(tree, gcMask, &gcValues);

    if (w) /* left */
    {
	Tree_VDotLine(tree, drawable, gc, x, y, y + height);
    }
    if (n) /* top */
    {
	Tree_HDotLine(tree, drawable, gc, x, y, x + width);
    }
    if (e) /* right */
    {
	Tree_VDotLine(tree, drawable, gc, x + width - 1, y, y + height);
    }
    if (s) /* bottom */
    {
	Tree_HDotLine(tree, drawable, gc, x, y + height - 1, x + width);
    }
#else
    int wx = x + tree->drawableXOrigin;
    int wy = y + tree->drawableYOrigin;
    int w = !(open & RECT_OPEN_W);
    int n = !(open & RECT_OPEN_N);
    int e = !(open & RECT_OPEN_E);
    int s = !(open & RECT_OPEN_S);
    int nw, ne, sw, se;
    int i;
    XGCValues gcValues;
    unsigned long gcMask;
    GC gc;

    /* Dots on even pixels only */
    nw = !(wx & 1) == !(wy & 1);
    ne = !((wx + width - 1) & 1) == !(wy & 1);
    sw = !(wx & 1) == !((wy + height - 1) & 1);
    se = !((wx + width - 1) & 1) == !((wy + height - 1) & 1);

    gcValues.function = GXcopy;
    gcMask = GCFunction;
    gc = Tree_GetGC(tree, gcMask, &gcValues);

    if (w) /* left */
    {
	for (i = !nw; i < height; i += 2) {
	    XDrawPoint(tree->display, drawable, gc, x, y + i);
	}
    }
    if (n) /* top */
    {
	for (i = nw ? w * 2 : 1; i < width; i += 2) {
	    XDrawPoint(tree->display, drawable, gc, x + i, y);
	}
    }
    if (e) /* right */
    {
	for (i = ne ? n * 2 : 1; i < height; i += 2) {
	    XDrawPoint(tree->display, drawable, gc, x + width - 1, y + i);
	}
    }
    if (s) /* bottom */
    {
	for (i = sw ? w * 2 : 1; i < width - (se && e); i += 2) {
	    XDrawPoint(tree->display, drawable, gc, x + i, y + height - 1);
	}
    }
#endif
}

/*
 * The following structure is used when drawing a number of dotted XOR
 * rectangles.
 */
struct DotStatePriv
{
    TreeCtrl *tree;
#if 1
    MacContextSetup dc;
    CGContextRef context;
#else
    Drawable drawable;
    GC gc;
    TkRegion rgn;
#endif
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
#if 1
    struct DotStatePriv *dotState = (struct DotStatePriv *) p;
    MacDrawable *macDraw = (MacDrawable *) drawable;
    TreeRectangle tr;
    CGContextRef context;
    CGRect r;

    if (sizeof(*dotState) > sizeof(*p))
	panic("TreeDotRect_Setup: DotState hack is too small");

    dotState->context = NULL;
    if (!(macDraw->flags & TK_IS_PIXMAP)) {
	return;
    }

    tr.x = 0, tr.y = 0, tr.width = 1, tr.height = 1;
    context = dotState->context = TreeMacOSX_GetContext(tree, drawable, tr,
	&dotState->dc);
    if (context == NULL) {
	return;
    }

    /* Keep drawing inside the contentbox. */
    CGContextBeginPath(context);
    r = CGRectMake(Tree_ContentLeft(tree),
	Tree_ContentTop(tree),
	Tree_ContentWidth(tree),
	Tree_ContentHeight(tree));
    CGContextAddRect(context, r);
    CGContextClip(context);
#else
    struct DotStatePriv *dotState = (struct DotStatePriv *) p;
    XGCValues gcValues;
    unsigned long mask;
    XRectangle xrect;

    if (sizeof(*dotState) > sizeof(*p))
	panic("TreeDotRect_Setup: DotState hack is too small");

    dotState->tree = tree;
    dotState->drawable = drawable;

    gcValues.line_style = LineOnOffDash;
    gcValues.line_width = 1;
    gcValues.dash_offset = 0;
    gcValues.dashes = 1;

    /* Can't get a 1-pixel dash pattern when antialiasing is used. */
    /* See ::tk::mac::CGAntialiasLimit".*/
    gcValues.dashes = 2;
    gcValues.function = GXcopy;
    gcValues.cap_style = CapButt; /* doesn't affect display */
    mask = GCLineWidth | GCLineStyle | GCDashList | GCDashOffset | GCFunction | GCCapStyle;

    dotState->gc = Tk_GetGC(tree->tkwin, mask, &gcValues);

    /* Keep drawing inside the contentbox. */
    dotState->rgn = Tree_GetRegion(tree);
    xrect.x = Tree_ContentLeft(tree);
    xrect.y = Tree_ContentTop(tree);
    xrect.width = Tree_ContentRight(tree) - xrect.x;
    xrect.height = Tree_ContentBottom(tree) - xrect.y;
    TkUnionRectWithRegion(&xrect, dotState->rgn, dotState->rgn);
    TkSetRegion(tree->display, dotState->gc, dotState->rgn);
#endif
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
    CGContextRef context = dotState->context;
    CGRect r = CGRectMake(x, y, width, height);

    if (dotState->context == NULL)
	return;
    CGContextAddRect(context, r);
#else
    XDrawRectangle(dotState->tree->display, dotState->drawable, dotState->gc,
	x, y, width - 1, height - 1);
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
#if 1
    CGContextRef context = dotState->context;
    CGFloat lengths[1] = {1.0f};

    if (dotState->context == NULL)
	return;

    CGContextSetRGBStrokeColor(context, 0.0, 0.0, 0.0, 1.0);
    CGContextSetLineWidth(context, 0.01);
    CGContextSetLineDash(context, 0, lengths, 1);
    CGContextSetShouldAntialias(context, 0);
    CGContextStrokePath(context);

    TreeMacOSX_ReleaseContext(dotState->tree, &dotState->dc);
#else
    XSetClipMask(dotState->tree->display, dotState->gc, None);
    Tree_FreeRegion(dotState->tree, dotState->rgn);
    Tk_FreeGC(dotState->tree->display, dotState->gc);
#endif
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
    XRectangle box;

    TkClipBox(rgn, &box);
    TkSetRegion(display, gc, rgn);
    XFillRectangle(display, drawable, gc, box.x, box.y, box.width, box.height);
    XSetClipMask(display, gc, None);
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
    HIShapeOffset((HIMutableShapeRef) region, xOffset, yOffset);
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
    HIShapeUnion((HIShapeRef) rgnA, (HIShapeRef) rgnB,
	(HIMutableShapeRef) rgnOut);
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
    int result = TkScrollWindow(tree->tkwin, gc, x, y, width, height, dx, dy,
	damageRgn);
#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION < 6)
    {
	MacDrawable *macWin = (MacDrawable *) Tk_WindowId(tree->tkwin);
	/* BUG IN TK? */
	Tree_OffsetRegion(damageRgn, -macWin->xOff, -macWin->yOff);
    }
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
    XSetClipOrigin(tree->display, gc, dest_x, dest_y);
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
    unsigned long red_shift, green_shift, blue_shift;

    Tk_PhotoBlank(photoH);

    /* See TkPoscriptImage */

    red_shift = green_shift = blue_shift = 0;
    while ((0x0001 & (ximage->red_mask >> red_shift)) == 0)
	red_shift++;
    while ((0x0001 & (ximage->green_mask >> green_shift)) == 0)
	green_shift++;
    while ((0x0001 & (ximage->blue_mask >> blue_shift)) == 0)
	blue_shift++;

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

	    r = (pixel & ximage->red_mask) >> red_shift;
	    g = (pixel & ximage->green_mask) >> green_shift;
	    b = (pixel & ximage->blue_mask) >> blue_shift;

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
    TkRegion rgn = NULL;

    if (clip && clip->type == TREE_CLIP_RECT) {
	rgn = Tree_GetRegion(tree);
	Tree_SetRectRegion(rgn, &clip->tr);
	TkSetRegion(tree->display, gc, rgn);
    }
    if (clip && clip->type == TREE_CLIP_AREA) {
	int x1, y1, x2, y2;
	XRectangle xr;
	if (Tree_AreaBbox(tree, clip->area, &x1, &y1, &x2, &y2) == 0)
	    return;
	xr.x = x1, xr.y = y1, xr.width = x2 - x1, xr.height = y2 - y1;
	rgn = Tree_GetRegion(tree);
	TkUnionRectWithRegion(&xr, rgn, rgn);
	TkSetRegion(tree->display, gc, rgn);
    }
    if (clip && clip->type == TREE_CLIP_REGION) {
	TkSetRegion(tree->display, gc, clip->region);
    }

    XFillRectangle(tree->display, td.drawable, gc, tr.x, tr.y, tr.width, tr.height);

    XSetClipMask(tree->display, gc, None);

    if (rgn != NULL)
	Tree_FreeRegion(tree, rgn);
}

/*** Themes ***/

static HIThemeButtonDrawInfo
GetThemeButtonDrawInfo(
    TreeCtrl *tree,
    int state,
    int arrow)
{
    HIThemeButtonDrawInfo info;

    info.version = 0;
    switch (state) {
	case COLUMN_STATE_ACTIVE:  info.state = kThemeStateActive /* kThemeStateRollover */; break;
	case COLUMN_STATE_PRESSED: info.state = kThemeStatePressed; break;
	default:		   info.state = kThemeStateActive; break;
    }
    /* Background window */
    if (!tree->isActive)
	info.state = kThemeStateInactive;
    info.kind = kThemeListHeaderButton;
    info.value = (arrow != COLUMN_ARROW_NONE) ? kThemeButtonOn : kThemeButtonOff;
    switch (arrow) {
	case COLUMN_ARROW_NONE: info.adornment = kThemeAdornmentHeaderButtonNoSortArrow; break;
	case COLUMN_ARROW_UP: info.adornment = kThemeAdornmentHeaderButtonSortUp; break;
	case COLUMN_ARROW_DOWN:
	default:
	    info.adornment = kThemeAdornmentDefault; break;
    }

    return info;
}

int TreeTheme_DrawHeaderItem(TreeCtrl *tree, Drawable drawable, int state,
    int arrow, int visIndex, int x, int y, int width, int height)
{
    MacDrawable *macDraw = (MacDrawable *) drawable;
    CGRect bounds;
    HIThemeButtonDrawInfo info;
    MacContextSetup dc;
    CGContextRef context;
    HIShapeRef boundsRgn;
    int leftEdgeOffset;
    TreeRectangle tr;

    if (!(macDraw->flags & TK_IS_PIXMAP))
	return TCL_ERROR;

    info = GetThemeButtonDrawInfo(tree, state, arrow);

    tr.x = x, tr.y = y, tr.width = width, tr.height = height;
    context = TreeMacOSX_GetContext(tree, drawable, tr, &dc);
    if (context == NULL)
	return TCL_ERROR;

    /* See SF patch 'aqua header drawing - ID: 1356447' */
    /* The left edge overlaps the right edge of the previous column. */
    /* Only show the left edge if this is the first column or the
     * "blue" column (with a sort arrow). */
    if (visIndex == 0 || arrow == COLUMN_ARROW_NONE)
	leftEdgeOffset = 0;
    else
	leftEdgeOffset = -1;

    /* Create a clipping region as big as the header. */
    bounds.origin.x = macDraw->xOff + x + leftEdgeOffset;
    bounds.origin.y = macDraw->yOff + y;
    bounds.size.width = width - leftEdgeOffset;
    bounds.size.height = height;
    boundsRgn = HIShapeCreateWithRect(&bounds);

    /* Set the clipping region */
    HIShapeReplacePathInCGContext(boundsRgn, context);
    CGContextEOClip(context);

    /* See SF patch 'aqua header drawing - ID: 1356447' */
    if (visIndex == 0)
	leftEdgeOffset = 0;
    else
	leftEdgeOffset = -1;
    bounds.origin.x = macDraw->xOff + x + leftEdgeOffset;
    bounds.size.width = width - leftEdgeOffset;

    (void) HIThemeDrawButton(&bounds, &info, context,
	kHIThemeOrientationNormal, NULL);

    TreeMacOSX_ReleaseContext(tree, &dc);
    CFRelease(boundsRgn);

    return TCL_OK;
}

/* List headers are a fixed height on Aqua */
int TreeTheme_GetHeaderFixedHeight(TreeCtrl *tree, int *heightPtr)
{
    SInt32 metric;

    GetThemeMetric(kThemeMetricListHeaderHeight, &metric);
    *heightPtr = metric;
    return TCL_OK;
}

int TreeTheme_GetHeaderContentMargins(TreeCtrl *tree, int state, int arrow,
    int bounds[4])
{
    CGRect inBounds, outBounds;
    HIThemeButtonDrawInfo info;
    SInt32 metric;

    inBounds.origin.x = 0;
    inBounds.origin.y = 0;
    inBounds.size.width = 100;
    GetThemeMetric(kThemeMetricListHeaderHeight, &metric);
    inBounds.size.height = metric;

    info = GetThemeButtonDrawInfo(tree, state, arrow);

    (void) HIThemeGetButtonContentBounds(
	&inBounds,
	&info,
	&outBounds);

    bounds[0] = CGRectGetMinX(outBounds) - CGRectGetMinX(inBounds);
    bounds[1] = CGRectGetMinY(outBounds) - CGRectGetMinY(inBounds);
    bounds[2] = CGRectGetMaxX(inBounds) - CGRectGetMaxX(outBounds);
    bounds[3] = CGRectGetMaxY(inBounds) - CGRectGetMaxY(outBounds);

    return TCL_OK;
}

int TreeTheme_DrawHeaderArrow(TreeCtrl *tree, Drawable drawable, int up, int x, int y, int width, int height)
{
    return TCL_ERROR;
}

int TreeTheme_DrawButton(TreeCtrl *tree, Drawable drawable, int open,
    int x, int y, int width, int height)
{
    MacDrawable *macDraw = (MacDrawable *) drawable;
    CGRect bounds;
    HIThemeButtonDrawInfo info;
    MacContextSetup dc;
    CGContextRef context;
    HIShapeRef clipRgn;
    TreeRectangle tr;

    if (!(macDraw->flags & TK_IS_PIXMAP))
	return TCL_ERROR;

    bounds.origin.x = macDraw->xOff + x;
    bounds.origin.y = macDraw->yOff + y;
    bounds.size.width = width;
    bounds.size.height = height;

    info.version = 0;
    info.state = kThemeStateActive;
    info.kind = kThemeDisclosureButton;
    info.value = open ? kThemeDisclosureDown : kThemeDisclosureRight;
    info.adornment = kThemeAdornmentDrawIndicatorOnly;

    tr.x = x, tr.y = y, tr.width = width, tr.height = height;
    context = TreeMacOSX_GetContext(tree, drawable, tr, &dc);
    if (context == NULL)
	return TCL_ERROR;

    /* Set the clipping region */
    clipRgn = HIShapeCreateWithRect(&bounds);
    HIShapeReplacePathInCGContext(clipRgn, context);
    CGContextEOClip(context);

    (void) HIThemeDrawButton(&bounds, &info, context,
	kHIThemeOrientationNormal, NULL);

    TreeMacOSX_ReleaseContext(tree, &dc);
    CFRelease(clipRgn);

    return TCL_OK;
}

int TreeTheme_GetButtonSize(TreeCtrl *tree, Drawable drawable, int open, int *widthPtr, int *heightPtr)
{
    SInt32 metric;

    (void) GetThemeMetric(
	kThemeMetricDisclosureTriangleWidth,
	&metric);
    *widthPtr = metric;

    (void) GetThemeMetric(
	kThemeMetricDisclosureTriangleHeight,
	&metric);
    *heightPtr = metric;

    return TCL_OK;
}

int TreeTheme_GetArrowSize(TreeCtrl *tree, Drawable drawable, int up, int *widthPtr, int *heightPtr)
{
    return TCL_ERROR;
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
#if 1 /* with gradient marquee, can't draw into window, need a pixmap */
    return FALSE;
#else
    return TRUE;
#endif
}

void TreeTheme_ThemeChanged(TreeCtrl *tree)
{
}

int TreeTheme_Init(TreeCtrl *tree)
{
    return TCL_OK;
}

int TreeTheme_Free(TreeCtrl *tree)
{
    return TCL_OK;
}

int TreeTheme_InitInterp(Tcl_Interp *interp)
{
    return TCL_OK;
}

/*** Gradients ***/

#if MAC_TK_COCOA /*(TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION >= 6)*/

/*
 * THIS WON'T WORK FOR DRAWING IN A WINDOW.
 */
CGContextRef
TreeMacOSX_GetContext(
    TreeCtrl *tree,
    Drawable d,
    TreeRectangle tr,
    MacContextSetup *dc
    )
{
    MacDrawable *macDraw = (MacDrawable *) d;
    CGAffineTransform t = { .a = 1, .b = 0, .c = 0, .d = -1, .tx = 0,
	.ty = macDraw->size.height};

    if ((macDraw->flags & TK_IS_PIXMAP) && !macDraw->context) {
	GC gc = Tk_3DBorderGC(tree->tkwin, tree->border, TK_3D_FLAT_GC);
	XFillRectangle(tree->display, d, gc, tr.x, tr.y, tr.width, tr.height);
    }

    dc->context = macDraw->context;
    if (dc->context) {
	CGContextSaveGState(dc->context);
	CGContextConcatCTM(dc->context, t);
    }
    
    return dc->context;
}

void
TreeMacOSX_ReleaseContext(
    TreeCtrl *tree,
    MacContextSetup *dc
    )
{
    if (dc->context != NULL) {
	CGContextSynchronize(dc->context);
	CGContextRestoreGState(dc->context);
    }
}

#endif /* Tk 8.6 */

#if MAC_TK_CARBON /*(TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION < 6)*/

/*
 * THIS WON'T WORK FOR DRAWING IN A WINDOW.
 */
CGContextRef
TreeMacOSX_GetContext(
    TreeCtrl *tree,
    Drawable d,
    TreeRectangle tr,
    MacContextSetup *dc
    )
{
    MacDrawable *macDraw = (MacDrawable *) d;
    CGAffineTransform t = { .a = 1, .b = 0, .c = 0, .d = -1, .tx = 0,
	.ty = macDraw->size.height};

    dc->port = TkMacOSXGetDrawablePort(d);
    dc->context = NULL;
    dc->portChanged = False;
    if (dc->port) {
	dc->portChanged = QDSwapPort(dc->port, &dc->savePort);
	if (QDBeginCGContext(dc->port, &dc->context) == noErr) {
	    SyncCGContextOriginWithPort(dc->context, dc->port);
	    CGContextSaveGState(dc->context);
	    CGContextConcatCTM(dc->context, t);
	}
    }
    
    return dc->context;
}

void
TreeMacOSX_ReleaseContext(
    TreeCtrl *tree,
    MacContextSetup *dc
    )
{
    if (dc->context != NULL) {
	CGContextSynchronize(dc->context);
	CGContextRestoreGState(dc->context);
	if (dc->port) {
	    QDEndCGContext(dc->port, &dc->context);
	}
    }
    if (dc->portChanged) {
	QDSwapPort(dc->savePort, NULL);
    }
}

#endif /* Tk 8.4 and 8.5 */

/* Copy-and-paste from TkPath */

const float kValidDomain[2] = {0, 1};
const float kValidRange[8] = {0, 1, 0, 1, 0, 1, 0, 1};

/* Seems to work for both Endians. */
#define BlueFloatFromXColorPtr(xc)   (float) ((((xc)->pixel >> 0)  & 0xFF)) / 255.0
#define GreenFloatFromXColorPtr(xc)  (float) ((((xc)->pixel >> 8)  & 0xFF)) / 255.0
#define RedFloatFromXColorPtr(xc)    (float) ((((xc)->pixel >> 16) & 0xFF)) / 255.0

#if 0
typedef struct ShadeData {
    int nstops;
    unsigned short red[50], green[50], blue[50];
    float offset[50];
    float opacity[50];
} ShadeData;

static void
ShadeEvaluate(
    void *info,
    const float *in,
    float *out)
{
    ShadeData *data = info;
    int nstops = data->nstops;
    int iStop1, iStop2;
    float offset1, offset2;
    float opacity1, opacity2;
    int i = 0;
    float par = *in;
    float f1, f2;

    /* Find the two stops for this point. Tricky! */
    while ((i < nstops) && (data->offset[i] < par)) {
        i++;
    }
    if (i == 0) {
        /* First stop > 0. */
        iStop1 = iStop2 = 0;
    } else if (i == nstops) {
        /* We have stepped beyond the last stop; step back! */
        iStop1 = iStop2 = nstops - 1;
    } else {
        iStop1 = i-1, iStop2 = i;
    }
    opacity1 = data->opacity[iStop1];
    opacity2 = data->opacity[iStop2];
    offset1 = data->offset[iStop1];
    offset2 = data->offset[iStop2];
    /* Interpolate between the two stops. 
     * "If two gradient stops have the same offset value, 
     * then the latter gradient stop controls the color value at the 
     * overlap point."
     */
    if (fabs(offset2 - offset1) < 1e-6) {
        *out++ = data->red[iStop2];
        *out++ = data->green[iStop2];
        *out++ = data->blue[iStop2]; 
        *out++ = opacity2;
    } else {
    	int range, increment;
    	float factor = (par - offset1)/(offset2 - offset1);
   
	range = (data->red[iStop2] - data->red[iStop1]);
	increment = (int)(range * factor);
	*out++ = CLAMP(data->red[iStop1] + increment,0,USHRT_MAX)/((float)USHRT_MAX);

	range = (data->green[iStop2] - data->green[iStop1]);
	increment = (int)(range * factor);
	*out++ = CLAMP(data->green[iStop1] + increment,0,USHRT_MAX)/((float)USHRT_MAX);

	range = (data->blue[iStop2] - data->blue[iStop1]);
	increment = (int)(range * factor);
	*out++ = CLAMP(data->blue[iStop1] + increment,0,USHRT_MAX)/((float)USHRT_MAX);

        *out++ = 1.0f /*f1 * opacity1 + f2 * opacity2*/;
    }
}
#elif 1
typedef struct ShadeData {
    int nstops;
    float red[50], green[50], blue[50];
    float offset[50];
    float opacity[50];
} ShadeData;

static void
ShadeEvaluate(
    void *info,
    const float *in,
    float *out)
{
    ShadeData *data = info;
    int nstops = data->nstops;
    int iStop1, iStop2;
    float offset1, offset2;
    float opacity1, opacity2;
    int i = 0;
    float par = *in;
    float f1, f2;

    /* Find the two stops for this point. Tricky! */
    while ((i < nstops) && (data->offset[i] < par)) {
        i++;
    }
    if (i == 0) {
        /* First stop > 0. */
        iStop1 = iStop2 = 0;
    } else if (i == nstops) {
        /* We have stepped beyond the last stop; step back! */
        iStop1 = iStop2 = nstops - 1;
    } else {
        iStop1 = i-1, iStop2 = i;
    }
    opacity1 = data->opacity[iStop1];
    opacity2 = data->opacity[iStop2];
    offset1 = data->offset[iStop1];
    offset2 = data->offset[iStop2];
    /* Interpolate between the two stops. 
     * "If two gradient stops have the same offset value, 
     * then the latter gradient stop controls the color value at the 
     * overlap point."
     */
    if (fabs(offset2 - offset1) < 1e-6) {
        *out++ = data->red[iStop2];
        *out++ = data->green[iStop2];
        *out++ = data->blue[iStop2]; 
        *out++ = opacity2;
    } else {
        f1 = (offset2 - par)/(offset2 - offset1);
        f2 = (par - offset1)/(offset2 - offset1);
        *out++ = f1 * data->red[iStop1] + f2 * data->red[iStop2];
        *out++ = f1 * data->green[iStop1] + f2 * data->green[iStop2];
        *out++ = f1 * data->blue[iStop1] + f2 * data->blue[iStop2];
        *out++ = f1 * opacity1 + f2 * opacity2;
    }
}
#else
static void
ShadeEvaluate(
    void *info,
    const float *in,
    float *out)
{
    TreeGradient gradient = info;
    GradientStopArray *stopArrPtr = gradient->stopArrPtr;
    GradientStop **stopPtrPtr = stopArrPtr->stops;
    GradientStop *stop1 = NULL, *stop2 = NULL;
    int nstops = stopArrPtr->nstops;
    int i = 0;
    float par = *in;
    float f1, f2;

    /* Find the two stops for this point. Tricky! */
    while ((i < nstops) && ((*stopPtrPtr)->offset < par)) {
        stopPtrPtr++, i++;
    }
    if (i == 0) {
        /* First stop > 0. */
        stop1 = *stopPtrPtr;
        stop2 = stop1;
    } else if (i == nstops) {
        /* We have stepped beyond the last stop; step back! */
        stop1 = *(stopPtrPtr - 1);
        stop2 = stop1;
    } else {
        stop1 = *(stopPtrPtr - 1);
        stop2 = *stopPtrPtr;
    }
    /* Interpolate between the two stops. 
     * "If two gradient stops have the same offset value, 
     * then the latter gradient stop controls the color value at the 
     * overlap point."
     */
    if (fabs(stop2->offset - stop1->offset) < 1e-6) {
        *out++ = RedFloatFromXColorPtr(stop2->color);
        *out++ = GreenFloatFromXColorPtr(stop2->color);
        *out++ = BlueFloatFromXColorPtr(stop2->color); 
        *out++ = stop2->opacity;
    } else {
        f1 = (stop2->offset - par)/(stop2->offset - stop1->offset);
        f2 = (par - stop1->offset)/(stop2->offset - stop1->offset);
        *out++ = f1 * RedFloatFromXColorPtr(stop1->color) + 
                f2 * RedFloatFromXColorPtr(stop2->color);
        *out++ = f1 * GreenFloatFromXColorPtr(stop1->color) + 
                f2 * GreenFloatFromXColorPtr(stop2->color);
        *out++ = f1 * BlueFloatFromXColorPtr(stop1->color) + 
                f2 * BlueFloatFromXColorPtr(stop2->color);
        *out++ = f1 * stop1->opacity + f2 * stop2->opacity;
    }
}
#endif
static void
ShadeRelease(void *info)
{
    /* Not sure if anything to do here. */
}

#if 0
/* Get the colorspace used by the monitor. */
/* FIXME: 8.4+ only */
CGColorSpaceRef CreateSystemColorSpace() {
    CMProfileRef sysprof = NULL;
    CGColorSpaceRef dispColorSpace = NULL;
    if (CMGetSystemProfile(&sysprof) == noErr) {
	dispColorSpace = CGColorSpaceCreateWithPlatformColorSpace(sysprof);
	CMCloseProfile(sysprof);
    }
    return dispColorSpace;
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * Tree_HasNativeGradients --
 *
 *	Determine if this platform supports gradients natively.
 *
 * Results:
 *	1.
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
    return 1;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeGradient_FillRect --
 *
 *	Paint a rectangle with a gradient.
 *
 * Results:
 *	If the gradient has <2 stops then nothing is drawn.
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
    MacDrawable *macDraw = (MacDrawable *) td.drawable;
    MacContextSetup dc;
    CGContextRef context;
    CGFunctionCallbacks callbacks;
    CGFunctionRef function;
    CGColorSpaceRef colorSpaceRef;
    CGPoint start, end;
    CGShadingRef shading;
    CGRect r;
    ShadeData data;
    int i;

    if (!(macDraw->flags & TK_IS_PIXMAP) || !tree->nativeGradients) {
	TreeGradient_FillRectX11(tree, td, clip, gradient, trBrush, tr);
	return;
    }

    context = TreeMacOSX_GetContext(tree, td.drawable, tr, &dc);
    if (context == NULL) {
	TreeGradient_FillRectX11(tree, td, clip, gradient, trBrush, tr);
	return;
    }

    data.nstops = gradient->stopArrPtr->nstops;
    for (i = 0; i < gradient->stopArrPtr->nstops; i++) {
	GradientStop *stop = gradient->stopArrPtr->stops[i];
#if 0
        data.red[i] = stop->color->red;
        data.green[i] = stop->color->green;
        data.blue[i] = stop->color->blue;
#else
        data.red[i] = RedFloatFromXColorPtr(stop->color);
        data.green[i] = GreenFloatFromXColorPtr(stop->color);
        data.blue[i] = BlueFloatFromXColorPtr(stop->color);
#endif
        data.offset[i] = stop->offset;
        data.opacity[i] = stop->opacity;
    }

/*    colorSpaceRef = CGColorSpaceCreateDeviceRGB();*/
/*    colorSpaceRef = CreateSystemColorSpace();*/
/*    colorSpaceRef = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);*/
    colorSpaceRef = CGBitmapContextGetColorSpace(context);
    CGColorSpaceRetain(colorSpaceRef);

    callbacks.version = 0;
    callbacks.evaluate = ShadeEvaluate;
    callbacks.releaseInfo = ShadeRelease;
    function = CGFunctionCreate((void *) &data,
	1, kValidDomain,
	4, kValidRange,
	&callbacks);

    if (gradient->vertical) {
	start = CGPointMake(trBrush.x, trBrush.y);
	end = CGPointMake(trBrush.x, trBrush.y + trBrush.height);
    } else {
	start = CGPointMake(trBrush.x, trBrush.y);
	end = CGPointMake(trBrush.x + trBrush.width, trBrush.y);
    }
    shading = CGShadingCreateAxial(colorSpaceRef, start, end, function, 1, 1);

    /* Must clip to the area to be painted otherwise the entire context
     * is filled with the gradient. */
    CGContextBeginPath(context);
    r = CGRectMake(tr.x, tr.y, tr.width, tr.height);
    CGContextAddRect(context, r);
    CGContextClip(context);

    CGContextDrawShading(context, shading);

    CGShadingRelease(shading);
    CGFunctionRelease(function);
    CGColorSpaceRelease(colorSpaceRef);

    TreeMacOSX_ReleaseContext(tree, &dc);
}

void
Tree_DrawRoundRect(
    TreeCtrl *tree,		/* Widget info. */
    TreeDrawable td,		/* Where to draw. */
    XColor *xcolor,		/* Color. */
    TreeRectangle tr,		/* Where to draw. */
    int outlineWidth,
    int rx, int ry,		/* Corner radius */
    int open			/* RECT_OPEN_x flags */
    )
{
    GC gc = Tk_GCForColor(xcolor, Tk_WindowId(tree->tkwin));
    Tree_DrawRoundRectX11(tree, td, gc, tr, outlineWidth, rx, ry, open);
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
    GC gc = Tk_GCForColor(xcolor, Tk_WindowId(tree->tkwin));
    Tree_FillRoundRectX11(tree, td, gc, tr, rx, ry, open);
}

int
TreeDraw_InitInterp(
    Tcl_Interp *interp
    )
{
    return TCL_OK;
}
