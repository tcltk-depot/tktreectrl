# tktreectrl

This is a fork of the original tktreectrl project at https://tktreectrl.sourceforge.net/.

The modifications in this fork are

* Addition of nmake/Visual C++ makefiles in the win directory

* Bugfix for a performance issue when table cells contain even moderately large amount of text

* Renaming of the treectrl specific shellicon extension to tcshellicon. 
  Note, that tclshellicon does not work with Unicode file or directory names.
  Demo explorer.tcl shows how to use the generic shellicon extension (https://sourceforge.net/projects/shellicon/) instead.

Please see the original website above for documentation, examples, demos etc. This fork adds no new functionality.
