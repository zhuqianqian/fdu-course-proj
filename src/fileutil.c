#include "../include/fileutil.h"
#include <errno.h>

FILE *_fptr;
int  _mf;
/* _bc: Record the total blocks read/written */
unsigned long _bc;

int 
setfile (const char *filename, int mode)
{
	char fom[4];
	_fptr = 0;
	_mf = 0;
	_bc = 0;
	if(mode == FM_READ)
		strcpy(fom, "rb");
	if(mode == FM_WRITE)
		strcpy(fom, "wb");
	if(mode == FM_RW)
		strcpy(fom, "rwb");
	if(_fptr)
		fclose(_fptr);
	_fptr = fopen(filename, fom);
	if(_fptr == 0)
	{
		return errno;
	}
	_mf = mode;
	if((_mf & FM_READ) == FM_READ)
		fread(&_bc, sizeof(unsigned long), 1, _fptr);
	else if((_mf & FM_WRITE) == FM_WRITE)
		fseek(_fptr, sizeof(unsigned long), SEEK_SET);
	return 0;
}

FILE * 
getfile ()
{
	return _fptr;
}

unsigned int
getblockcount()
{
	return _bc;
}

void
finish()
{
	if(_fptr == NULL)
		return;
	if((_mf & (FM_WRITE)) == FM_WRITE)
	{
		fseek(_fptr, SEEK_SET, 0);
		fwrite(&_bc, sizeof(unsigned long), 1, _fptr);
	}
	fclose(_fptr);
	_fptr = NULL;
	_mf = 0;
}

int
writeblock (char * databuf, unsigned int size,
	    unsigned int *index, unsigned int count)
{
	int ret;
	unsigned int sizewrite[2];
	sizewrite[0] = count;
	sizewrite[1] = size;
	fwrite(sizewrite, sizeof(unsigned int), 2, _fptr);
	fwrite(index, sizeof(unsigned int), count, _fptr);
	fwrite(databuf, sizeof(char), size, _fptr);
	ret = _bc;
	++_bc;
	return ret;
}

int 
readblock (char *databuf, unsigned int *size,
	   unsigned int *index, unsigned int *count)
{
	unsigned int sizeread[2];
	int ret;

	if(fread(sizeread, sizeof(unsigned int), 2, _fptr) < 2)
		return 0;
	*count = sizeread[0];
	*size = sizeread[1];
	if(fread(index, sizeof(unsigned int), *count, _fptr) < *count)
		return 0;
	if(fread(databuf, sizeof(char), *size, _fptr) < *size)
		return 0;
	ret = _bc;
	++_bc;
	return ret;
}

