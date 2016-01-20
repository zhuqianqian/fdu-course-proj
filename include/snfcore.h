/*
* snfcore.h: Declares the most core functions for the network sniffer
*	
* Copyright (c) Zhu Qianqian <zhuqianqian.299@gmail.com>
* 2010, All Rights Reserved
*/

#ifndef SNFCORE_H
#define SNFCORE_H

/* Windows socket is really fucking complicated! */
#if defined (WIN32)
#define sockaddr SOCKADDR
#define sockaddr_in SOCKADDR_IN 
#endif /* WIN32 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
* Function pointer for callback, usually, user define a function of this type,
*  first argument is a memory block, and the second specify the size, the last
*  two arguments are for user-defined parameters.
*/
typedef unsigned long (* callback)(char *, int, void *, void *);


/*
* startmonitor(): Start the thread to listen at a local port to monitor the 
*	network stream on local system.
* Parameter: 
*	addr: [in] a pointer to a sockaddr_in structure, the socket will be
*		binded to this address.
*	addrlen: [in] the size of addr parameter.
*	cbfunc: [in] a function pointer for callback. When data is received, this
*		function will be called. NULL is NOT allowed.
*	arg1, arg2: [in] user defined argument for callback function.
*	err: [out] an address to receive possible error code.
*	userbuf: [in] buffer will be used to receive data, by default, a default
*		buffer will be used, if it is not NULL, use user-defined buffer.
* Return value: If everything works well, the return value indicate a thread
*	object, on Windows, it's a thread handle. If some error occurs, return value
*	is 0, error code is saved in *err.
* Remarks:
*	For better performance consideration, userbuf should be a block of memory 
*	with enough large size, in this case, the callback function must return 
*	another buffer pointer to use for next recv call.
*/
unsigned long 
startmonitor (sockaddr *addr, int addrlen, callback cbfunc,
			  void *arg1, void *arg2, unsigned long *err, char *userbuf);

/*
* stopmonitor(): Stop the monitor thread to receive data.
*/
void 
stopmonitor ();

/*
* pausemonitor(): Pause the monitor thread to receive data.
*/
void
pausemonitor ();

/*
* resumemonitor(): Resume paused monitor thread to continue receive data.
*/
void
resumemonitor ();

int 
stopdone ();


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SNFCORE_H */