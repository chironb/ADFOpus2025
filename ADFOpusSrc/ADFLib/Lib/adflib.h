#ifndef ADFLIB_H
#define ADFLIB_H 1

/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	adflib.h
 *  \brief	Include file.
 *
 *	Declaring _DEBUG_PRINTF_ in your compiler's preprocessor definitions for ADFLib will enable
 *	debug console output. This declaration is not made by default to avoid console problems
 *	for those not using the console.\n
 *	Please do not remove the special formatting characters contained within comment blocks
 *	as you will break the Doxygen documentation. These include exclamation marks, less than
 *	symbols and anything prefixed by a backslash.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Visual C++ DLL specific, define  WIN32DLL or not in the makefile */

#ifdef WIN32DLL
#define PREFIX __declspec(dllimport)
#else
#define PREFIX 
#endif /* WIN32DLL */

#include "adf_defs.h"
#include "adf_str.h"

/* util */
PREFIX struct List* newCell(struct List* list, void* content);
PREFIX void freeList(struct List* list);

/* dir */
PREFIX RETCODE adfToRootDir(struct Volume *vol);
PREFIX RETCODE adfCreateDir(struct Volume* vol, SECTNUM parent, char* name);
PREFIX RETCODE adfChangeDir(struct Volume* vol, char *name);
PREFIX RETCODE adfParentDir(struct Volume* vol);
PREFIX RETCODE adfRemoveEntry(struct Volume *vol, SECTNUM pSect, char *name);
PREFIX struct List* adfGetDirEnt(struct Volume* vol, SECTNUM nSect );
PREFIX struct List* adfGetRDirEnt(struct Volume* vol, SECTNUM nSect, BOOL recurs );
PREFIX void printEntry(struct Entry* entry);
PREFIX void adfFreeDirList(struct List* list);
PREFIX void adfFreeEntry(struct Entry *);
PREFIX RETCODE adfRenameEntry(struct Volume *vol, SECTNUM, char *old,SECTNUM, char *pNew);	/* BV */
PREFIX RETCODE adfSetEntryAccess(struct Volume*, SECTNUM, char*, long);
PREFIX RETCODE adfSetEntryComment(struct Volume*, SECTNUM, char*, char*);

/* file */
PREFIX long adfFileRealSize(unsigned long size, int blockSize, long *dataN, long *extN);
PREFIX struct File* adfOpenFile(struct Volume *vol, char* name, char *mode);
PREFIX void adfCloseFile(struct File *file);
PREFIX long adfReadFile(struct File* file, long n, unsigned char *buffer);
PREFIX BOOL adfEndOfFile(struct File* file);
PREFIX long adfWriteFile(struct File *file, long n, unsigned char *buffer);
PREFIX void adfFlushFile(struct File *file);
PREFIX void adfFileSeek(struct File *file, unsigned long pos);

/* volume */
PREFIX RETCODE adfInstallBootBlock(struct Volume *vol,unsigned char*);
PREFIX struct Volume* adfMount( struct Device *dev, int nPart, BOOL readOnly );
PREFIX void adfUnMount(struct Volume *vol);
PREFIX void adfVolumeInfo(struct Volume *vol);

/* device */
PREFIX void adfDeviceInfo(struct Device *dev);
PREFIX struct Device* adfMountDev( char* filename,BOOL ro);
PREFIX void adfUnMountDev( struct Device* dev);
PREFIX RETCODE adfCreateHd(struct Device* dev, int n, struct Partition** partList );
PREFIX RETCODE adfCreateFlop(struct Device* dev, char* volName, int volType );
PREFIX RETCODE adfCreateHdFile(struct Device* dev, char* volName, int volType);

/* dump device */
PREFIX struct Device* adfCreateDumpDevice(char* filename, long cyl, long heads, long sec);

/* env */
PREFIX void adfEnvInitDefault();
PREFIX void adfEnvCleanUp();
PREFIX void adfChgEnvProp(int prop, void *pNew);											/* BV */
PREFIX char* adfGetVersionNumber();
PREFIX char* adfGetVersionDate();
/* obsolete */
PREFIX void adfSetEnvFct( void(*e)(char*), void(*w)(char*), void(*v)(char*) );

/* link */
PREFIX RETCODE adfBlockPtr2EntryName(struct Volume *, SECTNUM, SECTNUM,char **, long *);

/* salv */
PREFIX struct List* adfGetDelEnt(struct Volume *vol);
PREFIX RETCODE adfUndelEntry(struct Volume* vol, SECTNUM parent, SECTNUM nSect);
PREFIX void adfFreeDelList(struct List* list);
PREFIX RETCODE adfCheckEntry(struct Volume* vol, SECTNUM nSect, int level);

/* middle level API */

PREFIX BOOL isSectNumValid(struct Volume *vol, SECTNUM nSect);

/* low level API */

PREFIX RETCODE adfReadBlock(struct Volume* , long nSect, unsigned char* buf);
PREFIX RETCODE adfWriteBlock(struct Volume* , long nSect, unsigned char* buf);
PREFIX long adfCountFreeBlocks(struct Volume* vol);


#ifdef __cplusplus
}
#endif

#endif /* ADFLIB_H */
/*##########################################################################*/

/*******************************/
/* Main page for doxygen docs. */
/*******************************/

/*! \mainpage ADF Library
 *
 * ADFlib is a free, portable and open implementation of the Amiga filesystem.
 * ADFLib is copyright © 1997-2002 Laurent Clévy and made available under the
 * terms of the <A HREF="../../license.txt">GNU General Public License</A>.
 *
 * \author Laurent Clévy - ADFLib.\n
 * Dan Sutherland - WinNT native driver.\n
 * Gary Harris - Doxygen documentation, V0.7.9b code, V0.7.9c & V0.7.9d updates.\n
 * Bjark Viksoe - Enhancements for V0.7.9c.
 * \version 0.7.9d
 * \date    17 November, 2002
 *
 * <b> Web Pages: </b>\n
 * <A HREF="http://perso.club-internet.fr/lclevy/adflib">http://perso.club-internet.fr/lclevy/adflib</A>
 *
 * \section intro Introduction
 * (Laurent's README doc, somewhat out of date.)
 * \include README
 *
 * \section additional Additional Documentation
 * These links are relative and will only work if this documentation remains in its default location within
 * the source tree.
 *
 * \subsection sub1 1. Laurent's ADFLib API HTML Docs 
 * <A HREF="../../api_device.html">Device</A> : create and mount/unmount harddisk, floppydisk, .ADF, hardfiles.\n
 * <A HREF="../../api_volume.html">Volume</A> : create and mount/unmount volumes.\n
 * <A HREF="../../api_dir.html">Directory</A> : list, create, change of directories.\n
 * <A HREF="../../api_file.html">File</A> : create, read and write files.\n
 * <A HREF="../../api_env.html">Environment</A> : initialize and shutdown the library, control its behaviour.\n
 * <A HREF="../../api_salv.html">Salvage</A> : undeletion, recovery.\n
 * <A HREF="../../api_native.html">Native API</A> : how to write real devices drivers on your OS.\n
 *  
 * \subsection sub2 2. Laurent's Old ADFLib API text file
 * <A HREF="../../API.txt">API.txt</A> : The original ADFLib API text document.\n
 * 
 * \subsection sub3 3. Changes
 * <A HREF="../../../CHANGES">CHANGES</A> : Changes to ADFLib.
 * 
 * \subsection sub4 4. The .ADF (Amiga Disk File) format FAQ
 * <A HREF="../../../Faq/adf_info.html">adf_info.html</A> : Frequently asked questions about the ADF format.
 *	
 * \subsection sub5 5. The Amiga Floppy Disk Format
 * <A HREF="../../../Faq/adf_info_V0_9.txt">adf_info_V0_9.txt</A> : A description of the Amiga floppy disk structure.
 * 
 * \subsection sub6 6. Win32 Readme
 * <A HREF="../../../Lib/Win32/readme_win32.html">readme_win32.html</A> : Additional information about using ADFLib
 *	under 32-bit Windows.
 *
 * \section feedback Feedback
 * Feedback, corrections or suggestions regarding this documentation should be sent to
 * <A HREF="mailto:gharris@zip.com.au">gharris@zip.com.au</A>, in the first instance, or
 * <A HREF="mailto:lclevy@club-internet.fr">lclevy@club-internet.fr</A>.
 *
 * Other matters concerning the ADF Library should go to 
 * <A HREF="mailto:lclevy@club-internet.fr">lclevy@club-internet.fr</A>.
 */


