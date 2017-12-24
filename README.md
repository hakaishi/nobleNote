# nobleNote

nobleNote is a cross-platform note taking application. 
Manage notes in a simple two-pane layout with notebooks and an integrated rich text editor. 
Notes are stored in the html format for maximum compatibility with other applications. 

It is compatible to the [nobleNoteAndroid](https://github.com/taiko000/nobleNoteAndroid) app for Android.

## Supported Platforms

Any Qt: Windows, Linux, macOS, Android

## Sync notes between desktop and mobile devices

1.) Install [nobleNote](https://github.com/hakaishi/nobleNote) on your Linux/Windows/macOS device and find the folder containing the nobleNote notebooks. 

2.) In nobleNoteAndroid, select a folder on the external storage (sd-card) using the overflow menu on the main screen.

3.) Use a sync software of your choice (e.g. [Syncthing](https://syncthing.net), Dropbox) to sync the folder containining the notebooks with the same folder on your Linux/Windows/macOS device. nobleNote will detect when notes have been changed on the file system and reload them automatically. 

## Screenshots

![Alt text](/screenshot/Screenshot0.png?raw=true "")

![Alt text](/screenshot/Screenshot1.png?raw=true "")

## Installation

Note for Windows users: Binary releases can be found on the releases page of this github repository

Requires at least Qt 5.0 or newer.

It can be build using the QtCreator IDE. 

## Building From CLI

Dependencies: build-essential, qtbase5-dev, qttools5-dev-tools

To compile from source, open a terminal and change into the nobleNote source folder.
To install type:

qmake
sudo make install distclean

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
