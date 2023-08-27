# BhdTool
This is a tool I made for fun while learning some Reverse Engineering stuff

The goal of this tool is to offer utilities within the Resident Evil HD REmaster game, mainly aimed at helping speedrunning practises.

## Features
- Ability to toggle Door Skip on & off at runtime.
- Tool to jump to any room in the game
- Inventory editor
- Save anywhere tool
- Various cheats

## How to build
This project was made using Visual Studio 2019 Community.

So any version of Visual Studio 2019 or newer should be more than enough to build the project. You will need to edit the Post-Build step though as it copies the dll to a hardset path.

There are two external dependencies to imgui & json11 but their files are included within the project, so you just need to clone this repository (with --recursive) and it should be just fine.

## How to run
Simply paste the built dinput8.dll from any of the releases (or your own build) where the game's executable is and run the game.

To bring up the tool's UI, simply press F5 while ingame.

You will also need a "rooms.json" files next to the dll file for room jumping tool to properly work. This file is provided with any of the releases. You can also generate it yourself using the python script provided in the "scripts" folder. However you will need to set up a few things before being able to run it like extracting the xfs filles of all the rooms. A better tool might get added in the future.

Upon first use, the tool will generate a bhdtool.ini file when the game is closed, containing the tool's settings. If you wish to completely disable the tool without having to remove the dinput8.dll, simply set the "EnableBhdTool" field to 0 within it.

## Contributing
If you wish to request or add more features to the project (or fix bugs), do not hesitate to open issues and/or pull requests and I'll look at them when I can.

## List of planned features
- Various informations about the game (like character coords, enemies, etc...)
- Controller support
- Flag editor (Save/Kill Barry, Mansion Part 1 & 2, etc...)
- And more...

## Donations
If you feel generous and want to buy me a coffee or anything, here's a link to my Ko-Fi : https://ko-fi.com/eleval777

## License
This project is distributed under the GNU General Public License v3.0 (GNU GPLv3).
