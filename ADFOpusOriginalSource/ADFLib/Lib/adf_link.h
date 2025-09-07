#ifndef ADF_LINK_H
#define ADF_LINK_H 1

/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	adf_link.h
 *  \brief	Read entry name functions header.
 */

#include"prefix.h"

PREFIX RETCODE adfBlockPtr2EntryName(struct Volume *vol, SECTNUM nSect, SECTNUM lPar, 
	char **name, long *size);


#endif /* ADF_LINK_H */
/*##########################################################################*/
