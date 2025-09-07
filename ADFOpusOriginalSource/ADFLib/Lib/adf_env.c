/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	adf_env.c
 *  \brief	Environment routines.
 *
 *	\b Typical Usage \n
 *	See <A HREF="../../api_env.html">api_env.html</A> for an example of typical usage of the environment functions.
 */

#include<stdio.h>
#include<stdlib.h>

#include"adf_defs.h"
#include"adf_str.h"
#include"adf_nativ.h"
#include"adf_env.h"

#include"defendian.h"

union u{
    long l;
    char c[4];
    };

ENV_DECLARATION;

void rwHeadAccess(SECTNUM physical, SECTNUM logical, BOOL write)
{
    /* display the physical sector, the logical block, and if the access is read or write */

    fprintf(stderr, "phy %ld / log %ld : %c\n", physical, logical, write ? 'W' : 'R');
}

void progressBar(int perCentDone)
{
    fprintf(stderr,"%d %% done\n",perCentDone);
}

void Warning(char* msg) {
    fprintf(stderr,"Warning <%s>\n",msg);
}

void Error(char* msg) {
    fprintf(stderr,"Error <%s>\n",msg);
/*    exit(1);*/
}

void Verbose(char* msg) {
    fprintf(stderr,"Verbose <%s>\n",msg);
}

void Changed(SECTNUM nSect, int changedType)
{
    switch(changedType) {
    case ST_FILE:
        fprintf(stderr,"Notification : sector %ld (FILE)\n",nSect);
        break;
    case ST_DIR:
        fprintf(stderr,"Notification : sector %ld (DIR)\n",nSect);
        break;
    case ST_ROOT:
        fprintf(stderr,"Notification : sector %ld (ROOT)\n",nSect);
        break;
    default:
        fprintf(stderr,"Notification : sector %ld (???)\n",nSect);
    }
}

/*
 * adfEnvInitDefault
 */
/*!	\brief	Initialise the library data structures and default values.
 *	\param	None.
 *	\return	Void.
 *
 *	Initialise the library data structures and default values. Must be done before any other call to the library.
 *	There is 4 callback functions which can be (and must be) overridden, the library must not write anything to
 *	the console. 
 *	\li The verbose message redirection;
 *	\li the warning message redirection;
 *	\li the error message redirection (must stop the library);
 *	\li The notification callback : when the current directory has changed.
 *	\li The progress bar : this function is called with an int first set to 0. Then the value increases up to 100.
 *		It can be used to display a progress bar.
 *
 *	By default, those functions write a message to stderr. Another environment property is the ability to use or
 *	not the dir cache blocks to get the content of a directory. By default, it is not used. See adfChgEnvProp()
 *	to learn how to change those properties.
 *
 *	\b Internals \n
 *	1. Sets the default values.  \n
 *	2. Prints the library version with the verbose callback. \n
 *	3. Allocates the native functions structure. \n
 *	4. Calls adfInitNativeFct().
 *
 *	\b Examples: \n
 *	See <A HREF="../../api_env.html">api_env.html</A> for example code.
 */
void adfEnvInitDefault()
{
#ifdef _DEBUG_PRINTF_
    char str[80];
#endif /*_DEBUG_PRINTF_*/

    union u val;

    /* internal checking */

    if (sizeof(short)!=2) 
        { fprintf(stderr,"Compilation error : sizeof(short)!=2\n"); exit(1); }
    if (sizeof(long)!=4) 
        { fprintf(stderr,"Compilation error : sizeof(short)!=2\n"); exit(1); }
    if (sizeof(struct bEntryBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bEntryBlock)!=512\n"); exit(1); }
    if (sizeof(struct bRootBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bRootBlock)!=512\n"); exit(1); }
    if (sizeof(struct bDirBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bDirBlock)!=512\n"); exit(1); }
    if (sizeof(struct bBootBlock)!=1024)
        { fprintf(stderr,"Internal error : sizeof(struct bBootBlock)!=1024\n"); exit(1); }
    if (sizeof(struct bFileHeaderBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bFileHeaderBlock)!=512\n"); exit(1); }
    if (sizeof(struct bFileExtBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bFileExtBlock)!=512\n"); exit(1); }
    if (sizeof(struct bOFSDataBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bOFSDataBlock)!=512\n"); exit(1); }
    if (sizeof(struct bBitmapBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bBitmapBlock)!=512\n"); exit(1); }
    if (sizeof(struct bBitmapExtBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bBitmapExtBlock)!=512\n"); exit(1); }
    if (sizeof(struct bLinkBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bLinkBlock)!=512\n"); exit(1); }

    val.l=1L;
/* if LITT_ENDIAN not defined : must be BIG endian */
#ifndef LITT_ENDIAN
    if (val.c[3]!=1) /* little endian : LITT_ENDIAN must be defined ! */
        { fprintf(stderr,"Compilation error : #define LITT_ENDIAN must exist\n"); exit(1); }
#else
    if (val.c[3]==1) /* big endian : LITT_ENDIAN must not be defined ! */
        { fprintf(stderr,"Compilation error : #define LITT_ENDIAN must not exist\n"); exit(1); }
#endif

    adfEnv.wFct = Warning;
    adfEnv.eFct = Error;
    adfEnv.vFct = Verbose;
    adfEnv.notifyFct = Changed;
    adfEnv.rwhAccess = rwHeadAccess;
    adfEnv.progressBar = progressBar;
	
    adfEnv.useDirCache = FALSE;
    adfEnv.useRWAccess = FALSE;
    adfEnv.useNotify = FALSE;
    adfEnv.useProgressBar = FALSE;

#ifdef _DEBUG_PRINTF_
    sprintf(str,"ADFlib %s (%s)",adfGetVersionNumber(),adfGetVersionDate());
    (*adfEnv.vFct)(str);
#endif /*_DEBUG_PRINTF_*/

    adfEnv.nativeFct=(struct nativeFunctions*)malloc(sizeof(struct nativeFunctions));
    if (!adfEnv.nativeFct) (*adfEnv.wFct)("adfInitDefaultEnv : malloc");

    adfInitNativeFct();
}


/*
 * adfEnvCleanUp
 */
/*!	\brief	Cleans up the environment.
 *	\param	None.
 *	\return	Void.
 *
 *	\b Internals \n
 *	Frees the native functions structure. 
 */
void adfEnvCleanUp()
{
    free(adfEnv.nativeFct);
}


/*
 * adfChgEnvProp
 */
/*!	\brief	Change value of one of the environment properties.
 *	\param	prop - the environment property to change.
 *	\param	new  - the new property value.
 *	\return	Void.
 *
 *	Change the default or lastest value of one of the environment property. The new value has the void*,
 *	this is the only way to transmit it for several types. A cast is made depending of the property name
 *	inside adfChgEnvProp().
 *
 *	<TABLE>
 *	<TR><TD> PR_VFCT			<TD> Display verbose messages, (void(*)(char*)).
 *	<TR><TD> PR_WFCT			<TD> Display warning messages, (void(*)(char*)).
 *	<TR><TD> PR_EFCT			<TD> Display error messages, (void(*)(char*)).
 *	<TR><TD> PR_NOTFCT			<TD> Directory object change notification, (void(*)(SECTNUM,int)).
 *	<TR><TD> PR_USE_NOTFCT		<TD> Toggle on/off (default = off = false). BOOL.
 *	<TR><TD> PR_PROGBAR			<TD> Progress bar, (void(*)(int))
 *	<TR><TD> PR_USE_PROGBAR		<TD> Use progress bar (default = off). BOOL. The functions that support 'progress bar'
 *										are : adfCreateFlop(), adfCreateHd(), adfCreateHdFile().
 *	<TR><TD> PR_RWACCESS		<TD> Read (BOOL = false) or write (BOOL = true) operation, logical block and physical
 *										sector accessed, (void(*)(SECTNUM, SECTNUM, BOOL)).
 *	<TR><TD> PR_USE_RWACCESS	<TD> Use read/write access (default = off). BOOL.
 *	<TR><TD> PR_USEDIRC			<TD> Use dircache blocks. BOOL (default = off).
 *	</TABLE>
 *
 *	For the non pointer types (int with PR_USEDIRC), you have to use a temporary variable. To successfully override
 *	a function, the easiest is to reuse the default function located in adf_env.c, and to change it for your needs.
 */
void adfChgEnvProp(int prop, void *new)
{
	BOOL *newBool;

    switch(prop) {
    case PR_VFCT:
        adfEnv.vFct = (void(*)(char*))new;
        break;
    case PR_WFCT:
        adfEnv.wFct = (void(*)(char*))new;
        break;
    case PR_EFCT:
        adfEnv.eFct = (void(*)(char*))new;
        break;
    case PR_NOTFCT:
        adfEnv.notifyFct = (void(*)(SECTNUM,int))new;
        break;
    case PR_USE_NOTFCT:
        newBool = (BOOL*)new;
		adfEnv.useNotify = *newBool;        
        break;
    case PR_PROGBAR:
        adfEnv.progressBar = (void(*)(int))new;
        break;
    case PR_USE_PROGBAR:
        newBool = (BOOL*)new;
		adfEnv.useProgressBar = *newBool;        
        break;
    case PR_USE_RWACCESS:
        newBool = (BOOL*)new;
		adfEnv.useRWAccess = *newBool;        
        break;
    case PR_RWACCESS:
        adfEnv.rwhAccess = (void(*)(SECTNUM,SECTNUM,BOOL))new;
        break;
    case PR_USEDIRC:
        newBool = (BOOL*)new;
		adfEnv.useDirCache = *newBool;
        break;
    }
}

/*
 *  adfSetEnv
 *
 */
void adfSetEnvFct( void(*eFct)(char*), void(*wFct)(char*), void(*vFct)(char*),
    void(*notFct)(SECTNUM,int)  )
{
    if (*eFct!=0)
		adfEnv.eFct = *eFct;
    if (*wFct!=0)
		adfEnv.wFct = *wFct;
    if (*vFct!=0)
		adfEnv.vFct = *vFct;
    if (*notFct!=0)
        adfEnv.notifyFct = *notFct;
}


/*
 * adfGetVersionNumber
 */
/*!	\brief	Get the current ADFLib version number.
 *	\param	None
 *	\return	The version string.
 */
char* adfGetVersionNumber()
{
	return(ADFLIB_VERSION);
}


/*
 * adfGetVersionDate
 *
 */
/*!	\brief	Get the current ADFLib creation date.
 *	\param	None
 *	\return	The date string.
 */
char* adfGetVersionDate()
{
	return(ADFLIB_DATE);
}




/*##################################################################################*/
