/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	adf_disk.c
 *  \brief	Logical disk/volume code.
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "adf_str.h"
#include "adf_disk.h"
#include "adf_raw.h"
#include "adf_hd.h"
#include "adf_bitm.h"
#include "adf_util.h"
#include "adf_nativ.h"
#include "adf_dump.h"
#include "adf_err.h"
#include "adf_cache.h"

extern struct Env adfEnv;

unsigned long bitMask[32] = { 
    0x1, 0x2, 0x4, 0x8,
	0x10, 0x20, 0x40, 0x80,
    0x100, 0x200, 0x400, 0x800,
	0x1000, 0x2000, 0x4000, 0x8000,
	0x10000, 0x20000, 0x40000, 0x80000,
	0x100000, 0x200000, 0x400000, 0x800000,
	0x1000000, 0x2000000, 0x4000000, 0x8000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000 };



/*!	\brief	Install a bootblock on a floppy disk (ADF).
 *	\param	vol  - the volume to make bootable.
 *	\param	code - the bootblock to install.
 *	\return	RC_OK or RC_ERROR.
 *
 *	Install a bootblock on a floppy disk. Won't work on any other device. You must provide the 1024 bytes
 *	large bootblock. Doesn't modify the initial 'DOS' header and dosType. Recalculates the checksum. 
 *	A valid AmigaDOS 3 bootblock can be found in the Boot subdirectory in the file stdboot3.bbk.
 */
RETCODE adfInstallBootBlock(struct Volume *vol, unsigned char* code)
{
    int i;
    struct bBootBlock boot;

    if (vol->dev->devType!=DEVTYPE_FLOPDD && vol->dev->devType!=DEVTYPE_FLOPHD)
        return RC_ERROR;

    if (adfReadBootBlock(vol, &boot)!=RC_OK)
        return RC_ERROR;

    boot.rootBlock = 880;
    for(i=0; i<1024-12; i++)         /* bootcode */
        boot.data[i] = code[i+12];

    if (adfWriteBootBlock(vol, &boot)!=RC_OK)
		return RC_ERROR;

	vol->bootCode = TRUE;

    return RC_OK;
}


/*
 * isSectNumValid
 */
/*!	\brief	Determine whether a sector is valid.
 *	\param	vol   - the parent volume.
 *	\param	nSect - the target sector.
 *	\return	TRUE or FALSE.
 */	
BOOL isSectNumValid(struct Volume *vol, SECTNUM nSect)
{
    return( 0<=nSect && nSect<=(vol->lastBlock - vol->firstBlock) );
}	
	


/*
 * adfVolumeInfo
 */
/*!	\brief	Get information about a volume.
 *	\param	vol - the volume in question. 
 *	\return	Void.
 *
 *	This function uses console output and should only be used in a console application. See adfVolumeInfoWin in
 *	Lib/Win32/adfvolinfo.c for a Windows version that uses message boxes to display the volume data. 
 */
void adfVolumeInfo(struct Volume *vol)
{
	struct bRootBlock root;
	char diskName[35];
	int days,month,year;
	
	if (adfReadRootBlock(vol, vol->rootBlock, &root)!=RC_OK)
		return;
	
	memset(diskName, 0, 35);
	memcpy(diskName, root.diskName, root.nameLen);
	
	printf ("Name : %-30s\n",vol->volName);
	printf ("Type : ");
	switch(vol->dev->devType) {
		case DEVTYPE_FLOPDD:
			printf ("Floppy Double Density : 880 KBytes\n");
			break;
		case DEVTYPE_FLOPHD:
			printf ("Floppy High Density : 1760 KBytes\n");
			break;
		case DEVTYPE_HARDDISK:
			printf ("Hard Disk partition : %3.1f KBytes\n", 
				(vol->lastBlock - vol->firstBlock +1) * 512.0/1024.0);
			break;
		case DEVTYPE_HARDFILE:
			printf ("HardFile : %3.1f KBytes\n", 
				(vol->lastBlock - vol->firstBlock +1) * 512.0/1024.0);
			break;
		default:
			printf ("Unknown devType!\n");
	}
	printf ("Filesystem : ");
	printf("%s ",isFFS(vol->dosType) ? "FFS" : "OFS");
	if (isINTL(vol->dosType))
		printf ("INTL ");
	if (isDIRCACHE(vol->dosType))
		printf ("DIRCACHE ");
	putchar('\n');

    printf("Free blocks = %ld\n", adfCountFreeBlocks(vol));
    if (vol->readOnly)
        printf("Read only\n");
    else
        printf("Read/Write\n");
 	
    /* created */
	adfDays2Date(root.coDays, &year, &month, &days);
    printf ("created %d/%02d/%02d %ld:%02ld:%02ld\n",days,month,year,
	    root.coMins/60,root.coMins%60,root.coTicks/50);	
	adfDays2Date(root.days, &year, &month, &days);
    printf ("last access %d/%02d/%02d %ld:%02ld:%02ld,   ",days,month,year,
	    root.mins/60,root.mins%60,root.ticks/50);	
	adfDays2Date(root.cDays, &year, &month, &days);
    printf ("%d/%02d/%02d %ld:%02ld:%02ld\n",days,month,year,
	    root.cMins/60,root.cMins%60,root.cTicks/50);	
}



/*
 * adfMount
 */
/*!	\brief	Mount a volume.
 *	\param	dev      - The device containing the volume to mount.
 *	\param	nPart    - The volume number to mount.
 *	\param	readOnly - TRUE if read only access is desired, FALSE otherwise.
 *	\return	The Volume, NULL in case of error.
 *
 *	Mounts a designated volume (nPart) of the Device (dev), with specified read/write access (readOnly).
 *	The first partition is #0. The current working directory is the root block. 
 *
 *	\b Internals \n
 *	1. Read the bootblock to determine vol->dosType and vol->datablockSize. \n
 *	2. Read the rootblock, fills vol->curDirPtr. \n
 *	3. Read and allocate the bitmap : vol->bitmapBlocks[], vol->bitmapTable[], vol->bitmapSize, vol->bitmapBlocksChg[]. \n
 */
struct Volume* adfMount( struct Device *dev, int nPart, BOOL readOnly )
{
    long nBlock;
    struct bRootBlock root;
	struct bBootBlock boot;
	struct Volume* vol;

    if (dev==NULL || nPart<nPart || nPart >= dev->nVol) {
        (*adfEnv.eFct)("adfMount : invalid parameter(s)");
        return NULL;
    }

    vol = dev->volList[nPart];
	vol->dev = dev;
    vol->mounted = TRUE;

#ifdef _DEBUG_PRINTF_
	printf("first=%ld last=%ld root=%ld\n",vol->firstBlock, vol->lastBlock, vol->rootBlock);
#endif /*_DEBUG_PRINTF_*/

    if (adfReadBootBlock(vol, &boot)!=RC_OK) {
        (*adfEnv.wFct)("adfMount : BootBlock invalid");
        return NULL;
    }       
    
	vol->dosType = boot.dosType[3];
	if (isFFS(vol->dosType))
		vol->datablockSize=512;
	else
		vol->datablockSize=488;

    if (dev->readOnly /*|| isDIRCACHE(vol->dosType)*/)
       vol->readOnly = TRUE;
    else
       vol->readOnly = readOnly;
	   	
	if (adfReadRootBlock(vol, vol->rootBlock, &root)!=RC_OK) {
        (*adfEnv.wFct)("adfMount : RootBlock invalid");       
        return NULL;
    }

    nBlock = vol->lastBlock - vol->firstBlock +1 - 2;

	adfReadBitmap( vol, nBlock, &root );
    vol->curDirPtr = vol->rootBlock;

#ifdef _DEBUG_PRINTF_
	printf("blockSize=%d\n",vol->blockSize);
#endif /*_DEBUG_PRINTF_*/

    return( vol );
}


/*
 *	adfUnMount
 */
/*!	\brief	Dismount a volume.
 *	\param	vol - the volume to dismount.
 *	\return	Void.
 *
 *	Release a Volume. Free the bitmap structures. Free the current directory.
 */
void adfUnMount(struct Volume *vol)
{
	if (!vol) {
		(*adfEnv.eFct)("adfUnMount : vol is null");
        return;
    }

    adfFreeBitmap(vol);

    vol->mounted = FALSE;
	
}



/*
 * adfCreateVol
 *
 * 
 */
struct Volume* adfCreateVol( struct Device* dev, long start, long len, 
    char* volName, int volType )
{
    struct bBootBlock boot;
    struct bRootBlock root;
    SECTNUM blkList[2];
    struct Volume* vol;
    int nlen;
#ifdef _DEBUG_PRINTF_
	int	i;
#endif /*_DEBUG_PRINTF_*/


    if (adfEnv.useProgressBar)
        (*adfEnv.progressBar)(0);

    vol=(struct Volume*)malloc(sizeof(struct Volume));
    if (!vol) { 
		(*adfEnv.eFct)("adfCreateVol : malloc vol");
        return NULL;
    }
	
    vol->dev = dev;
    vol->firstBlock = (dev->heads * dev->sectors)*start;
    vol->lastBlock = (vol->firstBlock + (dev->heads * dev->sectors)*len)-1;
    vol->rootBlock = (vol->lastBlock - vol->firstBlock+1)/2;

#ifdef _DEBUG_PRINTF_
	printf("first=%ld last=%ld root=%ld\n",vol->firstBlock, vol->lastBlock, vol->rootBlock);
#endif /*_DEBUG_PRINTF_*/

    vol->curDirPtr = vol->rootBlock;

    vol->readOnly = dev->readOnly;

    vol->mounted = TRUE;

    nlen = min( MAXNAMELEN, strlen(volName) );
    vol->volName = (char*)malloc(nlen+1);
    if (!vol->volName) { 
		(*adfEnv.eFct)("adfCreateVol : malloc");
		free(vol); return NULL;
    }
    memcpy(vol->volName, volName, nlen);
    vol->volName[nlen]='\0';

    if (adfEnv.useProgressBar)
        (*adfEnv.progressBar)(25);

    memset(&boot, 0, 1024);
    boot.dosType[3] = volType;

#ifdef _DEBUG_PRINTF_
	printf("first=%d last=%d\n", vol->firstBlock, vol->lastBlock);
	printf("name=%s root=%d\n", vol->volName, vol->rootBlock);
#endif /*_DEBUG_PRINTF_*/

    if (adfWriteBootBlock(vol, &boot)!=RC_OK) {
        free(vol->volName); free(vol);
        return NULL;
    }

    if (adfEnv.useProgressBar)
        (*adfEnv.progressBar)(20);

    if (adfCreateBitmap( vol )!=RC_OK) {
        free(vol->volName); free(vol);
        return NULL;
    }

    if (adfEnv.useProgressBar)
        (*adfEnv.progressBar)(40);

#ifdef _DEBUG_PRINTF_
	for(i=0; i<127; i++)
		printf("%3d %x, ",i,vol->bitmapTable[0]->map[i]);
#endif /*_DEBUG_PRINTF_*/

    if ( isDIRCACHE(volType) )
        adfGetFreeBlocks( vol, 2, blkList );
    else
        adfGetFreeBlocks( vol, 1, blkList );

#ifdef _DEBUG_PRINTF_
	printf("[0]=%d [1]=%d\n",blkList[0],blkList[1]);
#endif /*_DEBUG_PRINTF_*/

    memset(&root, 0, LOGICAL_BLOCK_SIZE);

    if (strlen(volName)>MAXNAMELEN)
        volName[MAXNAMELEN]='\0';
    root.nameLen = strlen(volName);
    memcpy(root.diskName,volName,root.nameLen);
    adfTime2AmigaTime(adfGiveCurrentTime(),&(root.coDays),&(root.coMins),&(root.coTicks));

    /* dircache block */
    if ( isDIRCACHE(volType) ) {
        root.extension = 0L;
        root.secType = ST_ROOT; /* needed by adfCreateEmptyCache() */
        adfCreateEmptyCache(vol, (struct bEntryBlock*)&root, blkList[1]);
    }

    if (adfEnv.useProgressBar)
        (*adfEnv.progressBar)(60);

    if (adfWriteRootBlock(vol, blkList[0], &root)!=RC_OK) {
        free(vol->volName); free(vol);
        return NULL;
    }

   /* fills root->bmPages[] and writes filled bitmapExtBlocks */
    if (adfWriteNewBitmap(vol)!=RC_OK)
		return NULL;

    if (adfEnv.useProgressBar)
        (*adfEnv.progressBar)(80);

    if (adfUpdateBitmap(vol)!=RC_OK)
		return NULL;

    if (adfEnv.useProgressBar)
        (*adfEnv.progressBar)(100);

#ifdef _DEBUG_PRINTF_
	printf("free blocks %ld\n",adfCountFreeBlocks(vol));
#endif /*_DEBUG_PRINTF_*/

    /* will be managed by adfMount() later */
    adfFreeBitmap(vol);

    vol->mounted = FALSE;

    return(vol);
}


/*-----*/

/*
 * adfReadBlock
 */
/*!	\brief	Read a logical block.
 *	\param	vol   - the parent volume.
 *	\param	nSect - the location of the target block.
 *	\param	buf   - a buffer to receive the read data.
 *	\return	RC_OK or RC_ERROR.
 */
RETCODE adfReadBlock(struct Volume* vol, long nSect, unsigned char* buf)
{
#ifdef _DEBUG_PRINTF_
	char strBuf[80];
#endif /*_DEBUG_PRINTF_*/

    long pSect;
    struct nativeFunctions *nFct;
    RETCODE rc;

    if (!vol->mounted) {
        (*adfEnv.eFct)("the volume isn't mounted, adfReadBlock not possible");
        return RC_ERROR;
    }

    /* translate logical sect to physical sect */
    pSect = nSect+vol->firstBlock;

    if (adfEnv.useRWAccess)
        (*adfEnv.rwhAccess)(pSect,nSect,FALSE);

#ifdef _DEBUG_PRINTF_
	printf("psect=%ld nsect=%ld\n",pSect,nSect);
    sprintf(strBuf,"ReadBlock : accessing logical block #%ld", nSect);	
	(*adfEnv.vFct)(strBuf);
#endif /*_DEBUG_PRINTF_*/

    if (pSect<vol->firstBlock || pSect>vol->lastBlock) {
        (*adfEnv.wFct)("adfReadBlock : nSect out of range");
        
    }

#ifdef _DEBUG_PRINTF_
	printf("pSect R =%ld\n",pSect);
#endif /*_DEBUG_PRINTF_*/

    nFct = adfEnv.nativeFct;
    if (vol->dev->isNativeDev)
        rc = (*nFct->adfNativeReadSector)(vol->dev, pSect, 512, buf);
    else
        rc = adfReadDumpSector(vol->dev, pSect, 512, buf);

#ifdef _DEBUG_PRINTF_
	printf("rc=%ld\n",rc);
#endif /*_DEBUG_PRINTF_*/

    if (rc!=RC_OK)
        return RC_ERROR;
    else
        return RC_OK;
}


/*
 * adfWriteBlock
 */
/*!	\brief	Write data to a logical block.
 *	\param	vol   - the parent volume.
 *	\param	nSect - the location of the target block.
 *	\param	buf   - a buffer containing to the data to write.
 *	\return	RC_OK or RC_ERROR.
 */
RETCODE adfWriteBlock(struct Volume* vol, long nSect, unsigned char *buf)
{
    long pSect;
    struct nativeFunctions *nFct;
    RETCODE rc;

    if (!vol->mounted) {
        (*adfEnv.eFct)("the volume isn't mounted, adfWriteBlock not possible");
        return RC_ERROR;
    }

    if (vol->readOnly) {
        (*adfEnv.wFct)("adfWriteBlock : can't write block, read only volume");
        return RC_ERROR;
    }

    pSect = nSect+vol->firstBlock;

#ifdef _DEBUG_PRINTF_
	printf("write nsect=%ld psect=%ld\n",nSect,pSect);
#endif /*_DEBUG_PRINTF_*/

    if (adfEnv.useRWAccess)
        (*adfEnv.rwhAccess)(pSect,nSect,TRUE);
 
    if (pSect<vol->firstBlock || pSect>vol->lastBlock) {
        (*adfEnv.wFct)("adfWriteBlock : nSect out of range");
    }

    nFct = adfEnv.nativeFct;

#ifdef _DEBUG_PRINTF_
	printf("nativ=%d\n",vol->dev->isNativeDev);
#endif /*_DEBUG_PRINTF_*/

    if (vol->dev->isNativeDev)
        rc = (*nFct->adfNativeWriteSector)(vol->dev, pSect, 512, buf);
    else
        rc = adfWriteDumpSector(vol->dev, pSect, 512, buf);

    if (rc!=RC_OK)
        return RC_ERROR;
    else
        return RC_OK;
}



/*#######################################################################################*/
