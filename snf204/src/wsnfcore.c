/*
* wsnfcore.c: The implementation for of functions declared in snfcore.h
*	This implementation is for Windows platform.
*/

#include <winsock2.h>
#include <mstcpip.h>
#include <process.h>
#include "../include/snfcore.h"

#define S_STOPPED	(0)
#define S_RUNNING	(1)
#define S_PAUSED	(2)
#define S_STOPDONE	(3)

typedef struct __thread_arg
{
	SOCKET fd;
	callback cbfunc;
	void * arg1;
	void * arg2;
} _thread_arg;

unsigned long _status = S_STOPDONE;
char * _bufptr;
_thread_arg	ta;

unsigned __stdcall
_monitor(void *pArg)
{
	char buf[2048];
	sockaddr_in addr;
	int addrlen, recvlen;
	callback _funcptr;
	char *ptr;

	if(_bufptr == NULL)
		ptr = buf;
	else
		ptr = _bufptr;
	_funcptr = ta.cbfunc;
	addrlen = sizeof(addr);
	while(_status)
	{
		if((recvlen = recvfrom(ta.fd, ptr, 2048, 0, (sockaddr *)&addr, &addrlen))
			== SOCKET_ERROR)
		{
			_funcptr(0, 0, ta.arg1, ta.arg2);
			break;
		}
		else if(_status == S_RUNNING)
		{
			ptr = (char *)_funcptr(ptr, recvlen, ta.arg1, ta.arg2);
			if(_bufptr == NULL)
				ptr = buf;
		}
	}
	_status = S_STOPDONE;
	return 0;
}

unsigned long
startmonitor (sockaddr *addr, int addrlen, callback cbfunc,
			  void *arg1, void *arg2, unsigned long *err, char * userbuf)
{
	unsigned long mode;
	SOCKET	s;
	HANDLE  hThread;
	_status = S_STOPDONE;
	if(err == NULL)
	{
		SetLastError(0x2726);
		return 0;
	}
	if(cbfunc == NULL || addr == NULL)
	{
		*err = 0x2726;
		return 0;
	}
	s = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if(s == INVALID_SOCKET)
	{
		*err = WSAGetLastError();
		return 0;
	}
	if(bind(s, addr, addrlen))
	{
		*err = WSAGetLastError();
		closesocket(s);
		return 0;
	}
	mode = 1;
	if(ioctlsocket(s, SIO_RCVALL, &mode))
	{
		*err = WSAGetLastError();
		closesocket(s);
		return 0;
	}
	ta.fd = s;
	ta.cbfunc = cbfunc;
	ta.arg1 = arg1;
	ta.arg2 = arg2;
	_bufptr = userbuf;
	_status = S_RUNNING;
	hThread = (HANDLE)_beginthreadex(0, 0, _monitor, 0, 0, 0);
	return (unsigned long)hThread;
}

void
stopmonitor ()
{
	_status = S_STOPPED;
	closesocket(ta.fd);
}

void
pausemonitor()
{
	_status = S_PAUSED;
}

void
resumemonitor()
{
	_status = S_RUNNING;
}

int
stopdone()
{
	if(_status == S_STOPDONE)
		return 1;
	return 0;
}