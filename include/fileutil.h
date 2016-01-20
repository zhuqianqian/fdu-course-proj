/*
* fileutil.h: Define the file operation for the project.
*/

#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <stdio.h>

#define FM_READ		(1)
#define FM_WRITE	(2)
#define FM_RW		(FM_READ | FM_WRITE)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
* setfile(): Set the file to read/save data.
* Parameter:
*	filename: the file name.
*	mode: A flag to indicate what mode the file will be used.
* Return value:
*	If the file opened successfully, the return value is 0.
*	Otherwise, the error code is returned.
*/
int 
setfile (const char *filename, int mode);

/*
* getfile(): Get the file descriptor for current opened file.
*/
FILE *
getfile ();

/*
* finish(): Finish the read/write. File is closed.
*/
void
finish ();

/*
* getblockcount(): Get the total block count
*/
unsigned int
getblockcount();

/*
* writeblock: Write a block data to file.
*/
int 
writeblock (char * databuf, unsigned int size,
	    unsigned int *index, unsigned int count);

/*
* readblock: Read a block data from file.
*/
int
readblock (char * databuf, unsigned int * size,
	   unsigned int * index, unsigned int * count);	

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FILEUTIL_H */
