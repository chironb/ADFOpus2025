# ADFOpus2025

![ADF Opus 2025 Logo](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/ADF_Opus_2025_Logo.png?raw=true)

ADF Opus is a free, Open-Source GPL v2 licensed, Windows application that lets you open, browse, and manage Amiga .ADF disk-images natively without an emulator. With Greaseweazle support built-in you can read and write to real floppy disks with the compatible hardware and software. This new 2025 edition by Chiron Bramberger is a refresh and revive of this application for Windows users in 2025!

If you'd like to support development you can check the ADF Opus tip page on my store: 
https://www.chiron-studios.com/products/adf-opus-2025

![ADF Opus 2025 Screenshot 1](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/readme.png?raw=true)

![ADF Opus 2025 Screenshot 2](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/readme2.png?raw=true)

![ADF Opus 2025 Screenshot 3](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/readme3.png?raw=true)

![ADF Opus 2025 Screenshot 4](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/readme4.png?raw=true)

![ADF Opus 2025 Screenshot 5](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/readme5.png?raw=true)

![ADF Opus 2025 Screenshot 6](https://raw.githubusercontent.com/chironb/ADFOpus2025/refs/heads/main/readme6.png?raw=true)

New features in the 2025 edition include: 
- The power to copy, rename, delete and transfer files directly between disk images and your PC’s filesystem.
- Greaseweazle floppy read/write integration! Read and write your .ADF images to and from real floppy disks using the Greaseweazle hardware and software solution! Make sure you've got the hardware, software and drivers setup for Greaseweazle and then make sure it's installed in: "C:\Program Files\Greaseweazle\gw.exe" and you're good to go!
- Windows Explorer integration for .ADF files. Double-click an .ADF file and it opens in ADF Opus 2025 with the folder that the .ADF came from in the Local File Manager window beside it! This is what originally motivated me to try and revive the original codebase from 2003!
- New disk-label editing. Now you can relabel a disk after it's been created.
- Built-in HEX Viewer to compliment the original Text Viewer.
- The Built-in Bootblock Viewer also allows you to save bootblocks as either hexadecimal decoded text or in binary format.
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
- Added custom sound design for alerts and warnings and confirmation! There's a happy confirmation sound and a grumpy "warning this might be bad a thing" sound. Also updated Preferences so that you can disable all sounds if you'd like!
- Added a new option in Preferences that enables or disables the warning before you cut a new floppy with Greaseweazle.
- ENHANCE: The Windows dialog box for Properties looks similar to the Amiga one with the added ability to change date and time as well!
- UPDATE: All the copy functions for files and folders and all the combinations between Amiga and Windows now preserve the dates! This is super cool for archival purposes! It's also cool because you can now edit a disk in almost every way, from filenames, flash, comments, and dates, as well as disk labels themselves, all within ADF Opus! I'm super pumped about this!
- Changed the Amiga Properties dialog so that the OK button should effectively be a CANCEL button and APPLY should do what OK does now! That way you don't accidentally make changes just pressing OK.
- Updated the Amiga file list view so that the flags match the look of AmigaDOS Shell like this: Default Flags: ----rwed All Flags: hsparwed
- OMG Dates work now! Yeah that's right - the date display properly! New folders get the right date, the list view shows the right date, and the Properties dialog shows the right date! And you can edit the date and it gets applied back to the file! This took HOURS to sort out and I had to create some files on a disk on a real Amiga with the correctly set date in order to confirm that everything was reading, displaying, and writing the correct date. As well I needed to make sure creating new folders created the correct date as well! Wow! This was worth it though!
- Properties dialog box for Amiga file systems now let's you set a custom date and time that updates the time and date on the file!
- Folder dates are preserved! I like this feature! I don't care! I think all file managers should support the manual editing of file dates in some way!
- FIXED: BUG: Copying a folder from an Amiga filesystem to another Amiga filesystem looses the comments and flags! This is fixed and now those are retained.
- TWEAK: Tightened up a small layout issue. The button at the bottom of the Preferences box wasn't centered. Now it's all good!
- Updated file listing to support Windows Style with separate files and folders or all mixed and sorted together like on a Mac. There's also an option for it in Preferences!
- FIXED: Sorting now works like you would expect on a Windows Explorer based computer and the ADF files are not broken out into their own group.
- USABILITY ENHANCEMENT: If you right click on a file and pick Greaseweazle Write (to make a floppy) the drop down menu in the Greaseweazle dialog should automatically have picked the Amiga disk image you right clicked on as the default choice from all the currently open Amiga disk images. You can still change it if you want to.
- When opening new Amiga disk images, if that image has already been opened previously, then an alert is shown and the already open window is brought to the front. This prevents two or more windows being open of the same disk image. Which is confusing and could lead to accidental problems.
- Added the ability to double-click on an .adf file in a Windows List View and instead of before where it would open a whole new instance of the program, it will now just open that .adf disk image and a new Amiga List View opens up within the program! Also made the Windows List View show the .adf files with their own icon that's different from the file icon so that you know you can double-click to open up and work with those Amiga disk images! Woot!
- Update file list view to show dates! Also updated the size to condense it into KB or MB or GB depending on file size.
- Copying files now preserves their date when going from anything to anything! I think this make the most sense from an archival perspective. Properties for Windows files were updated to show the size and date. Layout now matches the Amiga Properties dialog box.
- FIXED: No longer crashed when opening disk images with files set to NOT READABLE. The program used to check to see if a file is set to be NOT READABLE and if it was it would return NULL and end the whole program. However... You can set that flag yourself... and then the whole program ends! So as much as it would be cool to pretend that we are an Amiga OS trying to honour the flags... that makes no sense. This is a disk image file management tool. How can you manage a disk when you can't even open it? This is fixed.
- Now when you copy from one Amiga file system directly to another, all the file metadata is preserved in the copy: Comments, Flags, Date and Time. Before when you were copying files directly from one Amiga disk image to another, the file comments, date and time, and even flags weren't copied. This shouldn't happen because I would expect an Amiga to Amiga copy to preserve all that metadata. Additionally, since there is no way as of yet to move a file to another folder within an Amiga disk image without moving it to another filesystem or disk image first, then you especially want to preserve that metadata because it's the only way to reorganize a disk's file system!

I consider this to be all the essentials I wanted to cover in updating this app for 2025! Woot! I hope you find using ADF Opus 2025 to be as enjoyable as I do! It's not perfect but it's got you covered.

I hope managing Amiga disk images just a little bit easier for you!
