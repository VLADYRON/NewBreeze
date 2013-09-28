NewBreeze - Fast and light-weight File Manager
==============================================

Built on top of the Powerful Qt4 GUI Toolkit, this file manager is sleek, stylish and fast.
Some of its features are
    - Fast startup and browsing
    - Folder, text, html and image quick preview
    - File Association support
    - Provides List and Icon View
    - Lists the drives/boomarks in the sidebar
    - Full drag and drop support
    - Bookmark Support
    - Auto update devices list
    - Custom Folder Icons and Thumbnail Preview
    - Complete mime-icon support
    - BreadCrumbs Navigation Bar
    - Inline terminal

NOTE: Due to changes in theme names, you'll have to run the settings once to fix
theme related issues. Or make changes in NewBreeze.conf
  New             Old
=============================
 TransDark       Transparent
 TransLight      **New
 DullBlack       DullBlack
 LightGray       Natural

Notes for compiling - linux:
    - Download and Extract the tar file
    - Go to NewBreeze
    - Open the terminal and type: qmake && make

Additional notes for compiling on windows
    - Download and Extract the tar file
    - Go to NewBreeze
    - Please edit the $$PREFIX qmake variable to your choice
    - Open the terminal and type: qmake && make && make install
      This installs the data files and font files required by New Breeze

Notes for installing icon themes (Windows only):
    - Copy the icon theme into the folder $$PREFIX/icons/
    - Each theme should be in a separate folder
    - Each theme folder must contain index.theme file
    - To know how each theme must be structured check
        > http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
        > http://malcer.deviantart.com/art/Caledonia-Icons-264978107

Dependencies:
    - Qt4
    - libmagic and libmagic-dev

Windows and Linux Source Available

Bugs
    - Large paths run away from BreadCrumbs Bar
    - Some issues with cut-copy-paste files and folders
    - Issues with delete folders

List of shortcuts
+==============================================================+============================+
|       Action                                                 |         Key Sequence       |
+==============================================================+============================+
|   - Open file/folder                                         |         Enter/Return       |
|   - Open another instance                                    |         Ctrl+N             |
|   - Peek                                                     |         Ctrl+Return        |
|   - Properties                                               |         Alt+Return         |
|   - Reload                                                   |         F5                 |
|   - Close                                                    |         Alt+F4             |
|                                                              |                            |
|   - Goto Previous Directory                                  |         Alt+Left           |
|   - Goto Next Directory                                      |         Alt+Right          |
|   - Goto Parent Directory                                    |         Alt+Up             |
|                                                              |                            |
|   - Cut                                                      |         Ctrl+X             |
|   - Copy                                                     |         Ctrl+C             |
|   - Paste                                                    |         Ctrl+V             |
|                                                              |                            |
|   - Rename                                                   |         F2                 |
|   - Rename Inline                                            |         Shift+F2           |
|                                                              |                            |
|   - Delete                                                   |         Shift+Delete       |
|                                                              |                            |
|   - New Folder                                               |         F10                |
|   - New File                                                 |         Ctrl+Shift+N       |
|                                                              |                            |
|   - Toggle Icon and List mode                                |         Ctrl+Shift+V       |
|   - Toggle CrumbsView and LineEdit View                      |         Ctrl+T             |
|   - Open terminal Here                                       |         F4                 |
|   - Show inline terminal                                     |         Shift+F4           |
|                                                              |                            |
|   - Settings                                                 |         Ctrl+S             |
|                                                              |                            |
|   - About NewBreeze                                          |         Shift+F1           |
|   - NewBreeze Full Info                                      |         Ctrl+I             |
+==============================================================+============================+

Upcoming
    - File/folder delete GUI
    - New functionality to file/folder properties
        * to set open with defaults
    - Any other feature you request for... :)
