#ifndef _PREFIX_H
#define _PREFIX_H 1
/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	prefix.h
 *  \brief	Prefix definition header.
 */ 

#ifdef WIN32DLL
#define PREFIX __declspec(dllexport)		/*!< Windows dll PREFIX definition.	*/
#else
#define PREFIX 								/*!< Standard PREFIX definition.	*/
#endif /* WIN32DLL */

#endif /* _PREFIX_H */
