/* ADF Opus Copyright 1998-2002 by 
 * Dan Sutherland <dan@chromerhino.demon.co.uk> and Gary Harris <gharris@zip.com.au>.	
 *
 * BatchConvert.h - definitions for batch converter
 */

#ifndef BATCHCONVERT_H
#define BATCHCONVERT_H


LRESULT CALLBACK BatchConvertProc(HWND, UINT, WPARAM, LPARAM);

/* local prototypes */
void BCAddFiles(HWND);
void BCConvert(HWND);
void BCUpdateButtons(HWND);
void BCDeleteSelected(HWND);
void BCDeleteAll(HWND);
BOOL GZCompress(HWND win, char *infile, char *outfile);
BOOL GZDecompress(HWND win, char *infile, char *outfile);


// Index of selected filter in OPENFILENAME structure. Used for button en/disabling.
	int iFileTypeSelected;

#endif /* BATCHCONVERT_H */
