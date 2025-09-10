# ADFOpus2025

![ADF Opus 2025 Logo](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/ADF_Opus_2025_Logo.png?raw=true)

ADF Opus is a free, Open-Source GPL v2 licenced, Windows application that lets you open, browse, and manage Amiga .ADF disk-images natively without an emulator. With Greaseweazle support built-in you can read and write to real floppy disks with the compatible hardware and software. This new 2025 edition by Chiron Bramberger is a refresh and revive of this application for Windows users in 2025!

![ADF Opus 2025 Screenshot 1](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/readme.png?raw=true)

![ADF Opus 2025 Screenshot 2](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/readme2.png?raw=true)

![ADF Opus 2025 Screenshot 3](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/readme3.png?raw=true)

![ADF Opus 2025 Screenshot 4](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/readme4.png?raw=true)

![ADF Opus 2025 Screenshot 5](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/readme5.png?raw=true)

New features in the 2025 edition include: 
- The power to copy, rename, delete and transfer files directly between disk images and your PC’s filesystem.
- Greaseweazle floppy read/write integration! Read and write your .ADF images to and from real floppy disks using the Greaseweazle hardware and software solution! Make sure you've got the hardware, software and drivers setup for Greaseweazle and then make sure it's installed in: "C:\Program Files\Greaseweazle\gw.exe" and you're good to go!
- Windows Explorer integration for .ADF files. Double-click an .ADF file and it opens in ADF Opus 2025 with the folder that the .ADF came from in the Local File Manager window beside it! This is what originally motivated me to try and revive the original codebase from 2003!
- New disk-label editing. Now you can relabel a disk after it's been created.
- Built-in HEX Viewer to compliment the original Text Viewer.
- The Built-in Bootblock Viewer also allows you to save bootblocks as either hexidecial decoded text or in binary format.
- In addition to the original Install Bootblock feature you can also write raw boot blocks onto the disk without re-calculating any checksums. Great for learning and experimentation such as inspecting a game's custom bootblock or getting a peek at classic virus' bootblocks like the famous MegaMighty SCA Virus!
- Enriches right-click context menus with familiar file-manager commands.
- Supports OFS and FFS volumes and compressed formats (Gzip or DMS).
- Supports batch processing to convert between different Amiga disk formats. 
- UI refresh brings rich icon support, an updated toolbar, and a redesigned look and feel across the application.
- New logo design and application icon design! I tried to create a modern logo with a vintage 80's flare while at the same time honouring the original logo design language and colors!
- Removes unsupported legacy DISK2FDI. This tech requires a PC motherboard with on-board dual floppy disk drive support which is no longer supported on modern systems. This feature is replaced with support for the far more flexible and modern Greaseweazle!
- Legacy help .HLP system removed. A new modern .CHM help system has replaced it with the original context decompiled and recompiled for the modern format.
- Uses classic Amiga mouse cursor design in the file viewer windows!  
- Downloads as a (hopefully) turnkey Visual Studio 2022 solution for effortless cloning, building, and contribution.
- File Properties for Amiga files now show: Filename, Size, Date, Flags, and Comments. I really like this! File size is shown in a nice way with KB or MB instead of just raw byte size, as well as showing the date in a nice format like this: August 2, 2025, 10:52:18 PM for example. Also made the layout a little nicer looking with a that's more consistent with the rest of the program's dialog boxes. Cool!
- Options: You can set defaults for: new disk image files, and new labels for disk image files. Also made an option to enable or disable Auto Horizontal Tile which I personally really like but I'm sure others would like the option. There's also an option for Activate Pane on Hover which means the window under the mouse automatically becomes active when the mouse hovers over it. 
- Additional warnings when you try to overwrite files.
- Added a warning before writing to disk with Greaseweazle to let the user make sure they know they are about to blow away a physical floppy disk! Brings up a yes/no dialog box for confirmation.
- The Greaseweazle Create Floppy Disk dialog box is now not as wide and looks a little better!
- When using Greaseweazle it updates the status bar to let you know that it's running. This functionality also purposely prevents you from using the program, so you don't make any changes while a disk is being written. Also, this allows the program to wait for reading a disk, so that it can automatically open the disk image in ADF Opus. 
- There was a bug where if you rename a file and then try to copy it the program crashes. However, if you rename a file and then refresh and then try to copy it then it works. This bug is fixed now! The view is automatically refreshed whenever a file is renamed.
- There was another bug: If you double click on the empty area of the list view the program opens the last file that had been clicked on. These weird quirky UI issues are much better now! There are several fixes that should smooth over the experience!
- Dynamic context menus. Now if you click a file, a folder, or the empty background in the file list view, and you right-click, you'll get a different more focused menu showing you only the tasks you can do with the selected item. Cool!
- New Text Viewer! This one now has a Word Wrap feature so you can turn it on for files that do not have hard line breaks in them. You can still resize the window to whatever size you like!

I consider this to be all the essentials I wanted to cover in updating this app for 2025! Woot! I hope you find using ADF Opus 2025 to be as enjoyable as I do! It's not perfect but it's got you covered.

I hope managing Amiga disk images just a little bit easier for you!
