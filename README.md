# nobleNote

nobleNote is a cross-platform application to manage notes in a simle two pane layout with folders and text files in the html format.

## Supported Platforms

Any Qt: Windows, Linux, macOS, Android

## Screenshots

![Alt text](/screenshot/Screenshot0.png?raw=true "")

![Alt text](/screenshot/Screenshot1.png?raw=true "")

## Install

Note for Windows users: Binary releases can be found on the releases page of this github repository

Requires at least Qt 4.8, Qt 5.0 or newer.

Dependencies: build-essential, qtbase5-dev, qttools5-dev-tools (or libqt4-dev)

To compile from source, open a terminal and change into the nobleNote source folder.
To install type:

qmake
sudo make install distclean

## Uninstall

To uninstall type:

qmake     (if the Makefile has been removed)
sudo make deinstall

(Note that ~/.config/nobleNote.conf and
 the notes at their location won't be removed.
Backups are in their default data location.)

## License

nobleNote is licensed under the MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
