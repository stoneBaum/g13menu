# g13menu - On device menu for your Logitech G13

## Installation

You have to have the g13 driver (https://github.com/khampf/g13) installed.

Then just run the install script (needs sudo rights).

After that either just restart your PC or run:

```
$ sudo systemctl daemon-reload
$ sudo systemctl restart g13-menu.service
```
in your terminal to enable it.

If you only want to test it or run it manually, you can also just run:

```
$ mkdir -p build
$ g++ -o build/g13menu g13menu.cpp
$ build/g13menu /run/g13d/g13-0_out /run/g13d/g13-0 profiles
```
from within the g13menu folder.

## Usage

To use the menu press the BD button on your g13. Thats the upper leftmost button (round and without any label).

From there use the "\/" and "/\" buttons to navigate through the profiles and use the "o" button to activate the selected profile.

To display the profile art/name either select a profile through the menu or press the BD button again.

## Adding profiles

### Keybinds

To add profiles to the menu, just add a folder with the name of the profile into the "profiles" folder (/etc/g13/profiles if installed).
Then add a keybindfile called "Keybinds.bind" into it.
Please note: all keys not set in the Keybinds.bind file will have the same binding as the default profile (in the folder "Default"),
also the Keys BD, L1, L2, L3, L4, M1, M2, M3 and MR are reserved for g13menu.

### Profile art
If you want to add profile art just add a LCD.lpbm.
To create a .lpbm file you can use pbm2lpbm which is included in the g13 driver.

Useage: `$ pbm2lpbm < myProfileArt.pbm > LCD.lpbm`.

Please note: The pbm file must be 160x43 pixels.

Alternatively you can also add a LCD.bind file to the folder to display your own text based art.

If neither a LCD.lpbm nor a LCD.bind file is in the profile folder the name of the profile will be displayed.
