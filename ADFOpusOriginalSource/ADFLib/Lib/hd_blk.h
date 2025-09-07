/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	hd_blk.h
 *  \brief	Hard disk block structures header.
 *
 *	\sa <A HREF="../../../Faq/adf_info.html">adf_info.html</A> and
 *	<A HREF="../../../Faq/adf_info_V0_9.txt">adf_info_V0_9.txt</A> for detailed information about the Amiga
 *	floppy disk structure and the ADF format.
 */


#ifndef _HD_BLK_H
#define _HD_BLK_H 1

#include "adf_str.h"

/* ------- RDSK ---------*/

/*! \brief Rigid Disk Block Struct */
struct bRDSKblock {
	char	id[4];							/*!< 000 \n "RDSK".													*/
	long 	size;							/*!< 004 \n Size (= 64).											*/
	ULONG	checksum;						/*!< 008 \n Checksum.												*/ 
	long	hostID;							/*!< 00c \n 7 for IDE and ZIP disks.								*/
	long 	blockSize;						/*!< 010 \n Typically 512 bytes, but can be other powers of 2.		*/
	long 	flags;							/*!< 014 \n Typically 0x17.									<BR><BR>
					<TABLE>
					<TR><TD>\b	Bit	<TD><B> If Set, Means	</B>
					<TR><TD>	0	<TD>	No disks exist to be configured after this one on this controller.
					<TR><TD>	1 	<TD>	No LUNs exist to be configured greater than this one at this SCSI Target ID.
					<TR><TD>	2 	<TD>	No target IDs exist to be configured greater than this one on this SCSI bus.
					<TR><TD>	3 	<TD>	Don't bother trying to perform reselection when talking to this drive.
					<TR><TD>	4 	<TD>	Disk identification valid.
					<TR><TD>	5 	<TD>	Controller identification valid.
					<TR><TD>	6 	<TD>	Drive supports SCSI synchronous mode (can be dangerous if it doesn't).
					</TABLE>																					*/
	long 	badBlockList;					/*!< 018 \n Block pointer (-1 means last block).					*/
	long 	partitionList;					/*!< 01c \n Block pointer (-1 means last block).					*/
	long 	fileSysHdrList;					/*!< 020 \n Block pointer (-1 means last block).					*/
	long 	driveInit;						/*!< 024 \n Optional drive-specific init code.						*/
	long 	r1[6];							/*!< 028 \n RESERVED (= -1).										*/
	long 	cylinders;						/*!< 040 \n Sumber of drive cylinders.								*/
	long 	sectors;						/*!< 044 \n Sectors per track.										*/ 
	long 	heads;							/*!< 048 \n Number of drive heads.									*/
	long 	interleave;						/*!< 04c \n Interleave.												*/
	long 	parkingZone;					/*!< 050 \n Head parking cylinders.									*/
	long 	r2[3];							/*!< 054 \n RESERVED (= 0).											*/
	long 	writePreComp;					/*!< 060 \n Starting cylinder: write precompensation.				*/
	long 	reducedWrite;					/*!< 064 \n Starting cylinder: reduced write current.				*/
	long 	stepRate;						/*!< 068 \n Drive step rate.										*/
	long 	r3[5];							/*!< 06c \n RESERVED (= 0).											*/
	long 	rdbBlockLo;						/*!< 080 \n Low block of range reserved for this block.				*/
	long 	rdbBlockHi;						/*!< 084 \n High block of range reserved for this block.			*/
	long 	loCylinder;						/*!< 088 \n Low cylinder of partitionable disk area.				*/
	long 	hiCylinder;						/*!< 08c \n High cylinder of partitionable disk area.				*/
	long 	cylBlocks;						/*!< 090 \n Number of blocks available per cylinder.				*/
	long 	autoParkSeconds;				/*!< 094 \n Time required for autopark. Zero for no autopark.		*/
	long 	highRDSKBlock;					/*!< 098 \n Highest block used by RDSK.								*/
	long 	r4;								/*!< 09c \n RESERVED (= 0).											*/
	char 	diskVendor[8];					/*!< 0a0 \n Disk vendor e.g. "IOMEGA".								*/
	char 	diskProduct[16];				/*!< 0a8 \n Disk product name e.g. "ZIP 100".						*/
	char 	diskRevision[4];				/*!< 0b8 \n Disk revision number e.g. "R.41".						*/
	char 	controllerVendor[8];			/*!< 0bc \n Controller vendor.										*/
	char 	controllerProduct[16];			/*!< 0c4 \n Controller product name.								*/
	char 	controllerRevision[4];			/*!< 0d4 \n Controller revsion number.								*/
	long 	r5[10];							/*!< 0d8 \n RESERVED (= 0).											*/
};											/*!< 100															*/


/*! \brief Bad Block Entry Struct */
struct bBADBentry {
	long 	badBlock;						/*!< 000 \n Block number of bad block.			*/
	long 	goodBlock;						/*!< 004 \n Block number of replacement block.	*/
};

/*! \brief Bad Block Struct */
struct bBADBblock {
	char	id[4];						/*!< 000 \n "BADB"											*/
	long 	size;						/*!< 004 \n Size = 128 for BSIZE = 512.						*/
	ULONG	checksum;					/*!< 008 \n Checksum.										*/ 
	long	hostID;						/*!< 00c \n = 7.											*/
	long 	next;						/*!< 010 \n Next bad block.									*/
	long 	r1;							/*!< 014 \n RESERVED.										*/
	struct bBADBentry blockPairs[61];	/*!< 018 \n Bad block entry table. Size = ((BSIZE/4)-6)/2
													(for BSIZE=512 = 61*8 byte entries).			*/
};


/*! \brief Partition Block Struct */
struct bPARTblock {
	char	id[4];				/*!< 000 \n "PART"																	*/
	long 	size;				/*!< 004 \n Size of checksummed structure (= 64).									*/
	ULONG	checksum;			/*!< 008 \n Checksum.																*/
	long	hostID;				/*!< 00c \n SCSI Target ID of host (= 7).											*/
	long 	next;				/*!< 010 \n Block number of the next partition block.								*/
	long 	flags;				/*!< 014 \n Flags.															<BR><BR>
									<TABLE>
									<TR><TD>\b		Bit		<TD><B>		If Set, Means	</B>
									<TR><TD>		0		<TD>		This partition is bootable.
									<TR><TD>		1		<TD>		No automount.		
									</TABLE>																		*/
	long 	r1[2];				/*!< 018 \n RESERVED.																*/
	long 	devFlags;			/*!< 020 \n Preferred flags for OpenDevice.											*/
	char 	nameLen;			/*!< 024 \n Length of drive name (e.g. '3').										*/
	char 	name[31];			/*!< 025 \n Drive name e.g. "DH0".													*/
	long 	r2[15];				/*!< 044 \n RESERVED.																*/

	long 	vectorSize;			/*!< 080 \n Vector size. Often 16. 11 is the minimal value.							*/
	long 	blockSize;			/*!< 084 \n Block size (= 128 for BSIZE = 512).										*/
	long 	secOrg;				/*!< 088 \n Originating sector.														*/
	long 	surfaces;			/*!< 08c \n Number of drive heads (surfaces).										*/
	long 	sectorsPerBlock;	/*!< 090 \n Sectors per block (= 1).												*/
	long 	blocksPerTrack;		/*!< 094 \n Blocks per track.														*/
	long 	dosReserved;		/*!< 098 \n DOS reserved blocks at start of partition, usually = 2 (minimum 1).		*/
	long 	dosPreAlloc;		/*!< 09c \n DOS reserved blocks at end of partition, normally set to 0.				*/
	long 	interleave;			/*!< 0a0 \n Interleave (= 0).														*/
	long 	lowCyl;				/*!< 0a4 \n First cylinder of a partition.											*/
	long 	highCyl;			/*!< 0a8 \n Last cylinder of a partition.											*/
	long 	numBuffer;			/*!< 0ac \n Number of buffers, often 30.											*/
	long 	bufMemType;			/*!< 0b0 \n Type of memory to allocate for buffers (= 0).							*/
	long 	maxTransfer;		/*!< 0b4 \n Max number of type to transfer at a time, often 0x7fff ffff.			*/
	long 	mask;				/*!< 0b8 \n Address mask to block out certain memory, often 0xffff fffe.			*/
	long 	bootPri;			/*!< 0bc \n Boot priority for autoboot.												*/
	char 	dosType[4];			/*!< 0c0 \n Dos Type. "DOS" and the FFS/OFS flag only. \n\n Also, \n
											"UNI"\0 = AT&T SysV filesystem;\n
											"UNI"\1 = UNIX boot filesystem;\n
											"UNI"\2 = BSD filesystem for SysV;\n
											"resv" = reserved (swap space).											*/
	long 	r3[15];				/*!< 0c4 \n RESERVED.																*/
};

/*! \brief LoadSeg Block Struct
 *	This block contains filesystem code. It isn't needed to reach partitions. 
 */
struct bLSEGblock {
	char	id[4];				/*!< 000 \n "LSEG".														*/
	long 	size;				/*!< 004 \n Size of this checksummed structure (= BSIZE/4 = 128).		*/
	ULONG	checksum;			/*!< 008 \n Checksum.													*/
	long	hostID;				/*!< 00c \n SCSI Target ID of host (often 7).							*/
	long 	next;				/*!< 010 \n Block number of the next LoadSegBlock (-1 for the last).	*/
	char 	loadData[123*4];	/*!< 014 \n Code stored like an executable, with relocation hunks. 
											Size = ((BSIZE/4) - 5).										*/
};


/*! \brief File System Header Block Struct
 *	This block contains information on how to lauch the task which will manage the filesystem.
 *	You don't need it to reach partitions. 
 */
struct bFSHDblock {
	char	id[4];			/*!< 000 \n "FSHD".															*/
	long 	size;			/*!< 004 \n Size (= 64).													*/
	ULONG	checksum;		/*!< 008 \n Checksum.														*/
	long	hostID;			/*!< 00c \n SCSI Target ID of host (often 7).								*/
	long 	next;			/*!< 010 \n Block number of next FileSysHeaderBlock.						*/
	long 	flags;			/*!< 014 \n Flags.															*/
	long 	r1[2];			/*!< 018 \n RESERVED.														*/
	char 	dosType[4];		/*!< 020: "DOS" and OFS/FFS DIRCACHE INTL bits.								*/
	short 	majVersion;		/*!< 024 \n Filesystem major version number. 0x0027001b == 39.27.			*/
	short 	minVersion;		/*!< 026 \n Filesystem minor version number.								*/
	long 	patchFlags;		/*!< 028 \n Bits set for any of the following items that need to be
										substituted into a standard device node for this filesystem
										e.g. 0x180 to substitute SegList and GlobalVec.					*/

	long 	type;			/*!< 02c \n Device node type (= 0).											*/
	long 	task;			/*!< 030 \n Standard DOS "task" field (= 0).								*/
	long 	lock;			/*!< 034 \n Not used (= 0).													*/
	long 	handler;		/*!< 038 \n Filename to loadseg (= 0).										*/
	long 	stackSize;		/*!< 03c \n Stack size to use when starting task (=0).						*/
	long 	priority;		/*!< 040 \n Task priority when starting task (= 0).							*/
	long 	startup;		/*!< 044 \n Startup message (= 0).											*/
	long 	segListBlock;	/*!< 048 \n First of linked list of LoadSegBlocks. Note that this entry
										requires some processing before substitution.					*/
	long 	globalVec;		/*!< 04c \n BCPL global vector when starting task (= -1).					*/
	long 	r2[23];			/*!< 050 \n RESERVED.														*/
	long 	r3[21];			/*!< 0ac \n RESERVED.														*/
};


#endif /* _HD_BLK_H */
/*##########################################################################*/
