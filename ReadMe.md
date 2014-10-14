## Club N69

### About

Club N69 - old school style FPS game parody on "Duke Nukem 3D" and "Wolfenstein 3D".
Game was inspired by disappointment of recently released "Duke Nukem Forever", when I was thinking how could "Duke Nukem 3D" look like if it was developed by "id Software" in 1992.

Winner of [Independent Games Developers Contest](http://igdc.ru) number 69.

This game was mentioned as one of the most remarkable indie games in Russia of August 2011 by "Igromania" game magazine. Also it was published on DVD coming along with printed magazine.

Game uses [DGLE](http://dglengine.org) engine and is a good example of using its low level API in old school procedural style.

You can learn more about this game and download compiled binaries on its [page](http://dronprogs.org#project-club69).

### License

Source code of this game is distributed under the [GNU GPL license](https://en.wikipedia.org/wiki/GNU_General_Public_License).

Note that this game uses graphics and sounds from games by "id Software" and "3D Realms" and music theme by "Abakus".

### Building

Originally game was developed using Microsoft Visual Studio 2010 and compiles only for Windows.
But it should be easy to recompile it under any platform supported by the engine almost without any changes.

To build the game you must proceed to these steps:
* You need to get DGLE binaries and C++ headers from the [official website](http://dglengine.org) or compile latest version from the [sources](https://github.com/DGLE-HQ/DGLE).
* You need to put compiled "DGLE.dll" into ".\bin\" folder and "Mad.dll" into ".\bin\plugins\" folder.
* Put "DGLE.h" and "DGLE_Types.h" headers into ".\dgle\" folder.
* Locate provided project file for Microsoft Visual Studio 2010 in ".\source\" folder.

Note that path to game resources is set to run Debug configuration from IDE (typically pressing F5 key) and Release configuration directly by executing compiled ".\bin\ClubN69.exe" file.

## Programming and Idea

Andrey Korotkov aka DRON

## Contacts

See some other games on my homepage [dronprogs.org](http://dronprogs.org).

Feel free to [mail me](mailto:dron@dronprogs.org) with any questions and suggestions.
