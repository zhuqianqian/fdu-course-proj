#include <windows.h>
#include <errno.h>
#include <iphlpapi.h>
#include "../include/snfcore.h"
#include "../include/proto.h"
#include "../include/fileutil.h"
#include "../include/snfcon.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")

#define BUFFERSIZE	(32<<20)
#define INDEXCOUNT	(64<<10)

unsigned long count = 0;
unsigned long total = 0;
char *buffer;
unsigned int *index;
unsigned int ic = 0;
unsigned int bs = 0;

unsigned long 
on_recvdata (char * buf, int len, void *arg1, void *arg2);

void track();

void load();

unsigned long getipaddr();

void display (char *buf, int len);

int
startgui(const char *name);

int main(int argc, char **argv)
{
	int cmd, errcode;
	cmd = get_command(argc, argv);
	switch(cmd)
	{
	case CMD_ERROR:
		printf("Error: invalid argument '%s'\n", errarg);
		break;
	case CMD_MUTEX:
		printf("Error: '-t' and '-l' cannot be specified at the same time.\n");
		break;

	case CMD_MUTEXEX:
		printf("Error: '-o' and '-l' cannot be specified at the same time.\n");
		break;
		
	case CMD_HELP:
		show_help(appname);
		break;

	case CMD_VERSION:
		show_version(appname, version);
		break;

	case CMD_GUI:
		errcode = startgui("snf204w.exe");
		if(errcode)
			printf("Error: Cannot start GUI program. Code:%x\n", errcode);
		break;

	case CMD_TRACK:
		{
			if(save && file == 0)
			{
				printf("Error: No output file following '-o'.\n");
			}
			else
			{
				if(save && setfile(file, FM_WRITE)!=0)
				{
					printf("Error: Cannot write file '%s'.\n", file);
				}
				else 
				{
					track();
				}
			}
		}
		break;

	case CMD_LOAD:
		{
			if(file == 0)
				printf("Error: No input file following '-l'.\n");
			else
			{
				if(setfile(file, FM_READ)!=0)
				{
					printf("Error: Cannot open file '%s'.\n", file);
				}
				else
				{
					load();
					finish();
				}
			}
		}
		break;
	
	default: break;
	}
	return 0;
}


u_long getipaddr()
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter;
	u_long ip_ret;
	u_long ulOutBufLen;
	u_long count;

	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS) {
		free (pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
	}
	count = 0;
	if ((ip_ret = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		if(pAdapter==NULL)
		{
			printf("No available network interfaces found on this system!\n");
			free(pAdapterInfo);
			return 0;
		}
		printf("The following network interfaces are found in your system:\n");
		while (pAdapter) {
			printf("%s, %s\n", pAdapter->Description, pAdapter->IpAddressList.IpAddress.String);
			pAdapter = pAdapter->Next;
			++count;
		}
	}
	else 
	{
		printf("Error to retrieve your network interface: 0x%x\n", GetLastError());
		free(pAdapterInfo);
		return 0;
	}
	if(count > 1)
	{
		do {
			printf("\nPlease select one interface to use from above %d entries by entering 1-%d: ", count, count);
			scanf("%d", &ip_ret);
		}while(ip_ret > count || ip_ret <= 0);
	}
	else
		ip_ret = 1;
	pAdapter = pAdapterInfo;
	count = 1;
	while(count < ip_ret)
	{
		pAdapter = pAdapter->Next;
		count++;
	}
	ip_ret = inet_addr(pAdapter->IpAddressList.IpAddress.String);
	free(pAdapterInfo);
	return ip_ret;
}

void track()
{
	u_char	c;
	HANDLE hMonitor;
	int status;
	sockaddr_in addr;
	unsigned long ip, errcode;
	WSADATA wd;

	if(WSAStartup(0x202, &wd))
	{
		printf("Failed to load windows socket library.\nError code:0x%x\n", WSAGetLastError());
		return;
	}
	
	addr.sin_addr.S_un.S_addr = getipaddr();
	if(addr.sin_addr.S_un.S_addr == INADDR_NONE)
	{
		printf("You selected an interface with invalid IP address\n");
		WSACleanup();
		return;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(204);
	index = (unsigned int *)malloc(BUFFERSIZE+(sizeof(unsigned int)*INDEXCOUNT));
	if(index == NULL)
	{
		printf("Cannot allocate enough memory.\n");
		WSACleanup();
		return;
	}
	buffer = (char *)(index + INDEXCOUNT);
	hMonitor = (HANDLE)startmonitor((sockaddr *)&addr, sizeof(addr), on_recvdata, 0, 0, &errcode, buffer);
	if(hMonitor == 0)
	{
		printf("Error to start. Error code:0x%x\n", errcode);
		WSACleanup();
		return;
	}
	CloseHandle(hMonitor);
	printf("Tracking, you can press 'p' to pause and 'c' to stop at any time...\n");

	status = 1;
	while(1)
	{
		c = getch();
		if(c == 'c')
		{
			stopmonitor();
			break;
		}
		else if(c == 'p')
		{
			if(status == 1)
			{
				pausemonitor();
				printf("Paused, press 'p' to continue, 'c' to terminate.\r");
				status = 0;
			}
			else
			{
				printf("Tracking, press 'p' to pause, 'c' to terminate.\r");
				resumemonitor();
				status = 1;
			}
		}
	}
	if(save)
	{
		writeblock(buffer, bs, index, ic);
		finish();
	}
	while(!stopdone())
		Sleep(100);
	free(index);
	WSACleanup();
}

unsigned long on_recvdata(char * buf, int len, void *arg1, void *arg2)
{
	unsigned long retptr;

	if(show)
		display(buf, len);
	*(index+ic) = len;
	bs += len;
	++ic;
	if(BUFFERSIZE - bs < 2048 || INDEXCOUNT - ic < 2)
	{
		if(save)
			writeblock(buffer, bs, index, ic);
		retptr = (unsigned long)buffer;
		total += bs;
		ic = 0;
		bs = 0;
	}
	else
		retptr = (unsigned long)(buf+len);
	return retptr;
}

void load()
{
	unsigned int count, i, j;
	char *p;
	index = (unsigned int *)malloc(BUFFERSIZE+(sizeof(unsigned int)*INDEXCOUNT));
	if(index == NULL)
	{
		printf("Cannot allocate enough memory.\n");
		WSACleanup();
		return;
	}
	buffer = (char *)(index + INDEXCOUNT);
	count = getblockcount();
	for(i=0; i< count; ++i)
	{
		readblock(buffer, &bs, index, &ic);
		p = buffer;
		for(j = 0; j < ic; ++j)
		{
			display(p, *(index+j));
			p += *(index+j);
		}
	}
}

void display(char *buf, int len)
{
	iphdr_ptr iphdr;

	if(buf!=NULL)
	{
		iphdr = (iphdr_ptr)buf;
		printf("%d    ", ++count);
		printf("%d.%d.%d.%d    %d.%d.%d.%d", iphdr->srcip.un.ip_b.b1, 
			iphdr->srcip.un.ip_b.b2, iphdr->srcip.un.ip_b.b3, iphdr->srcip.un.ip_b.b4,
			iphdr->destip.un.ip_b.b1, iphdr->destip.un.ip_b.b2, iphdr->destip.un.ip_b.b3,
			iphdr->destip.un.ip_b.b4);
		if(iphdr->protocal == PF_ICMP)
			printf("    ICMP");
		else if(iphdr->protocal == PF_TCP)
			printf("    TCP");
		else if(iphdr->protocal == PF_UDP)
			printf("    UDP");
		printf("    %d\n", len);
	}
}

int
startgui(const char *name)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	char name_c[260];
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	strcpy(name_c, name); 
	if(CreateProcess(0, name_c, 0, 0, 0, 0, 0, 0, &si, &pi) == 0)
	{
		return GetLastError();
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 0;
}