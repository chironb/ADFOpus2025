/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 */
/*! \file	adf_defs.h
 *  \brief	Global definitions header.
 */


#ifndef _ADF_DEFS_H
#define _ADF_DEFS_H 1

#define ADFLIB_VERSION "0.7.9d"							/*!< Version String.	*/
#define ADFLIB_DATE "17 November, 2002"					/*!< Date String.		*/

#define SECTNUM long									/*!< Sector Number.		*/
#define RETCODE long									/*!< Return Code.		*/

#define TRUE    1										/*!< Boolean true.		*/
#define FALSE   0										/*!< Boolean false.		*/

#define ULONG   unsigned long							/*!< ULONG	*/
#define USHORT  unsigned short							/*!< USHORT	*/
#define UCHAR   unsigned char							/*!< UCHAR	*/
#define BOOL    int										/*!< BOOL	*/


/* defines max and min */

#ifndef max
#define max(a,b)        (a)>(b) ? (a) : (b)				/*!< Max macro. */
#endif
#ifndef min
#define min(a,b)        (a)<(b) ? (a) : (b)				/*!< Min macro. */
#endif


/* (*byte) to (*short) and (*byte) to (*long) conversion */

#define Short(p) ((p)[0]<<8 | (p)[1])					/*!< Byte to short conversion.	*/
#define Long(p) (Short(p)<<16 | Short(p+2))				/*!< Byte to long conversion.	*/


/* swap short and swap long macros for little endian machines */

#define swapShort(p) ((p)[0]<<8 | (p)[1])				/*!< Swap short macro.	*/
#define swapLong(p) (swapShort(p)<<16 | swapShort(p+2))	/*!< Swap long macro.	*/



#endif /* _ADF_DEFS_H */
/*##########################################################################*/
