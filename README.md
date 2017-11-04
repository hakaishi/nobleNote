nobleNote is a cross-platform application to manage notes in a simle two pane layout with folders and text files in the html format.


## INSTALL:

Note for Windows users: Binary releases can be found on the releases page of this github repository

Requires at least Qt 5.0 or newer.

Dependencies: build-essential, qtbase5-dev, qttools5-dev-tools (or libqt4-dev)

To compile from source, open a terminal and change into the nobleNote source folder.
To install type:

qmake
sudo make install distclean

## UNINSTALL:

To uninstall type:

qmake     (if the Makefile has been removed)
sudo make deinstall

(Note that ~/.config/nobleNote.conf and
 the notes at their location won't be removed.
Backups are in their default data location.)
