/*
 * tkTreeHeaders.c --
 *
 *	This module implements the treectrl widget's column headers.
 *
 * Copyright (c) 2011 Tim Baker
 */

#include "tkTreeCtrl.h"

typedef struct TreeHeader_ TreeHeader_;
typedef struct TreeHeaderColumn_ HeaderColumn;

/*
 * The following structure holds information about a single column header
 * of a TreeHeader.
 */
struct TreeHeaderColumn_
{
    TreeItemColumn itemColumn;

    Tcl_Obj *textObj;		/* -text */
    char *text;			/* -text */
    Tk_Font tkfont;		/* -font */
    Tk_Justify justify;		/* -justify */
    PerStateInfo border;	/* -background */
    Tcl_Obj *borderWidthObj;	/* -borderwidth */
    int borderWidth;		/* -borderwidth */
    PerStateInfo textColor;	/* -textcolor */
    char *imageString;		/* -image */
    PerStateInfo arrowBitmap;	/* -arrowbitmap */
    PerStateInfo arrowImage;	/* -arrowimage */
    Pixmap bitmap;		/* -bitmap */
    int button;			/* -button */
    Tcl_Obj *textPadXObj;	/* -textpadx */
    int *textPadX;		/* -textpadx */
    Tcl_Obj *textPadYObj;	/* -textpady */
    int *textPadY;		/* -textpady */
    Tcl_Obj *imagePadXObj;	/* -imagepadx */
    int *imagePadX;		/* -imagepadx */
    Tcl_Obj *imagePadYObj;	/* -imagepady */
    int *imagePadY;		/* -imagepady */
    Tcl_Obj *arrowPadXObj;	/* -arrowpadx */
    int *arrowPadX;		/* -arrowpadx */
    Tcl_Obj *arrowPadYObj;	/* -arrowpady */
    int *arrowPadY;		/* -arrowpady */

    int arrow;			/* -arrow */

#define SIDE_LEFT 0
#define SIDE_RIGHT 1
    int arrowSide;		/* -arrowside */
    int arrowGravity;		/* -arrowgravity */

    int state;			/* -state */

    int textLen;
    int textWidth;
    Tk_Image image;
    int neededWidth;		/* calculated from borders + image/bitmap +
				 * text + arrow */
    int neededHeight;		/* calculated from borders + image/bitmap +
				 * text */
    GC bitmapGC;
    TextLayout textLayout;	/* multi-line titles */
    int textLayoutWidth;	/* width passed to TextLayout_Compute */
    int textLayoutInvalid;
#define TEXT_WRAP_NULL -1
#define TEXT_WRAP_CHAR 0
#define TEXT_WRAP_WORD 1
    int textWrap;		/* -textwrap */
    int textLines;		/* -textlines */
};

/*
 * The following structure holds information about a single TreeHeader.
 */
struct TreeHeader_
{
    TreeCtrl *tree;
    TreeItem item;
    int ownerDrawn;
};

static CONST char *arrowST[] = { "none", "up", "down", (char *) NULL };
static CONST char *arrowSideST[] = { "left", "right", (char *) NULL };
static CONST char *stateST[] = { "normal", "active", "pressed", (char *) NULL };

#define COLU_CONF_IMAGE		0x0001
#define COLU_CONF_NWIDTH	0x0002	/* neededWidth */
#define COLU_CONF_NHEIGHT	0x0004	/* neededHeight */
#define COLU_CONF_TWIDTH	0x0008	/* totalWidth */
#define COLU_CONF_DISPLAY	0x0040
#define COLU_CONF_JUSTIFY	0x0080
#define COLU_CONF_TEXT		0x0200
#define COLU_CONF_BITMAP	0x0400

static Tk_OptionSpec columnSpecs[] = {
    {TK_OPTION_STRING_TABLE, "-arrow", (char *) NULL, (char *) NULL,
     "none", -1, Tk_Offset(HeaderColumn, arrow),
     0, (ClientData) arrowST, COLU_CONF_NWIDTH | COLU_CONF_NHEIGHT | COLU_CONF_DISPLAY},
    {TK_OPTION_CUSTOM, "-arrowbitmap", (char *) NULL, (char *) NULL,
     (char *) NULL,
     Tk_Offset(HeaderColumn, arrowBitmap.obj), Tk_Offset(HeaderColumn, arrowBitmap),
     TK_OPTION_NULL_OK, (ClientData) NULL,
     COLU_CONF_NWIDTH | COLU_CONF_NHEIGHT | COLU_CONF_DISPLAY},
    {TK_OPTION_STRING_TABLE, "-arrowgravity", (char *) NULL, (char *) NULL,
     "left", -1, Tk_Offset(HeaderColumn, arrowGravity),
     0, (ClientData) arrowSideST, COLU_CONF_DISPLAY},
    {TK_OPTION_CUSTOM, "-arrowimage", (char *) NULL, (char *) NULL,
     (char *) NULL,
     Tk_Offset(HeaderColumn, arrowImage.obj), Tk_Offset(HeaderColumn, arrowImage),
     TK_OPTION_NULL_OK, (ClientData) NULL,
     COLU_CONF_NWIDTH | COLU_CONF_NHEIGHT | COLU_CONF_DISPLAY},
    {TK_OPTION_CUSTOM, "-arrowpadx", (char *) NULL, (char *) NULL,
     "6", Tk_Offset(HeaderColumn, arrowPadXObj), Tk_Offset(HeaderColumn, arrowPadX),
     0, (ClientData) &TreeCtrlCO_pad, COLU_CONF_NWIDTH | COLU_CONF_DISPLAY},
    {TK_OPTION_CUSTOM, "-arrowpady", (char *) NULL, (char *) NULL,
     "0", Tk_Offset(HeaderColumn, arrowPadYObj), Tk_Offset(HeaderColumn, arrowPadY),
     0, (ClientData) &TreeCtrlCO_pad, COLU_CONF_NWIDTH | COLU_CONF_DISPLAY},
    {TK_OPTION_STRING_TABLE, "-arrowside", (char *) NULL, (char *) NULL,
     "right", -1, Tk_Offset(HeaderColumn, arrowSide),
     0, (ClientData) arrowSideST, COLU_CONF_NWIDTH | COLU_CONF_DISPLAY},
     /* NOTE: -background is a per-state option, so DEF_BUTTON_BG_COLOR
      * must be a list of one element */
    {TK_OPTION_CUSTOM, "-background", (char *) NULL, (char *) NULL,
     (char *) NULL /* initialized later */,
     Tk_Offset(HeaderColumn, border.obj), Tk_Offset(HeaderColumn, border),
     0, (ClientData) NULL, COLU_CONF_DISPLAY},
    {TK_OPTION_BITMAP, "-bitmap", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, Tk_Offset(HeaderColumn, bitmap),
     TK_OPTION_NULL_OK, (ClientData) NULL,
     COLU_CONF_BITMAP | COLU_CONF_NWIDTH | COLU_CONF_NHEIGHT | COLU_CONF_DISPLAY},
    {TK_OPTION_PIXELS, "-borderwidth", (char *) NULL, (char *) NULL,
     "2", Tk_Offset(HeaderColumn, borderWidthObj), Tk_Offset(HeaderColumn, borderWidth),
     0, (ClientData) NULL, COLU_CONF_TWIDTH | COLU_CONF_NWIDTH | COLU_CONF_NHEIGHT | COLU_CONF_DISPLAY},
    {TK_OPTION_BOOLEAN, "-button", (char *) NULL, (char *) NULL,
     "1", -1, Tk_Offset(HeaderColumn, button),
     0, (ClientData) NULL, 0},
    {TK_OPTION_FONT, "-font", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, Tk_Offset(HeaderColumn, tkfont),
     TK_OPTION_NULL_OK, (ClientData) NULL, COLU_CONF_NWIDTH |
     COLU_CONF_NHEIGHT | COLU_CONF_DISPLAY | COLU_CONF_TEXT},
    {TK_OPTION_STRING, "-image", (char *) NULL, (char *) NULL,
     (char *) NULL, -1, Tk_Offset(HeaderColumn, imageString),
     TK_OPTION_NULL_OK, (ClientData) NULL,
     COLU_CONF_IMAGE | COLU_CONF_NWIDTH | COLU_CONF_NHEIGHT | COLU_CONF_DISPLAY},
    {TK_OPTION_CUSTOM, "-imagepadx", (char *) NULL, (char *) NULL,
     "6", Tk_Offset(HeaderColumn, imagePadXObj),
     Tk_Offset(HeaderColumn, imagePadX), 0, (ClientData) &TreeCtrlCO_pad,
     COLU_CONF_NWIDTH | COLU_CONF_DISPLAY},
    {TK_OPTION_CUSTOM, "-imagepady", (char *) NULL, (char *) NULL,
     "0", Tk_Offset(HeaderColumn, imagePadYObj),
     Tk_Offset(HeaderColumn, imagePadY), 0, (ClientData) &TreeCtrlCO_pad,
     COLU_CONF_NHEIGHT | COLU_CONF_DISPLAY},
    {TK_OPTION_JUSTIFY, "-justify", (char *) NULL, (char *) NULL,
     "left", -1, Tk_Offset(HeaderColumn, justify),
     0, (ClientData) NULL, COLU_CONF_DISPLAY | COLU_CONF_JUSTIFY},
    {TK_OPTION_STRING_TABLE, "-state", (char *) NULL, (char *) NULL,
     "normal", -1, Tk_Offset(HeaderColumn, state), 0, (ClientData) stateST,
     COLU_CONF_NWIDTH | COLU_CONF_NHEIGHT | COLU_CONF_DISPLAY},
    {TK_OPTION_STRING, "-text", (char *) NULL, (char *) NULL,
     (char *) NULL, Tk_Offset(HeaderColumn, textObj), Tk_Offset(HeaderColumn, text),
     TK_OPTION_NULL_OK, (ClientData) NULL,
     COLU_CONF_TEXT | COLU_CONF_NWIDTH | COLU_CONF_NHEIGHT | COLU_CONF_DISPLAY},
    /* -textcolor initialized by TreeHeader_InitInterp() */
    {TK_OPTION_CUSTOM, "-textcolor", (char *) NULL, (char *) NULL,
     (char *) NULL, Tk_Offset(HeaderColumn, textColor.obj),
     Tk_Offset(HeaderColumn, textColor), 0, (ClientData) NULL, COLU_CONF_DISPLAY},
    {TK_OPTION_INT, "-textlines", (char *) NULL, (char *) NULL,
     "1", -1, Tk_Offset(HeaderColumn, textLines),
     0, (ClientData) NULL, COLU_CONF_TEXT | COLU_CONF_NWIDTH |
     COLU_CONF_NHEIGHT | COLU_CONF_DISPLAY},
    {TK_OPTION_CUSTOM, "-textpadx", (char *) NULL, (char *) NULL,
     "6", Tk_Offset(HeaderColumn, textPadXObj),
     Tk_Offset(HeaderColumn, textPadX), 0, (ClientData) &TreeCtrlCO_pad,
     COLU_CONF_NWIDTH | COLU_CONF_DISPLAY},
    {TK_OPTION_CUSTOM, "-textpady", (char *) NULL, (char *) NULL,
     "0", Tk_Offset(HeaderColumn, textPadYObj),
     Tk_Offset(HeaderColumn, textPadY), 0, (ClientData) &TreeCtrlCO_pad,
     COLU_CONF_NHEIGHT | COLU_CONF_DISPLAY},
    {TK_OPTION_END, (char *) NULL, (char *) NULL, (char *) NULL,
     (char *) NULL, 0, -1, 0, 0, 0}
};

#define HEADER_CONF_DISPLAY 0x0001

/* We can also configure -height, -tags and -visible item options */
static Tk_OptionSpec headerSpecs[] = {
    {TK_OPTION_BOOLEAN, "-ownerdrawn", (char *) NULL, (char *) NULL,
     "0", -1, Tk_Offset(TreeHeader_, ownerDrawn),
     0, (ClientData) NULL, HEADER_CONF_DISPLAY},
    {TK_OPTION_END, (char *) NULL, (char *) NULL, (char *) NULL,
     (char *) NULL, 0, -1, 0, 0, 0}
};

/*
 *----------------------------------------------------------------------
 *
 * HeaderCO_Set --
 *
 *	Tk_ObjCustomOption.setProc(). Converts a Tcl_Obj holding a
 *	header description into a TreeHeader.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	May store a TreeHeader pointer into the internal representation
 *	pointer.  May change the pointer to the Tcl_Obj to NULL to indicate
 *	that the specified string was empty and that is acceptable.
 *
 *----------------------------------------------------------------------
 */

static int
HeaderCO_Set(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Current interpreter. */
    Tk_Window tkwin,		/* Window for which option is being set. */
    Tcl_Obj **value,		/* Pointer to the pointer to the value object.
				 * We use a pointer to the pointer because
				 * we may need to return a value (NULL). */
    char *recordPtr,		/* Pointer to storage for the widget record. */
    int internalOffset,		/* Offset within *recordPtr at which the
				 * internal value is to be stored. */
    char *saveInternalPtr,	/* Pointer to storage for the old value. */
    int flags			/* Flags for the option, set Tk_SetOptions. */
    )
{
    TreeCtrl *tree = (TreeCtrl *) ((TkWindow *) tkwin)->instanceData;
    int objEmpty;
    TreeHeader new, *internalPtr;

    if (internalOffset >= 0)
	internalPtr = (TreeHeader *) (recordPtr + internalOffset);
    else
	internalPtr = NULL;

    objEmpty = ObjectIsEmpty((*value));

    if ((flags & TK_OPTION_NULL_OK) && objEmpty)
	(*value) = NULL;
    else {
	if (TreeHeader_FromObj(tree, (*value), &new) != TCL_OK)
	    return TCL_ERROR;
    }
    if (internalPtr != NULL) {
	if ((*value) == NULL)
	    new = NULL;
	*((TreeHeader *) saveInternalPtr) = *internalPtr;
	*internalPtr = new;
    }

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * HeaderCO_Get --
 *
 *	Tk_ObjCustomOption.getProc(). Converts a TreeHeader into a
 *	Tcl_Obj string representation.
 *
 * Results:
 *	Tcl_Obj containing the string representation of the header.
 *	Returns NULL if the TreeHeader is NULL.
 *
 * Side effects:
 *	May create a new Tcl_Obj.
 *
 *----------------------------------------------------------------------
 */

static Tcl_Obj *
HeaderCO_Get(
    ClientData clientData,	/* Not used. */
    Tk_Window tkwin,		/* Window for which option is being set. */
    char *recordPtr,		/* Pointer to widget record. */
    int internalOffset		/* Offset within *recordPtr containing the
				 * sticky value. */
    )
{
    TreeHeader value = *(TreeHeader *) (recordPtr + internalOffset);
/*    TreeCtrl *tree = (TreeCtrl *) ((TkWindow *) tkwin)->instanceData;*/
    if (value == NULL)
	return NULL;
#if 0
    if (value == COLUMN_ALL)
	return Tcl_NewStringObj("all", -1);
#endif
    return TreeHeader_ToObj(value);
}

/*
 *----------------------------------------------------------------------
 *
 * HeaderCO_Restore --
 *
 *	Tk_ObjCustomOption.restoreProc(). Restores a TreeHeader value
 *	from a saved value.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Restores the old value.
 *
 *----------------------------------------------------------------------
 */

static void
HeaderCO_Restore(
    ClientData clientData,	/* Not used. */
    Tk_Window tkwin,		/* Not used. */
    char *internalPtr,		/* Where to store old value. */
    char *saveInternalPtr)	/* Pointer to old value. */
{
    *(TreeHeader *) internalPtr = *(TreeHeader *) saveInternalPtr;
}

/*
 * The following structure contains pointers to functions used for processing
 * a custom config option that handles Tcl_Obj<->TreeHeader conversion.
 * A header description must refer to a single header.
 */
Tk_ObjCustomOption TreeCtrlCO_header =
{
    "header",
    HeaderCO_Set,
    HeaderCO_Get,
    HeaderCO_Restore,
    NULL,
    (ClientData) 0
};

/*
 *----------------------------------------------------------------------
 *
 * Column_Configure --
 *
 *	This procedure is called to process an objc/objv list to set
 *	configuration options for a HeaderColumn.
 *
 * Results:
 *	The return value is a standard Tcl result.  If TCL_ERROR is
 *	returned, then an error message is left in interp's result.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font,
 *	etc. get set for column;  old resources get freed, if there
 *	were any.  Display changes may occur.
 *
 *----------------------------------------------------------------------
 */

static int
Column_Configure(
    TreeHeader header,
    TreeHeaderColumn column,	/* Column record. */
    TreeColumn treeColumn,
    int objc,			/* Number of arguments. */
    Tcl_Obj *CONST objv[],	/* Argument values. */
    int createFlag		/* TRUE if the Column is being created. */
    )
{
    TreeCtrl *tree = header->tree;
    HeaderColumn saved;
    Tk_SavedOptions savedOptions;
    int error;
    Tcl_Obj *errorResult = NULL;
    int mask, maskFree = 0;
    XGCValues gcValues;
    unsigned long gcMask;

    /* Init these to prevent compiler warnings */
    saved.image = NULL;

    for (error = 0; error <= 1; error++) {
	if (error == 0) {
	    if (Tk_SetOptions(tree->interp, (char *) column,
			tree->headerColumnOptionTable, objc, objv, tree->tkwin,
			&savedOptions, &mask) != TCL_OK) {
		mask = 0;
		continue;
	    }

	    /* Wouldn't have to do this if Tk_InitOptions() would return
	     * a mask of configured options like Tk_SetOptions() does. */
	    if (createFlag) {
		if (column->imageString != NULL)
		    mask |= COLU_CONF_IMAGE;
	    }

	    /*
	     * Step 1: Save old values
	     */

	    if (mask & COLU_CONF_IMAGE)
		saved.image = column->image;

	    /*
	     * Step 2: Process new values
	     */

	    if (mask & COLU_CONF_IMAGE) {
		if (column->imageString == NULL) {
		    column->image = NULL;
		} else {
		    column->image = Tree_GetImage(tree, column->imageString);
		    if (column->image == NULL)
			continue;
		    maskFree |= COLU_CONF_IMAGE;
		}
	    }

	    /*
	     * Step 3: Free saved values
	     */

	    if (mask & COLU_CONF_IMAGE) {
		if (saved.image != NULL)
		    Tree_FreeImage(tree, saved.image);
	    }
	    Tk_FreeSavedOptions(&savedOptions);
	    break;
	} else {
	    errorResult = Tcl_GetObjResult(tree->interp);
	    Tcl_IncrRefCount(errorResult);
	    Tk_RestoreSavedOptions(&savedOptions);

	    /*
	     * Free new values.
	     */
	    if (maskFree & COLU_CONF_IMAGE)
		Tree_FreeImage(tree, column->image);

	    /*
	     * Restore old values.
	     */
	    if (mask & COLU_CONF_IMAGE)
		column->image = saved.image;

	    Tcl_SetObjResult(tree->interp, errorResult);
	    Tcl_DecrRefCount(errorResult);
	    return TCL_ERROR;
	}
    }

    /* Wouldn't have to do this if Tk_InitOptions() would return
    * a mask of configured options like Tk_SetOptions() does. */
    if (createFlag) {
	if (column->textObj != NULL)
	    mask |= COLU_CONF_TEXT;
	if (column->bitmap != None)
	    mask |= COLU_CONF_BITMAP;
    }

    if (mask & COLU_CONF_TEXT) {
	if (column->textObj != NULL)
	    (void) Tcl_GetStringFromObj(column->textObj, &column->textLen);
	else
	    column->textLen = 0;
	if (column->textLen) {
	    Tk_Font tkfont = column->tkfont ? column->tkfont : tree->tkfont;
	    column->textWidth = Tk_TextWidth(tkfont, column->text, column->textLen);
	} else
	    column->textWidth = 0;
    }

    if (mask & COLU_CONF_BITMAP) {
	if (column->bitmapGC != None) {
	    Tk_FreeGC(tree->display, column->bitmapGC);
	    column->bitmapGC = None;
	}
	if (column->bitmap != None) {
	    gcValues.clip_mask = column->bitmap;
	    gcValues.graphics_exposures = False;
	    gcMask = GCClipMask | GCGraphicsExposures;
	    column->bitmapGC = Tk_GetGC(tree->tkwin, gcMask, &gcValues);
	}
    }

    if (mask & (COLU_CONF_NWIDTH | COLU_CONF_TWIDTH))
	mask |= COLU_CONF_NHEIGHT;
    if (mask & (COLU_CONF_JUSTIFY | COLU_CONF_TEXT))
	column->textLayoutInvalid = TRUE;

    if (mask & COLU_CONF_NWIDTH)
	column->neededWidth = -1;
    if (mask & COLU_CONF_NHEIGHT) {
	column->neededHeight = -1;
	tree->headerHeight = -1;
    }

    /* Redraw everything */
    if (mask & (COLU_CONF_TWIDTH | COLU_CONF_NWIDTH | COLU_CONF_NHEIGHT)) {
	Tree_InvalidateColumnWidth(tree, treeColumn); /* invalidate width of items */
/*	tree->widthOfColumns = -1;
	tree->widthOfColumnsLeft = tree->widthOfColumnsRight = -1;*/
	Tree_DInfoChanged(tree, DINFO_CHECK_COLUMN_WIDTH | DINFO_DRAW_HEADER);
    }

    /* Redraw header only */
    else if (mask & COLU_CONF_DISPLAY) {
	Tree_DInfoChanged(tree, DINFO_DRAW_HEADER);
    }

    return TCL_OK;
}

int
TreeHeader_ConsumeColumnCget(
    TreeCtrl *tree,		/* Widget info. */
    TreeColumn treeColumn,
    Tcl_Obj *objPtr		/* Option name. */
    )
{
    TreeItemColumn itemColumn;
    TreeHeaderColumn column;
    Tcl_Obj *resultObjPtr;

    if (treeColumn == tree->columnTail)
	return TCL_OK;

    if (tree->headerItems == NULL) {
	panic("the default header was deleted!");
    }
    itemColumn = TreeItem_FindColumn(tree, tree->headerItems, TreeColumn_Index(treeColumn));
    if (itemColumn == NULL) {
	panic("the default header is missing column %s%d!",
	    tree->columnPrefix, TreeColumn_GetID(treeColumn));
    }
    column = TreeItemColumn_GetHeaderColumn(tree, itemColumn);
    resultObjPtr = Tk_GetOptionValue(tree->interp, (char *) column,
	    tree->headerColumnOptionTable, objPtr, tree->tkwin);
    if (resultObjPtr == NULL)
	return TCL_ERROR;
    Tcl_SetObjResult(tree->interp, resultObjPtr);
    return TCL_OK;
}

int
TreeHeader_ConsumeColumnConfig(
    TreeCtrl *tree,		/* Widget info. */
    TreeColumn treeColumn,
    int objc,			/* Number of arguments. */
    Tcl_Obj *CONST objv[]	/* Argument values. */
    )
{
    TreeItemColumn itemColumn;
    TreeHeaderColumn column;

    if ((objc <= 0) || (treeColumn == tree->columnTail))
	return TCL_OK;

    if (tree->headerItems == NULL) {
	panic("the default header was deleted!");
    }
    itemColumn = TreeItem_FindColumn(tree, tree->headerItems, TreeColumn_Index(treeColumn));
    if (itemColumn == NULL) {
	panic("the default header is missing column %s%d!",
	    tree->columnPrefix, TreeColumn_GetID(treeColumn));
    }
    column = TreeItemColumn_GetHeaderColumn(tree, itemColumn);
    return Column_Configure(TreeItem_GetHeader(tree, tree->headerItems), column, treeColumn, objc, objv, FALSE);
}


Tcl_Obj *
TreeHeader_ConsumeColumnOptionInfo(
    TreeCtrl *tree,		/* Widget info. */
    TreeColumn treeColumn,
    Tcl_Obj *objPtr		/* Option name or NULL. */
    )
{
    TreeItemColumn itemColumn;
    TreeHeaderColumn column;

    if (tree->headerItems == NULL) {
	panic("the default header was deleted!");
    }
    itemColumn = TreeItem_FindColumn(tree, tree->headerItems, TreeColumn_Index(treeColumn));
    if (itemColumn == NULL) {
	panic("the default header is missing column %s%d!",
	    tree->columnPrefix, TreeColumn_GetID(treeColumn));
    }
    column = TreeItemColumn_GetHeaderColumn(tree, itemColumn);
    return Tk_GetOptionInfo(tree->interp, (char *) column,
	tree->headerColumnOptionTable, objPtr,  tree->tkwin);
}

/*
 *----------------------------------------------------------------------
 *
 * ColumnStateFromObj --
 *
 *	Parses a string object containing "state" or "!state" to a
 *	state bit flag.
 *	This function is passed to PerStateInfo_FromObj().
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ColumnStateFromObj(
    TreeCtrl *tree,		/* Widget info. */
    Tcl_Obj *obj,		/* String object to parse. */
    int *stateOff,		/* OR'd with state bit if "!state" is
				 * specified. Caller must initialize. */
    int *stateOn		/* OR'd with state bit if "state" is
				 * specified. Caller must initialize. */
    )
{
    Tcl_Interp *interp = tree->interp;
    int i, op = STATE_OP_ON, op2, op3, length, state = 0;
    char ch0, *string;
    CONST char *stateNames[4] = { "normal", "active", "pressed", "up" };
    int states[3];

    states[STATE_OP_ON] = 0;
    states[STATE_OP_OFF] = 0;
    states[STATE_OP_TOGGLE] = 0;

    string = Tcl_GetStringFromObj(obj, &length);
    if (length == 0)
	goto unknown;
    ch0 = string[0];
    if (ch0 == '!') {
	op = STATE_OP_OFF;
	++string;
	ch0 = string[0];
    } else if (ch0 == '~') {
	if (1) {
	    FormatResult(interp, "can't specify '~' for this command");
	    return TCL_ERROR;
	}
	op = STATE_OP_TOGGLE;
	++string;
	ch0 = string[0];
    }
    for (i = 0; i < 4; i++) {
	if ((ch0 == stateNames[i][0]) && !strcmp(string, stateNames[i])) {
	    state = 1L << i;
	    break;
	}
    }
    if (state == 0)
	goto unknown;

    if (op == STATE_OP_ON) {
	op2 = STATE_OP_OFF;
	op3 = STATE_OP_TOGGLE;
    }
    else if (op == STATE_OP_OFF) {
	op2 = STATE_OP_ON;
	op3 = STATE_OP_TOGGLE;
    } else {
	op2 = STATE_OP_ON;
	op3 = STATE_OP_OFF;
    }
    states[op2] &= ~state;
    states[op3] &= ~state;
    states[op] |= state;

    *stateOn |= states[STATE_OP_ON];
    *stateOff |= states[STATE_OP_OFF];

    return TCL_OK;

unknown:
    FormatResult(interp, "unknown state \"%s\"", string);
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Column_MakeState --
 *
 *	Return a bit mask suitable for passing to the PerState_xxx
 *	functions.
 *
 * Results:
 *	State flags for the column's current state.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
Column_MakeState(
    TreeHeaderColumn column	/* Column record. */
    )
{
    int state = 0;
    if (column->state == COLUMN_STATE_NORMAL)
	state |= 1L << 0;
    else if (column->state == COLUMN_STATE_ACTIVE)
	state |= 1L << 1;
    else if (column->state == COLUMN_STATE_PRESSED)
	state |= 1L << 2;
    if (column->arrow == COLUMN_ARROW_UP)
	state |= 1L << 3;
    return state;
}

/*
 *----------------------------------------------------------------------
 *
 * Column_UpdateTextLayout --
 *
 *	Recalculate the TextLayout for the text displayed in the
 *	column header. The old TextLayout (if any) is freed. If
 *	there is no text or if it is only one line then no TextLayout
 *	is created.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory may be allocated/deallocated.
 *
 *----------------------------------------------------------------------
 */

static void
Column_UpdateTextLayout(
    TreeCtrl *tree,
    HeaderColumn *column,	/* Column info. */
    int width			/* Maximum line length. Zero means there
				 * is no limit. */
    )
{
    Tk_Font tkfont;
    char *text = column->text;
    int textLen = column->textLen;
    int justify = column->justify;
    int maxLines = MAX(column->textLines, 0); /* -textlines */
    int wrap = TEXT_WRAP_WORD; /* -textwrap */
    int flags = 0;
    int i, multiLine = FALSE;

    if (column->textLayout != NULL) {
	TextLayout_Free(column->textLayout);
	column->textLayout = NULL;
    }

    if ((text == NULL) || (textLen == 0))
	return;

    for (i = 0; i < textLen; i++) {
	if ((text[i] == '\n') || (text[i] == '\r')) {
	    multiLine = TRUE;
	    break;
	}
    }

#ifdef MAC_OSX_TK
    /* The height of the header is fixed on Aqua. There is only room for
     * a single line of text. */
    if (column->header->tree->useTheme)
	maxLines = 1;
#endif

    if (!multiLine && ((maxLines == 1) || (!width || (width >= column->textWidth))))
	return;

    tkfont = column->tkfont ? column->tkfont : tree->tkfont;

    if (wrap == TEXT_WRAP_WORD)
	flags |= TK_WHOLE_WORDS;

    column->textLayout = TextLayout_Compute(tkfont, text,
	    Tcl_NumUtfChars(text, textLen), width, justify, maxLines,
	    0, 0, flags);
}

/*
 *----------------------------------------------------------------------
 *
 * Column_GetArrowSize --
 *
 *	Return the size of the sort arrow displayed in the column header
 *	for the column's current state.
 *
 * Results:
 *	Height and width of the arrow.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static void
Column_GetArrowSize(
    TreeCtrl *tree,
    HeaderColumn *column,	/* Column info. */
    int *widthPtr,		/* Returned width. */
    int *heightPtr		/* Returned height. */
    )
{
    int state = Column_MakeState(column);
    int arrowWidth = -1, arrowHeight;
    Tk_Image image;
    Pixmap bitmap;

    /* image > bitmap > theme > draw */
    image = PerStateImage_ForState(tree, &column->arrowImage,
	state, NULL);
    if (image != NULL) {
	Tk_SizeOfImage(image, &arrowWidth, &arrowHeight);
    }
    if (arrowWidth == -1) {
	bitmap = PerStateBitmap_ForState(tree, &column->arrowBitmap,
	    state, NULL);
	if (bitmap != None) {
	    Tk_SizeOfBitmap(tree->display, bitmap, &arrowWidth, &arrowHeight);
	}
    }
    if ((arrowWidth == -1) && tree->useTheme &&
	TreeTheme_GetArrowSize(tree, Tk_WindowId(tree->tkwin),
	column->arrow == COLUMN_ARROW_UP, &arrowWidth, &arrowHeight) == TCL_OK) {
    }
    if (arrowWidth == -1) {
	Tk_Font tkfont = column->tkfont ? column->tkfont : tree->tkfont;
	Tk_FontMetrics fm;
	Tk_GetFontMetrics(tkfont, &fm);
	arrowWidth = (fm.linespace + column->textPadY[PAD_TOP_LEFT] +
	    column->textPadY[PAD_BOTTOM_RIGHT] + column->borderWidth * 2) / 2;
	if (!(arrowWidth & 1))
	    arrowWidth--;
	arrowHeight = arrowWidth;
    }

    (*widthPtr) = arrowWidth;
    (*heightPtr) = arrowHeight;
}

/*
 * The following structure holds size/position info for all the graphical
 * elements of a column header.
 */
struct Layout
{
    Tk_Font tkfont;
    Tk_FontMetrics fm;
    int width; /* Provided by caller */
    int height; /* Provided by caller */
    int textLeft;
    int textWidth;
    int bytesThatFit;
    int imageLeft;
    int imageWidth;
    int arrowLeft;
    int arrowWidth;
    int arrowHeight;
};

/*
 * The following structure is used by the Column_DoLayout() procedure to
 * hold size/position info for each graphical element displayed in the
 * header.
 */
struct LayoutPart
{
    int padX[2];
    int padY[2];
    int width;
    int height;
    int left;
    int top;
};

/*
 *----------------------------------------------------------------------
 *
 * Column_DoLayout --
 *
 *	Arrange all the graphical elements making up a column header.
 *
 * Results:
 *	Layout info is returned.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static void
Column_DoLayout(
    TreeCtrl *tree,
    HeaderColumn *column,	/* Column info. */
    struct Layout *layout	/* Returned layout info. The width and
				 * height fields must be initialized. */
    )
{
    struct LayoutPart *parts[3];
    struct LayoutPart partArrow, partImage, partText;
    int i, padList[4], widthList[3], n = 0;
    int iArrow = -1, iImage = -1, iText = -1;
    int left, right;
    int widthForText = 0;
    int arrowSide = column->arrowSide;
    int arrowGravity = column->arrowGravity;
#if defined(MAC_OSX_TK)
    int margins[4];
#endif

#if defined(MAC_OSX_TK)
    /* Under Aqua, we let the Appearance Manager draw the sort arrow. */
    if (tree->useTheme) {
	arrowSide = SIDE_RIGHT;
	arrowGravity = SIDE_RIGHT;
    }
#endif

    padList[0] = 0;

#if defined(MAC_OSX_TK)
    if (tree->useTheme && (column->arrow != COLUMN_ARROW_NONE)) {
	if (TreeTheme_GetHeaderContentMargins(tree, column->state,
		column->arrow, margins) == TCL_OK) {
	    partArrow.width = margins[2];
	} else {
	    partArrow.width = 12;
	}
	/* NOTE: -arrowpadx[1] and -arrowpady ignored. */
	partArrow.padX[PAD_TOP_LEFT] = column->arrowPadX[PAD_TOP_LEFT];
	partArrow.padX[PAD_BOTTOM_RIGHT] = 0;
	partArrow.padY[PAD_TOP_LEFT] = partArrow.padY[PAD_BOTTOM_RIGHT] = 0;
	partArrow.height = 1; /* bogus value */
    }
    else
#endif
    if (column->arrow != COLUMN_ARROW_NONE) {
	Column_GetArrowSize(tree, column, &partArrow.width, &partArrow.height);
	partArrow.padX[PAD_TOP_LEFT] = column->arrowPadX[PAD_TOP_LEFT];
	partArrow.padX[PAD_BOTTOM_RIGHT] = column->arrowPadX[PAD_BOTTOM_RIGHT];
	partArrow.padY[PAD_TOP_LEFT] = column->arrowPadY[PAD_TOP_LEFT];
	partArrow.padY[PAD_BOTTOM_RIGHT] = column->arrowPadY[PAD_BOTTOM_RIGHT];
    }
    if ((column->arrow != COLUMN_ARROW_NONE) && (arrowSide == SIDE_LEFT)) {
	parts[n] = &partArrow;
	padList[n] = partArrow.padX[PAD_TOP_LEFT];
	padList[n + 1] = partArrow.padX[PAD_BOTTOM_RIGHT];
	iArrow = n++;
    }
    if ((column->image != NULL) || (column->bitmap != None)) {
	if (column->image != NULL)
	    Tk_SizeOfImage(column->image, &partImage.width, &partImage.height);
	else
	    Tk_SizeOfBitmap(tree->display, column->bitmap, &partImage.width, &partImage.height);
	partImage.padX[PAD_TOP_LEFT] = column->imagePadX[PAD_TOP_LEFT];
	partImage.padX[PAD_BOTTOM_RIGHT] = column->imagePadX[PAD_BOTTOM_RIGHT];
	partImage.padY[PAD_TOP_LEFT] = column->imagePadY[PAD_TOP_LEFT];
	partImage.padY[PAD_BOTTOM_RIGHT] = column->imagePadY[PAD_BOTTOM_RIGHT];
	parts[n] = &partImage;
	padList[n] = MAX(partImage.padX[PAD_TOP_LEFT], padList[n]);
	padList[n + 1] = partImage.padX[PAD_BOTTOM_RIGHT];
	iImage = n++;
    }
    if (column->textLen > 0) {
	struct LayoutPart *parts2[3];
	int n2 = 0;

	partText.padX[PAD_TOP_LEFT] = column->textPadX[PAD_TOP_LEFT];
	partText.padX[PAD_BOTTOM_RIGHT] = column->textPadX[PAD_BOTTOM_RIGHT];
	partText.padY[PAD_TOP_LEFT] = column->textPadY[PAD_TOP_LEFT];
	partText.padY[PAD_BOTTOM_RIGHT] = column->textPadY[PAD_BOTTOM_RIGHT];

	/* Calculate space for the text */
	if (iArrow != -1)
	    parts2[n2++] = &partArrow;
	if (iImage != -1)
	    parts2[n2++] = &partImage;
	parts2[n2++] = &partText;
	if ((column->arrow != COLUMN_ARROW_NONE) && (arrowSide == SIDE_RIGHT))
	    parts2[n2++] = &partArrow;
	widthForText = layout->width;
	for (i = 0; i < n2; i++) {
	    if (i)
		widthForText -= MAX(parts2[i]->padX[0], parts2[i-1]->padX[1]);
	    else
		widthForText -= parts2[i]->padX[0];
	    if (parts2[i] != &partText)
		widthForText -= parts2[i]->width;
	}
	widthForText -= parts2[n2-1]->padX[1];
    }
    layout->bytesThatFit = 0;
    if (widthForText > 0) {
	if (column->textLayoutInvalid || (column->textLayoutWidth != widthForText)) {
	    Column_UpdateTextLayout(tree, column, widthForText);
	    column->textLayoutInvalid = FALSE;
	    column->textLayoutWidth = widthForText;
	}
	if (column->textLayout != NULL) {
	    TextLayout_Size(column->textLayout, &partText.width, &partText.height);
	    parts[n] = &partText;
	    padList[n] = MAX(partText.padX[PAD_TOP_LEFT], padList[n]);
	    padList[n + 1] = partText.padX[PAD_BOTTOM_RIGHT];
	    iText = n++;
	} else {
	    layout->tkfont = column->tkfont ? column->tkfont : tree->tkfont;
	    Tk_GetFontMetrics(layout->tkfont, &layout->fm);
	    if (widthForText >= column->textWidth) {
		partText.width = column->textWidth;
		partText.height = layout->fm.linespace;
		layout->bytesThatFit = column->textLen;
	    } else {
		partText.width = widthForText;
		partText.height = layout->fm.linespace;
		layout->bytesThatFit = Tree_Ellipsis(layout->tkfont,
			column->text, column->textLen, &widthForText,
			"...", FALSE);
	    }
	    parts[n] = &partText;
	    padList[n] = MAX(partText.padX[PAD_TOP_LEFT], padList[n]);
	    padList[n + 1] = partText.padX[PAD_BOTTOM_RIGHT];
	    iText = n++;
	}
    }
    if ((column->arrow != COLUMN_ARROW_NONE) && (arrowSide == SIDE_RIGHT)) {
	parts[n] = &partArrow;
	padList[n] = MAX(partArrow.padX[PAD_TOP_LEFT], padList[n]);
	padList[n + 1] = partArrow.padX[PAD_BOTTOM_RIGHT];
	iArrow = n++;
    }

    if (n == 0)
	return;

    for (i = 0; i < n; i++) {
	padList[i] = parts[i]->padX[0];
	if (i)
	    padList[i] = MAX(padList[i], parts[i-1]->padX[1]);
	padList[i + 1] = parts[i]->padX[1];
	widthList[i] = parts[i]->width;
    }
    if (iText != -1) {
	switch (column->justify) {
	    case TK_JUSTIFY_LEFT:
		partText.left = 0;
		break;
	    case TK_JUSTIFY_RIGHT:
		partText.left = layout->width;
		break;
	    case TK_JUSTIFY_CENTER:
		if (iImage == -1)
		    partText.left = (layout->width - partText.width) / 2;
		else
		    partText.left = (layout->width - partImage.width -
			    padList[iText] - partText.width) / 2 + partImage.width +
			padList[iText];
		break;
	}
    }

    if (iImage != -1) {
	switch (column->justify) {
	    case TK_JUSTIFY_LEFT:
		partImage.left = 0;
		break;
	    case TK_JUSTIFY_RIGHT:
		partImage.left = layout->width;
		break;
	    case TK_JUSTIFY_CENTER:
		if (iText == -1)
		    partImage.left = (layout->width - partImage.width) / 2;
		else
		    partImage.left = (layout->width - partImage.width -
			    padList[iText] - partText.width) / 2;
		break;
	}
    }

    if (iArrow == -1)
	goto finish;

    switch (column->justify) {
	case TK_JUSTIFY_LEFT:
	    switch (arrowSide) {
		case SIDE_LEFT:
		    partArrow.left = 0;
		    break;
		case SIDE_RIGHT:
		    switch (arrowGravity) {
			case SIDE_LEFT:
			    partArrow.left = 0;
			    break;
			case SIDE_RIGHT:
			    partArrow.left = layout->width;
			    break;
		    }
		    break;
	    }
	    break;
	case TK_JUSTIFY_RIGHT:
	    switch (arrowSide) {
		case SIDE_LEFT:
		    switch (arrowGravity) {
			case SIDE_LEFT:
			    partArrow.left = 0;
			    break;
			case SIDE_RIGHT:
			    partArrow.left = layout->width;
			    break;
		    }
		    break;
		case SIDE_RIGHT:
		    partArrow.left = layout->width;
		    break;
	    }
	    break;
	case TK_JUSTIFY_CENTER:
	    switch (arrowSide) {
		case SIDE_LEFT:
		    switch (arrowGravity) {
			case SIDE_LEFT:
			    partArrow.left = 0;
			    break;
			case SIDE_RIGHT:
			    if (n == 3)
				partArrow.left =
				    (layout->width - widthList[1] - padList[2] -
					    widthList[2]) / 2 - padList[1] - widthList[0];
			    else if (n == 2)
				partArrow.left =
				    (layout->width - widthList[1]) / 2 -
				    padList[1] - widthList[0];
			    else
				partArrow.left = layout->width;
			    break;
		    }
		    break;
		case SIDE_RIGHT:
		    switch (arrowGravity) {
			case SIDE_LEFT:
			    if (n == 3)
				partArrow.left =
				    (layout->width - widthList[0] - padList[1] -
					    widthList[1]) / 2 + widthList[0] + padList[1] +
				    widthList[1] + padList[2];
			    else if (n == 2)
				partArrow.left =
				    (layout->width - widthList[0]) / 2 +
				    widthList[0] + padList[1];
			    else
				partArrow.left = 0;
			    break;
			case SIDE_RIGHT:
			    partArrow.left = layout->width;
			    break;
		    }
		    break;
	    }
	    break;
    }

finish:
    right = layout->width - padList[n];
    for (i = n - 1; i >= 0; i--) {
	if (parts[i]->left + parts[i]->width > right)
	    parts[i]->left = right - parts[i]->width;
	right -= parts[i]->width + padList[i];
    }
    left = padList[0];
    for (i = 0; i < n; i++) {
	if (parts[i]->left < left)
	    parts[i]->left = left;
	left += parts[i]->width + padList[i + 1];
    }

    if (iArrow != -1) {
	layout->arrowLeft = partArrow.left;
	layout->arrowWidth = partArrow.width;
	layout->arrowHeight = partArrow.height;
    }
    if (iImage != -1) {
	layout->imageLeft = partImage.left;
	layout->imageWidth = partImage.width;
    }
    if (iText != -1) {
	layout->textLeft = partText.left;
	layout->textWidth = partText.width;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TreeHeader_NeededWidth --
 *
 *	Return the total width requested by all the graphical elements
 *	that make up a column header.  The width is recalculated if it
 *	is marked out-of-date.
 *
 * Results:
 *	The width needed by the current arrangement of the
 *	bitmap/image/text/arrow.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
TreeHeaderColumn_NeededWidth(
    TreeHeader header,		/* Header token. */
    TreeHeaderColumn column	/* Column token. */
    )
{
    TreeCtrl *tree = header->tree;
    int i, widthList[3], padList[4], n = 0;
    int arrowWidth, arrowHeight, arrowPadX[2];
    int arrowSide = column->arrowSide;
#if defined(MAC_OSX_TK)
    int margins[4];
#endif

    if (!tree->showHeader)
	return 0;

    if (header->ownerDrawn)
	return 0;

    if (column->neededWidth >= 0)
	return column->neededWidth;

    for (i = 0; i < 3; i++) widthList[i] = 0;
    for (i = 0; i < 4; i++) padList[i] = 0;

    for (i = 0; i < 2; i++) arrowPadX[i] = column->arrowPadX[i];

#if defined(MAC_OSX_TK)
    /* Under OSX we let the Appearance Manager draw the sort arrow. This code
     * assumes the arrow is on the right. */
    if (tree->useTheme && (column->arrow != COLUMN_ARROW_NONE)) {
	if (TreeTheme_GetHeaderContentMargins(tree, column->state,
		column->arrow, margins) == TCL_OK) {
	    arrowWidth = margins[2];
	} else {
	    arrowWidth = 12;
	}
	arrowHeight = 1; /* bogus value */
	arrowSide = SIDE_RIGHT;
	arrowPadX[PAD_BOTTOM_RIGHT] = 0;
    }
    else
#endif
    if (column->arrow != COLUMN_ARROW_NONE)
	Column_GetArrowSize(tree, column, &arrowWidth, &arrowHeight);
    if ((column->arrow != COLUMN_ARROW_NONE) && (arrowSide == SIDE_LEFT)) {
	widthList[n] = arrowWidth;
	padList[n] = arrowPadX[PAD_TOP_LEFT];
	padList[n + 1] = arrowPadX[PAD_BOTTOM_RIGHT];
	n++;
    }
    if ((column->image != NULL) || (column->bitmap != None)) {
	int imgWidth, imgHeight;
	if (column->image != NULL)
	    Tk_SizeOfImage(column->image, &imgWidth, &imgHeight);
	else
	    Tk_SizeOfBitmap(tree->display, column->bitmap, &imgWidth, &imgHeight);
	padList[n] = MAX(column->imagePadX[PAD_TOP_LEFT], padList[n]);
	padList[n + 1] = column->imagePadX[PAD_BOTTOM_RIGHT];
	widthList[n] = imgWidth;
	n++;
    }
    if (column->textLen > 0) {
	padList[n] = MAX(column->textPadX[PAD_TOP_LEFT], padList[n]);
	padList[n + 1] = column->textPadX[PAD_BOTTOM_RIGHT];
	if (column->textLayoutInvalid || (column->textLayoutWidth != 0)) {
	    Column_UpdateTextLayout(tree, column, 0);
	    column->textLayoutInvalid = FALSE;
	    column->textLayoutWidth = 0;
	}
	if (column->textLayout != NULL)
	    TextLayout_Size(column->textLayout, &widthList[n], NULL);
	else
	    widthList[n] = column->textWidth;
	n++;
    }
    if ((column->arrow != COLUMN_ARROW_NONE) && (arrowSide == SIDE_RIGHT)) {
	widthList[n] = arrowWidth;
	padList[n] = MAX(arrowPadX[PAD_TOP_LEFT], padList[n]);
	padList[n + 1] = arrowPadX[PAD_BOTTOM_RIGHT];
	n++;
    }

    column->neededWidth = 0;
    for (i = 0; i < n; i++)
	column->neededWidth += widthList[i] + padList[i];
    column->neededWidth += padList[n];

    /* Notice I'm not considering column->borderWidth. */

    return column->neededWidth;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeHeaderColumn_NeededHeight --
 *
 *	Return the total height requested by all the graphical elements
 *	that make up a column header.  The height is recalculated if it
 *	is marked out-of-date.
 *
 * Results:
 *	The height needed by the current arrangement of the
 *	bitmap/image/text/arrow.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
TreeHeaderColumn_NeededHeight(
    TreeHeader header,		/* Header token. */
    TreeHeaderColumn column	/* Column token. */
    )
{
    TreeCtrl *tree = header->tree;
    int margins[4];

    if (column->neededHeight >= 0)
	return column->neededHeight;

#if defined(MAC_OSX_TK)
    /* List headers are a fixed height on Aqua */
    if (tree->useTheme &&
	(TreeTheme_GetHeaderFixedHeight(tree, &column->neededHeight) == TCL_OK)) {
	return column->neededHeight;
    }
#endif

    column->neededHeight = 0;
    if (column->arrow != COLUMN_ARROW_NONE) {
	int arrowWidth, arrowHeight;
	Column_GetArrowSize(tree, column, &arrowWidth, &arrowHeight);
	arrowHeight += column->arrowPadY[PAD_TOP_LEFT]
	    + column->arrowPadY[PAD_BOTTOM_RIGHT];
	column->neededHeight = MAX(column->neededHeight, arrowHeight);
    }
    if ((column->image != NULL) || (column->bitmap != None)) {
	int imgWidth, imgHeight;
	if (column->image != NULL)
	    Tk_SizeOfImage(column->image, &imgWidth, &imgHeight);
	else
	    Tk_SizeOfBitmap(tree->display, column->bitmap, &imgWidth, &imgHeight);
	imgHeight += column->imagePadY[PAD_TOP_LEFT]
	    + column->imagePadY[PAD_BOTTOM_RIGHT];
	column->neededHeight = MAX(column->neededHeight, imgHeight);
    }
    if (column->text != NULL) {
	struct Layout layout;
	layout.width = TreeColumn_UseWidth(
			    Tree_FindColumn(tree,
				TreeItemColumn_Index(tree, header->item, column->itemColumn)));
	layout.height = -1;
	Column_DoLayout(tree, column, &layout);
	if (column->textLayout != NULL) {
	    int height;
	    TextLayout_Size(column->textLayout, NULL, &height);
	    height += column->textPadY[PAD_TOP_LEFT]
		+ column->textPadY[PAD_BOTTOM_RIGHT];
	    column->neededHeight = MAX(column->neededHeight, height);
	} else {
	    Tk_Font tkfont = column->tkfont ? column->tkfont : tree->tkfont;
	    Tk_FontMetrics fm;
	    Tk_GetFontMetrics(tkfont, &fm);
	    fm.linespace += column->textPadY[PAD_TOP_LEFT]
		+ column->textPadY[PAD_BOTTOM_RIGHT];
	    column->neededHeight = MAX(column->neededHeight, fm.linespace);
	}
    }
    if (tree->useTheme &&
	(TreeTheme_GetHeaderContentMargins(tree, column->state,
		column->arrow, margins) == TCL_OK)) {
#ifdef WIN32
	/* I'm hacking these margins since the default XP theme does not give
	 * reasonable ContentMargins for HP_HEADERITEM */
	int bw = MAX(column->borderWidth, 3);
	margins[1] = MAX(margins[1], bw);
	margins[3] = MAX(margins[3], bw);
#endif /* WIN32 */
	column->neededHeight += margins[1] + margins[3];
    } else {
	column->neededHeight += column->borderWidth * 2;
    }

    return column->neededHeight;
}

/*
 *----------------------------------------------------------------------
 *
 * Column_DrawArrow --
 *
 *	Draw the sort arrow for a column.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn in a drawable.
 *
 *----------------------------------------------------------------------
 */

static void
Column_DrawArrow(
    TreeHeader header,		/* Header token. */
    TreeHeaderColumn column,	/* Column token. */
    TreeDrawable td,		/* Where to draw. */
    int x, int y,		/* Top-left corner of the column's header. */
    struct Layout layout	/* Size/position info. */
    )
{
    TreeCtrl *tree = header->tree;
    int height = TreeItem_Height(tree, header->item);
    int sunken = column->state == COLUMN_STATE_PRESSED;
    Tk_Image image = NULL;
    Pixmap bitmap;
    Tk_3DBorder border;
    int state = Column_MakeState(column);
    int arrowPadTop = column->arrowPadY[PAD_TOP_LEFT];
    int arrowPadY = arrowPadTop + column->arrowPadY[PAD_BOTTOM_RIGHT];
    int arrowTop = y + (height - (layout.arrowHeight + arrowPadY)) / 2
	+ arrowPadTop;

    if (column->arrow == COLUMN_ARROW_NONE)
	return;

    image = PerStateImage_ForState(tree, &column->arrowImage, state, NULL);
    if (image != NULL) {
	Tree_RedrawImage(image, 0, 0, layout.arrowWidth, layout.arrowHeight,
	    td,
	    x + layout.arrowLeft + sunken,
	    arrowTop + sunken);
	return;
    }

    bitmap = PerStateBitmap_ForState(tree, &column->arrowBitmap, state, NULL);
    if (bitmap != None) {
	int bx, by;
	bx = x + layout.arrowLeft + sunken;
	by = arrowTop + sunken;
	Tree_DrawBitmap(tree, bitmap, td.drawable, NULL, NULL,
		0, 0,
		(unsigned int) layout.arrowWidth, (unsigned int) layout.arrowHeight,
		bx, by);
	return;
    }

    if (tree->useTheme) {
	if (TreeTheme_DrawHeaderArrow(tree, td, column->state,
	    column->arrow == COLUMN_ARROW_UP, x + layout.arrowLeft + sunken,
	    arrowTop + sunken,
	    layout.arrowWidth, layout.arrowHeight) == TCL_OK)
	    return;
    }

    if (1) {
	int arrowWidth = layout.arrowWidth;
	int arrowHeight = layout.arrowHeight;
	int arrowBottom = arrowTop + arrowHeight;
	XPoint points[5];
	int color1 = 0, color2 = 0;
	int i;

	switch (column->arrow) {
	    case COLUMN_ARROW_UP:
		points[0].x = x + layout.arrowLeft;
		points[0].y = arrowBottom - 1;
		points[1].x = x + layout.arrowLeft + arrowWidth / 2;
		points[1].y = arrowTop - 1;
		color1 = TK_3D_DARK_GC;
		points[4].x = x + layout.arrowLeft + arrowWidth / 2;
		points[4].y = arrowTop - 1;
		points[3].x = x + layout.arrowLeft + arrowWidth - 1;
		points[3].y = arrowBottom - 1;
		points[2].x = x + layout.arrowLeft;
		points[2].y = arrowBottom - 1;
		color2 = TK_3D_LIGHT_GC;
		break;
	    case COLUMN_ARROW_DOWN:
		points[0].x = x + layout.arrowLeft + arrowWidth - 1;
		points[0].y = arrowTop;
		points[1].x = x + layout.arrowLeft + arrowWidth / 2;
		points[1].y = arrowBottom;
		color1 = TK_3D_LIGHT_GC;
		points[2].x = x + layout.arrowLeft + arrowWidth - 1;
		points[2].y = arrowTop;
		points[3].x = x + layout.arrowLeft;
		points[3].y = arrowTop;
		points[4].x = x + layout.arrowLeft + arrowWidth / 2;
		points[4].y = arrowBottom;
		color2 = TK_3D_DARK_GC;
		break;
	}
	for (i = 0; i < 5; i++) {
	    points[i].x += sunken;
	    points[i].y += sunken;
	}

	border = PerStateBorder_ForState(tree, &column->border, state, NULL);
	if (border == NULL)
	    border = tree->border;
	XDrawLines(tree->display, td.drawable,
		Tk_3DBorderGC(tree->tkwin, border, color2),
		points + 2, 3, CoordModeOrigin);
	XDrawLines(tree->display, td.drawable,
		Tk_3DBorderGC(tree->tkwin, border, color1),
		points, 2, CoordModeOrigin);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Column_Draw --
 *
 *	Draw the header for a column.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stuff is drawn in a drawable.
 *
 *----------------------------------------------------------------------
 */

static void
Column_Draw(
    TreeHeader header,		/* Header token. */
    TreeHeaderColumn column,	/* Column token. */
    int lock,			/* COLUMN_LOCK_XXX */
    TreeDrawable td,		/* Where to draw. */
    int x, int y,		/* Area of the column header to draw. */
    int width, int height,	/* ^ */
    int visIndex,		/* 0-based index in the list of visible
				 * columns. */
    int dragImage		/* TRUE if we are creating a transparent
				 * drag image for this header. */
    )
{
    TreeCtrl *tree = header->tree;
    struct Layout layout;
    int sunken = column->state == COLUMN_STATE_PRESSED;
    int relief = sunken ? TK_RELIEF_SUNKEN : TK_RELIEF_RAISED;
    Tk_3DBorder border;
    int theme = TCL_ERROR;
    GC gc = None;
    TkRegion clipRgn = NULL;

    /* Hack on */
    if (visIndex == 0 && lock == COLUMN_LOCK_NONE) {
	x += tree->canvasPadX[PAD_TOP_LEFT];
	width -= tree->canvasPadX[PAD_TOP_LEFT];
    }

    layout.width = width;
    layout.height = height;
    Column_DoLayout(tree, column, &layout);

    /* Hack off */
    if (visIndex == 0 && lock == COLUMN_LOCK_NONE) {
	x -= tree->canvasPadX[PAD_TOP_LEFT];
	width += tree->canvasPadX[PAD_TOP_LEFT];
    }

    border = PerStateBorder_ForState(tree, &column->border,
	Column_MakeState(column), NULL);
    if (border == NULL)
	border = tree->border;

    if (0 && dragImage) {
	GC gc = Tk_GCForColor(tree->columnDrag.color, Tk_WindowId(tree->tkwin));
	XFillRectangle(tree->display, td.drawable, gc, x, y, width, height);
    } else {
	if (tree->useTheme) {
	    theme = TreeTheme_DrawHeaderItem(tree, td, column->state,
		    column->arrow, visIndex, x, y, width, height);
	}
	if (theme != TCL_OK) {
	    Tk_Fill3DRectangle(tree->tkwin, td.drawable, border,
		    x, y, width, height, 0, TK_RELIEF_FLAT);
	}
    }

    /* Hack on */
    if (visIndex == 0 && lock == COLUMN_LOCK_NONE) {
	x += tree->canvasPadX[PAD_TOP_LEFT];
	width -= tree->canvasPadX[PAD_TOP_LEFT];
    }

    if (column->image != NULL) {
	int imgW, imgH, ix, iy, h;
	Tk_SizeOfImage(column->image, &imgW, &imgH);
	ix = x + layout.imageLeft + sunken;
	h = column->imagePadY[PAD_TOP_LEFT] + imgH
	    + column->imagePadY[PAD_BOTTOM_RIGHT];
	iy = y + (height - h) / 2 + sunken;
	iy += column->imagePadY[PAD_TOP_LEFT];
	Tree_RedrawImage(column->image, 0, 0, imgW, imgH, td, ix, iy);
    } else if (column->bitmap != None) {
	int imgW, imgH, bx, by, h;

	Tk_SizeOfBitmap(tree->display, column->bitmap, &imgW, &imgH);
	bx = x + layout.imageLeft + sunken;
	h = column->imagePadY[PAD_TOP_LEFT] + imgH
	    + column->imagePadY[PAD_BOTTOM_RIGHT];
	by = y + (height - h) / 2 + sunken;
	by += column->imagePadY[PAD_TOP_LEFT];
	Tree_DrawBitmapWithGC(tree, column->bitmap, td.drawable, column->bitmapGC,
		0, 0, (unsigned int) imgW, (unsigned int) imgH,
		bx, by);
    }

    /* Get a graphics context for drawing the text */
    if ((column->text != NULL) && ((column->textLayout != NULL) || (layout.bytesThatFit != 0))) {
	TreeColor *tc;
	XColor *textColor = tree->defColumnTextColor;
	XGCValues gcValues;
	unsigned long mask;
	TreeRectangle trClip;

	tc = PerStateColor_ForState(tree, &column->textColor,
	    Column_MakeState(column), NULL);
	if (tc == NULL || tc->color == NULL) {
	    if (tree->useTheme && TreeTheme_GetColumnTextColor(tree, column->state, &textColor)
		    != TCL_OK) {
		/*textColor = tree->fgColorPtr*/;
	    }
	} else {
	    textColor = tc->color;
	}
	gcValues.font = Tk_FontId(column->tkfont ? column->tkfont : tree->tkfont); /* layout.tkfont */
	gcValues.foreground = textColor->pixel;
	gcValues.graphics_exposures = False;
	mask = GCFont | GCForeground | GCGraphicsExposures;
	gc = Tree_GetGC(tree, mask, &gcValues);

	TreeRect_SetXYWH(trClip, x + layout.textLeft + sunken, y,
		MIN(layout.textWidth, td.width), MIN(height, td.height));
	clipRgn = Tree_GetRectRegion(tree, &trClip);
	TkSetRegion(tree->display, gc, clipRgn);
    }

    if ((column->text != NULL) && (column->textLayout != NULL)) {
	int h;
	TextLayout_Size(column->textLayout, NULL, &h);
	h += column->textPadY[PAD_TOP_LEFT] + column->textPadY[PAD_BOTTOM_RIGHT];
	TextLayout_Draw(tree->display, td.drawable, gc,
		column->textLayout,
		x + layout.textLeft + sunken,
		y + (height - h) / 2 + column->textPadY[PAD_TOP_LEFT] + sunken,
		0, -1, -1);
    } else if ((column->text != NULL) && (layout.bytesThatFit != 0)) {
	char staticStr[256], *text = staticStr;
	int textLen = column->textLen;
	char *ellipsis = "...";
	int ellipsisLen = (int) strlen(ellipsis);
	int tx, ty, h;

	if (textLen + ellipsisLen > sizeof(staticStr))
	    text = ckalloc(textLen + ellipsisLen);
	memcpy(text, column->text, textLen);
	if (layout.bytesThatFit != textLen) {
	    textLen = abs(layout.bytesThatFit);
	    if (layout.bytesThatFit > 0) {
		memcpy(text + layout.bytesThatFit, ellipsis, ellipsisLen);
		textLen += ellipsisLen;
	    }
	}

	tx = x + layout.textLeft + sunken;
	h = column->textPadY[PAD_TOP_LEFT] + layout.fm.linespace
	    + column->textPadY[PAD_BOTTOM_RIGHT];
	ty = y + (height - h) / 2 + layout.fm.ascent + sunken;
	ty += column->textPadY[PAD_TOP_LEFT];
	Tk_DrawChars(tree->display, td.drawable, gc,
		layout.tkfont, text, textLen, tx, ty);
	if (text != staticStr)
	    ckfree(text);
    }

    if (clipRgn != NULL) {
	Tree_UnsetClipMask(tree, td.drawable, gc);
	Tree_FreeRegion(tree, clipRgn);
    }

    if (0 && dragImage)
	return;

#if defined(MAC_OSX_TK)
    /* Under Aqua, we let the Appearance Manager draw the sort arrow */
    if (theme != TCL_OK)
#endif
    Column_DrawArrow(header, column, td, x, y, layout);

    if (theme != TCL_OK) {
	/* Hack */
	if (visIndex == 0 && lock == COLUMN_LOCK_NONE) {
	    x -= tree->canvasPadX[PAD_TOP_LEFT];
	    width += tree->canvasPadX[PAD_TOP_LEFT];
	}
	Tk_Draw3DRectangle(tree->tkwin, td.drawable, border,
		x, y, width, height, column->borderWidth, relief);
    }
}

void
TreeHeaderColumn_Draw(
    TreeHeader header,		/* Header token. */
    TreeHeaderColumn column,	/* Column token. */
    int visIndex,
    int lock,			/* COLUMN_LOCK_XXX */
    StyleDrawArgs *drawArgs
    )
{
    TreeCtrl *tree = header->tree;
    TreeDrawable td = drawArgs->td;
    int x = drawArgs->x, y = drawArgs->y, width = drawArgs->width, height = drawArgs->height;
    int isDragHeader = tree->columnDrag.header == header;
    int isDragColumn = 0;

#if 0
    if (column == NULL) { /* the tail column! */
	if (!TreeColumn_Visible(tree->columnTree)) {
	    Tk_Fill3DRectangle(tree->tkwin, td.drawable, tree->border,
		    x, y, width, height, 0, TK_RELIEF_FLAT);
	} else if (tree->useTheme &&
	    (TreeTheme_DrawHeaderItem(tree, td, 0, 0, tree->columnCountVis,
		x, y, width, height) == TCL_OK)) {
	} else {
	    Tk_3DBorder border;
	    border = PerStateBorder_ForState(tree, &column->border,
		Column_MakeState(column), NULL);
	    if (border == NULL)
		border = tree->border;
	    Tk_Fill3DRectangle(tree->tkwin, td.drawable, border,
		    x, y, width, height, 4/*column->borderWidth*/, TK_RELIEF_RAISED);
	}
	return;
    }
#endif

    if (isDragHeader && tree->columnDrag.column != NULL) {
	TreeColumn treeColumn = Tree_FindColumn(tree, TreeItemColumn_Index(tree, header->item, column->itemColumn));
	if (tree->columnDrag.column == treeColumn)
	    isDragColumn = 1;
    }
    if (isDragColumn && tree->columnDrag.indColumn != NULL)
	return;
    if (isDragHeader && tree->columnDrag.indColumn != NULL && tree->columnDrag.column != NULL) {
	int index1 = TreeColumn_Index(tree->columnDrag.column);
	int index2 = TreeColumn_Index(tree->columnDrag.indColumn);
	int index3 = TreeItemColumn_Index(tree, header->item, column->itemColumn);
	TreeRectangle bbox;
#if 0
	if (isDragColumn) {
	    if (TreeItem_GetRects(tree, header->item, tree->columnDrag.indColumn,
		    0, NULL, &bbox) == 1) {
		if (index1 > index2)
		    x = bbox.x;
		else
		    x = bbox.x + bbox.width - width;
		x -= tree->drawableXOrigin;
	    }
	} else
#endif
	if (index3 >= MIN(index1,index2) && index3 <= MAX(index1,index2)) {
	    if (TreeItem_GetRects(tree, header->item, tree->columnDrag.column,
		    0, NULL, &bbox) == 1) {
		if (index1 < index2)
		    x -= bbox.width;
		else
		    x += bbox.width;
	    }
	    drawArgs->x = x;
	}
    }
    if (header->ownerDrawn || isDragColumn) {
	GC gc = Tk_3DBorderGC(tree->tkwin, tree->border, TK_3D_FLAT_GC);
	TreeRectangle tr;

	TreeRect_SetXYWH(tr, x, y, width, height);
	Tree_FillRectangle(tree, td, NULL, gc, tr);
    } else {
	Column_Draw(header, column, lock, td, x, y, width, height, visIndex, FALSE);
    }

    if (!isDragColumn && (drawArgs->style != NULL)) {
	TreeStyle_Draw(drawArgs); /* may change drawArgs! */
    }
}

/*
 *----------------------------------------------------------------------
 *
 * SetImageForColumn --
 *
 *	Set a photo image containing a simplified picture of the header
 *	of a column. This image is used when dragging and dropping a column
 *	header.
 *
 * Results:
 *	Token for a photo image, or NULL if the image could not be
 *	created.
 *
 * Side effects:
 *	A photo image called "::TreeCtrl::ImageColumn" will be created if
 *	it doesn't exist. The image is set to contain a picture of the
 *	column header.
 *
 *----------------------------------------------------------------------
 */

static Tk_Image
SetImageForColumn(
    TreeHeader header,		/* Header token. */
    TreeHeaderColumn column,	/* Column record. */
    int lock,
    int width,
    int height
    )
{
    TreeCtrl *tree = header->tree;
    TreeItem item = header->item;
    Tk_PhotoHandle photoH;
    TreeDrawable td;
    XImage *ximage;
    int visIndex = 0; /* FIXME */

    photoH = Tk_FindPhoto(tree->interp, "::TreeCtrl::ImageColumn");
    if (photoH == NULL) {
	Tcl_GlobalEval(tree->interp, "image create photo ::TreeCtrl::ImageColumn");
	photoH = Tk_FindPhoto(tree->interp, "::TreeCtrl::ImageColumn");
	if (photoH == NULL)
	    return NULL;
    }

    td.width = width;
    td.height = height;
    td.drawable = Tk_GetPixmap(tree->display, Tk_WindowId(tree->tkwin),
	    width, height, Tk_Depth(tree->tkwin));

    if (header->ownerDrawn) {
	GC gc = Tk_3DBorderGC(tree->tkwin, tree->border, TK_3D_FLAT_GC);
	TreeRectangle tr;

	TreeRect_SetXYWH(tr, 0, 0, width, height);
	Tree_FillRectangle(tree, td, NULL, gc, tr);
    } else
	Column_Draw(header, column, lock, td, 0, 0, width, height, visIndex, TRUE);

    if (TreeItemColumn_GetStyle(tree, column->itemColumn) != NULL) {
	StyleDrawArgs drawArgs;
	int area;
	switch (lock) {
	    case COLUMN_LOCK_LEFT:
		area = TREE_AREA_HEADER_LEFT;
		break;
	    case COLUMN_LOCK_NONE:
		area = TREE_AREA_HEADER_NONE;
		break;
	    case COLUMN_LOCK_RIGHT:
		area = TREE_AREA_HEADER_RIGHT;
		break;
	}
	if (!Tree_AreaBbox(tree, area, &drawArgs.bounds)) {
	    TreeRect_SetXYWH(drawArgs.bounds, 0, 0, 0, 0);
	}
	drawArgs.tree = tree;
	drawArgs.item = item; /* needed for gradients */
	drawArgs.td = td;
	drawArgs.state = TreeItem_GetState(tree, item) |
	    TreeItemColumn_GetState(tree, column->itemColumn);
	drawArgs.style = TreeItemColumn_GetStyle(tree, column->itemColumn);
	if (lock == COLUMN_LOCK_NONE && visIndex == 0)
	    drawArgs.indent = tree->canvasPadX[PAD_TOP_LEFT];
	else
	    drawArgs.indent = 0;
	drawArgs.x = 0;
	drawArgs.y = 0;
	drawArgs.width = width,
	drawArgs.height = height;
	drawArgs.justify = column->justify;
	drawArgs.column = Tree_FindColumn(tree, TreeItemColumn_Index(tree, item, column->itemColumn));
	TreeStyle_Draw(&drawArgs);
    }

    /* Pixmap -> XImage */
    ximage = XGetImage(tree->display, td.drawable, 0, 0,
	    (unsigned int)width, (unsigned int)height, AllPlanes, ZPixmap);
    if (ximage == NULL)
	panic("tkTreeColumn.c:SetImageForColumn() ximage is NULL");

    /* XImage -> Tk_Image */
    Tree_XImage2Photo(tree->interp, photoH, ximage, 0, tree->columnDrag.alpha);

    XDestroyImage(ximage);
    Tk_FreePixmap(tree->display, td.drawable);

    return Tk_GetImage(tree->interp, tree->tkwin, "::TreeCtrl::ImageColumn",
	NULL, (ClientData) NULL);
}

void
TreeHeader_DrawDragImagery(
    TreeHeader header,		/* Header token. */
    int lock,			/* COLUMN_LOCK_XXX */
    TreeDrawable td,		/* Where to draw. */
    int x, int y,		/* Item bbox. */
    int width, int height	/* ^ */
    )
{
    TreeCtrl *tree = header->tree;
    TreeRectangle bbox;
    Tk_Image image;
    TreeItemColumn itemColumn;
    TreeHeaderColumn column;

    if (tree->columnDrag.header != header)
	return;

    if (tree->columnDrag.column == NULL)
	return;

    if (lock != TreeColumn_Lock(tree->columnDrag.column))
	return;

    if (TreeItem_GetRects(tree, header->item, tree->columnDrag.column,
	    0, NULL, &bbox) == 1) {
	int ix = 0, iy = 0, iw = bbox.width, ih = bbox.height;

	/* Erase the area to be occupied by the dragged column. */
	if (tree->columnDrag.indColumn != NULL) {
	    int index1 = TreeColumn_Index(tree->columnDrag.column);
	    int index2 = TreeColumn_Index(tree->columnDrag.indColumn);
	    TreeRectangle bbox2;
	    if (TreeItem_GetRects(tree, header->item, tree->columnDrag.indColumn,
		    0, NULL, &bbox2) == 1) {
		GC gc = Tk_3DBorderGC(tree->tkwin, tree->border, TK_3D_FLAT_GC);
		if (index1 > index2)
		    bbox.x = bbox2.x;
		else
		    bbox.x = bbox2.x + bbox2.width - bbox.width;
		bbox.x -= tree->drawableXOrigin;
		bbox.y = y;
		Tree_FillRectangle(tree, td, NULL, gc, bbox);
	    }
	}

	itemColumn = TreeItem_FindColumn(tree, header->item, TreeColumn_Index(tree->columnDrag.column));
	column = TreeItemColumn_GetHeaderColumn(tree, itemColumn);
	image = SetImageForColumn(header, column, TreeColumn_Lock(tree->columnDrag.column), iw, ih);
	x += tree->columnDrag.offset;
	Tree_RedrawImage(image, ix, iy, iw, ih, td, x + TreeColumn_Offset(tree->columnDrag.column), y);
	Tk_FreeImage(image);
    }
}

Tk_Justify
TreeHeaderColumn_Justify(
    TreeHeader header,		/* Header token. */
    TreeHeaderColumn column	/* Column token. */
    )
{
    return column->justify;
}

static int
Header_Configure(
    TreeHeader header,
    int objc,			/* Number of arguments. */
    Tcl_Obj *CONST objv[],	/* Argument values. */
    int createFlag		/* TRUE if the Column is being created. */
    )
{
    TreeCtrl *tree = header->tree;
    Tk_SavedOptions savedOptions;
    int error;
    Tcl_Obj *errorResult = NULL;
    int mask;
    int objC = 0, iObjC = 0;
    Tcl_Obj *staticObjV[STATIC_SIZE], **objV = staticObjV;
    Tcl_Obj *staticIObjV[STATIC_SIZE], **iObjV = staticIObjV;
    int i, oldVisible = TreeItem_ReallyVisible(tree, header->item);
    int ownerDrawn = header->ownerDrawn;

    /* Hack -- Pass some options to the underlying item. */
    STATIC_ALLOC(objV, Tcl_Obj *, objc);
    STATIC_ALLOC(iObjV, Tcl_Obj *, objc);
    for (i = 0; i < objc; i += 2) {
	Tk_OptionSpec *specPtr = headerSpecs;
	int length;
	CONST char *optionName = Tcl_GetStringFromObj(objv[i], &length);
	while (specPtr->type != TK_OPTION_END) {
	    if (strncmp(specPtr->optionName, optionName, length) == 0) {
		objV[objC++] = objv[i];
		if (i + 1 < objc)
		    objV[objC++] = objv[i + 1];
		break;
	    }
	    specPtr++;
	}
	if (specPtr->type == TK_OPTION_END) {
	    iObjV[iObjC++] = objv[i];
	    if (i + 1 < objc)
		iObjV[iObjC++] = objv[i + 1];
	}
    }
    if (TreeItem_ConsumeHeaderConfig(tree, header->item, iObjC, iObjV) != TCL_OK) {
	STATIC_FREE(objV, Tcl_Obj *, objc);
	STATIC_FREE(iObjV, Tcl_Obj *, objc);
	return TCL_ERROR;
    }

    for (error = 0; error <= 1; error++) {
	if (error == 0) {
	    if (Tk_SetOptions(tree->interp, (char *) header,
			tree->headerOptionTable, objC, objV, tree->tkwin,
			&savedOptions, &mask) != TCL_OK) {
		mask = 0;
		continue;
	    }

	    /* Wouldn't have to do this if Tk_InitOptions() would return
	     * a mask of configured options like Tk_SetOptions() does. */
	    if (createFlag) {
	    }

	    /*
	     * Step 1: Save old values
	     */

	    /*
	     * Step 2: Process new values
	     */

	    /*
	     * Step 3: Free saved values
	     */

	    Tk_FreeSavedOptions(&savedOptions);

	    STATIC_FREE(objV, Tcl_Obj *, objc);
	    STATIC_FREE(iObjV, Tcl_Obj *, objc);
	    break;
	} else {
	    errorResult = Tcl_GetObjResult(tree->interp);
	    Tcl_IncrRefCount(errorResult);
	    Tk_RestoreSavedOptions(&savedOptions);

	    /*
	     * Free new values.
	     */

	    /*
	     * Restore old values.
	     */

	    Tcl_SetObjResult(tree->interp, errorResult);
	    Tcl_DecrRefCount(errorResult);

	    STATIC_FREE(objV, Tcl_Obj *, objc);
	    STATIC_FREE(iObjV, Tcl_Obj *, objc);
	    return TCL_ERROR;
	}
    }

    if ((oldVisible != TreeItem_ReallyVisible(tree, header->item)) ||
	    (ownerDrawn != header->ownerDrawn)) {
	tree->headerHeight = -1;
	Tree_InvalidateColumnWidth(tree, NULL);
	Tree_DInfoChanged(tree, DINFO_DRAW_HEADER);
    }

    return TCL_OK;
}

TreeHeaderColumn
TreeHeaderColumn_CreateWithItemColumn(
    TreeHeader header,
    TreeItemColumn itemColumn
    )
{
    TreeCtrl *tree = header->tree;
    TreeHeaderColumn column;

    column = (TreeHeaderColumn) ckalloc(sizeof(HeaderColumn));
    memset(column, '\0', sizeof(HeaderColumn));
    if (Tk_InitOptions(tree->interp, (char *) column,
	    tree->headerColumnOptionTable, tree->tkwin) != TCL_OK) {
	WFREE(column, HeaderColumn);
	return NULL;
    }
    column->itemColumn = itemColumn;
    column->neededWidth = column->neededHeight = -1;
tree->headerHeight = -1;
    return column;
}

TreeHeader
TreeHeader_CreateWithItem(
    TreeCtrl *tree,
    TreeItem item
    )
{
    TreeHeader header;

    header = (TreeHeader) ckalloc(sizeof(TreeHeader_));
    memset(header, '\0', sizeof(TreeHeader_));
    if (Tk_InitOptions(tree->interp, (char *) header,
	    tree->headerOptionTable, tree->tkwin) != TCL_OK) {
	WFREE(header, TreeHeader_);
	return NULL;
    }
    header->tree = tree;
    header->item = item;
    return header;
}

void
TreeHeaderColumn_FreeResources(
    TreeCtrl *tree,
    TreeHeaderColumn column
    )
{
    if (column->bitmapGC != None)
	Tk_FreeGC(tree->display, column->bitmapGC);
    if (column->image != NULL)
	Tree_FreeImage(tree, column->image);
    if (column->textLayout != NULL)
	TextLayout_Free(column->textLayout);

    Tk_FreeConfigOptions((char *) column, tree->headerColumnOptionTable, tree->tkwin);
    WFREE(column, HeaderColumn);
}

void
TreeHeader_FreeResources(
    TreeHeader header
    )
{
    TreeCtrl *tree = header->tree;

    Tk_FreeConfigOptions((char *) header, tree->headerOptionTable, tree->tkwin);
    WFREE(header, TreeHeader_);
}

int
TreeHeader_NeededHeight(
    TreeHeader header
    )
{
    TreeCtrl *tree = header->tree;
    TreeItemColumn itemColumn;
    int maxHeight = 0;

    if (header->ownerDrawn)
	return 0;

    for (itemColumn = TreeItem_GetFirstColumn(tree, header->item);
	    itemColumn != NULL;
	    itemColumn = TreeItemColumn_GetNext(tree, itemColumn)) {
	maxHeight = MAX(maxHeight, TreeHeaderColumn_NeededHeight(header,
	    TreeItemColumn_GetHeaderColumn(tree, itemColumn)));
    }
    return maxHeight;
}

/*
 *----------------------------------------------------------------------
 *
 * Tree_HeightOfHeaderItems --
 *
 *	Returns the height of the header items.
 *
 * Results:
 *	Pixel height.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Tree_HeightOfHeaderItems(
    TreeCtrl *tree		/* Widget info. */
    )
{
    TreeItem item = tree->headerItems;
    int totalHeight = 0;

    while (item != NULL) {
	totalHeight += TreeItem_Height(tree, item);
	item = TreeItem_GetNextSibling(tree, item);
    }

    return totalHeight;
}

TreeItem
Tree_HeaderUnderPoint(
    TreeCtrl *tree,		/* Widget info. */
    int *x_, int *y_,		/* In: window coordinates.
				 * Out: coordinates relative to top-left
				 * corner of the returned column. */
    int *lock			/* Returned COLUMN_LOCK_XXX. */
    )
{
    int y;
    TreeItem item;

    if (Tree_HitTest(tree, *x_, *y_) != TREE_AREA_HEADER)
	return NULL;

    y = Tree_BorderTop(tree);
    item = tree->headerItems;
    if (!TreeItem_ReallyVisible(tree, item))
	item = TreeItem_NextSiblingVisible(tree, item);
    while (item != NULL) {
	if (*y_ < y + TreeItem_Height(tree, item)) {
	    if (*x_ < Tree_ContentLeft(tree)) {
		(*x_) -= Tree_BorderLeft(tree);
		(*lock) = COLUMN_LOCK_LEFT;
	    } else if (*x_ >= Tree_ContentRight(tree)) {
		(*x_) -= Tree_ContentRight(tree);
		(*lock) = COLUMN_LOCK_RIGHT;
	    } else {
		(*x_) += tree->xOrigin /*- tree->canvasPadX[PAD_TOP_LEFT]*/;
		(*lock) = COLUMN_LOCK_NONE;
	    }
	    (*y_) = (*y_) - y;
	    return item;
	}
	y += TreeItem_Height(tree, item);
	item = TreeItem_NextSiblingVisible(tree, item);
    }
    return NULL;
}

int
TreeHeaderColumn_FromObj(
    TreeHeader header,		/* Header token. */
    Tcl_Obj *objPtr,		/* Object to parse to a column. */
    TreeHeaderColumn *columnPtr	/* Returned column. */
    )
{
    TreeCtrl *tree = header->tree;
    TreeColumn treeColumn;
    TreeItemColumn itemColumn;

    if (TreeColumn_FromObj(tree, objPtr, &treeColumn, CFO_NOT_NULL) != TCL_OK)
	return TCL_ERROR;
    itemColumn = TreeItem_FindColumn(tree, header->item,
	TreeColumn_Index(treeColumn));
    (*columnPtr) = TreeItemColumn_GetHeaderColumn(tree, itemColumn);
    return TCL_OK;
}

int
TreeHeaderList_FromObj(
    TreeCtrl *tree,		/* Widget info. */
    Tcl_Obj *objPtr,		/* Object to parse to a header. */
    TreeItemList *items,	/* Uninitialized item list. Caller must free
				 * it with TreeItemList_Free unless the
				 * result of this function is TCL_ERROR. */
    int flags			/* IFO_xxx flags */
    )
{
    Tcl_Interp *interp = tree->interp;
    static CONST char *indexName[] = {
	"all", "end", "first", "last", (char *) NULL
    };
    enum indexEnum {
	INDEX_ALL, INDEX_END, INDEX_FIRST, INDEX_LAST
    };
    int id, index, listIndex, objc;
    Tcl_Obj **objv, *elemPtr;
    TreeItem item = NULL;

    TreeItemList_Init(tree, items, 0);

    if (Tcl_ListObjGetElements(NULL, objPtr, &objc, &objv) != TCL_OK)
	goto baditem;
    if (objc == 0)
	goto baditem;

    listIndex = 0;
    elemPtr = objv[listIndex];
    if (Tcl_GetIndexFromObj(NULL, elemPtr, indexName, NULL, 0, &index)
	    == TCL_OK) {
	switch ((enum indexEnum) index) {
	    case INDEX_ALL: {
		item = tree->headerItems;
		while (item != NULL) {
		    TreeItemList_Append(items, item);
		    item = TreeItem_GetNextSibling(tree, item);
		}
		item = NULL;
		break;
	    }
	    case INDEX_FIRST: {
		item = tree->headerItems;
		break;
	    }
	    case INDEX_END:
	    case INDEX_LAST: {
		item = tree->headerItems;
		while (item != NULL && TreeItem_GetNextSibling(tree, item) != NULL) {
		    item = TreeItem_GetNextSibling(tree, item);
		}
		break;
	    }
	}

    /* No indexName[] was found. */
    } else {
	int gotId = FALSE;

	/* Try an item ID. */
	if (Tcl_GetIntFromObj(NULL, elemPtr, &id) == TCL_OK)
	    gotId = TRUE;

	if (gotId) {
	    item = tree->headerItems;
	    while (item != NULL) {
		if (TreeItem_GetID(tree, item) == id)
		    break;
		item = TreeItem_GetNextSibling(tree, item);
	    }
	    goto gotFirstPart;
	}

	/* Try a tag or tag expression. */
	if (tree->itemTagExpr) { /* FIXME: headerTagExpr */
	    TagExpr expr;
	    if (TagExpr_Init(tree, elemPtr, &expr) != TCL_OK)
		goto errorExit;
	    item = tree->headerItems;
	    while (item != NULL) {
		if (TagExpr_Eval(&expr, TreeItem_GetTagInfo(tree, item)) /*&& Qualifies(&q, item)*/) {
		    TreeItemList_Append(items, item);
		}
		item = TreeItem_GetNextSibling(tree, item);
	    }
	    TagExpr_Free(&expr);
	} else {
	    Tk_Uid tag = Tk_GetUid(Tcl_GetString(elemPtr));
	    item = tree->headerItems;
	    while (item != NULL) {
		if (TreeItem_HasTag(item, tag) /*&& Qualifies(&q, item)*/) {
		    TreeItemList_Append(items, item);
		}
		item = TreeItem_GetNextSibling(tree, item);
	    }
	}
	item = NULL;
    }

gotFirstPart:

    /* This means a valid specification was given, but there is no such item */
    if ((TreeItemList_Count(items) == 0) && (item == NULL)) {
	if (flags & IFO_NOT_NULL)
	    goto noitem;
	/* Empty list returned */
	goto goodExit;
    }

    if ((flags & IFO_NOT_MANY) && (TreeItemList_Count(items) > 1)) {
	FormatResult(interp, "can't specify > 1 header for this command");
	goto errorExit;
    }

    TreeItemList_Append(items, item);

goodExit:
    return TCL_OK;

baditem:
    Tcl_AppendResult(interp, "bad header description \"", Tcl_GetString(objPtr),
	    "\"", NULL);
    goto errorExit;

noitem:
    Tcl_AppendResult(interp, "header \"", Tcl_GetString(objPtr),
	    "\" doesn't exist", NULL);

errorExit:
    TreeItemList_Free(items);
    return TCL_ERROR;
}

int
TreeHeader_FromObj(
    TreeCtrl *tree,		/* Widget info. */
    Tcl_Obj *objPtr,		/* Object to parse to a header. */
    TreeHeader *headerPtr
    )
{
    TreeItemList items;
    TreeItem item;

    if (TreeHeaderList_FromObj(tree, objPtr, &items, IFO_NOT_MANY | IFO_NOT_NULL) != TCL_OK)
	return TCL_ERROR;
    item = TreeItemList_Nth(&items, 0);
    (*headerPtr) = TreeItem_GetHeader(tree, item);
    TreeItemList_Free(&items);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeHeader_ToObj --
 *
 *	Convert a TreeHeader to a Tcl_Obj.
 *
 * Results:
 *	A new Tcl_Obj representing the TreeHeader.
 *
 * Side effects:
 *	Memory is allocated.
 *
 *----------------------------------------------------------------------
 */

Tcl_Obj *
TreeHeader_ToObj(
    TreeHeader header		/* Header token. */
    )
{
    TreeCtrl *tree = header->tree;
#if 0
    if (tree->itemPrefixLen) {
	char buf[100 + TCL_INTEGER_SPACE];
	(void) sprintf(buf, "%s%d", tree->itemPrefix, item->id);
	return Tcl_NewStringObj(buf, -1);
    }
#endif
    return Tcl_NewIntObj(TreeItem_GetID(tree, header->item));
}

static int
TreeHeaderCmd_Create(
    ClientData clientData,	/* Widget info. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *CONST objv[]	/* Argument values. */
    )
{
    TreeCtrl *tree = clientData;
    TreeItem item;
    TreeHeader header;

    item = TreeItem_CreateHeader(tree);
    header = TreeItem_GetHeader(tree, item);
    if (Header_Configure(header, objc - 3, objv + 3, TRUE) != TCL_OK) {
	TreeItem_Delete(tree, item);
	return TCL_ERROR;
    }
    tree->headerHeight = -1;
    Tree_DInfoChanged(tree, DINFO_DRAW_HEADER);
    Tcl_SetObjResult(interp, TreeItem_ToObj(tree, item));
    return TCL_OK;
}

static int
TreeHeaderCmd_Cget(
    ClientData clientData,	/* Widget info. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *CONST objv[]	/* Argument values. */
    )
{
    TreeCtrl *tree = clientData;
    TreeHeader header;
    TreeHeaderColumn column;
    Tcl_Obj *resultObjPtr;

    if (objc < 5 || objc > 6) {
	Tcl_WrongNumArgs(interp, 3, objv, "header ?column? option");
	return TCL_ERROR;
    }

    if (TreeHeader_FromObj(tree, objv[3], &header) != TCL_OK)
	return TCL_ERROR;

    /* T header cget H option */
    if (objc == 5) {
	{
	    Tk_OptionSpec *specPtr = headerSpecs;
	    int length;
	    CONST char *optionName = Tcl_GetStringFromObj(objv[4], &length);
	    while (specPtr->type != TK_OPTION_END) {
		if (strncmp(specPtr->optionName, optionName, length) == 0) {
		    break;
		}
		specPtr++;
	    }
	    if (specPtr->type == TK_OPTION_END) {
		return TreeItem_ConsumeHeaderCget(tree, header->item, objv[4]);
	    }
	}
	resultObjPtr = Tk_GetOptionValue(interp, (char *) header,
		tree->headerOptionTable, objv[4], tree->tkwin);
	if (resultObjPtr == NULL)
	    return TCL_ERROR;
	Tcl_SetObjResult(interp, resultObjPtr);
    } else {
	/* T header cget H C option */
	if (TreeHeaderColumn_FromObj(header, objv[4], &column) != TCL_OK)
	    return TCL_ERROR;
	resultObjPtr = Tk_GetOptionValue(interp, (char *) column,
	    tree->headerColumnOptionTable, objv[5], tree->tkwin);
	if (resultObjPtr == NULL)
	    return TCL_ERROR;
	Tcl_SetObjResult(interp, resultObjPtr);
    }

    return TCL_OK;
}

static int
TreeHeaderCmd_Configure(
    ClientData clientData,	/* Widget info. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *CONST objv[]	/* Argument values. */
    )
{
    TreeCtrl *tree = clientData;
    TreeHeader header;
    TreeHeaderColumn column;
    Tcl_Obj *resultObjPtr;
    CONST char *s;
    TreeItem item;
    TreeItemList items;
    ItemForEach iter;
    TreeColumnList columns;
    TreeColumn treeColumn;
    ColumnForEach citer;

    if (objc < 4) {
	Tcl_WrongNumArgs(interp, 3, objv, "header ?column? ?option? ?value? ?option value ...?");
	return TCL_ERROR;
    }

    /* T header configure H */
    if (objc == 4) {
	if (TreeHeader_FromObj(tree, objv[3], &header) != TCL_OK)
	    return TCL_ERROR;
	resultObjPtr = Tk_GetOptionInfo(interp, (char *) header,
	    tree->headerOptionTable,(Tcl_Obj *) NULL, tree->tkwin);
	if (resultObjPtr == NULL)
	    return TCL_ERROR;
	Tcl_SetObjResult(interp, resultObjPtr);
    } else {
	s = Tcl_GetString(objv[4]);
	if (s[0] == '-') {

	    /* T header configure H -option */
	    if (objc == 5) {
		if (TreeHeader_FromObj(tree, objv[3], &header) != TCL_OK)
		    return TCL_ERROR;
		resultObjPtr = Tk_GetOptionInfo(interp, (char *) header,
		    tree->headerOptionTable, objv[4], tree->tkwin);
		if (resultObjPtr == NULL)
		    return TCL_ERROR;
		Tcl_SetObjResult(interp, resultObjPtr);

	    /* T header configure H -option value ... */
	    } else {
		if (TreeHeaderList_FromObj(tree, objv[3], &items, 0) != TCL_OK)
		    return TCL_ERROR;
		ITEM_FOR_EACH(item, &items, NULL, &iter) {
		    header = TreeItem_GetHeader(tree, item);
		    if (Header_Configure(header, objc - 4, objv + 4, FALSE) != TCL_OK) {
			TreeItemList_Free(&items);
			return TCL_ERROR;
		    }
		}
		TreeItemList_Free(&items);
	    }
	} else {

	    /* T header configure H C ?-option? */
	    if (objc <= 6) {
		if (TreeHeader_FromObj(tree, objv[3], &header) != TCL_OK)
		    return TCL_ERROR;
		if (TreeHeaderColumn_FromObj(header, objv[4], &column) != TCL_OK)
		    return TCL_ERROR;
		resultObjPtr = Tk_GetOptionInfo(interp, (char *) column,
		    tree->headerColumnOptionTable,
		    (objc == 5) ? (Tcl_Obj *) NULL : objv[5], tree->tkwin);
		if (resultObjPtr == NULL)
		    return TCL_ERROR;
		Tcl_SetObjResult(interp, resultObjPtr);

	    /* T header configure H C -option value ... */
	    } else {
		if (TreeHeaderList_FromObj(tree, objv[3], &items, 0) != TCL_OK)
		    return TCL_ERROR;
		if (TreeColumnList_FromObj(tree, objv[4], &columns, CFO_NOT_TAIL) != TCL_OK)
		    return TCL_ERROR;
		ITEM_FOR_EACH(item, &items, NULL, &iter) {
		    header = TreeItem_GetHeader(tree, item);
		    COLUMN_FOR_EACH(treeColumn, &columns, NULL, &citer) {
			TreeItemColumn itemColumn = TreeItem_FindColumn(tree, item, TreeColumn_Index(treeColumn));
			column = TreeItemColumn_GetHeaderColumn(tree, itemColumn);
			if (Column_Configure(header, column, treeColumn, objc - 5, objv + 5, FALSE) != TCL_OK) {
			    TreeItemList_Free(&items);
			    TreeColumnList_Free(&columns);
			    return TCL_ERROR;
			}
		    }
		}
		TreeItemList_Free(&items);
		TreeColumnList_Free(&columns);
	    }
	}
    }

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeHeaderCmd --
 *
 *	This procedure is invoked to process the [header] widget
 *	command.  See the user documentation for details on what it
 *	does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

int
TreeHeaderCmd(
    ClientData clientData,	/* Widget info. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int objc,			/* Number of arguments. */
    Tcl_Obj *CONST objv[]	/* Argument values. */
    )
{
    TreeCtrl *tree = clientData;
    static CONST char *commandNames[] = {
	"bbox", "cget", "compare", "configure", "count", "create", "delete",
	"dragcget", "dragconfigure", "element", "id", "span",
	"state", "style", "tag", (char *) NULL
    };
    enum {
	COMMAND_BBOX, COMMAND_CGET, COMMAND_COMPARE, COMMAND_CONFIGURE,
	COMMAND_COUNT, COMMAND_CREATE, COMMAND_DELETE, COMMAND_DRAGCGET,
	COMMAND_DRAGCONF, COMMAND_ELEMENT, COMMAND_ID, COMMAND_SPAN,
	COMMAND_STATE, COMMAND_STYLE, COMMAND_TAG
    };
    int index;
    TreeItemList items;
    TreeItem item;
    ItemForEach iter;

    if (objc < 3) {
	Tcl_WrongNumArgs(interp, 2, objv, "command ?arg arg ...?");
	return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[2], commandNames, "command", 0,
		&index) != TCL_OK) {
	return TCL_ERROR;
    }

    /* FIXME: Tree_PreserveItems? */

    switch (index) {
	/* T header bbox I ?C? ?E? */
	case COMMAND_BBOX: {
	    TreeHeader header;
	    int count;
	    TreeColumn treeColumn;
	    TreeRectangle rect;

	    if (objc < 4 || objc > 6) {
		Tcl_WrongNumArgs(interp, 3, objv, "header ?column? ?element?");
		return TCL_ERROR;
	    }

	    if (TreeHeader_FromObj(tree, objv[3], &header) != TCL_OK)
		return TCL_ERROR;
	    item = header->item;

	    (void) Tree_GetOriginX(tree);
	    (void) Tree_GetOriginY(tree);

	    if (objc == 4) {
		/* If an item is visible but has zero height a valid bbox
		 * is returned. */
		if (Tree_ItemBbox(tree, item, COLUMN_LOCK_NONE, &rect) < 0)
		    break;
	    } else {
		if (TreeColumn_FromObj(tree, objv[4], &treeColumn,
			CFO_NOT_NULL | CFO_NOT_TAIL) != TCL_OK)
		    return TCL_ERROR;

		/* Bounds of a column. */
		if (objc == 5) {
		    objc = 0;
		    objv = NULL;

		/* Single element in a column. */
		} else {
		    objc -= 5;
		    objv += 5;
		}

		count = TreeItem_GetRects(tree, item, treeColumn,
			objc, objv, &rect);
		if (count == 0)
		    break;
		if (count == -1)
		    return TCL_ERROR;
	    }
	    /* Canvas -> window coordinates */
	    FormatResult(interp, "%d %d %d %d",
		    TreeRect_Left(rect) - tree->xOrigin,
		    TreeRect_Top(rect) - tree->yOrigin,
		    TreeRect_Left(rect) - tree->xOrigin + TreeRect_Width(rect),
		    TreeRect_Top(rect) - tree->yOrigin + TreeRect_Height(rect));
	    break;
	}

	case COMMAND_CREATE:
	    return TreeHeaderCmd_Create(clientData, interp, objc, objv);

	/* T header cget H ?C? option */
	case COMMAND_CGET:
	    return TreeHeaderCmd_Cget(clientData, interp, objc, objv);

	/* T header compare H op H */
	case COMMAND_COMPARE: {
	    static CONST char *opName[] = { "<", "<=", "==", ">=", ">", "!=", NULL };
	    enum { COP_LT, COP_LE, COP_EQ, COP_GE, COP_GT, COP_NE };
	    int op, compare = 0, index1 = 0, index2 = 0;
	    TreeHeader header1, header2;

	    if (objc != 6) {
		Tcl_WrongNumArgs(interp, 3, objv, "header1 op header2");
		return TCL_ERROR;
	    }

	    if (TreeHeader_FromObj(tree, objv[3], &header1) != TCL_OK)
		return TCL_ERROR;

	    if (Tcl_GetIndexFromObj(interp, objv[4], opName, "comparison operator", 0,
		    &op) != TCL_OK) {
		return TCL_ERROR;
	    }

	    if (TreeHeader_FromObj(tree, objv[5], &header2) != TCL_OK)
		return TCL_ERROR;

	    if (op != COP_EQ && op != COP_NE) {
		for (item = tree->headerItems; item != header1->item;
			item = TreeItem_GetNextSibling(tree, item)) {
		    index1++;
		}
		for (item = tree->headerItems; item != header2->item;
			item = TreeItem_GetNextSibling(tree, item)) {
		    index2++;
		}
	    }
	    switch (op) {
		case COP_LT:
		    compare = index1 < index2;
		    break;
		case COP_LE:
		    compare = index1 <= index2;
		    break;
		case COP_EQ:
		    compare = header1 == header2;
		    break;
		case COP_GE:
		    compare = index1 >= index2;
		    break;
		case COP_GT:
		    compare = index1 > index2;
		    break;
		case COP_NE:
		    compare = header1 != header2;
		    break;
	    }
	    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(compare));
	    break;
	}

	/* T header configure H ?C? ?option? ?value? ?option value ...? */
	case COMMAND_CONFIGURE:
	    return TreeHeaderCmd_Configure(clientData, interp, objc, objv);

	/* T header count ?H? */
	case COMMAND_COUNT: {
	    int count = tree->headerCount;

	    if (objc > 4) {
		Tcl_WrongNumArgs(interp, 3, objv, "?headerDesc?");
		return TCL_ERROR;
	    }

	    if (objc == 4) {
		if (TreeHeaderList_FromObj(tree, objv[3], &items, 0) != TCL_OK)
		    return TCL_ERROR;
		count = 0;
		ITEM_FOR_EACH(item, &items, NULL, &iter) {
		    count++;
		}
		TreeItemList_Free(&items);
	    }
	    Tcl_SetObjResult(interp, Tcl_NewIntObj(count));
	    break;
	}

	case COMMAND_DELETE: {
	    if (objc != 4) {
		Tcl_WrongNumArgs(interp, 3, objv, "header");
		return TCL_ERROR;
	    }
	    if (TreeHeaderList_FromObj(tree, objv[3], &items, 0) != TCL_OK)
		return TCL_ERROR;

	    ITEM_FOR_EACH(item, &items, NULL, &iter) {
		/* The default header can't be deleted */
		if (item == tree->headerItems)
		    continue;

		if (TreeItem_ReallyVisible(tree, item)) {
		    tree->headerHeight = -1;
		    Tree_InvalidateColumnWidth(tree, NULL);
		}

		if (tree->columnDrag.header == TreeItem_GetHeader(tree, item))
		    tree->columnDrag.header = NULL;

		/* FIXME: ITEM_FLAG_DELETED */
		TreeItem_Delete(tree, item);
	    }
	    TreeItemList_Free(&items);
	    break;
	}

	case COMMAND_ELEMENT:
	    return TreeItemCmd_Element(tree, objc, objv, TRUE);

	/* T header id H */
	case COMMAND_ID: {
	    Tcl_Obj *listObj;

	    if (objc != 4) {
		Tcl_WrongNumArgs(interp, 3, objv, "header");
		return TCL_ERROR;
	    }

	    if (TreeHeaderList_FromObj(tree, objv[3], &items, 0) != TCL_OK)
		return TCL_ERROR;

	    listObj = Tcl_NewListObj(0, NULL);
	    ITEM_FOR_EACH(item, &items, NULL, &iter) {
		Tcl_ListObjAppendElement(interp, listObj,
			TreeItem_ToObj(tree, item));
	    }
	    TreeItemList_Free(&items);
	    Tcl_SetObjResult(interp, listObj);
	    break;
	}

	/* T header span H ?C? ?span? ?C span ...? */
	case COMMAND_SPAN: {
	    if (objc < 4) {
		Tcl_WrongNumArgs(interp, 3, objv, "header ?column? ?span? ?column span ...?");
		return TCL_ERROR;
	    }
	    if (TreeHeaderList_FromObj(tree, objv[3], &items, IFO_NOT_NULL) != TCL_OK)
		return TCL_ERROR;
	    if (TreeItem_ConfigureSpans(tree, &items, objc - 4, objv + 4) != TCL_OK) {
		TreeItemList_Free(&items);
		return TCL_ERROR;
	    }
	    break;
	}

	case COMMAND_STATE:
	    return TreeItemCmd_State(tree, objc, objv, TRUE);

	case COMMAND_STYLE:
	    return TreeItemCmd_Style(tree, objc, objv, TRUE);
    }

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TreeHeader_TreeChanged --
 *
 *	Called when a TreeCtrl is configured. Performs any relayout
 *	necessary on column headers.
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
TreeHeader_TreeChanged(
    TreeCtrl *tree,		/* Widget info. */
    int flagT			/* TREE_CONF_xxx flags. */
    )
{
    TreeItem item = tree->headerItems;
    TreeItemColumn itemColumn;
    TreeHeaderColumn column;

    if (!(flagT & (TREE_CONF_FONT | TREE_CONF_RELAYOUT)))
	return;

    while (item != NULL) {
	itemColumn = TreeItem_GetFirstColumn(tree, item);
	while (itemColumn != NULL) {
	    column = TreeItemColumn_GetHeaderColumn(tree, itemColumn);
	    if (column == NULL)
		panic("TreeHeader_TreeChanged: item-column is missing its associated header-column");
	    if ((flagT & TREE_CONF_FONT) && (column->tkfont == NULL) &&
		    (column->textLen > 0)) {
		column->textWidth = Tk_TextWidth(tree->tkfont, column->text,
		    column->textLen);
		column->neededWidth = column->neededHeight = -1;
		column->textLayoutInvalid = TRUE;
	    }
	    /* Need to do this if the -usetheme option changes or the system
	    * theme changes. */
	    if (flagT & TREE_CONF_RELAYOUT) {
		column->neededWidth = column->neededHeight = -1;
	    }
	    itemColumn = TreeItemColumn_GetNext(tree, itemColumn);
	}
	item = TreeItem_GetNextSibling(tree, item);
    }

    if (flagT & TREE_CONF_FONT)
	tree->headerHeight = -1;
}

int
TreeHeader_Init(
    TreeCtrl *tree		/* Widget info. */
    )
{
    Tk_OptionSpec *specPtr;
    Tcl_DString dString;

    Tcl_InitHashTable(&tree->headerHash, TCL_ONE_WORD_KEYS);

    specPtr = Tree_FindOptionSpec(columnSpecs, "-background");
    if (specPtr->defValue == NULL) {
	Tcl_DStringInit(&dString);
	Tcl_DStringAppendElement(&dString, DEF_BUTTON_BG_COLOR);
	Tcl_DStringAppendElement(&dString, "normal");
	Tcl_DStringAppendElement(&dString, DEF_BUTTON_ACTIVE_BG_COLOR);
	Tcl_DStringAppendElement(&dString, "");
	specPtr->defValue = ckalloc(Tcl_DStringLength(&dString) + 1);
	strcpy((char *)specPtr->defValue, Tcl_DStringValue(&dString));
	Tcl_DStringFree(&dString);
    }

    PerStateCO_Init(columnSpecs, "-arrowbitmap", &pstBitmap, ColumnStateFromObj);
    PerStateCO_Init(columnSpecs, "-arrowimage", &pstImage, ColumnStateFromObj);
    PerStateCO_Init(columnSpecs, "-background", &pstBorder, ColumnStateFromObj);
    PerStateCO_Init(columnSpecs, "-textcolor", &pstColor, ColumnStateFromObj);

    tree->headerOptionTable = Tk_CreateOptionTable(tree->interp, headerSpecs);
    tree->headerColumnOptionTable = Tk_CreateOptionTable(tree->interp, columnSpecs);

    /* Create the default/topmost header item.  It can't be deleted. */
    tree->headerItems = TreeItem_CreateHeader(tree);

    return TCL_OK;
}

void
TreeHeader_Free(
    TreeCtrl *tree		/* Widget info. */
    )
{
    TreeItem item;

    item = tree->headerItems;
    while (item != NULL) {
	TreeItem_FreeResources(tree, item);
	item = TreeItem_GetNextSibling(tree, item);
    }

    Tcl_DeleteHashTable(&tree->headerHash);
}
