/* 
 * tkUnixTree.c --
 *
 *	Platform-specific parts of TkTreeCtrl for X11.
 *
 * Copyright (c) 2010 Tim Baker
 *
 * RCS: @(#) $Id$
 */

#include "tkTreeCtrl.h"

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

    nw = !(wx & 1) == !(wy & 1);
    for (x1 += !nw; x1 < x2; x1 += 2) {
	XDrawPoint(tree->display, drawable, gc, x1, y1);
    }
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

    nw = !(wx & 1) == !(wy & 1);
    for (y1 += !nw; y1 < y2; y1 += 2) {
	XDrawPoint(tree->display, drawable, gc, x1, y1);
    }
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
    XGCValues gcValues;
    unsigned long gcMask;
    GC gc;

    /* Dots on even pixels only */
    nw = !(wx & 1) == !(wy & 1);
    ne = !((wx + width - 1) & 1) == !(wy & 1);
    sw = !(wx & 1) == !((wy + height - 1) & 1);
    se = !((wx + width - 1) & 1) == !((wy + height - 1) & 1);

    gcValues.function = GXinvert;
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
}

/*
 * The following structure is used when drawing a number of dotted XOR
 * rectangles.
 */
struct DotStatePriv
{
    TreeCtrl *tree;
    Drawable drawable;
    GC gc;
    TkRegion rgn;
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
    XGCValues gcValues;
    unsigned long mask;
    XRectangle xrect;

    dotState->tree = tree;
    dotState->drawable = drawable;

    gcValues.line_style = LineOnOffDash;
    gcValues.line_width = 1;
    gcValues.dash_offset = 0;
    gcValues.dashes = 1;
    gcValues.function = GXinvert;
    mask = GCLineWidth | GCLineStyle | GCDashList | GCDashOffset | GCFunction;
    dotState->gc = Tk_GetGC(tree->tkwin, mask, &gcValues);

    /* Keep drawing inside the contentbox. */
    dotState->rgn = Tree_GetRegion(tree);
    xrect.x = Tree_ContentLeft(tree);
    xrect.y = Tree_ContentTop(tree);
    xrect.width = Tree_ContentRight(tree) - xrect.x;
    xrect.height = Tree_ContentBottom(tree) - xrect.y;
    TkUnionRectWithRegion(&xrect, dotState->rgn, dotState->rgn);
    TkSetRegion(tree->display, dotState->gc, dotState->rgn);
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

    XDrawRectangle(dotState->tree->display, dotState->drawable, dotState->gc,
	x, y, width - 1, height - 1);
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

    XSetClipMask(dotState->tree->display, dotState->gc, None);
    Tree_FreeRegion(dotState->tree, dotState->rgn);
    Tk_FreeGC(dotState->tree->display, dotState->gc);
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
    XOffsetRegion((Region) region, xOffset, yOffset);
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
    XUnionRegion((Region) rgnA, (Region) rgnB, (Region) rgnOut);
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
    Tk_Window tkwin = Tk_MainWindow(interp);
    Display *display = Tk_Display(tkwin);
    Visual *visual = Tk_Visual(tkwin);
    Tk_PhotoImageBlock photoBlock;
    unsigned char *pixelPtr;
    int x, y, w = ximage->width, h = ximage->height;
    int i, ncolors;
    XColor *xcolors;
    unsigned long red_shift, green_shift, blue_shift;
    int separated = 0;

    Tk_PhotoBlank(photoH);

    /* See TkPoscriptImage */

    ncolors = visual->map_entries;
    xcolors = (XColor *) ckalloc(sizeof(XColor) * ncolors);

    if ((visual->class == DirectColor) || (visual->class == TrueColor)) {
	separated = 1;
	red_shift = green_shift = blue_shift = 0;
	/* ximage->red_mask etc are zero */
	while ((0x0001 & (visual->red_mask >> red_shift)) == 0)
	    red_shift++;
	while ((0x0001 & (visual->green_mask >> green_shift)) == 0)
	    green_shift++;
	while ((0x0001 & (visual->blue_mask >> blue_shift)) == 0)
	    blue_shift++;
	for (i = 0; i < ncolors; i++) {
	    xcolors[i].pixel =
		((i << red_shift) & visual->red_mask) |
		((i << green_shift) & visual->green_mask) |
		((i << blue_shift) & visual->blue_mask);
	}
    } else {
	red_shift = green_shift = blue_shift = 0;
	for (i = 0; i < ncolors; i++)
	    xcolors[i].pixel = i;
    }

    XQueryColors(display, Tk_Colormap(tkwin), xcolors, ncolors);

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

	    pixel = XGetPixel(ximage, x, y);

	    /* Set alpha=0 for transparent pixel in the source XImage */
	    if (trans != 0 && pixel == trans) {
		pixelPtr[y * photoBlock.pitch + x * 4 + 3] = 0;
		continue;
	    }

	    if (separated) {
		r = (pixel & visual->red_mask) >> red_shift;
		g = (pixel & visual->green_mask) >> green_shift;
		b = (pixel & visual->blue_mask) >> blue_shift;
		r = ((double) xcolors[r].red / USHRT_MAX) * 255;
		g = ((double) xcolors[g].green / USHRT_MAX) * 255;
		b = ((double) xcolors[b].blue / USHRT_MAX) * 255;
	    } else {
		r = ((double) xcolors[pixel].red / USHRT_MAX) * 255;
		g = ((double) xcolors[pixel].green / USHRT_MAX) * 255;
		b = ((double) xcolors[pixel].blue / USHRT_MAX) * 255;
	    }
	    pixelPtr[y * photoBlock.pitch + x * 4 + 0] = r;
	    pixelPtr[y * photoBlock.pitch + x * 4 + 1] = g;
	    pixelPtr[y * photoBlock.pitch + x * 4 + 2] = b;
	    pixelPtr[y * photoBlock.pitch + x * 4 + 3] = alpha;
	}
    }

    TK_PHOTOPUTBLOCK(interp, photoH, &photoBlock, 0, 0, w, h,
	    TK_PHOTO_COMPOSITE_SET);

    Tcl_Free((char *) pixelPtr);
    ckfree((char *) xcolors);
}

typedef struct {
    TreeCtrl *tree;
    TreeClip *clip;
    GC gc;
    TkRegion region;
} TreeClipStateGC;

void
TreeClip_ToGC(
    TreeCtrl *tree,		/* Widget info. */
    TreeClip *clip,		/* Clipping area or NULL. */
    GC gc,			/* Graphics context. */
    TreeClipStateGC *state
    )
{
    state->tree = tree;
    state->clip = clip;
    state->gc = gc;
    state->region = None;

    if (clip && clip->type == TREE_CLIP_RECT) {
	state->region = Tree_GetRegion(tree);
	Tree_SetRectRegion(state->region, &clip->tr);
	TkSetRegion(tree->display, gc, state->region);
    }
    if (clip && clip->type == TREE_CLIP_AREA) {
	int x1, y1, x2, y2;
	XRectangle xr;
	if (Tree_AreaBbox(tree, clip->area, &x1, &y1, &x2, &y2) == 0)
	    return;
	xr.x = x1, xr.y = y1, xr.width = x2 - x1, xr.height = y2 - y1;
	state->region = Tree_GetRegion(tree);
	TkUnionRectWithRegion(&xr, state->region, state->region);
	TkSetRegion(tree->display, gc, state->region);
    }
    if (clip && clip->type == TREE_CLIP_REGION) {
	TkSetRegion(tree->display, gc, clip->region);
    }
}

void
TreeClip_FinishGC(
    TreeClipStateGC *state
    )
{
    XSetClipMask(state->tree->display, state->gc, None);
    if (state->region != None)
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
    TreeClipStateGC clipState;

    TreeClip_ToGC(tree, clip, gc, &clipState);
    XFillRectangle(tree->display, td.drawable, gc, tr.x, tr.y, tr.width, tr.height);
    TreeClip_FinishGC(&clipState);
}

/*** Themes ***/

#ifdef TREECTRL_GTK

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>
#include <gdk/gdkx.h>

TCL_DECLARE_MUTEX(themeMutex)

/* Per-interp data */
typedef struct {
} TreeThemeData_;

/* Per-application data */
typedef struct {
    int gtk_init;
    GtkWidget *gtkWindow;
    GtkWidget *protoLayout;
    GtkWidget *gtkArrow;
    GtkWidget *gtkTreeView;
    GtkWidget *gtkTreeHeader;
} TreeThemeAppData;

static TreeThemeAppData *appThemeData = NULL;

int TreeTheme_DrawHeaderItem(TreeCtrl *tree, Drawable drawable, int state,
    int arrow, int visIndex, int x, int y, int width, int height)
{
    GtkWidget *widget;
    GtkStyle *style;
    GtkStateType state_type = GTK_STATE_NORMAL;
    GtkShadowType shadow_type = GTK_SHADOW_OUT;
    GdkRectangle area = {0, 0, width, height}; /* clip */
    GdkPixmap *gdkPixmap;
    GdkPixbuf *pixbuf;
    
    if (appThemeData == NULL || appThemeData->gtkTreeHeader == NULL)
	return TCL_ERROR;

    widget = appThemeData->gtkTreeHeader;
    style = gtk_widget_get_style(widget);
    
    switch (state) {
	case COLUMN_STATE_ACTIVE:
	    state_type = GTK_STATE_PRELIGHT;
	    break;
	case COLUMN_STATE_PRESSED:
	    state_type = GTK_STATE_ACTIVE;
	    shadow_type = GTK_SHADOW_IN;
	    break;
	case COLUMN_STATE_NORMAL: 
	    break;
    }

    /* Allocate GdkPixmap to draw background in */
    gdkPixmap = gdk_pixmap_new(appThemeData->gtkWindow->window, width, height, -1);
    if (gdkPixmap == NULL) {
	return TCL_ERROR;
    }
    
    /* Paint the background */
    gtk_paint_box (style, gdkPixmap, state_type, shadow_type, &area, widget,
	"button", 0, 0, width, height);

    /* Copy GdkPixmap to Tk Pixmap */
    pixbuf = gdk_pixbuf_get_from_drawable(NULL, gdkPixmap, NULL, 0, 0, 0, 0,
	width, height);
    if (pixbuf == NULL) {
	g_object_unref(gdkPixmap);
	return TCL_ERROR;
    }
    gdk_pixbuf_xlib_render_to_drawable(pixbuf, drawable, tree->copyGC,
	0, 0, x, y, width, height, XLIB_RGB_DITHER_NONE, 0, 0);

    gdk_pixbuf_unref(pixbuf);
    g_object_unref(gdkPixmap);

    return TCL_OK;
}

int TreeTheme_GetHeaderContentMargins(TreeCtrl *tree, int state, int arrow, int bounds[4])
{
    return TCL_ERROR;
}

int TreeTheme_DrawHeaderArrow(TreeCtrl *tree, Drawable drawable, int up, int x, int y, int width, int height)
{
    GtkWidget *widget;
    GtkStyle *style;
    GtkStateType state = GTK_STATE_NORMAL; /* FIXME: mouseover GTK_STATE_PRELIGHT */
    GdkRectangle area = {0, 0, width, height}; /* clip */
    const gchar *detail = "arrow";
    GtkShadowType shadow_type;
    GtkArrowType effective_arrow_type = up ? GTK_ARROW_DOWN : GTK_ARROW_UP; /* INVERTED!!! */
    GdkPixmap *gdkPixmap;
    GdkRectangle clipped;
    GdkPixbuf *pixbuf;

    if (appThemeData == NULL || appThemeData->gtkArrow == NULL)
	return TCL_ERROR;

    widget = appThemeData->gtkArrow;
    style = gtk_widget_get_style(widget);
    shadow_type = GTK_ARROW(widget)->shadow_type;
    
    if (appThemeData->gtkTreeView != NULL) {
	gboolean alternative = FALSE;
	GtkSettings *settings = gtk_widget_get_settings(appThemeData->gtkTreeView);
	g_object_get(settings, "gtk-alternative-sort-arrows", &alternative, NULL);
	if (alternative)
		effective_arrow_type = up ? GTK_ARROW_UP : GTK_ARROW_DOWN;
    }

    /* This gives warning "widget class `GtkArrow' has no property named `shadow-type'" */
/*
    gtk_widget_style_get(widget, "shadow-type", &shadow_type, NULL);
*/

    clipped.x = x, clipped.y = y, clipped.width = width, clipped.height = height;
    if (clipped.x < 0)
	clipped.width += clipped.x, clipped.x = 0;
    if (clipped.y < 0)
	clipped.height += clipped.y, clipped.y = 0;
    
    if (clipped.width < 1 || clipped.height < 1)
	return TCL_ERROR;

    /* Copy background from Tk Pixmap -> GdkPixbuf */
    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 1, 8, width, height);
    if (pixbuf == NULL)
	return TCL_ERROR;
    pixbuf = gdk_pixbuf_xlib_get_from_drawable(pixbuf, drawable,
	Tk_Colormap(tree->tkwin), Tk_Visual(tree->tkwin), clipped.x, clipped.y,
	x < 0 ? -x : 0, y < 0 ? -y : 0, clipped.width, clipped.height);
    if (pixbuf == NULL)
	return TCL_ERROR;
    
    /* Allocate GdkPixmap to draw button in */
    gdkPixmap = gdk_pixmap_new(appThemeData->gtkWindow->window, width, height, -1);
    if (gdkPixmap == NULL) {
	gdk_pixbuf_unref(pixbuf);
	return TCL_ERROR;
    }
    
    /* Copy GdkPixbuf containing background to GdkPixmap */
    gdk_pixbuf_render_to_drawable(pixbuf, gdkPixmap, NULL, 0, 0, 0, 0,
	width, height, GDK_RGB_DITHER_NONE, 0, 0);

    /* Draw the button */
    gtk_paint_arrow(style, gdkPixmap, state, shadow_type, &area, widget,
	detail, effective_arrow_type, TRUE, 0, 0, width, height);
    
    /* Copy GdkPixmap to Tk Pixmap */
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, gdkPixmap, NULL, 0, 0, 0, 0,
	width, height);
    if (pixbuf == NULL) {
	g_object_unref(gdkPixmap);
	return TCL_ERROR;
    }
    gdk_pixbuf_xlib_render_to_drawable(pixbuf, drawable, tree->copyGC,
	0, 0, x, y, width, height, XLIB_RGB_DITHER_MAX, 0, 0);

    gdk_pixbuf_unref(pixbuf);
    g_object_unref(gdkPixmap);

    return TCL_OK;
}

int TreeTheme_DrawButton(TreeCtrl *tree, Drawable drawable, int open, int x, int y, int width, int height)
{
    GtkWidget *widget;
    GtkStyle *style;
    GtkStateType state = GTK_STATE_NORMAL; /* FIXME: mouseover GTK_STATE_PRELIGHT */
    GdkRectangle area = {0, 0, width, height}; /* clip */
    const gchar *detail = "treeview";
    GtkExpanderStyle expander_style = open ? GTK_EXPANDER_EXPANDED : GTK_EXPANDER_COLLAPSED;
    GdkPixmap *gdkPixmap;
    GdkRectangle clipped;
    GdkPixbuf *pixbuf;

    if (appThemeData == NULL || appThemeData->gtkTreeView == NULL)
	return TCL_ERROR;

    widget = appThemeData->gtkTreeView;
    style = gtk_widget_get_style(widget);

    clipped.x = x, clipped.y = y, clipped.width = width, clipped.height = height;
    if (clipped.x < 0)
	clipped.width += clipped.x, clipped.x = 0;
    if (clipped.y < 0)
	clipped.height += clipped.y, clipped.y = 0;
    
    if (clipped.width < 1 || clipped.height < 1)
	return TCL_ERROR;

    /* Copy background from Tk Pixmap -> GdkPixbuf */
    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 1, 8, width, height);
    if (pixbuf == NULL)
	return TCL_ERROR;
    pixbuf = gdk_pixbuf_xlib_get_from_drawable(pixbuf, drawable,
	Tk_Colormap(tree->tkwin), Tk_Visual(tree->tkwin), clipped.x, clipped.y,
	x < 0 ? -x : 0, y < 0 ? -y : 0, clipped.width, clipped.height);
    if (pixbuf == NULL)
	return TCL_ERROR;
    
    /* Allocate GdkPixmap to draw button in */
    gdkPixmap = gdk_pixmap_new(appThemeData->gtkWindow->window, width, height, -1);
    if (gdkPixmap == NULL) {
	gdk_pixbuf_unref(pixbuf);
	return TCL_ERROR;
    }
    
    /* Copy GdkPixbuf containing background to GdkPixmap */
    gdk_pixbuf_render_to_drawable(pixbuf, gdkPixmap, NULL, 0, 0, 0, 0,
	width, height, GDK_RGB_DITHER_NONE, 0, 0);

    /* Draw the button */
    gtk_paint_expander(style, gdkPixmap, state, &area, widget, detail,
	width / 2, height / 2, expander_style);
    
    /* Copy GdkPixmap to Tk Pixmap */
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, gdkPixmap, NULL, 0, 0, 0, 0,
	width, height);
    if (pixbuf == NULL) {
	g_object_unref(gdkPixmap);
	return TCL_ERROR;
    }
    gdk_pixbuf_xlib_render_to_drawable(pixbuf, drawable, tree->copyGC,
	0, 0, x, y, width, height, XLIB_RGB_DITHER_MAX, 0, 0);

    gdk_pixbuf_unref(pixbuf);
    g_object_unref(gdkPixmap);

    return TCL_OK;
}

int TreeTheme_GetButtonSize(TreeCtrl *tree, Drawable drawable, int open, int *widthPtr, int *heightPtr)
{
    GtkWidget *widget;
    const gchar *property_name = "expander-size";
    gint expander_size;

    if (appThemeData == NULL || appThemeData->gtkTreeView == NULL)
	return TCL_ERROR;
    
    widget = appThemeData->gtkTreeView;

    gtk_widget_style_get(widget, property_name, &expander_size, NULL);
    (*widthPtr) = (*heightPtr) = expander_size;

    return TCL_OK;
}

int TreeTheme_GetArrowSize(TreeCtrl *tree, Drawable drawable, int up, int *widthPtr, int *heightPtr)
{
    GtkWidget *widget;
    GtkRequisition requisition;
    gfloat arrow_scaling = 1.0f;
    GtkMisc *misc;
    gint width, height, extent;
     
    if (appThemeData == NULL || appThemeData->gtkArrow == NULL)
	return TCL_ERROR;

    widget = appThemeData->gtkArrow;
    misc = GTK_MISC(widget);
    
    gtk_widget_size_request(widget, &requisition);
    gtk_widget_style_get(widget, "arrow-scaling", &arrow_scaling, NULL);
    width = requisition.width - misc->xpad * 2;
    height = requisition.height - misc->ypad * 2;
    extent = MIN(width, height) * arrow_scaling;

    (*widthPtr) = extent;
    (*heightPtr) = extent;

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
    return FALSE;
}

void TreeTheme_ThemeChanged(TreeCtrl *tree)
{
}

int TreeTheme_Init(TreeCtrl *tree)
{
    if (appThemeData != NULL && appThemeData->gtk_init) {
	Tk_MakeWindowExist(tree->tkwin);
	
	gdk_pixbuf_xlib_init(Tk_Display(tree->tkwin), 0);
	/* Needed for gdk_pixbuf_xlib_render_to_drawable() */
	xlib_rgb_init(Tk_Display(tree->tkwin), Tk_Screen(tree->tkwin));
    }

    return TCL_OK;
}

int TreeTheme_Free(TreeCtrl *tree)
{
    return TCL_OK;
}

int TreeTheme_InitInterp(Tcl_Interp *interp)
{
    Tcl_MutexLock(&themeMutex);
    
    if (appThemeData == NULL) {
	int argc = 1;
	char **argv = g_new0(char*, 2);
	argv[0] = (char *) Tcl_GetNameOfExecutable();
	GtkTreeViewColumn *column;

	appThemeData = (TreeThemeAppData*) ckalloc(sizeof(TreeThemeAppData));
	appThemeData->gtk_init = gtk_init_check(&argc, &argv);
	if (!appThemeData->gtk_init) {
	    ckfree((char *) appThemeData);
	    appThemeData = NULL;
	    Tcl_MutexUnlock(&themeMutex);
	    return TCL_ERROR;
	}

	appThemeData->gtkWindow = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_widget_realize(appThemeData->gtkWindow);

	appThemeData->protoLayout = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(appThemeData->gtkWindow),
	    appThemeData->protoLayout);

	appThemeData->gtkTreeView = gtk_tree_view_new();
	gtk_container_add(GTK_CONTAINER(appThemeData->protoLayout),
	    appThemeData->gtkTreeView);
	gtk_widget_realize(appThemeData->gtkTreeView);

	/* GTK_SHADOW_IN is default for GtkTreeView */
	appThemeData->gtkArrow = gtk_arrow_new(GTK_ARROW_NONE, GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(appThemeData->protoLayout),
	    appThemeData->gtkArrow);
	gtk_widget_realize(appThemeData->gtkArrow);

	/* Create *three* columns, and use the middle column when drawing
	 * headers. */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(appThemeData->gtkTreeView),
	    column);
	
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(appThemeData->gtkTreeView),
	    column);
	appThemeData->gtkTreeHeader = column->button;
	
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(appThemeData->gtkTreeView),
	    column);
    }
    
    Tcl_MutexUnlock(&themeMutex);

    return TCL_OK;
}

#endif /* TREECTRL_GTK */

#ifndef TREECTRL_GTK
int TreeTheme_DrawHeaderItem(TreeCtrl *tree, Drawable drawable, int state,
    int arrow, int visIndex, int x, int y, int width, int height)
{
    return TCL_ERROR;
}

int TreeTheme_GetHeaderContentMargins(TreeCtrl *tree, int state, int arrow, int bounds[4])
{
    return TCL_ERROR;
}

int TreeTheme_DrawHeaderArrow(TreeCtrl *tree, Drawable drawable, int up, int x, int y, int width, int height)
{
    return TCL_ERROR;
}

int TreeTheme_DrawButton(TreeCtrl *tree, Drawable drawable, int open, int x, int y, int width, int height)
{
    return TCL_ERROR;
}

int TreeTheme_GetButtonSize(TreeCtrl *tree, Drawable drawable, int open, int *widthPtr, int *heightPtr)
{
    return TCL_ERROR;
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
    return FALSE;
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

#endif /* not TREECTRL_GTK */

/*** Gradients ***/

/*
 *----------------------------------------------------------------------
 *
 * Tree_HasNativeGradients --
 *
 *	Determine if this platform supports gradients natively.
 *
 * Results:
 *	0.
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
    return 0; /* TODO: cairo */
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
    TreeGradient_FillRectX11(tree, td, clip, gradient, trBrush, tr);

    /* FIXME: Can use 'cairo' on Unix, but need to add it to configure + Make */
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
    TreeGradient_FillRoundRectX11(tree, td, NULL, gradient, trBrush, tr, rx, ry, open);

    /* FIXME: Can use 'cairo' on Unix, but need to add it to configure + Make */
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
    /* FIXME: MacOSX + Cocoa, Unix + cairo */
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
    /* FIXME: MacOSX + Cocoa, Unix + cairo */
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

