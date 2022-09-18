# BhdTool
This is a tool I made for fun while learning some Reverse Engineering stuff
The goal of this tool is to offer utilities within the games, mainly aimed at helping speedrunning practises.

## Features
- Ability to toggle Door Skip on & off at runtime.
- Tool to jump to any room in the game
- Inventory editor
- Save anywhere tool

## How to build
This project was made using Visual Studio 2019 Community.
So any version of Visual Studio 2019 or more recent should be more than enough to build the project.
There are two external dependencies to imgui & json11 but their files are included within the project, so you just need to clone this repository (with --recursive) and it should be just fine.

## How to run
Simply paste the built dinput8.dll from any of the releases (or your own build) where the game's executable is and run the game.
You will also need a "rooms.json" files next to the ini file for room jumping tool to properly work. This file is provided with any of the releases. You can also generate it yourself using the python script provided in the "scripts" folder. However you will need to set up a few things before being able to run it like extracting the xfs filles of all the rooms. A better tool might get added in the future.
Upon first use, the tool will generate a bhdtool.ini file when the game is closed, containing the tool's settings. If you wish to completely disable the tool without having to remove the dinput8.dll, simply set the "EnableBhdTool" field to 0 within it.

## License
TODO