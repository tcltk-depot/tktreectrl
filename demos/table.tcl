namespace eval DemoTable {}

proc DemoTable {} {
    DemoTable::Run [DemoList]
}

proc DemoTable::Run {T} {
    variable Priv

    $T configure -showroot no -usetheme no -xscrollsmoothing yes

    $T column create -tags rowtitle -background gray90 -borderwidth 1 -lock left
    for {set i 1} {$i <= 10} {incr i} {
	$T column create -text $i -minwidth 20 -background gray90 -borderwidth 1
    }

    $T item state define zapL
    $T item state define zapR
    $T item state define drag
    $T item state define drop

    $T element create rowtitle.border border \
	-background gray90 -thickness 1 -relief raised -filled yes
    $T element create rowtitle.text text
    set S [$T style create rowtitle]
    $T style elements $S {rowtitle.border rowtitle.text}
    $T style layout $S rowtitle.border \
	-union rowtitle.text -ipadx 3 -ipady 2 -iexpand news
    $T style layout $S rowtitle.text -center x

    $T element create et text
    $T element create eb border -background gray -thickness 2 -relief groove
    $T element create cell.bg rect -fill {{light green} drag gray90 drop}
    $T element create zL rect -fill red -width 5 -height 10
    $T element create zR rect -fill red -width 5 -height 10

    $T style create s
    $T style elements s {cell.bg eb et zL zR}
    $T style layout s cell.bg -detach yes -iexpand xy -visible {yes drag yes drop no {}}
    $T style layout s eb -union et -iexpand wens -ipadx 2 -ipady 2
    $T style layout s et -squeeze x
    $T style layout s zL -detach yes -expand ens -draw {yes zapL no {}}
    $T style layout s zR -detach yes -expand wns -draw {yes zapR no {}}

    $T column configure 0 -itemstyle rowtitle
    $T column configure {range 1 10} -itemstyle s
    foreach I [$T item create -count 5000 -parent root] {
	$T item text $I rowtitle [$T item order $I]
    }
    $T item text {root children} {range 1 10} "edit me"

    $T notify bind DemoTable <Edit-accept> {
	%T item text %I %C %t
	set DemoTable::EditAccepted 1
    }
    $T notify bind DemoTable <Edit-end> {
	if {!$DemoTable::EditAccepted} {
	    %T item element configure %I %C %E -text $DemoTable::OrigText
	}
	%T item element configure %I %C %E -textvariable ""
    }

    set height [font metrics [$T cget -font] -linespace]
    incr height [expr {[$T style layout s eb -ipady] * 2}]
    incr height 2 ; # entry widget YPAD
    $T configure -minitemheight $height

    set Priv(zap) {}
    set Priv(buttonMode) ""

    bind DemoTable <ButtonPress-1> {
	DemoTable::ButtonPress1 %W %x %y
    }
    bind DemoTable <Button1-Motion> {
	DemoTable::Button1Motion %W %x %y
    }
    bind DemoTable <ButtonRelease-1> {
	DemoTable::ButtonRelease1 %W %x %y
    }
    bind DemoTable <Motion> {
	DemoTable::Motion %W %x %y
    }
    bindtags $T [list $T DemoTable TreeCtrl [winfo toplevel $T] all]

    return
}

proc DemoTable::ButtonPress1 {T x y} {
    variable Priv
    if {[winfo exists $T.entry] && [winfo ismapped $T.entry]} {
	TreeCtrl::EditClose $T entry 1 0
    }
    set Priv(buttonMode) ""
    $T identify -array id $x $y
    if {$id(where) ne "item"} return
    if {$id(column) eq "tail"} return
    if {[$T column compare $id(column) == first]} return

    set prev [$T column id "$id(column) prev"]
    set next [$T column id "$id(column) next !tail"]
    switch -- [WhichSide $T $id(item) $id(column) $x $y] {
	left {
	    set Priv(buttonMode) resize
	    set Priv(item) $id(item)
	    set Priv(column) [StartOfPrevSpan $T $id(item) $id(column)]
	    set Priv(y) $y
	    return
	}
	right {
	    set Priv(buttonMode) resize
	    set Priv(item) $id(item)
	    set Priv(column) $id(column)
	    set Priv(y) $y
	    return
	}
    }

    set Priv(buttonMode) dragWait
    set Priv(item) $id(item)
    set Priv(column) $id(column)
    set Priv(x) $x
    set Priv(y) $y

    return
}

proc DemoTable::Button1Motion {T x y} {
    variable Priv
    switch $Priv(buttonMode) {
	dragWait {
	    if {(abs($Priv(x) - $x) > 4) || (abs($Priv(y) - $y) > 4)} {
		set Priv(buttonMode) drag
		set Priv(highlight) ""
		$T item state forcolumn $Priv(item) $Priv(column) drag
		set Priv(cx) [$T canvasx $x]
		set Priv(cy) [$T canvasy $y]
		$T dragimage clear
		$T dragimage add $Priv(item) $Priv(column) eb
		$T dragimage configure -visible yes
	    }
	}
	drag {
	    $T identify -array id $x $y
	    if {$Priv(highlight) ne ""} {
		eval $T item state forcolumn $Priv(highlight) !drop
		set Priv(highlight) ""
	    }
	    if {$id(where) eq "item" && [$T column cget $id(column) -lock] eq "none"} {
		$T item state forcolumn $id(item) $id(column) drop
		set Priv(highlight) [list $id(item) $id(column)]
	    }
	    set dx [expr {[$T canvasx $x] - $Priv(cx)}]
	    set dy [expr {[$T canvasy $y] - $Priv(cy)}]
	    $T dragimage offset $dx $dy
	}
	resize {
	    $T identify -array id $x $Priv(y)
	    if {$id(where) eq "item"} {
		set C [ColumnUnderPoint $T $x $y]
		if {[WhichHalf $T $C $x $y] eq "right"} {
		    if {[$T column compare $id(column) > $Priv(column)]} {
			IncrSpan $T $Priv(item) $Priv(column) $C
		    }
		    if {[$T column compare $C >= $Priv(column)] &&
			    ([$T item span $Priv(item) $Priv(column)] > 1)} {
			DecrSpan $T $Priv(item) $Priv(column) $C
		    }
		}
		if {[WhichHalf $T $C $x $y] eq "left"} {
		    if {[$T column compare $C == $Priv(column)]} {
			DecrSpan $T $Priv(item) $Priv(column) $C
		    }
		}
	    }
	}
    }
    return
}

proc DemoTable::ButtonRelease1 {T x y} {
    variable Priv
    switch $Priv(buttonMode) {
	dragWait {
	    # FIXME: EntryExpanderOpen doesn't work with master elements
	    set text [$T item text $Priv(item) $Priv(column)]
	    $T item text $Priv(item) $Priv(column) $text
	    set exists [winfo exists $T.entry]
	    TreeCtrl::EntryExpanderOpen $T $Priv(item) $Priv(column) et
	    if {!$exists} {
		$T.entry configure -borderwidth 0
	    }
	    $T notify unbind $T.entry <Scroll>
	    set ::DemoTable::TextVar $text
	    set ::DemoTable::OrigText $text
	    set ::DemoTable::EditAccepted 0
	    $T.entry configure -textvariable ::DemoTable::TextVar
	    $T item element configure $Priv(item) $Priv(column) et \
		-textvariable ::DemoTable::TextVar -text ""
	}
	drag {
	    $T dragimage configure -visible no
	    $T item state forcolumn $Priv(item) $Priv(column) !drag
	    if {$Priv(highlight) ne ""} {
		eval $T item state forcolumn $Priv(highlight) !drop
		set Priv(highlight) ""
	    }
	    $T identify -array id $x $y
	    if {$id(where) ne "item"} return
	    if {[$T column cget $id(column) -lock] ne "none"} return
	    if {[$T item compare $id(item) == $Priv(item)] &&
		[$T column compare $id(column) == $Priv(column)]} return
	    set text [$T item text $id(item) $id(column)]
	    $T item text $id(item) $id(column) [$T item text $Priv(item) $Priv(column)]
	    $T item text $Priv(item) $Priv(column) $text
	}
    }
    return
}

proc DemoTable::Motion {T x y} {
    variable Priv
    $T identify -array id $x $y
    if {$Priv(zap) ne ""} {
	$T item state forcolumn $Priv(zap) all {!zapL !zapR}
	set Priv(zap) ""
    }
#    $T configure -cursor ""
    if {$id(where) ne "item"} return
    if {$id(column) eq "tail"} return
    if {[$T column compare $id(column) == first]} return

    switch -- [WhichSide $T $id(item) $id(column) $x $y] {
	left {
	    if {[$T column id "$id(column) prev"] ne ""} {
		$T item state forcolumn $id(item) $id(column) zapL
		set Priv(zap) $id(item)
#		$T configure -cursor sb_h_double_arrow
	    }
	}
	right {
	    if {[$T column id "$id(column) next !tail"] ne ""} {
		$T item state forcolumn $id(item) $id(column) zapR
		set Priv(zap) $id(item)
#		$T configure -cursor sb_h_double_arrow
	    }
	}
    }
    return
}

proc DemoTable::StartOfNextSpan {T I C} {
    set span [$T item span $I $C]
    set last [$T column id "$C span $span"]
    return [$T column id "$last next visible"]
}

proc DemoTable::StartOfPrevSpan {T I C} {
    set prev [$T column id "$C prev visible"]
    if {$prev ne ""} {
	set starts [GetSpanStarts $T $I]
	return [lindex $starts [$T column order $prev]]
    }
    return ""
}

proc DemoTable::IncrSpan {T I C newLast} {
    set span [expr {[$T column order $newLast] - [$T column order $C] + 1}]
    $T item span $I $C $span
    return
}

proc DemoTable::DecrSpan {T I C newLast} {
    set span [expr {[$T column order $newLast] - [$T column order $C] + 1}]
    $T item span $I $C $span
    return
}

proc DemoTable::ColumnUnderPoint {T x y} {
    #return [$T column id "nearest $x $y"]
    set totalWidth 0
    foreach C [$T column id "lock none"] {
	incr totalWidth [$T column width $C]
	if {[$T canvasx $x] < $totalWidth} {
	    return $C
	}
    }
    return ""
}

proc DemoTable::WhichSide {T I C x y} {
    scan [$T item bbox $I $C] "%d %d %d %d" x1 y1 x2 y2
    if {$x < $x1 + 5} { return left }
    if {$x >= $x2 - 5} { return right }
    return
}

proc DemoTable::WhichHalf {T C x y} {
    scan [$T column bbox $C] "%d %d %d %d" x1 y1 x2 y2
    if {$x < $x1 + ($x2 - $x1) / 2} { return left }
    return right
}

proc DemoTable::GetSpanStarts {T I} {
    set columns [list]
    set spans [$T item span $I]
    if {[lindex [lsort -integer $spans] end] eq 1} {
	return [$T column list]
    }
    for {set index 0} {$index < [$T column count]} {}  {
	set Cspan [$T column id "order $index"]
	set span [lindex $spans $index]
	if {![$T column cget $Cspan -visible]} {
	    set span 1
	}
	while {$span > 0 && $index < [$T column count]} {
	    if {[$T column cget "order $index" -lock] ne [$T column cget $Cspan -lock]} break
	    lappend columns $Cspan
	    incr span -1
	    incr index
	}
    }
    return $columns
}

