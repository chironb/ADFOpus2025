Welcome to ADF Opus 2025!

-----

Original Version: Copyright © 1998-2002 Dan Sutherland && Gary Harris
New Version:      Copyright © 2025 Chiron Bramberger
Website:          https://github.com/chironb/ADFOpus2025
Licence:          GNU GPL v2

-----

ADF Opus is a free, Open-Source GPL v2 licenced, Windows application that lets you open, browse, and manage Amiga .ADF disk-images natively without an emulator. With Greaseweazle support built-in you can read and write to real floppy disks with the compatible hardware and software. This new 2025 edition by Chiron Bramberger is a refresh and revive of this application for Windows users in 2025!

New features in the 2025 edition include:

    The power to copy, rename, delete and transfer files directly between disk images and your PC’s filesystem.
    Greaseweazle floppy read/write integration! Read and write your .ADF images to and from real floppy disks using the Greaseweazle hardware and software solution! Make sure you've got the hardware, software and drivers setup for Greaseweazle and then make sure it's installed in: "C:\Program Files\Greaseweazle\gw.exe" and you're good to go!
    Windows Explorer integration for .ADF files. Double-click an .ADF file and it opens in ADF Opus 2025 with the folder that the .ADF came from in the Local File Manager window beside it! This is what originally motivated me to try and revive the original codebase from 2003!
    New disk-label editing. Now you can relabel a disk after it's been created.
    Built-in HEX Viewer to compliment the original Text Viewer.
    The Built-in Bootblock Viewer also allows you to save bootblocks as either hexidecial decoded text or in binary format.
    In addition to the original Install Bootblock feature you can also write raw boot blocks onto the disk without re-calculating any checksums. Great for learning and experimentation such as inspecting a game's custom bootblock or getting a peek at classic virus' bootblocks like the famous MegaMighty SCA Virus!
    Enriches right-click context menus with familiar file-manager commands.
    Supports OFS and FFS volumes and compressed formats (Gzip or DMS).
    Supports batch processing to convert between different Amiga disk formats.
    UI refresh brings rich icon support, an updated toolbar, and a redesigned look and feel across the application.
    New logo design and application icon design! I tried to create a modern logo with a vintage 80's flare while at the same time honouring the original logo design language and colors!
    Removes unsupported legacy DISK2FDI. This tech requires a PC motherboard with on-board dual floppy disk drive support which is no longer supported on modern systems. This feature is replaced with support for the far more flexible and modern Greaseweazle!
    Legacy help .HLP system removed. A new modern .CHM help system has replaced it with the original context decompiled and recompiled for the modern format.
    Uses classic Amiga mouse cursor design in the file viewer windows!
    Downloads as a (hopefully) turnkey Visual Studio 2022 solution for effortless cloning, building, and contribution.
    File Properties for Amiga files now show: Filename, Size, Date, Flags, and Comments. I really like this! File size is shown in a nice way with KB or MB instead of just raw byte size, as well as showing the date in a nice format like this: August 2, 2025, 10:52:18 PM for example. Also made the layout a little nicer looking with a that's more consistent with the rest of the program's dialog boxes. Cool!
    Options: You can set defaults for: new disk image files, and new labels for disk image files. Also made an option to enable or disable Auto Horizontal Tile which I personally really like but I'm sure others would like the option. There's also an option for Activate Pane on Hover which means the window under the mouse automatically becomes active when the mouse hovers over it.
    Additional warnings when you try to overwrite files.
    Added a warning before writing to disk with Greaseweazle to let the user make sure they know they are about to blow away a physical floppy disk! Brings up a yes/no dialog box for confirmation.
    The Greaseweazle Create Floppy Disk dialog box is now not as wide and looks a little better!
    When using Greaseweazle it updates the status bar to let you know that it's running. This functionality also purposely prevents you from using the program, so you don't make any changes while a disk is being written. Also, this allows the program to wait for reading a disk, so that it can automatically open the disk image in ADF Opus.
    There was a bug where if you rename a file and then try to copy it the program crashes. However, if you rename a file and then refresh and then try to copy it then it works. This bug is fixed now! The view is automatically refreshed whenever a file is renamed.
    There was another bug: If you double click on the empty area of the list view the program opens the last file that had been clicked on. These weird quirky UI issues are much better now! There are several fixes that should smooth over the experience!

I consider this to be all the essentials I wanted to cover in updating this app for 2025! Woot! I hope you find using ADF Opus 2025 to be as enjoyable as I do! It's not perfect but it's got you covered.

I hope managing Amiga disk images just a little bit easier for you!

----- 

This software was originally released and licenced under the:
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991 
https://web.archive.org/web/20250626140938/https://adfopus.sourceforge.net/index/license.htm

The original help file in .HLP format was decompiled and ported to the modern .CHM format with help from the HelpDecoGui2 project found here:
https://sourceforge.net/projects/helpdecogui2/

Additional help was providing by this excellent guide:
https://help-info.de/wayback/en/Help_Info_WinHelp/hw_converting.htm

What follows below is the original contents of the "About" dialog box from the last original release of v1.2 in the early 2000's. 

-----

ADF Opus v1.2

Copyright © 1998-2002 Dan Sutherland && Gary Harris.

ADF Opus comes with ABSOLUTELY NO WARRANTY.

This is free software, and you are welcome to redistribute
if under certain conditions; see help for details.

For updates visit the ADF Opus web page:
http://adfopus.sourceforge.net/

Any comments, ideas of bug reports? 
Mail the author. 
mailto:gharris@zip.com.au

----- About -----

ADFlib is a portable, GPLed implementation of the Amiga filesystem written by Laurent Clévy. For more information visit the ADFlib homepage:

http://perso.club-internet.fr/lclevy/adflib/

This copy of ADF Opus i susing ADFlib version 0.7.9d.
17 November, 2002

----- ADFLib -----

ADF Opus uses xDMS 1.3 for its DMS decompression facility.
xDMS is a public domain decompression utility for Diskmasher,
a disk cruncher popular on the Amiga computer, written by
André R. de la Rocha

xDMS uses source code fragments or informations from the following
public domain or freeware programs:
Unix  LHA  by Masaru Oki
LZHUF  by Haruyasu Yoshizaki
testdms  by  Bjorn Stenberg

The xDMS program and source code are available on Aminet.

----- xDMS -----

ADF Opus uses ZLib 1.1.4 for its GZip compression support. 
ZLib is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
ZLib is a freely available, general purpose data compression library 
with extensions available to provide access to GZip and Zip formats.
You can find ZLib at:

http://www.cdrom.com/pub/infozip/zlib/

----- ZLib -----

The Credits (in order of appearance)
Dan Sutherland                     Idea, design, code
Laurent Clévy                      Amiga filesystem code (ADFLib), ideas and help
Gary Harris                        Help file, bug reports, ideas, more help and more code
André R. de la Rocha               DMS conversion routines (xDMS)
James Thomas                       The ADF Opus logo
Jean-loup Gailly and Mark Adler    ZLib

----- Greets -----

A few shouts go out to:
Lt. Col. Arif Majothi III
Director General Kenneth D. Barker
Drug-free American Brian Moss
Professional blag artist Naseer
Rock 'n' Roll Star Dean Stott
The original South Yorkshire Mass Murderer Craig Lowe
Amiga emu heroes Bernd, Petter, Brian, Mathias and all the rest
Nitro 9, Jaybee, The Man, PJ and all the other Amiga site maintainers
The Jester, Negative Action, Polo Breath, Whiskey and all the ex-Killers
The man with real teeth and a false head: Ali Owrak
All-round dodgy geezer Bobby Shah
Wise-old-man-trapped-in-the-body-of-a-little-girl Angela Webster

-----

Good Work Fella!