#ifndef _ADF_STR_H
#define _ADF_STR_H 1

/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	adf_str.h
 *  \brief	Structures/constants definitions header.
 */

#include<stdio.h>

#include"adf_defs.h"
#include"adf_blk.h"
#include"adf_err.h"

/* ----- VOLUME ----- */

/*! \brief Volume Struct
 *
 *	If vol is one Volume structure returned by adfMount() :\n
 *	\li The devType is vol->dev->devType.
 *	\li The dosType is OFS or FFS (exclusive) and may have the DIRCACHE and INTERNATIONAL modes enabled.
 *	Use isFFS(vol->dosType), isOFS(), isINTL() and isDIRCACHE() to determine it.
 *
 *	<b> Warning! Even if isINTL() returns FALSE, if isDIRCACHE() is TRUE, the Volume is considered
 *	(as with AmigaDOS) as having the international mode enabled! </b>
 */
struct Volume {
    struct Device* dev;					/*!< The parent device. */

    SECTNUM firstBlock;					/*!< First block of data area (from beginning of device).			*/
    SECTNUM lastBlock;					/*!< Last block of data area  (from beginning of device).			*/
    SECTNUM rootBlock;					/*!< Root block (from firstBlock)									*/

    char dosType;						/*!< FFS/OFS, DIRCACHE, INTERNATIONAL.								*/
    BOOL bootCode;						/*!< Boot code flag. TRUE if a floppy is bootable.					*/
    BOOL readOnly;						/*!< Read-only flag.												*/
    int datablockSize;				    /*!< 488 or 512.													*/
    int blockSize;						/*!< 512.															*/

    char *volName;						/*!< Volume name.													*/

    BOOL mounted;						/*!< Mount flag.													*/

    long bitmapSize;					/*!< Bitmap size in blocks, excluding the bitmapExtension blocks.	*/
    SECTNUM *bitmapBlocks;				/*!< Bitmap blocks pointer, (excluding bitmap extensions blocks).	*/
    struct bBitmapBlock **bitmapTable;	/*!< Pointer to an array of bitmap block structs.					*/
    BOOL *bitmapBlocksChg;				/*!< Array of bitmap block change flags. TRUE if bitmapTable[i} has
											 changed and needs to be written at bitmapBlocks[i].			*/
    SECTNUM curDirPtr;					/*!< The sector number of the current directory.					*/
};


/*! \brief Partition Struct */
struct Partition {
    long startCyl;			/*!< The partition's starting cylinder.								*/
    long lenCyl;			/*!< The partition length in cylinders.								*/
    char* volName;			/*!< The volume name.												*/
    int volType;			/*!< The volume type. FSMASK_FFS, FSMASK_INTL or FSMASK_DIRCACHE.	*/
};

/* ----- DEVICES ----- */

#define DEVTYPE_FLOPDD 		1			/*!< Double-density floppy drive.	*/
#define DEVTYPE_FLOPHD 		2			/*!< High-density floppy drive.		*/
#define DEVTYPE_HARDDISK 	3			/*!< Hard disk.						*/
#define DEVTYPE_HARDFILE 	4			/*!< Hard file.						*/

/*! \brief Device Struct */
struct Device {
    int devType;						/*!< DEVTYPE_FLOPDD, DEVTYPE_FLOPHD or DEVTYPE_HARDDISK.	*/
    BOOL readOnly;						/*!< Read-only flag.										*/
    long size;							/*!< Device size in bytes. Limited to 4GB.					*/

    int nVol;							/*!< The number of partitions.								*/
    struct Volume** volList;  			/*!< A pointer to an array of volume structs.				*/
	
    long cylinders;						/*!< Disk geometry - number of cylinders.					*/
    long heads;							/*!< Disk geometry - number of read/write heads.			*/
    long sectors;						/*!< Disk geometry - number of sectors per track.			*/

    BOOL isNativeDev;  					/*!< Native device flag.									*/
    void *nativeDev;  					/*!< A pointer to a native device.							*/
};


/* ----- FILE ----- */

/*! \brief File Struct */
struct File {
    struct Volume *volume;  			/*!< The parent volume.														*/

    struct bFileHeaderBlock* fileHdr;  	/*!< The file's header block.												*/
    void *currentData;  				/*!< A pointer to the current data block (cast to the appropriate type).	*/
    struct bFileExtBlock* currentExt;  	/*!< The file's extension block.											*/

    long nDataBlock;  					/*!< The number of data blocks occupied by the file.						*/
    SECTNUM curDataPtr;  				/*!< A pointer to the current data block.									*/
    unsigned long pos;  				/*!< Position within the file.												*/

    int posInDataBlk;  					/*!< Position within the current data block.								*/
    int posInExtBlk;  					/*!< Position within the current extension block.							*/
    BOOL eof;  							/*!< End of file flag. Use adfEndOfFile().									*/
    BOOL writeMode;  					/*!< Write mode flag. TRUE if adfOpenFile() was called with "mode" = "w".	*/
    };


/* ----- ENTRY ---- */

/*! \brief Entry Struct */
struct Entry{
    int type;					/*!< The entry type: ST_DIR or ST_FILE.			*/
    char* name;  				/*!< Name.										*/
    SECTNUM sector;  			/*!< Sector at which the entry is located.		*/
    SECTNUM real;  				/*!< Real size of the entry.					*/
    SECTNUM parent;  			/*!< Location of the entry's parent directory.	*/
    char* comment;  			/*!< An optional comment.						*/
    unsigned long size;  		/*!< File size. 0 for a directory.				*/
	long access;  				/*!< RWEDAPSH access flags.						*/
    int year;  					/*!< Date. */
	int month;  				/*!< Date. */
	int days;  					/*!< Date. */
    int hour;			  		/*!< Time. */
	int mins;			  		/*!< Time. */
	int secs;			  		/*!< Time. */
};

/*! \brief Cache Entry Struct */
struct CacheEntry{
    long header;  				/*!< Entry block.										*/
	long size;  				/*!< File size. 0 for a directory.						*/
	long protect;  				/*!< Protection flags - RWEDAPSH.						*/
    short days;					/*!< Date.												*/
	short mins;					/*!< Time.												*/
	short ticks;  				/*!< 1/50 second.										*/
    signed char type;  			/*!< ST_DIR, ST_FILE, ST_LFILE, ST_LDIR or ST_LSOFT.	*/
    char nLen;					/*!< Name length: >=1, <=30.							*/
	char cLen;  				/*!< Comment length: >=0, <=79.							*/
    char name[MAXNAMELEN+1];	/*!< Entry name.										*/
	char comm[MAXCMMTLEN+1];  	/*!< Entry comment.										*/
/*    char *name, *comm;*/

};



/*! \brief Date And Time Struct */
struct DateTime{
    int year;	/*!< Year.		*/
	int mon;	/*!< Month.		*/
	int day;	/*!< Day.		*/
	int hour;	/*!< Hour.		*/
	int min;	/*!< Minute.	*/
	int sec;	/*!< Second.	*/
};

/* ----- ENVIRONMENT ----- */

#define PR_VFCT			1	/*!< Verbose message display function.			*/
#define PR_WFCT			2	/*!< Warning message display function.			*/
#define PR_EFCT			3	/*!< Error message display function.			*/
#define PR_NOTFCT		4	/*!< Directory object change notification.		*/
#define PR_USEDIRC		5	/*!< Use directory cache blocks.				*/
#define PR_USE_NOTFCT 	6	/*!< Use directory object change notification.	*/
#define PR_PROGBAR 		7	/*!< Progress bar function.						*/
#define PR_USE_PROGBAR 	8	/*!< Use progress bar.							*/
#define PR_RWACCESS 	9	/*!< Read/write access function.				*/
#define PR_USE_RWACCESS 10	/*!< Use read/write access.						*/

/*! \brief Environment Struct */
struct Env{
    void (*vFct)(char*);						/*!< Verbose callback function.					*/
    void (*wFct)(char*);						/*!< Warning callback function.					*/
    void (*eFct)(char*);						/*!< Error callback function.					*/

    void (*notifyFct)(SECTNUM, int);			/*!< Directory object change notification.		*/
    BOOL useNotify;								/*!< Use directory object change notification.	*/

    void (*rwhAccess)(SECTNUM,SECTNUM,BOOL);	/*!< Read/write access function.				*/
    BOOL useRWAccess;							/*!< Use read/write access.						*/

    void (*progressBar)(int);					/*!< Progress bar function.						*/
    BOOL useProgressBar;						/*!< Use progress bar.							*/

    BOOL useDirCache;							/*!< Use directory cache blocks.				*/
	
    void *nativeFct;							/*!< Native device access function.				*/
};



/*! \brief List Struct */
struct List{				/* generic linked tree */
    void *content;			/*!< Filled with struct Entry* type.														*/
    struct List* subdir;	/*!< If the cell content is a dir, its entry list is stored here, else filled with NULL.	*/
    struct List* next;		/*!< Next cell.																				*/
};

/*! \brief Generic Block Struct */
struct GenBlock{
    SECTNUM sect;		/*!< Current sector.														*/
    SECTNUM parent;		/*!< Parent block.															*/
    int type;			/*!< Block type: T_HEADER, ST_ROOT, T_LIST, T_DATA or T_DIRC.				*/
    int secType;		/*!< Secondary block type: ST_DIR, ST_FILE, ST_LFILE, ST_LDIR or ST_LSOFT.	*/
    char *name;			/*!< Name. (If type = T_HEADER and secType = ST_DIR or secType = ST_FILE).	*/
};

/*! \brief File Block Struct */
struct FileBlocks{
    SECTNUM header;		/*!< Header sector location.			*/
    long nbExtens;		/*!< Number of file extension blocks.	*/
    SECTNUM* extens;	/*!< Extension sector location.			*/
    long nbData;		/*!< Number of data blocks.				*/
    SECTNUM* data;		/*!< File data.							*/
};

/*! \brief Entry Block Struct */
struct bEntryBlock {
	long	type;					/*!< 000 \n Primary block typr = T_HEADER.										*/
	long	headerKey;				/*!< 004 \n Self pointer.														*/
	long	r1[3];					/*!< RESERVED (= 0).															*/
	unsigned long	checkSum;		/*!< 014 \n Checksum.															*/
	long	hashTable[HT_SIZE];		/*!< 018 \n UNUSED (= 0).														*/
	long	r2[2];					/*!< RESERVED (= 0).															*/
	long	access;					/*!< 140 \n Protection flags (set to 0 by default).					<BR><BR>
											If MultiUser FileSystem : Owner
											<TABLE>
											<TR><TD>\b	Bit	<TD><B> If Set, Means	</B>
											<TR><TD>	0	<TD>	Delete forbidden (D)
											<TR><TD>	1	<TD>	Not executable (E)
											<TR><TD>	2	<TD>	Not writable (W)
											<TR><TD>	3	<TD>	Not readable (R)
											<TR><TD>	4	<TD>	Is archived (A)
											<TR><TD>	5	<TD>	Pure (re-entrant safe), can be made resident (P)
											<TR><TD>	6	<TD>	File is a script (Arexx or Shell) (S)
											<TR><TD>	7	<TD>	Hold bit. if H+P (and R+E) are set the file can
																	be made resident on first load (OS 2.x and 3.0)
											<TR><TD>    8	<TD>	Group (D) : is delete protected 
											<TR><TD>    9	<TD>	Group (E) : is executable 
											<TR><TD>   10	<TD>	Group (W) : is writable 
											<TR><TD>   11	<TD>	Group (R) : is readable 
											<TR><TD>   12	<TD>	Other (D) : is delete protected 
											<TR><TD>   13	<TD>	Other (E) : is executable 
											<TR><TD>   14	<TD>	Other (W) : is writable 
											<TR><TD>   15	<TD>	Other (R) : is readable 
											<TR><TD> 16-30	<TD>	Reserved
											<TR><TD>   31	<TD>	SUID, MultiUserFS Only	
											</TABLE>																*/
	long	byteSize;				/*!< 144 \n File size in bytes.													*/
	char	commLen;				/*!< 148 \n File comment length.												*/
	char	comment[MAXCMMTLEN+1];	/*!< 149 \n Comment (max. 79 chars permitted).									*/
	char	r3[91-(MAXCMMTLEN+1)];	/*!< RESERVED (= 0).															*/
	long	days;					/*!< 1a4 \n Date of last change (days since 1 jan 78).							*/
	long	mins;					/*!< 1a8 \n Time of last change (mins since midnight).							*/
	long	ticks;					/*!< 1ac \n Time of last change (1/50ths of a second since last min).			*/
	char	nameLen;				/*!< 1b0 \n Filename length.													*/
	char	name[MAXNAMELEN+1];		/*!< 1b1 \n Filename (max. 30 chars permitted).									*/
	long	r4;						/*!< RESERVED (= 0).															*/
	long	realEntry;				/*!< 1d4 UNUSED (= 0).															*/
	long	nextLink;				/*!< 1d8 \n FFS: linked list of hard link (first = newest).						*/
	long	r5[5];					/*!< RESERVED (= 0).															*/
	long	nextSameHash;			/*!< 1f0 \n Next entry with same hash.											*/
	long	parent;					/*!< 1f4 \n Parent directory.													*/
	long	extension;				/*!< 1f8 \n Pointer to first extension block.									*/
	long	secType;				/*!< 1fc \n Secondary type = ST_FILE.											*/
	};


#define ENV_DECLARATION struct Env adfEnv	/*!< The environment struct. */


#endif /* _ADF_STR_H */
/*##########################################################################*/
