# nobleNote

nobleNote is a cross-platform note taking application. 
Manage notes in a simple two-pane layout with notebooks and an integrated rich text editor. 
Notes are stored in the html format for maximum compatibility with other applications. 

It is compatible to the [nobleNoteAndroid](https://github.com/taiko000/nobleNoteAndroid) app for Android.

## Supported Platforms

Any platform supported by Qt: Windows, Linux, macOS

[Qt 5.9](https://doc.qt.io/archives/qt-5.9/) is the latest version that will build without any changes to the project.

## Sync notes between desktop and mobile devices

1.) Install [nobleNote](https://github.com/hakaishi/nobleNote) on your Linux/Windows/macOS device and find the folder containing the nobleNote notebooks. 

2.) In nobleNoteAndroid, select a folder on the external storage (sd-card) using the overflow menu on the main screen.

3.) Use a sync software of your choice (e.g. [Syncthing](https://syncthing.net), Dropbox) to sync the folder containining the notebooks with the same folder on your Linux/Windows/macOS device. nobleNote will detect when notes have been changed on the file system and reload them automatically. 

## Screenshots

![Alt text](/screenshot/Screenshot0.png?raw=true "")

![Alt text](/screenshot/Screenshot1.png?raw=true "")

## Installation

Windows: Binary releases can be found on the releases page of this github repository

Debian based Linux distributions (Ubuntu): 

`sudo apt-get install noblenote`

Building from source requires at least Qt 5.0 or newer.

It can also be build using the QtCreator IDE. 

## Building From CLI

Dependencies: build-essential, qtbase5-dev, qttools5-dev-tools

To compile from source, open a terminal and change into the nobleNote source folder.
To install type:

`qmake`

`sudo make install distclean`

To uninstall type:

`qmake`     (if the Makefile has been removed)

`sudo make deinstall`

(Note that ~/.config/nobleNote.conf and
 the notes at their location won't be removed.
Backups are in their default data location.)

## Building From Visual Studio 2019

1. Install the [Qt Creator IDE](http://qt.io/download).  You will only need the Qt 5.9 component.

2. Install the [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2019) extension from the Visual Studio Marketplace.

3. Import your .pro projects into Visual Studio. To do that, select the Qt VS Tools > Open Qt Project File (.pro) to let the extension create a VS solution and project from your existing Qt .pro file.

4. You will probably need to retarget your solution in VS 2019 to get the build to work.  Make sure you clean and rebuild.

[Source](https://devblogs.microsoft.com/cppblog/bring-your-existing-qt-projects-to-visual-studio/)

## License

nobleNote is licensed under the MIT License

(C) 2020 The creators of nobleNote

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
