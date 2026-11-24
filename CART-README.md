# NES Info

This plugin shows what a NES game's file says about itself. It doesn't play the game.

## Setup

Copy `NESINFO.CVP` into the `/PLUGINS` folder on the card. It opens `.nes` files in `/NES` or anywhere else on the card. Pick one in the menu to see its details.

If the NES plugin is on the card too, the menu asks which one should open a `.nes` file.

## Controls

There are no controls. Pick a `.nes` file in the menu and read the screen. Press RESET on the console, or hold both fire buttons and \* for 2 seconds, to go back to the menu. The menu comes back on the same file, with the mapper and CRC on its bottom line.

## What the screen shows

- Mapper is the mapper's number and its usual name, such as MMC1.
- PRG ROM, CHR and PRG RAM are the sizes of the game's ROM and RAM. PRG RAM also says whether the game has a battery.
- Mirroring shows how the screen is mirrored, and whether there's a trainer.
- TV shows whether the game is for NTSC or PAL.
- Header shows whether the header is iNES or NES 2.0.
- File is the file's size. It turns red if the file is shorter than the header says.
- CRC-32 is the checksum of the ROM after its header, read through the whole file. It turns green when it's done.

NES Info is also the example plugin in the cart's SDK.

## Credits

NES Info is by Parrot Cart. It uses no third-party emulator or library.
