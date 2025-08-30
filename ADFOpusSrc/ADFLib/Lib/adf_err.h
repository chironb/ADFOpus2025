#ifndef ADF_ERR_H
#define ADF_ERR_H

/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	adf_err.h
 *  \brief	Error values header.
 */


#define hasRC(rc,c) ((rc)&(c))						/*!< Check c for an error.	*/

#define RC_OK				0						/*!< RC_OK.					*/
#define RC_ERROR			-1						/*!< RC_ERROR.				*/

#define RC_MALLOC           1						/*!< RC_MALLOC.				*/
#define RC_VOLFULL			2						/*!< RC_VOLFULL.			*/


#define RC_FOPEN            1<<10					/*!< RC_FOPEN.				*/
#define RC_NULLPTR          1<<12					/*!< RC_NULLPTR.			*/

/* adfRead*Block() */

#define RC_BLOCKTYPE        1						/*!< RC_BLOCKTYPE.			*/
#define RC_BLOCKSTYPE       1<<1					/*!< RC_BLOCKSTYPE.			*/
#define RC_BLOCKSUM         1<<2					/*!< RC_BLOCKSUM.			*/
#define RC_HEADERKEY		1<<3					/*!< RC_HEADERKEY.			*/
#define RC_BLOCKREAD        1<<4					/*!< RC_BLOCKREAD.			*/

/* adfWrite*Block */
#define RC_BLOCKWRITE       1<<4					/*!< RC_BLOCKWRITE.			*/


/* adfReadBlock() */
#define RC_BLOCKOUTOFRANGE  1						/*!< RC_BLOCKOUTOFRANGE.	*/
#define RC_BLOCKNATREAD     1<<1					/*!< RC_BLOCKNATREAD.		*/

/* adfWriteBlock() */
/* RC_BLOCKOUTOFRANGE */
#define RC_BLOCKNATWRITE    1<<1					/*!< RC_BLOCKNATWRITE.		*/
#define RC_BLOCKREADONLY    1<<2					/*!< RC_BLOCKREADONLY.		*/

/* adfInitDumpDevice() */
/* RC_FOPEN */
/* RC_MALLOC */

/* adfNativeReadBlock(), adfReadDumpSector() */

#define RC_BLOCKSHORTREAD   1						/*!< RC_BLOCKSHORTREAD.		*/
#define RC_BLOCKFSEEK       1<<1					/*!< RC_BLOCKFSEEK..		*/

/* adfNativeWriteBlock(), adfWriteDumpSector() */

#define RC_BLOCKSHORTWRITE  1						/*!< RC_BLOCKSHORTWRITE.	*/
/* RC_BLOCKFSEEK */


/*-- adfReadRDSKblock --*/
#define RC_BLOCKID          1<<5					/*!< RC_BLOCKID.			*/

/*-- adfWriteRDSKblock() --*/
/*RC_BLOCKREADONLY*/

#endif /* ADF_ERR_H */

/*############################################################################*/
