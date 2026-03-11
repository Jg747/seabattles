# Introduction
This is a personal standard project to learn programming languages. Objective is building the "Battleship" game
with advanced features like multiplayer and custom configs

## Features
+ Singleplayer game
+ 3 difficulty bots (normal, hard, impossible)
+ FFA Multiplayer game (bots allowed)
+ Custom board size and ships
+ Custom config synchronization with other players
+ Multiplayer chat
+ Cheating debug string to reveal other players' placements (determined in config section)
    > Host independent (non-hosts can run with the argument if host's `debug` is known)
+ Optional: GUI (normally it executes as a terminal application)
+ Optional: custom textures (and multiplayer synchronization to visualize host's custom textures)

## Config
Config format is this
```json
{
    "board": {
        "width": {width},               // Board width (> 0)
        "height": {height}              // Board height (> 0)
    },
    "ships": [
        {
			"id": {int},                // ship's ID (must be different from other IDs)
            "length": {int},            // ship's length (> 0)
            "width": {int},             // ship's width (> 0)
            "sprite": "{string_path}"   // path to this ship's custom sprite,
                                        // leave empty if not present/used (default sprites if non-terminal GUI)
        },
        ...
    ],
    "debug": "{string}"                 // used with argument dbg="{string}" to activate cheating
}
```

Default config is this
```json
{
    "board": {
        "width": 10,
        "heigth": 10
    },
    "ships": [
        {
			"id": 1,
            "length": 5,
            "width": 1,
            "sprite": ""
        },
        {
			"id": 2,
            "length": 4,
            "width": 1,
            "sprite": ""
        },
        {
			"id": 3,
            "length": 3,
            "width": 1,
            "sprite": ""
        },
        {
			"id": 4,
            "length": 3,
            "width": 1,
            "sprite": ""
        },
        {
			"id": 5,
            "length": 2,
            "width": 1,
            "sprite": ""
        }
    ],
    "debug": "cheating_string"
}
```

## Protocol
You can find multiplayer communication protocol in `protocol_info.md`

## Execution arguments
+ `dbg={string}`: if {string} is equal to the host's config's "debug" field this will activate cheats
+ `verbose`: activate verbose and debug messages

# Installation
Download repository and switch to the C++ branch
```
git clone git@github.com/seabattles
cd seabattles
git switch C++
```

## Compile (executable)
Compile (needs g++ installed - edit `makefile` if you want to compile with other compilers)
```
make
./seabattles
```

## Installation
### Windows
If you want to install this program by adding it to your program list just type
```
make install
```
and then you will be able to use the program as normal executable

If you want to even have the possibility to call this program via cmd you can type
```
make install cmd=true
```

### Linux & MacOS
Linux / MacOS installation adds this program in bash/zsh command form, so after installing it
you can call this program by typing `seabattles <args>`
To install the program you can type
```
make install
```

## Uninstall
### Windows
On windows the program is installed in `C:\Programs\SeaBattles` but if you remove the program it's not
going to remove all files in your OS so it's better to open a command prompt in the folder and type
```
make remove
```

### Linux & MacOS
Linux / MacOS installations require to have the makefile of this repository, so you need to download this
repository (or just the makefile) and then type
```
make remove
```