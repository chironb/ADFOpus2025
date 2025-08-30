/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	adf_link.c
 *  \brief	Read entry name functions.
 */

#include<string.h>

#include"adf_defs.h"
#include"adf_str.h"
#include"adf_link.h"
#include"adf_dir.h"

extern struct Env adfEnv;

/*
 *
 *
 */
char* path(struct Volume *vol, SECTNUM parent)
{
    struct bEntryBlock entryBlk;
    char *tmpPath;
    int len;

    tmpPath = NULL;
    adfReadEntryBlock(vol, parent, &entryBlk);
    len = min(entryBlk.nameLen, MAXNAMELEN);
    memcpy(tmpPath,entryBlk.name,len);
    tmpPath[len]='\0';
/*    if (entryBlk.parent!=vol->rootBlock) {
        return(strcat(path(vol,entryBlk.parent), tmpPath));
    }
    else
   */     return(tmpPath);
}


/*
 *	adfBlockPtr2EntryName
 */
/*!	\brief	Read an entry name given a block pointer.
 *	\param	vol   - the parent volume.
 *	\param	nSect - the location of the target entry.
 *	\param	lPar  - the location of the entry's parent directory.
 *	\param	name  - a pointer to a string to receive the entry name.
 *	\param	size  - a pointer to a long to receive the entry size in bytes.
 *	\return	RC_OK or RC_MALLOC.
 */
RETCODE adfBlockPtr2EntryName(struct Volume *vol, SECTNUM nSect, SECTNUM lPar, 
	char **name, long *size)
{
    struct bEntryBlock entryBlk;
    struct Entry entry;

    if (*name==0) {
        adfReadEntryBlock(vol, nSect, &entryBlk);
        *size = entryBlk.byteSize;
return RC_OK;
        adfEntBlock2Entry(&entryBlk, &entry);	/*error*/

#ifdef _DEBUG_PRINTF_
        if (entryBlk.secType!=ST_ROOT && entry.parent!=lPar)
            printf("path=%s\n",path(vol,entry.parent));
#endif /*_DEBUG_PRINTF_*/

       *name = strdup("");
        if (*name==NULL)
            return RC_MALLOC;
        return RC_OK;
    }
    else

    return RC_OK;
}

/*##################################################################################*/
