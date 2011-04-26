namespace eval DemoHeaders {}

proc DemoHeaders {} {
    DemoHeaders::Run
    return
}

proc DemoHeaders::Run {} {
    set T [DemoList]

    $T configure \
	-showroot no -xscrollsmoothing yes -yscrollsmoothing yes \
	-selectmode multiple -xscrollincrement 20 -canvaspadx 40

    #
    # Create one locked column on each side plus 8 non-locked columns
    #

    set itembg {linen {} #e0e8f0 {}}

    $T column create -text "Left" -tags Cleft -width 80 -justify center \
	-gridrightcolor gray90 -itembackground $itembg \
	-lock left -arrow none -arrowside left \
	-visible no

    for {set i 1} {$i <= 8} {incr i} {
	$T column create -text "C$i" -tags C$i -width 80 -justify center \
	    -gridrightcolor gray90 -itembackground $itembg
    }

    $T column create -text "Right" -tags Cright -width 80 -justify center \
	-gridrightcolor gray90 -itembackground $itembg  \
	-lock right -visible no

    #
    # Create an image element to use as the sort arrow for some header
    # styles.
    #

    InitSortImages blue
    $T element create header.sort image -statedomain header \
	-image {::DemoHeaders::arrow-down down ::DemoHeaders::arrow-up up}

    #
    # Create a style for our custom headers,
    # a raised border with centered text.
    #

    $T element create header.border border -statedomain header \
	-background $::SystemButtonFace \
	-relief {sunken pressed raised {}} -thickness 2 -filled yes
    $T element create header.text text -statedomain header \
	-lines 1

    set S [$T style create header1 -orient horizontal -statedomain header]
    $T style elements $S {header.border header.text header.sort}
    $T style layout $S header.border -detach yes -indent no -iexpand xy
    $T style layout $S header.text -center xy -padx 6 -pady 2
    $T style layout $S header.sort -expand nws -padx {0 6} \
	-visible {no {!down !up}}

    #
    # Create a style for our custom headers,
    # a light-blue rounded rectangle with centered text.
    #

    $T element create header.rrect rect -statedomain header \
	-rx 9 -fill {
	    #cee8f0 active
	    #87c6da pressed
	    #87c6da up
	    #87c6da down
	    {light blue} {}
	}

    set S [$T style create header2 -orient horizontal -statedomain header]
    $T style elements $S {header.rrect header.text header.sort}
    $T style layout $S header.rrect -detach yes -iexpand xy -padx {1 0} -pady 1
    $T style layout $S header.text -center xy -padx 6 -pady 4
    $T style layout $S header.sort -expand nws -padx {0 6} \
	-visible {no {!down !up}}

if 0 {
    #
    # Create a style for our custom headers,
    # a header element with centered text.
    #

    $T element create header.header header -thickness 1

    set S [$T style create header3 -orient horizontal -statedomain header]
    $T style elements $S {header.header header.text}
    $T style layout $S header.header -union header.text -iexpand news
    $T style layout $S header.text -expand wens -padx 6 -pady 2 -squeeze x ; # $T style layout $S header.text -expand ns -center x -padx 6 -pady 2 -squeeze x
}

    #
    # Create a style for our custom headers,
    # a header element with a checkbox image and centered text.
    #

    InitPics *checked

    $T header state define CHECK
    $T element create header.header header -statedomain header
    $T element create header.check image -statedomain header \
	-image {checked CHECK unchecked {}}
    set S [$T style create header4 -statedomain header]
    $T style elements $S {header.header header.check header.text}
    $T style layout $S header.header -union {header.check header.text} -iexpand news
    $T style layout $S header.check -expand nes -padx {6 0}
    $T style layout $S header.text -center xy -padx 6 -squeeze x

    #
    # Create a style for our custom headers,
    # a gradient-filled rectangle with centered text.
    #

    $T gradient create Gnormal -orient vertical -stops {{0.0 white} {0.5 gray87} {1.0 white}} -steps 6
    $T gradient create Gactive -orient vertical -stops {{0.0 white} {0.5 gray90} {1.0 white}} -steps 6
    $T gradient create Gpressed -orient vertical -stops {{0.0 white} {0.5 gray82} {1.0 white}} -steps 6
    $T gradient create Gsorted -orient vertical -stops {{0.0 white} {0.5 {sky blue}} {1.0 white}} -steps 6
    $T gradient create Gactive_sorted -orient vertical -stops {{0.0 white} {0.5 {light blue}} {1.0 white}} -steps 6
    $T gradient create Gpressed_sorted -orient vertical -stops {{0.0 white} {0.5 {sky blue}} {1.0 white}} -steps 6
    $T element create header.rect1 rect  -statedomain header \
    -fill {
	Gactive_sorted {active up}
	Gpressed_sorted {pressed up}
	Gactive_sorted {active down}
	Gpressed_sorted {pressed down}
	Gsorted up
	Gsorted down
	Gactive active
	Gpressed pressed
	Gnormal {}
    } -outline {
	{sky blue} up
	{sky blue} down
	gray {}
    } -outlinewidth 1 -open {
	nw !pressed
    }

    set S [$T style create header5 -orient horizontal -statedomain header]
    $T style elements $S {header.rect1 header.text header.sort}
    $T style layout $S header.rect1 -detach yes -iexpand xy
    $T style layout $S header.text -center xy -padx 6 -pady 2 -squeeze x
    $T style layout $S header.sort -expand nws -padx {0 6} \
	-visible {no {!down !up}}

    #
    # Create a style for our custom headers,
    # a gradient-filled rectangle with centered text.
    #

    $T gradient create G_orange1 -orient vertical -steps 4 \
	-stops {{0 #fde8d1} {0.3 #fde8d1} {0.3 #ffce69} {0.6 #ffce69} {1 #fff3c3}}
    $T gradient create G_orange2 -orient vertical -steps 4 \
	-stops {{0 #fffef6} {0.3 #fffef6} {0.3 #ffef9a} {0.6 #ffef9a} {1 #fffce8}}

    $T element create orange.outline rect -statedomain header \
	-outline #ffb700 -outlinewidth 1 \
	-rx 1 -open {
	    nw !pressed
	}
    $T element create orange.box rect -statedomain header \
	-fill {
	    G_orange1 active
	    G_orange1 up
	    G_orange1 down
	    G_orange2 {}
	}

    set S [$T style create header6 -orient horizontal -statedomain header]
    $T style elements $S {orange.outline orange.box header.text header.sort}
    $T style layout $S orange.outline -union orange.box -ipadx 2 -ipady 2
    $T style layout $S orange.box -detach yes -iexpand xy
    $T style layout $S header.text -center xy -padx 6 -pady 4 -squeeze x
    $T style layout $S header.sort -expand nws -padx {0 6} \
	-visible {no {!down !up}}

    #
    # Configure 3 rows of column headers
    #

    set S header2

    $T header configure first -ownerdrawn yes -tags header1
    set H header1
    $T header configure $H all -arrowgravity right -justify center
    $T header style set $H all $S
    $T header span $H all 4
    foreach {C text} [list Cleft Left C1 A C5 H Cright Right] {
	$T header configure $H $C -text $text
	$T header text $H $C $text
    }

    set H [$T header create -ownerdrawn yes -tags header2]
    $T header configure $H all -arrowgravity right -justify center
    $T header style set $H all $S
    $T header span $H all 2
    foreach {C text} [list Cleft Left C1 B C3 C C5 I C7 J Cright Right] {
	$T header configure $H $C -text $text
	$T header text $H $C $text
    }

    set H [$T header create -ownerdrawn yes -tags header3]
    $T header configure $H all -arrowgravity right -justify center
    $T header style set $H all $S
    foreach {C text} [list Cleft Left C1 D C2 E C3 F C4 G C5 K C6 L C7 M C8 N Cright Right] {
	$T header configure $H $C -text $text
	$T header text $H $C $text
    }

    #
    # Create a 4th row of column headers to test embedded windows.
    #

    $T element create header.window window -statedomain header -clip yes
    $T element create header.divider rect -statedomain header -fill gray -height 2

    set S [$T style create headerWin -orient horizontal -statedomain header]
    $T style elements $S {header.divider header.window}
    $T style layout $S header.divider -detach yes -expand n -iexpand x
    $T style layout $S header.window -iexpand x -squeeze x -padx 1 -pady {0 2}

    set H [$T header create -ownerdrawn yes -tags header4]
    $T header dragconfigure $H -enable no
    $T header style set $H all $S
    foreach C [$T column list] {
        set f [frame $T.frame${H}_$C -borderwidth 0]
	set w [entry $f.entry -highlightthickness 1]
	$w insert end $C
	$T header element configure $H $C header.window -window $f
    }

    #
    #
    #

    scan [$T column bbox {first lock none}] "%d %d %d %d" left top right bottom
    scan [$T column bbox {last lock none}] "%d %d %d %d" left2 top2 right2 bottom2
    set width [expr {$right2 - $left}]

    $T item state define current

    $T element create theme.rect rect \
	-fill {{light blue} current white {}} \
	-outline gray50 -outlinewidth 2 -open s
    $T element create theme.text text \
	-lines 0 -width $width
    $T element create theme.button window -clip yes
    set S [$T style create theme -orient vertical]
    $T style elements $S {theme.rect theme.text theme.button}
    $T style layout $S theme.rect -detach yes -iexpand xy
    $T style layout $S theme.text -padx 4 -pady 3
    $T style layout $S theme.button -expand we -pady {3 6}

    NewButtonItem "" \
	"Use no style, just the built-in header background, sort arrow and text.\nstyle=none, -ownerdrawn=no" \
	no
    NewButtonItem header1 \
	"Use the 'header1' style, consisting of a border element for the background and an image for the sort arrow.\nstyle=header1, -ownerdrawn=yes" \
	yes black
    NewButtonItem header2 \
	"Use the 'header2' style, consisting of a rounded rectangle element for the background and an image for the sort arrow.\nstyle=header2, -ownerdrawn=yes" \
	yes blue
    NewButtonItem header4 \
	"Use the 'header4' style, consisting only of an image element serving as a checkbutton.  The style is drawn overtop the built-in parts of the header.\nstyle=header4, -ownerdrawn=no" \
	no
    NewButtonItem header5 \
	"Use the 'header5' style, consisting .\nstyle=header5, -ownerdrawn=yes" \
	yes #0080FF
    NewButtonItem header6 \
	"Use the 'header6' style, consisting .\nstyle=header6, -ownerdrawn=yes" \
	yes orange

    $T item state set styleheader2 current

    #
    # Create 100 regular non-locked items
    #

    $T element create item.sel rect \
	-fill {gray {selected !focus} blue selected}
    $T element create item.text text \
	-text "Item" -fill {white selected}

    set S [$T style create item]
    $T style elements $S {item.sel item.text}
    $T style layout $S item.sel -detach yes -iexpand xy
    $T style layout $S item.text -expand news -padx 2 -pady 2

    $T column configure !tail -itemstyle $S
    $T item create -count 100 -parent root

    #
    # Set binding scripts to handle the <Header> dynamic event
    #

    variable Sort
    set Sort(header) ""
    set Sort(column) ""
    foreach C [$T column list] {
	set Sort(direction,$C) down
    }

    # The <Header-state> event is generated in response to Motion and
    # Button events in headers.
    $T notify install <Header-state>
    $T notify bind $T <Header-state> {
	DemoHeaders::HeaderState %H %C %s
    }

    # The <Header-invoke> event is generated when the left mouse button is
    # pressed and released over a column header.
    $T notify bind $T <Header-invoke> {
	DemoHeaders::HeaderInvoke %H %C
    }

    $T notify bind $T <ColumnDrag-begin> {
	DemoHeaders::ColumnDragBegin %H %C
    }

    $T notify configure DontDelete <ColumnDrag-receive> -active no
    $T notify bind $T <ColumnDrag-receive> {
	DemoHeaders::ColumnDragReceive %H %C %b
    }

    bindtags $T [list $T DemoHeaders TreeCtrl [winfo toplevel $T] all]
    bind DemoHeaders <ButtonPress-1> {
	DemoHeaders::ButtonPress1 %x %y
    }

    return
}

proc DemoHeaders::NewButtonItem {S text args} {
    set T [DemoList]
    set I [$T item create -parent root -tags [list style$S config]]
    $T item style set $I C1 theme
    $T item span $I all [$T column count {lock none}]
    $T item text $I C1 $text
    frame $T.frame$I -borderwidth 0
    $::buttonCmd $T.frame$I.button -text "Configure headers" \
	-command [eval list [list DemoHeaders::ChangeHeaderStyle $S] $args]
    $T item element configure $I C1 theme.button -window $T.frame$I
    return
}

proc DemoHeaders::InitSortImages {color} {
    set img ::DemoHeaders::arrow-down
    image create photo $img
    $img put [list [string repeat "$color " 9]] -to 0 0
    $img put [list [string repeat "$color " 7]] -to 1 1
    $img put [list [string repeat "$color " 5]] -to 2 2
    $img put [list [string repeat "$color " 3]] -to 3 3
    $img put [list [string repeat "$color " 1]] -to 4 4

    set img ::DemoHeaders::arrow-up
    image create photo $img
    $img put [list [string repeat "$color " 1]] -to 4 0
    $img put [list [string repeat "$color " 3]] -to 3 1
    $img put [list [string repeat "$color " 5]] -to 2 2
    $img put [list [string repeat "$color " 7]] -to 1 3
    $img put [list [string repeat "$color " 9]] -to 0 4
    return
}

proc DemoHeaders::ChangeHeaderStyle {style ownerDrawn {sortColor ""}} {
    variable HeaderStyle
    variable Sort
    set T [DemoList]
    if {$sortColor ne ""} {
	InitSortImages $sortColor
    }
    set HeaderStyle $style
    set S $HeaderStyle
    foreach H [$T header id !header4] {
	$T header style set $H all $S
	if {$S ne ""} {
	    $T header configure all all -textpadx 6
	    foreach C [$T column list] {
		$T header text $H $C [$T header cget $H $C -text]
	    }
	}
	$T header configure $H -ownerdrawn $ownerDrawn
    }
    if {$Sort(header) ne ""} {
	ShowSortArrow $Sort(header) $Sort(column)
    }
    $T item state set {state current} !current
    $T item state set style$style current
    return
}

# This procedure is called to handle the <Header-state> event generated by
# the treectrl.tcl library script.
# If the style in the given item-column contains a 'header' element, then
# that element's -state option is configured to the given state.
proc DemoHeaders::HeaderState {H C state} {
    set T [DemoList]
    set S [$T header style set $H $C]
    if {$S eq ""} return
    foreach E [$T style elements $S] {
	if {[$T element type $E] eq "header"} {
	    $T header element configure $H $C $E -state $state
	}
    }
    return
}

# This procedure is called to handle the <Header-invoke> event generated by
# the treectrl.tcl library script.
# If the given column header is already displaying a sort arrow, the sort
# arrow direction is toggled.  Otherwise the sort arrow is removed from all
# other column headers and displayed in the given column header.
proc DemoHeaders::HeaderInvoke {H C} {
    variable Sort
    set T [DemoList]
#    if {![$T item tag expr $I header3]} return
    if {$Sort(header) eq ""} {
	ShowSortArrow $H $C
    } else {
	if {[$T header compare $H == $Sort(header)] &&
		[$T column compare $C == $Sort(column)]} {
	    ToggleSortArrow $H $C
	} else {
	    HideSortArrow $Sort(header) $Sort(column)
	    ShowSortArrow $H $C
	}
    }
    set Sort(header) $H
    set Sort(column) $C
    return
}

# Sets the -arrow option of a column header to 'up' or 'down'.
# If the style in the given column header contains a 'header' element, then
# that element's -arrow option is configured to 'up' or 'down'.
proc DemoHeaders::ShowSortArrow {H C} {
    variable Sort
    set T [DemoList]
    $T header configure $H $C -arrow $Sort(direction,$C)
    set S [$T header style set $H $C]
    if {$S eq ""} return
    foreach E [$T style elements $S] {
	if {[$T element type $E] eq "header"} {
	    $T header element configure $H $C $E -arrow $Sort(direction,$C)
	}
    }
    return
}

# Sets the -arrow option of a column header to 'none'.
# If the style in the given column header contains a 'header' element, then
# that element's -arrow option is configured to 'none'.
proc DemoHeaders::HideSortArrow {H C} {
    variable Sort
    set T [DemoList]
    $T header configure $H $C -arrow none
    set S [$T header style set $H $C]
    if {$S eq ""} return
    foreach E [$T style elements $S] {
	if {[$T element type $E] eq "header"} {
	    $T header element configure $H $C $E -arrow none
	}
    }
    return
}

proc DemoHeaders::ToggleSortArrow {H C} {
    variable Sort
    if {$Sort(direction,$C) eq "up"} {
	set Sort(direction,$C) down
    } else {
	set Sort(direction,$C) up
    }
    ShowSortArrow $H $C
    return
}

# This procedure is called to handle the <ColumnDrag-begin> event generated
# by the treectrl.tcl library script.
proc DemoHeaders::ColumnDragBegin {H C} {
    set T [DemoList]
    $T header dragconfigure all -draw yes
    if {[$T header compare $H > header1]} {
	$T header dragconfigure header1 -draw no
    }
    if {[$T header compare $H > header2]} {
	$T header dragconfigure header2 -draw no
    }
    return
}

# This procedure is called to handle the <ColumnDrag-receive> event generated
# by the treectrl.tcl library script.
proc DemoHeaders::ColumnDragReceive {H C b} {
    set T [DemoList]

    # Get the range of columns in the span of the dragged header.
    set span [$T header span $H $C]
    set last [$T column id "$C span $span"]
    set columns [$T column id "range $C $last"]

    set span1 [$T header span header1]
    set span2 [$T header span header2]
    set text1 [$T header text header1]
    set text2 [$T header text header2]

    set columnLeft [$T column id "first lock none"]

    foreach C $columns {
	$T column move $C $b
    }

    if {[$T header compare $H > header1]} {
	foreach span $span1 text $text1 C [$T column list] {
	    $T header span header1 $C $span
	    $T header text header1 $C $text
	    $T header configure header1 $C -text $text
	}
    }
    if {[$T header compare $H > header2]} {
	foreach span $span2 text $text2 C [$T column list] {
	    $T header span header2 $C $span
	    $T header text header2 $C $text
	    $T header configure header2 $C -text $text
	}
    }

    # For each of the items displaying a button widget to change the header
    # style, transfer the style from the old left-most column to the new
    # left-most column.
    if {[$T column compare $columnLeft != "first lock none"]} {
	foreach I [$T item id "tag config"] {
	    TransferItemStyle $T $I $columnLeft "first lock none"
	}
    }

    return
}

proc DemoHeaders::TransferItemStyle {T I Cfrom Cto} {
    set S [$T item style set $I $Cfrom]
    $T item style set $I $Cto $S
    foreach E [$T item style elements $I $Cfrom] {
	foreach info [$T item element configure $I $Cfrom $E] {
	    lassign $info option x y z value
	    $T item element configure $I $Cto $E $option $value
	}
    }
    $T item style set $I $Cfrom ""
    return
}

proc DemoHeaders::ButtonPress1 {x y} {
    set T [DemoList]
    $T identify -array id $x $y
    if {$id(where) eq "header" && $id(element) eq "header.check"} {
	$T header state forcolumn $id(header) $id(column) ~CHECK
	return -code break
    }
    return
}

# Header options affecting layout:
# header elem options >> -arrowside -arrowgravity -arrowpadx/y -borderwidth (vertically)
# text elem options >> -textlines
# The options above don't require a new style, those below do:
# -image -bitmap -imagepadx/y -justify -textpadx/y
proc ::TreeCtrl::GetHeaderStyleParams {T H C} {
    lappend result justify [$T header cget $H $C -justify]
    if {[$T header cget $H $C -image] ne ""} {
	lappend result image 1
	lappend result bitmap 0
	lappend result imagepadx [$T header cget $H $C -imagepadx]
    } elseif {[$T header cget $H $C -bitmap] ne ""} {
	lappend result image 0
	lappend result bitmap 1
	lappend result imagepadx [$T header cget $H $C -imagepadx]
    } else {
	lappend result image 0
	lappend result bitmap 0
    }
    if {[$T header cget $H $C -text] ne ""} {
	lappend result text 1
	lappend result textpadx [$T header cget $H $C -textpadx]
    } else {
	lappend result text 0
    }
    return $result
}

proc ::TreeCtrl::CreateHeaderStyle {T H C} {
    variable HeaderStyleParams
    variable HeaderStyleId

    incr HeaderStyleId
    set S treectrl_header_style_$HeaderStyleId
    set elements {}

    set HeaderStyleParams($S) [GetHeaderStyleParams $T $H $C]
    array set params {image 0 bitmap 0 text 0}
    array set params $HeaderStyleParams($S)
if 0 {
    $T element create $S.header border -statedomain header \
	-thickness [$T header cget $H $C -borderwidth] \
	-background $::SystemButtonFace -relief raised -filled yes
} else {
    $T element create $S.header header -statedomain header \
	-arrowside [$T header cget $H $C -arrowside] \
	-arrowgravity [$T header cget $H $C -arrowgravity] \
	-arrowpadx [$T header cget $H $C -arrowpadx] \
	-arrowpady [$T header cget $H $C -arrowpady] \
	-borderwidth [$T header cget $H $C -borderwidth]
}
    lappend elements $S.header
    if {$params(image)} {
	$T element create $S.image image -statedomain header
	lappend elements $S.image
    } elseif {$params(bitmap)} {
	$T element create $S.bitmap bitmap -statedomain header
	lappend elements $S.bitmap
    }
    if {$params(text)} {
	$T element create $S.text text -statedomain header \
	    -lines [$T header cget $H $C -textlines]
	lappend elements $S.text
    }

    $T style create $S -statedomain header -orient horizontal
    $T style elements $S $elements
    if {[llength $elements] > 1} {
	$T style layout $S $S.header \
	    -union [lrange $elements 1 end] -iexpand news \
	    -ipadx 0 -ipady 0
    } else {
	$T style layout $S $S.header \
	    -iexpand xy
    }
    if {$params(image)} {
	$T style layout $S $S.image \
	    -padx [$T header cget $H $C -imagepadx] \
	    -pady [$T header cget $H $C -imagepady] \
	    -expand ns
	set imgPadRight [$T header cget $H $C -imagepadx]
    } elseif {$params(bitmap)} {
	$T style layout $S $S.bitmap \
	    -padx [$T header cget $H $C -imagepadx] \
	    -pady [$T header cget $H $C -imagepady] \
	    -expand ns
	set imgPadRight [$T header cget $H $C -imagepadx]
    } else {
	set imgPadRight 0
    }
    if {$params(text)} {
	if {[llength $imgPadRight] == 2} {
	    set imgPadRight [lindex $imgPadRight 1]
	}
	set textPadX [$T header cget $H $C -textpadx]
	if {[llength $textPadX] == 1} {
	    lappend textPadX $textPadX
	}
	set textPadLeft [lindex $textPadX 0]
	set textPadLeft [expr {$textPadLeft - $imgPadRight}]
	if {$textPadLeft < 0} {
	    set textPadLeft 0
	}
	set textPadX [lreplace $textPadX 0 0 $textPadLeft]
	$T style layout $S $S.text \
	    -padx $textPadX \
	    -pady [$T header cget $H $C -textpady] \
	    -squeeze x -expand ns
    }
    switch $params(justify) {
	left {
	}
	center {
	    if {$params(image)} {
		$T style layout $S $S.image -center x
	    } elseif {$params(bitmap)} {
		$T style layout $S $S.bitmap -center x
	    }
	    if {$params(text)} {
		$T style layout $S $S.text -center x
	    }
	}
	right {
	    if {$params(image)} {
		$T style layout $S $S.image -expand wns
	    } elseif {$params(bitmap)} {
		$T style layout $S $S.bitmap -expand wns
	    } elseif {$params(text)} {
		$T style layout $S $S.text -expand wns
	    }
	}
    }
if 0 {
    set HeaderStyleParams($S,justify) [$T header cget $H $C -justify]
    if {[$T header cget $H $C -image] ne ""} {
	set HeaderStyleParams($S,image) 1
	set HeaderStyleParams($S,bitmap) 0
	set HeaderStyleParams($S,imagepadx) [$T header cget $H $C -imagepadx]
    } elseif {[$T header cget $H $C -bitmap] ne ""} {
	set HeaderStyleParams($S,image) 0
	set HeaderStyleParams($S,bitmap) 1
	set HeaderStyleParams($S,imagepadx) [$T header cget $H $C -imagepadx]
    } else {
	set HeaderStyleParams($S,image) 0
	set HeaderStyleParams($S,bitmap) 0
    }
    if {[$T header cget $H $C -text] ne ""} {
	set HeaderStyleParams($S,text) 1
	set HeaderStyleParams($S,textpadx) [$T header cget $H $C -textpadx]
    } else {
	set HeaderStyleParams($S,text) 0
    }
}
    return $S
}

proc ::TreeCtrl::UpdateHeaderStyle {T H C args} {
    variable HeaderStyleParams

if 1 {
    set paramList [GetHeaderStyleParams $T $H $C]
    set match ""
    foreach S [$T style names] {
	if {![string match treectrl_header_style* $S]} continue
	if {$paramList ne $HeaderStyleParams($S)} continue
	set match $S
	break
    }
    array set params {text 0 image 0 bitmap 0}
    array set params $paramList
} else {
    set image [$T header cget $H $C -image]
    set bitmap [$T header cget $H $C -bitmap]
    if {$image ne ""} {
	set bitmap ""
    }
    set imagepadx  [$T header cget $H $C -imagepadx]
    set imagepady  [$T header cget $H $C -imagepady]
    set justify  [$T header cget $H $C -justify]
    set text [$T header cget $H $C -text]
    set textpadx  [$T header cget $H $C -textpadx]
    set textpady  [$T header cget $H $C -textpady]
    set match ""
    foreach S [$T style names] {
	if {![string match treectrl_header_style* $S]} continue
	if {($image ne "") ^ $HeaderStyleParams($S,image)} continue
	if {($bitmap ne "") ^ $HeaderStyleParams($S,bitmap)} continue
	if {($text ne "") ^ $HeaderStyleParams($S,text)} continue
	if {($image ne "") && ([$T style layout $S $S.image -padx] ne $HeaderStyleParams($S,imagepadx))} continue
	if {($bitmap ne "") && ([$T style layout $S $S.bitmap -padx] ne $HeaderStyleParams($S,imagepadx))} continue
	if {($text ne "") && ([$T style layout $S $S.text -padx] ne $HeaderStyleParams($S,textpadx))} continue
	if {$HeaderStyleParams($S,justify) ne $justify} continue
	set match $S
	break
    }
}
    if {$match eq ""} {
	set match [CreateHeaderStyle $T $H $C]
    }
    set S $match
    if {[$T column compare $C != tail] && [$T header style set $H $C] ne $S} {
	foreach optionInfo [$T header configure $H $C] {
	    lassign $optionInfo name x y default current
	    array set config [list $name $current]
	}
    }
    $T header style set $H $C $S
    array set config $args ; # FIXME could be abbreviations/synonyms
    if {$params(text) && [llength [array names config -text]]} {
	$T header element configure $H $C $S.text -text $config(-text)
    }
    if {$params(image) && [llength [array names config -image]]} {
	$T header element configure $H $C $S.image -image $config(-image)
    }
    if {$params(bitmap) && [llength [array names config -bitmap]]} {
	$T header element configure $H $C $S.bitmap -image $config(-bitmap)
    }
    set validOptions [list -arrow -arrowbitmap -arrowimage -arrowside \
	-arrowgravity -arrowpadx -arrowpady -background -borderwidth -state]
    set optval {}
    foreach {option value} $args {
	if {[lsearch -glob $validOptions $option*] != -1} {
	    lappend optval $option $value
	}
    }
if 1 {
    if {[llength $optval]} {
	eval $T header element configure $H $C $S.header $optval
    }
} else {
    $T header element configure $H $C $S.header \
	-arrow [$T header cget $H $C -arrow] \
	-arrowbitmap [$T header cget $H $C -arrowbitmap] \
	-arrowimage [$T header cget $H $C -arrowimage] \
	-arrowside [$T header cget $H $C -arrowside] \
	-arrowgravity [$T header cget $H $C -arrowgravity] \
	-arrowpadx [$T header cget $H $C -arrowpadx] \
	-arrowpady [$T header cget $H $C -arrowpady] \
	-background [$T header cget $H $C -background] \
	-borderwidth [$T header cget $H $C -borderwidth] \
	-state [$T header cget $H $C -state]
}
    return
}
