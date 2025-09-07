#ifndef ADF_ENV_H
#define ADF_ENV_H 1

/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	adf_env.h
 *  \brief	Environment routines header.
 */

#include"prefix.h"

PREFIX void adfEnvInitDefault();
PREFIX void adfSetEnvFct( void(*e)(char*), void(*w)(char*), void(*v)(char*),
	void(*n)(SECTNUM,int) );
PREFIX void adfEnvCleanUp();
PREFIX void adfChgEnvProp(int prop, void *new);
PREFIX char* adfGetVersionNumber();
PREFIX char* adfGetVersionDate();

#endif /* ADF_ENV_H */
/*##########################################################################*/
