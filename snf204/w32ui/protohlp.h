/*
* protohlp.h: Analyze the various protocols.
*/

#ifndef PROTOHLP_H
#define PROTOHLP_H

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

int
geticmpinfo(char * data, LPWSTR szInfo);

int
gettcpinfo(char *data, LPWSTR szInfo);

int 
getudpinfo(char *data, LPWSTR szInfo);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PROTOHLP_H */