#ifndef ADF_DUMP_H
#define ADF_DUMP_H 1

/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	adf_dump.h
 *  \brief	Amiga Dump File specific routines header..
 */

PREFIX     struct Device*
adfCreateDumpDevice(char* filename, long cyl, long heads, long sec);
PREFIX RETCODE adfCreateHdFile(struct Device* dev, char* volName, int volType);
/* GJH 7/11/02 - changed return values below to RETCODE to match main declarations. */
RETCODE adfInitDumpDevice(struct Device* dev, char* name,BOOL);
RETCODE adfReadDumpSector(struct Device *dev, long n, int size, unsigned char* buf);
RETCODE adfWriteDumpSector(struct Device *dev, long n, int size, unsigned char* buf);
RETCODE adfReleaseDumpDevice(struct Device *dev);


#endif /* ADF_DUMP_H */
/*##########################################################################*/
