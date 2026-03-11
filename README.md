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
+ [Optional] `gui={type}`: selects which GUI is used
    > `{type}` depends on the language

# Installation
Download repository and switch to the desired branch (branch equivalent to the programming language in which the
program was written)
```
git clone git@github.com/seabattles
cd seabattles
git switch <branch>
```

You'll find `README.md` in which is written how to compile and run the application