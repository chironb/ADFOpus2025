/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	adf_salv.c
 *  \brief	Undeletion and recovery code.
 *
 *	\b Typical Usage \n
 *	See <A HREF="../../api_salv.html">api_salv.html</A> for an example of typical usage of the salvage functions.
 */

#include<string.h>
#include<stdlib.h>

#include "adf_salv.h"
#include "adf_bitm.h"
#include "adf_util.h"
#include "adf_disk.h"
#include "adf_dir.h"
#include "adf_file.h"
#include "adf_cache.h"

extern struct Env adfEnv;

/*
 * adfFreeGenBlock
 *
 */
void adfFreeGenBlock(struct GenBlock* block)
{
    if (block->name!=NULL)
        free(block->name);
}


/*
 * adfFreeDelList
 */
/*!	\brief	Free the list of deleted entries.
 *	\param	list - the list to free.
 *	\return	Void.
 */
void adfFreeDelList(struct List* list)
{
    struct List *cell;

    cell = list;
    while(cell!=NULL) {
        adfFreeGenBlock((struct GenBlock*)cell->content);
        cell = cell->next;
    }
    freeList(list);
}


/*
 * adfGetDelEnt
 */
/*!	\brief	Find deleted entries.
 *	\param	vol - the volume to search.
 *	\return	A pointer to a list structure containing the deleted files' names. NULL otherwise.
 *
 *	Returns a list of the files/directories that MIGHT be undeleted. The entries must be checked before attempting
 *	undeletion using adfCheckEntry()!
 *
 *	\b Internals \n
 *	Scans all the blocks of the volume to find directory blocks and file header blocks that are not allocated in
 *	the bitmap.
 *	\sa See adfFreeDelList() to free the list. \n
 *	See adfCheckEntry() to check if the entry can be undeleted.
 */
struct List* adfGetDelEnt(struct Volume *vol)
{
    struct GenBlock *block;
    long i;
    struct List *list, *head;
    BOOL delEnt;

    list = head = NULL;
    block = NULL;
    delEnt = TRUE;
    for(i=vol->firstBlock; i<=vol->lastBlock; i++) {
        if (adfIsBlockFree(vol, i)) {
            if (delEnt) {
                block = (struct GenBlock*)malloc(sizeof(struct GenBlock));
                if (!block) return NULL;

#ifdef _DEBUG_PRINTF_
				printf("%p\n",block);
#endif /*_DEBUG_PRINTF_*/

            }

            adfReadGenBlock(vol, i, block);

            delEnt = (block->type==T_HEADER 
                && (block->secType==ST_DIR || block->secType==ST_FILE) );

            if (delEnt) {
                if (head==NULL)
                    list = head = newCell(NULL, (void*)block);
                else
                    list = newCell(list, (void*)block);
            }
        }
    }

    if (block!=NULL && list!=NULL && block!=list->content) {
        free(block);

#ifdef _DEBUG_PRINTF_
	    printf("%p\n",block);
#endif /*_DEBUG_PRINTF_*/

    }
    return head;
}


/*
 * adfReadGenBlock
 *
 */
RETCODE adfReadGenBlock(struct Volume *vol, SECTNUM nSect, struct GenBlock *block)
{
	unsigned char buf[LOGICAL_BLOCK_SIZE];
    int len;
    char name[MAXNAMELEN+1];

	if (adfReadBlock(vol, nSect, buf)!=RC_OK)
		return RC_ERROR;

    block->type =(int) swapLong(buf);
    block->secType =(int) swapLong(buf+vol->blockSize-4);
    block->sect = nSect;
    block->name = NULL;

    if (block->type==T_HEADER) {
        switch(block->secType) {
        case ST_FILE:
        case ST_DIR:
        case ST_LFILE:
        case ST_LDIR:
            len = min(MAXNAMELEN, buf[vol->blockSize-80]);
            strncpy(name, buf+vol->blockSize-79, len);
            name[len] = '\0';
            block->name = strdup(name);
            block->parent = swapLong(buf+vol->blockSize-12);
            break;
        case ST_ROOT:
            break;
        default: 
            ;
        }
    }
    return RC_OK;
}


/*
 * adfCheckParent
 *
 */
RETCODE adfCheckParent(struct Volume* vol, SECTNUM pSect)
{
    struct GenBlock block;

    if (adfIsBlockFree(vol, pSect)) {
        (*adfEnv.wFct)("adfCheckParent : parent doesn't exists");
        return RC_ERROR;
    }

    /* verify if parent is a DIR or ROOT */
    adfReadGenBlock(vol, pSect, &block);
    if ( block.type!=T_HEADER 
        || (block.secType!=ST_DIR && block.secType!=ST_ROOT) ) {
        (*adfEnv.wFct)("adfCheckParent : parent secType is incorrect");
        return RC_ERROR;
    }

    return RC_OK;
}


/*
 * adfUndelDir
 *
 */
RETCODE adfUndelDir(struct Volume* vol, SECTNUM pSect, SECTNUM nSect, 
    struct bDirBlock* entry)
{
    RETCODE rc;
    struct bEntryBlock parent;
    char name[MAXNAMELEN+1];

    /* check if the given parent sector pointer seems OK */
    if ( (rc=adfCheckParent(vol,pSect)) != RC_OK)
        return rc;

    if (pSect!=entry->parent) {
        (*adfEnv.wFct)("adfUndelDir : the given parent sector isn't the entry parent");
        return RC_ERROR;
    }

    if (!adfIsBlockFree(vol, entry->headerKey))
        return RC_ERROR;
    if (isDIRCACHE(vol->dosType) && !adfIsBlockFree(vol,entry->extension))
        return RC_ERROR;

    if (adfReadEntryBlock(vol, pSect, &parent)!=RC_OK)
		return RC_ERROR;

    strncpy(name, entry->dirName, entry->nameLen);
    name[(int)entry->nameLen] = '\0';
    /* insert the entry in the parent hashTable, with the headerKey sector pointer */
    adfSetBlockUsed(vol,entry->headerKey);
    adfCreateEntry(vol, &parent, name, entry->headerKey);

    if (isDIRCACHE(vol->dosType)) {
        adfAddInCache(vol, &parent, (struct bEntryBlock *)entry);
        adfSetBlockUsed(vol,entry->extension);
    }

    adfUpdateBitmap(vol);

    return RC_OK;
}


/*
 * adfUndelFile
 *
 */
RETCODE adfUndelFile(struct Volume* vol, SECTNUM pSect, SECTNUM nSect, struct bFileHeaderBlock* entry)
{
    long i;
    char name[MAXNAMELEN+1];
    struct bEntryBlock parent;
    RETCODE rc;
    struct FileBlocks fileBlocks;

    /* check if the given parent sector pointer seems OK */
    if ( (rc=adfCheckParent(vol,pSect)) != RC_OK)
        return rc;

    if (pSect!=entry->parent) {
        (*adfEnv.wFct)("adfUndelFile : the given parent sector isn't the entry parent");
        return RC_ERROR;
    }

    adfGetFileBlocks(vol, entry, &fileBlocks);

    for(i=0; i<fileBlocks.nbData; i++)
        if ( !adfIsBlockFree(vol,fileBlocks.data[i]) )
            return RC_ERROR;
        else
            adfSetBlockUsed(vol, fileBlocks.data[i]);
    for(i=0; i<fileBlocks.nbExtens; i++)
        if ( !adfIsBlockFree(vol,fileBlocks.extens[i]) )
            return RC_ERROR;
        else
            adfSetBlockUsed(vol, fileBlocks.extens[i]);

    free(fileBlocks.data);
    free(fileBlocks.extens);

    if (adfReadEntryBlock(vol, pSect, &parent)!=RC_OK)
		return RC_ERROR;

    strncpy(name, entry->fileName, entry->nameLen);
    name[(int)entry->nameLen] = '\0';
    /* insert the entry in the parent hashTable, with the headerKey sector pointer */
    adfCreateEntry(vol, &parent, name, entry->headerKey);

    if (isDIRCACHE(vol->dosType))
        adfAddInCache(vol, &parent, (struct bEntryBlock *)entry);

    adfUpdateBitmap(vol);

    return RC_OK;
}


/*
 * adfUndelEntry
 */
/*!	\brief	Recover a deleted file or dirctory.
 *	\param	vol    - the parent volume.
 *	\param	parent - the parent directory of the entry to be recovered.
 *	\param	nSect  - the location of the target entry.
 *	\return	RC_OK or RC_ERROR.
 *
 *	Undelete a directory or a file. The parent directory of an entry must exist.
 *
 *	\b Internals \n
 *	Adds the entry's first block pointer in the parent directory list and allocates the related blocks in the bitmap. 
 */
RETCODE adfUndelEntry(struct Volume* vol, SECTNUM parent, SECTNUM nSect)
{
    struct bEntryBlock entry;

    adfReadEntryBlock(vol,nSect,&entry);

    switch(entry.secType) {
    case ST_FILE:
		// Return error if unsuccessful - GJH 22/10/02.
        if(adfUndelFile(vol, parent, nSect, (struct bFileHeaderBlock*)&entry) == RC_ERROR)
			return RC_ERROR;
        break;
    case ST_DIR:
		// Return error if unsuccessful - GJH 22/10/02.
        if(adfUndelDir(vol, parent, nSect, (struct bDirBlock*)&entry) == RC_ERROR)
			return RC_ERROR;
        break;
    default:
        ;
    }

    return RC_OK;
}


/*
 * adfCheckFile
 *
 */
RETCODE adfCheckFile(struct Volume* vol, SECTNUM nSect,
    struct bFileHeaderBlock* file, int level)
{
    struct bFileExtBlock extBlock;
    struct bOFSDataBlock dataBlock;
    struct FileBlocks fileBlocks;
    int n;
 
    adfGetFileBlocks(vol,file,&fileBlocks);

#ifdef _DEBUG_PRINTF_
	printf("data %ld ext %ld\n",fileBlocks.nbData,fileBlocks.nbExtens);
#endif /*_DEBUG_PRINTF_*/

    if (isOFS(vol->dosType)) {
        /* checks OFS datablocks */
        for(n=0; n<fileBlocks.nbData; n++) {

#ifdef _DEBUG_PRINTF_
			printf("%ld\n",fileBlocks.data[n]);
#endif /*_DEBUG_PRINTF_*/

            adfReadDataBlock(vol,fileBlocks.data[n],&dataBlock);
            if (dataBlock.headerKey!=fileBlocks.header)
                (*adfEnv.wFct)("adfCheckFile : headerKey incorrect");
            if (dataBlock.seqNum!=n+1)
                (*adfEnv.wFct)("adfCheckFile : seqNum incorrect");
            if (n<fileBlocks.nbData-1) {
                if (dataBlock.nextData!=fileBlocks.data[n+1])
                    (*adfEnv.wFct)("adfCheckFile : nextData incorrect");
                if (dataBlock.dataSize!=vol->datablockSize)
                    (*adfEnv.wFct)("adfCheckFile : dataSize incorrect");
            }
            else { /* last datablock */
                if (dataBlock.nextData!=0)
                    (*adfEnv.wFct)("adfCheckFile : nextData incorrect");
            }
        }
    }
    for(n=0; n<fileBlocks.nbExtens; n++) {
        adfReadFileExtBlock(vol,fileBlocks.extens[n],&extBlock);
        if (extBlock.parent!=file->headerKey)
            (*adfEnv.wFct)("adfCheckFile : extBlock parent incorrect");
        if (n<fileBlocks.nbExtens-1) {
            if (extBlock.extension!=fileBlocks.extens[n+1])
                (*adfEnv.wFct)("adfCheckFile : nextData incorrect");
        }
        else
            if (extBlock.extension!=0)
                (*adfEnv.wFct)("adfCheckFile : nextData incorrect");
    }

    free(fileBlocks.data);
    free(fileBlocks.extens);

    return RC_OK;
}


/*
 * adfCheckDir
 *
 */
RETCODE adfCheckDir(struct Volume* vol, SECTNUM nSect, struct bDirBlock* dir, 
    int level)
{




    return RC_OK;
}


/*
 * adfCheckEntry
 */
/*!	\brief	Check if a directory or file can be undeleted.
 *	\param	vol   - the parent volume.
 *	\param	nSect - the location of the target entry.
 *	\param	level - (unused).
 *	\return	RC_OK or RC_ERROR.
 *
 *	The "level" argument is not used yet. Could be set to 0. 
 */
RETCODE adfCheckEntry(struct Volume* vol, SECTNUM nSect, int level)
{
    struct bEntryBlock entry;
    RETCODE rc;

    adfReadEntryBlock(vol,nSect,&entry);

    switch(entry.secType) {
    case ST_FILE:
        rc = adfCheckFile(vol, nSect, (struct bFileHeaderBlock*)&entry, level);
        break;
    case ST_DIR:
        rc = adfCheckDir(vol, nSect, (struct bDirBlock*)&entry, level);
        break;
    default:

#ifdef _DEBUG_PRINTF_
        printf("adfCheckEntry : not supported\n");					/* BV *//* GJH - Conditionally re-enabled */
#endif /*_DEBUG_PRINTF_*/

        rc = RC_ERROR;
    }

    return rc;
}


/*#############################################################################*/
