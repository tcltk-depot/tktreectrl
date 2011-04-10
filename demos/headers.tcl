namespace eval DemoHeaders {}

proc DemoHeaders {} {
    set T [DemoList]

    $T configure \
	-showroot no -xscrollsmoothing yes -itemheight 22 \
	-selectmode multiple

    #
    # Create one locked column on each side plus 8 non-locked columns
    #

    set itembg {linen {} #e0e8f0 {}}

    $T column create -text "Left" -tags Cleft -width 80 -justify center \
	-gridrightcolor gray90 -itembackground $itembg -borderwidth 1 \
	-lock left -arrow none -arrowside left -arrowgravity left \
	-visible no

    for {set i 1} {$i <= 8} {incr i} {
	$T column create -text "C$i" -tags C$i -width 80 -justify center \
	    -gridrightcolor gray90 -itembackground $itembg -borderwidth 1 
    }

    $T column create -text "Right" -tags Cright -width 80 -justify center \
	-gridrightcolor gray90 -itembackground $itembg -borderwidth 1  \
	-lock right -visible no

    #
    # Create custom item states to alter the appearance of the custom
    # headers.
    #

    $T state define headeractive
    $T state define headerpressed
    $T state define sortdown
    $T state define sortup

    DemoHeaders::InitSortImages blue
    $T element create header.sort image \
	-image {::DemoHeaders::arrow-down sortdown ::DemoHeaders::arrow-up sortup}

    #
    # Create a style for our custom headers,
    # a raised border with centered text.
    #

    $T element create header.border border \
	-background $::SystemButtonFace \
	-relief {sunken headerpressed raised {}} -thickness 2 -filled yes
    $T element create header.text text \
	-lines 1

    set S [$T style create header1 -orient horizontal]
    $T style elements $S {header.border header.text header.sort}
    $T style layout $S header.border -detach yes -iexpand xy
    $T style layout $S header.text -expand wens -padx 2 -pady 2 ; # $T style layout $S header.text -center x -expand ns -padx 2 -pady 2
    $T style layout $S header.sort -expand nws -padx 6 \
	-visible {no {!sortdown !sortup}}

    #
    # Create a style for our custom headers,
    # a light-blue rounded rectangle with centered text.
    #

    $T element create header.rrect rect \
	-rx 9 -fill {
	    #cee8f0 headeractive
	    #87c6da headerpressed
	    #87c6da sortup
	    #87c6da sortdown
	    {light blue} {}
	}

    set S [$T style create header2 -orient horizontal]
    $T style elements $S {header.rrect header.text header.sort}
    $T style layout $S header.rrect -detach yes -iexpand xy -padx {1 0} -pady 1
    $T style layout $S header.text -expand wens -padx 2 -pady 4 ; # $T style layout $S header.text -center x -expand ns -padx 2 -pady 4
    $T style layout $S header.sort -expand nws -padx 6 \
	-visible {no {!sortdown !sortup}}
if 0 {
    #
    # Create a style for our custom headers,
    # a header element with centered text.
    #

    $T element create header.header header -thickness 1

    set S [$T style create header3 -orient horizontal]
    $T style elements $S {header.header header.text}
    $T style layout $S header.header -union header.text -iexpand news
    $T style layout $S header.text -expand wens -padx 6 -pady 2 -squeeze x ; # $T style layout $S header.text -expand ns -center x -padx 6 -pady 2 -squeeze x
}
    set S header1
    set S header2
#    set S header3

    #
    # Create the three topmost -lock items
    #

    set text [$T column cget 0 -text]

    $T header configure first -tags header1
    set H [$T header id first]
    $T header style set $H Cleft $S C1 $S C5 $S Cright $S
    $T header span $H 1 4 C5 4
#    $T item text $I Cleft $text C1 A C5 H Cright Right
    foreach {C text} {Cleft $text C1 A C5 H Cright Right} {
	$T header element configure $H $C header.text -text $text
    }

    set H [$T header create -tags header2]
    $T header style set $H Cleft $S C1 $S C3 $S C5 $S C7 $S Cright $S
    $T header span $H C1 2 C3 2 C5 2 C7 2
#    $T item text $I Cleft $text C1 B C3 C C5 I C7 J Cright Right
    foreach {C text} {Cleft $text C1 B C3 C C5 I C7 J Cright Right} {
	$T header element configure $H $C header.text -text $text
    }

    set H [$T header create -tags header3]
    $T header style set $H all $S
#    $T item text $I Cleft $text C1 D C2 E C3 F C4 G C5 K C6 L C7 M C8 N Cright Right
    foreach {C text} {Cleft $text C1 D C2 E C3 F C4 G C5 K C6 L C7 M C8 N Cright Right} {
	$T header element configure $H $C header.text -text $text
    }

    #
    # Create 100 regular non-locked items
    #

    $T element create item.sel rect -fill {gray {selected !focus} blue selected}
    $T element create item.text text -text "Item" -fill {white selected}

    set S [$T style create item]
    $T style elements $S {item.sel item.text}
    $T style layout $S item.sel -detach yes -iexpand xy
    $T style layout $S item.text -expand news -padx 2 -pady 2

    $T column configure !tail -itemstyle $S
    $T item create -count 100 -parent root

    #
    # Create an interface to change the style used by the custom headers
    #

    set DemoHeaders::HeaderStyle header3
    set f [frame $T.fHeaderStyle -borderwidth 2 -relief raised -width 150]
    pack [$::radiobuttonCmd $f.style1 -text header1 \
	-variable ::DemoHeaders::HeaderStyle -value header1 \
	-command [list DemoHeaders::ChangeHeaderStyle yes black]] -side top -anchor w -pady 3
    pack [$::radiobuttonCmd $f.style2 -text header2 \
	-variable ::DemoHeaders::HeaderStyle -value header2 \
	-command [list DemoHeaders::ChangeHeaderStyle yes blue]] -side top -anchor w -pady 3
    pack [$::radiobuttonCmd $f.style3 -text header3 \
	-variable ::DemoHeaders::HeaderStyle -value header3 \
	-command [list DemoHeaders::ChangeHeaderStyle no]] -side top -anchor w -pady 3
    place $f -relx 0.5 -rely 0.3 -anchor n

    #
    # Set binding scripts to handle the <Header> dynamic event
    #

    set DemoHeaders::Sort(header) ""
    set DemoHeaders::Sort(column) ""
    foreach C [$T column list] {
	set DemoHeaders::Sort(direction,$C) down
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

# Called when the user clicks one of the radiobuttons to select one of the
# styles created by this demo.  The [item style map] call changes the style
# while perserving the -text of the header.text element used by all the
# styles.
proc DemoHeaders::ChangeHeaderStyle {ownerDrawn {sortColor ""}} {
    variable HeaderStyle
    variable Sort
    set T [DemoList]
    if {$sortColor ne ""} {
	InitSortImages $sortColor
    }
    set S $HeaderStyle
    foreach H [$T header id all] {
	$T header style map $H all $S {header.text header.text}
	$T header configure $H -ownerdrawn $ownerDrawn
    }
    if {$Sort(header) ne ""} {
	ShowSortArrow $Sort(header) $Sort(column)
    }
    return
}

# If state=active, the dynamic item state 'headeractive' is set in the given
# item-column.
# If state=pressed, the dynamic item state 'headerpressed' is set in the given
# item-column.
# If the style in the given item-column contains a 'header' element, then
# that element's -state option is configured to the given state.
proc DemoHeaders::HeaderState {H C state} {
    set T [DemoList]
#    if {![$T item tag expr $I header3]} return
    $T header state forcolumn $H $C {!headeractive !headerpressed}
    if {$state ne "normal"} {
	$T header state forcolumn $H $C header$state
    }
    set S [$T header style set $H $C]
    foreach E [$T style elements $S] {
	if {[$T element type $E] eq "header"} {
	    $T header element configure $H $C $E -state $state
	}
    }
    return
}

# Called when the left mouse button is pressed and released over an item-column
# in one of the -lock=top items.  If the given item-column is already displaying
# a sort arrow, the sort arrow direction is toggled.  Otherwise the sort arrow
# is removed from all other item-columns and displayed in the given item-column.
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

# Sets one of the dynamic item states 'sortup' or 'sortdown' in the given
# item-column.
# If the style in the given item-column contains a 'header' element, then
# that element's -arrow option is configured to 'up' or 'down'.
proc DemoHeaders::ShowSortArrow {H C} {
    variable Sort
    set T [DemoList]
    $T header state forcolumn $H $C [list !sortdown !sortup sort$Sort(direction,$C)]
    set S [$T header style set $H $C]
    foreach E [$T style elements $S] {
	if {[$T element type $E] eq "header"} {
	    $T header element configure $H $C $E -arrow $Sort(direction,$C)
	}
    }
    return
}

# Clears both of the dynamic item states 'sortup' and 'sortdown' in the given
# item-column.
# If the style in the given item-column contains a 'header' element, then
# that element's -arrow option is configured to 'none'.
proc DemoHeaders::HideSortArrow {H C} {
    variable Sort
    set T [DemoList]
    $T header state forcolumn $H $C [list !sortdown !sortup]
    set S [$T header style set $H $C]
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

