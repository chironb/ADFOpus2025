/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	adf_blk.h
 *  \brief	General blocks structures header..
 */


#ifndef ADF_BLK_H
#define ADF_BLK_H 1

#define ULONG   unsigned long							/*!< ULONG.  */
#define USHORT  unsigned short							/*!< USHORT. */
#define UCHAR   unsigned char							/*!< UCHAR.  */

#define LOGICAL_BLOCK_SIZE    512						/*!< Logical Block. */

/* ----- FILE SYSTEM ----- */

#define FSMASK_FFS         1							/*!< Fast File System.				*/
#define FSMASK_INTL        2							/*!< International File System.		*/
#define FSMASK_DIRCACHE    4							/*!< Directory Cache File System.	*/

#define isFFS(c)           ((c)&FSMASK_FFS)				/*!< Check if FFS.   */
#define isOFS(c)           (!((c)&FSMASK_FFS))			/*!< Check if OFS.   */
#define isINTL(c)          ((c)&FSMASK_INTL)			/*!< Check if IFS.   */
#define isDIRCACHE(c)      ((c)&FSMASK_DIRCACHE)		/*!< Check if DCFS.  */


/* ----- ENTRIES ----- */

/* access constants */

#define ACCMASK_D	(1<<0)								/*!< Deletable.	*/
#define ACCMASK_E	(1<<1)								/*!< Erasable.	*/
#define ACCMASK_W	(1<<2)								/*!< Writable.	*/
#define ACCMASK_R	(1<<3)								/*!< Readable.	*/
#define ACCMASK_A	(1<<4)								/*!< Archive.	*/
#define ACCMASK_P	(1<<5)								/*!< Pure.		*/
#define ACCMASK_S	(1<<6)								/*!< Script.	*/
#define ACCMASK_H	(1<<7)								/*!< Hold Bit.	*/

#define hasD(c)    ((c)&ACCMASK_D)						/*!< Check if deletable.	*/
#define hasE(c)    ((c)&ACCMASK_E)						/*!< Check if erasable.		*/
#define hasW(c)    ((c)&ACCMASK_W)						/*!< Check if writable.		*/
#define hasR(c)    ((c)&ACCMASK_R)						/*!< Check if readable.		*/
#define hasA(c)    ((c)&ACCMASK_A)						/*!< Check if archive.		*/
#define hasP(c)	   ((c)&ACCMASK_P)						/*!< Check if pure.			*/
#define hasS(c)    ((c)&ACCMASK_S)						/*!< Check if a script.		*/
#define hasH(c)    ((c)&ACCMASK_H)						/*!< Check if hold bit set.	*/


/* ----- BLOCKS ----- */

/* block constants */

#define BM_VALID	-1									/*!< Valid bitmap.			*/				
#define BM_INVALID	0									/*!< Invalid bitmap.		*/

#define HT_SIZE		72									/*!< Hash table size.		*/
#define BM_SIZE     25									/*!< Bitmap size.			*/
#define MAX_DATABLK	72									/*!< Max datablock size.	*/

#define MAXNAMELEN	30									/*!< Max name length.		*/
#define MAXCMMTLEN	79									/*!< Max comment length.	*/


/* block primary and secondary types */

#define T_HEADER	2									/*!< Header block type.						*/
#define ST_ROOT		1									/*!< Root secondary block type.				*/
#define ST_DIR		2									/*!< Directory secondary block type.		*/
#define ST_FILE		-3									/*!< File secondary block type.				*/
#define ST_LFILE	-4									/*!< File link secondary block type.		*/
#define ST_LDIR		4									/*!< Directory link secondary block type.	*/
#define ST_LSOFT	3									/*!< Soft link secondary block type.		*/
#define T_LIST		16									/*!< List block type.						*/
#define T_DATA		8									/*!< Data block type.						*/
#define T_DIRC		33									/*!< Directory cache block type.			*/


/*--- blocks structures --- */

/*! \brief Boot Block struct */
struct bBootBlock {
	char	dosType[4];		/*!< 000 \n 'D''O''S' + flags (FSMASK_FFS, FSMASK_INTL, FSMASK_DIRCACHE).	*/
	ULONG	checkSum;		/*!< 004 \n Checksum.														*/
	long	rootBlock;		/*!< 008 \n Rootblock (= 880 for DD and HD).								*/
	UCHAR	data[500+512];	/*!< 00c \n Bootblock code.													*/
};

/*! \brief Root Block struct */
struct bRootBlock {
	long	type;					/*!< 000 \n Primary block type = T_HEADER.								*/
	long	headerKey;				/*!< UNUSED (= 0).														*/
	long	highSeq;				/*!< UNUSED (= 0).														*/
	long	hashTableSize;			/*!< 00c \n Hash table size (= BSIZE/4 - 56). For floppy disk = 0x48.	*/
	long	firstData;				/*!< UNUSED (= 0).														*/
	ULONG	checkSum;				/*!< 014 \n Rootblock checksum.											*/
	long	hashTable[HT_SIZE];		/*!< 018 \n Hash table (entry block number) = (BSIZE/4) - 56.
												For floppy disk = 72 longwords								*/
	long	bmFlag;					/*!< 138 \n Bitmap flag. -1 means VALID									*/
	long	bmPages[BM_SIZE];		/*!< 13c \n Bitmap block pointers.										*/
	long	bmExt;					/*!< 1a0 \n First bitmap extension block (hard disks only).				*/
	long	cDays;					/*!< 1a4 \n Filesystem creation date: days since 1 jan 78.				*/
	long	cMins;					/*!< 1a8 \n Filesystem creation time: minutes past midnight.			*/
	long	cTicks;					/*!< 1ac \n Filesystem creation time: 1/50 sec past last minute.		*/
	char	nameLen;				/*!< 1b0 \n Volume name length.											*/
	char 	diskName[MAXNAMELEN+1];	/*!< 1b1 \n Volume name.												*/
	char	r2[8];					/*!< RESERVED (= 0).													*/
	long	days;					/*!< 1d8 \n Last root alteration date: days since 1 jan 78.				*/
	long	mins;					/*!< 1dc \n Hours and minutes in minutes past midnight.					*/
	long	ticks;					/*!< 1e0 \n Ticks (1/50 sec) past last minute.							*/
	long	coDays;					/*!< 1e4 \n Last disk alteration date: days since 1 jan 78.				*/
	long	coMins;					/*!< 1e8 \n Hours and minutes in minutes past midnight.					*/
	long	coTicks;				/*!< 1ec \n Ticks (1/50 sec) past last minute.							*/
	long	nextSameHash;			/*!< UNUSED (= 0).														*/
	long	parent;					/*!< UNUSED (= 0).														*/
	long	extension;				/*!< 1f8 \n FFS: first directory cache block, 0 otherwise.				*/
	long	secType;				/*!< 1fc \n Block secondary type = ST_ROOT.								*/
};


/*! \brief File Header Block struct */
struct bFileHeaderBlock {
	long	type;						/*!< 000 \n Primary block type = T_HEADER.									*/
	long	headerKey;					/*!< 004 \n Current block number.											*/
	long	highSeq;					/*!< 008 \n Number of data blocks in this header block.						*/
	long	dataSize;					/*!< 00c \n UNUSED (= 0).													*/
	long	firstData;					/*!< 010 \n Pointer to first data block.									*/
	ULONG	checkSum;					/*!< 014 \n Checksum.														*/
	long	dataBlocks[MAX_DATABLK];	/*!< 018 \n Data block pointers (first at BSIZE-204). Size = (BSIZE/4)-56.	*/
	long	r1;							/*!< 138 \n RESERVED (= 0).													*/
	long	r2;							/*!< 13c \n RESERVED (= 0).													*/
	long	access;						/*!< 140 \n Protection flags (set to 0 by default).					<BR><BR>
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
	unsigned long	byteSize;			/*!< 144 \n File size in bytes.												*/
	char	commLen;					/*!< 148 \n File comment length.											*/
	char	comment[MAXCMMTLEN+1];		/*!< 149 \n Comment (max. 79 chars permitted).								*/
	char	r3[91-(MAXCMMTLEN+1)];		/*!< RESERVED (= 0).														*/
	long	days;						/*!< 1a4 \n Date of last change (days since 1 jan 78).						*/
	long	mins;						/*!< 1a8 \n Time of last change (mins since midnight).						*/
	long	ticks;						/*!< 1ac \n Time of last change (1/50ths of a second since last min).		*/
	char	nameLen;					/*!< 1b0 \n Filename length.												*/
	char	fileName[MAXNAMELEN+1];		/*!< 1b1 \n Filename (max. 30 chars permitted).								*/
	long	r4;							/*!< RESERVED (= 0).														*/
	long	real;						/*!< 1d4 \n UNUSED (= 0).													*/
	long	nextLink;					/*!< 1d8 \n FFS: linked list of hard link (first = newest).				*/
	long	r5[5];						/*!< RESERVED (= 0).														*/
	long	nextSameHash;				/*!< 1f0 \n Next entry with same hash.										*/
	long	parent;						/*!< 1f4 \n Parent directory.												*/
	long	extension;					/*!< 1f8 \n Pointer to first extension block.								*/
	long	secType;					/*!< 1fc \n Secondary type = ST_FILE.										*/
};


/*--- file header extension block structure ---*/

/*! \brief File Header Extension Block struct */
struct bFileExtBlock {
	long	type;						/*!< 000 \n Primary block type = T_LIST.									*/
	long	headerKey;					/*!< 004 \n Self pointer.													*/
	long	highSeq;					/*!< 008 \n Number of data block pointers stored.							*/
	long	dataSize;					/*!< 00c \n UNUSED (= 0).													*/
	long	firstData;					/*!< 010 \n UNUSED (= 0).													*/
	ULONG	checkSum;					/*!< 014 \n Checksum.														*/
	long	dataBlocks[MAX_DATABLK];	/*!< 018 \n Data block pointer (first at BSIZE-204). Size = (BSIZE/4) - 56.	*/
	long	r[45];						/*!< RESERVED.																*/
	long	info;						/*!< UNUSED (= 0).															*/
	long	nextSameHash;				/*!< UNUSED (= 0).															*/
	long	parent;						/*!< 1f4 \n File header block.												*/
	long	extension;					/*!< 1f8 \n Next file header extension block. 0 for the last.				*/
	long	secType;					/*!< 1fc \n Secondary block type = ST_FILE.									*/
};


/*! \brief Directory Block struct */
struct bDirBlock {
	long	type;					/*!< 000 \n Primary block type = T_HEADER.										*/
	long	headerKey;				/*!< 004 \n Self pointer.														*/
	long	highSeq;				/*!< 008 \n UNUSED (= 0).														*/
	long	hashTableSize;			/*!< 00c \n UNUSED (= 0).														*/
	long	r1;						/*!< RESERVED (= 0).															*/
	ULONG	checkSum;				/*!< 014 \n Checksum.															*/
	long	hashTable[HT_SIZE];		/*!< 018 \n Hash table (entry block number). Size = (BSIZE/4) - 56. For
												floppy disks, size = 72.											*/
	long	r2[2];					/*!< RESERVED.																	*/
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
	long	r4;						/*!< RESERVED (= 0).															*/
	char	commLen;				/*!< 148 \n Directory comment length.											*/
	char	comment[MAXCMMTLEN+1];	/*!< 149 \n Comment (max. 79 chars permitted).									*/
	char	r5[91-(MAXCMMTLEN+1)];	/*!< UNUSED (= 0).																*/
	long	days;					/*!< 1a4 \n Last access date (days since 1 jan 78).								*/
	long	mins;					/*!< 1a8 \n Last access time (mins since midnight).								*/
	long	ticks;					/*!< 1ac \n Last access time (1/50ths of a second since last min).				*/
	char	nameLen;				/*!< 1b0 \n Directory name length.												*/
	char 	dirName[MAXNAMELEN+1];	/*!< 1b1 \n Directory name (max. 30 chars permitted).							*/
	long	r6;						/*!< RESERVED (=0).																*/
	long	real;					/*!< 1d4 UNUSED (=0).															*/
	long	nextLink;				/*!< 1d8 \n FFS: linked list of hard links (first = newest).					*/
	long	r7[5];					/*!< RESERVED (=0).																*/
	long	nextSameHash;			/*!< 1f0 \n Next entry pointer with same hash.									*/
	long	parent;					/*!< 1f4 \n Parent directory.													*/
	long	extension;				/*!< 1f8 \n FFS: first directory cache block.									*/
	long	secType;				/*!< 1fc \n secondary type = ST_DIR.											*/
};


/*! \brief Old File System Data Block struct */
struct bOFSDataBlock{
	long	type;		/*!< 000 \n Primary block type = T_DATA.			*/ 
	long	headerKey;	/*!< 004 \n Pointer to file header block.			*/
	long	seqNum;		/*!< 008 \n File data block number (first is 1).	*/
	long	dataSize;	/*!< 00c \n Data size (<= BSIZE-24).				*/
	long	nextData;	/*!< 010 \n Next data block (0 for last).			*/
	ULONG	checkSum;	/*!< 014 \n Checksum.								*/
	UCHAR	data[488];	/*!< 018 \n File data size (<= BSIZE-24).			*/
};						/*!< 200											*/


/* --- bitmap --- */

/*! \brief Bitmap Block struct */
struct bBitmapBlock {
	ULONG	checkSum;	/*!< 000 \n Checksum.	*/
	ULONG	map[127];	/*!< 004 \n Map.		*/
	};


/*! \brief Bitmap Extension Block struct */
struct bBitmapExtBlock {
	long	bmPages[127];	/*!< 000 \n Bitmap block pointers.		*/
	long	nextBlock;		/*!< 1fc \n Next block (0 for last).	*/
	};


/*! \brief Link Block struct */
struct bLinkBlock {
	long	type;				/*!< 000 \n Primary block type = T_HEADER.											*/
	long	headerKey;			/*!< 004 \n Self pointer.															*/
	long	r1[3];				/*!< RESERVED (= 0).																*/
	ULONG	checkSum;			/*!< 014 \n Checksum.																*/
	char	realName[64];		/*!< 018 \n	Hard Link: UNUSED (= 0). Size = (BSIZE/4) - 54. For floppy disk = 74.\n
											Soft Link: Path name to referenced object. Size = (BSIZE - 224) - 1).
                							For floppy disk = 288 - 1 chars.										*/
	long	r2[83];				/*!< RESERVED (= 0).																*/
	long	days;				/*!< 1a4 \n Last access date (days since 1 jan 78).									*/
	long	mins;				/*!< 1a8 \n Last access time (mins since midnight).									*/
	long	ticks;				/*!< 1ac \n Last access time (1/50ths of a second since last min).					*/
	char	nameLen;			/*!< 1b0 \n Link name length.														*/
	char 	name[MAXNAMELEN+1];	/*!< 1b1 \n Link name.																*/
	long	r3;					/*!< RESERVED (= 0).																*/
	long	realEntry;			/*!< 1d4 \n Hard Link: FFS: pointer to "real" file or directory. \n
											Soft Link: UNUSED (= 0).												*/
	long	nextLink;			/*!< 1d8 \n Hard Link: FFS : linked list of hardlinks (first = newest). \n
											Soft Link: UNUSED (= 0).												*/
	long	r4[5];				/*!< RESERVED (= 0).																*/
	long	nextSameHash;		/*!< 1f0 \n Next entry ptr with same hash.											*/
	long	parent;				/*!< 1f4 \n Parent directory.														*/
	long	r5;					/*!< RESERVED (= 0).																*/
	long	secType;			/*!< 1fc \n Secondary block type: \n
											Hard Link: ST_LINKFILE or T_LINKDIR. \n
											Soft Link: ST_SOFTLINK.													*/
	};



/*--- directory cache block structure ---*/

/*! \brief Directory Cache Block struct */
struct bDirCacheBlock {
	long	type;				/*!< 000 \n T_DIRC.											*/
	long	headerKey;			/*!< 004 \n Self pointer.									*/
	long	parent;				/*!< 008 \n Parent directory.								*/
	long	recordsNb;			/*!< 00c \n Numbe of directory entry records in this block. */
	long	nextDirC;			/*!< 010 \n Directory cache linked list.					*/
	ULONG	checkSum;			/*!< 014 \n Checksum.										*/
	unsigned char records[488];	/*!< 018 \n List of entries. Size = BSIZE-24.				*/
	};


#endif /* ADF_BLK_H */
/*##########################################################################*/
