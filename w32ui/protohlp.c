#include "protohlp.h"
#include "../include/proto.h"

/* 
* The descriptions of the type in ICMP.
* See RFC 1700 for more information
*/
const TCHAR szTypeInfo[][32] = {
	TEXT("Echo Reply"), /* 0 */
	TEXT(""), TEXT(""), /* 1,2 */
	TEXT("Destination Unreachable: "), /* 3 */
	TEXT("Source Quench"), /* 4 */
	TEXT("Redirect: "), /* 5 */
	TEXT("Alternate Host Address"), /* 6 */
	TEXT(""), /* 7 */
	TEXT("Echo Request"), /* 8 */
	TEXT("Router Advertisement"), /* 9 */
	TEXT("Router Selection"), /* 10 */
	TEXT("Time Exceeded"), /* 11 */
	TEXT("Parameter Problem: "), /* 12 */
	TEXT("Timestamp"), /* 13 */
	TEXT("Timestamp Reply"), /* 14 */
	TEXT("Information Request"), /* 15 */
	TEXT("Information Reply"), /* 16 */
	TEXT("Address Mask Request"), /* 17 */
	TEXT("Address Mask Reply"), /* 18 */
	TEXT(""), TEXT(""),TEXT(""),TEXT(""),
	TEXT(""),TEXT(""),TEXT(""),TEXT(""),
	TEXT(""),TEXT(""),TEXT(""), /* 19, 20-29 */
	TEXT("Traceroute"), /* 30 */
	TEXT("Datagram Conversion Error"), /* 31 */
	TEXT("Mobile Host Redirect"), /* 32 */
	TEXT("IPv6 Where-Are-You"), /* 33 */
	TEXT("IPv6 I-Am-Here"), /* 34 */
	TEXT("Mobile Registration Request"), /* 35 */
	TEXT("Mobile Registration Reply"), /* 36 */
	TEXT("Domain Name Request"), /* 37 */
	TEXT("Domain Name Reply"), /* 38 */
	TEXT("SKIP"), /* 39 */
	TEXT("Photuris") /* 40 */
};

const TCHAR szType3Info[][64] = {
	TEXT("Net unreachable"), /* 0 */
	TEXT("Host unreachable"), /* 1 */
	TEXT("Protocol unreachable"), /* 2 */
	TEXT("Port unreachable"), /* 3 */
	TEXT("Fragmentation Needed and DF was Set"), /* 4 */
	TEXT("Source route failed"), /* 5 */
	TEXT("Destination network unknown"), /* 6 */
	TEXT("Destination host unknown"), /* 7 */
	TEXT("Source host Isolated"), /* 8 */
	TEXT("Network is administratively prohibited"), /* 9 */
	TEXT("Host is administratively prohibited"), /* 10 */
	TEXT("Network unreachable for TOS"), /* 11 */
	TEXT("Host unreachable for TOS"), /* 12 */
	TEXT("Communication administritively prohibited"), /* 13 */
	TEXT("Host precedence violation"), /* 14 */
	TEXT("Precedence cutoff in effect") /* 15 */
};

const TCHAR szType5Info[][64] = {
	TEXT("Redirect datagram for network"), /* 0 */
	TEXT("Redirect datagram for host"), /* 1 */
	TEXT("Redirect datagram for network and TOS"), /* 2 */
	TEXT("Redirect datagram for host and TOS") /* 3 */
};

const TCHAR szType12Info[][64] = {
	TEXT("Pointer indicates the error"), /* 0 */
	TEXT("Missing a required option"), /* 1 */
	TEXT("Bad length") /* 2 */
};

int
geticmpinfo(char *data, LPWSTR szInfo)
{
	icmphdr_ptr icmp;
	unsigned char tc;

	icmp = (icmphdr_ptr)data;
	tc = icmp->tc_u.tc.type;
	lstrcpy(szInfo, TEXT("ICMP "));
	if(tc < 41)
		lstrcat(szInfo, szTypeInfo[tc]);
	tc = icmp->tc_u.tc.code;
	switch(tc)
	{
	case 3:
		if(tc < 16)
		{
			lstrcat(szInfo, szType3Info[tc]);
		}
		break;

	case 5:
		if(tc < 4)
		{
			lstrcat(szInfo, szType5Info[tc]);
		}
		break;

	case 12:
		if(tc < 3)
		{
			lstrcat(szInfo, szType12Info[tc]);
		}
		break;

	default:
		break;
	}
	return 0;
}

#define DNS_QR_MASK	(0x80)
#define DNS_PORT	(53)

int 
getudpinfo(char *data, LPWSTR szInfo)
{
	udphdr_ptr udp;
	char *p;
	udp = (udphdr_ptr)data;
	if(udp->srcport == DNS_PORT || udp->destport == DNS_PORT)
	{
		lstrcpy(szInfo, TEXT("DNS "));
		p = data + sizeof(udphdr) + 1;
		if((*p & DNS_QR_MASK)>0)
			lstrcat(szInfo, TEXT("Name Query Reply"));
		else
			lstrcat(szInfo, TEXT("Name Query Request"));
	}
	else
	{
		wsprintf(szInfo, TEXT("Source port:%u, Destination port:%u"),
		ntohs(udp->srcport), ntohs(udp->destport));
	}
	return 0;
}

int 
gettcpinfo(char *data, LPWSTR szInfo)
{
	TCHAR szExtra[128];
	TCHAR szTemp[32];
	tcphdr_ptr tcp;
	tcp = (tcphdr_ptr)data;
	szExtra[0] = 0;
	wsprintf(szInfo, TEXT("%u->%u "),ntohs(tcp->srcport), ntohs(tcp->destport));
	if(tcp->flag.un.flags.ack == 1)
	{
		lstrcat(szInfo,TEXT("[ACK] "));
		wsprintf(szTemp, TEXT("Ack=%u "), ntohl(tcp->ackno)); 
		lstrcat(szExtra, szTemp);
	}
	if(tcp->flag.un.flags.fin == 1)
		lstrcat(szInfo,TEXT("[FIN] "));
	if(tcp->flag.un.flags.rst == 1)
		lstrcat(szInfo,TEXT("[RST] "));
	if(tcp->flag.un.flags.syn == 1)
	{
		lstrcat(szInfo,TEXT("[SYN] "));
		wsprintf(szTemp, TEXT("Seq=%u "), ntohl(tcp->seqno));
		lstrcat(szExtra, szTemp);
	}
	lstrcat(szInfo, szExtra);
	wsprintf(szExtra, TEXT("Win=%u"), ntohs(tcp->rcvwnd));
	lstrcat(szInfo, szExtra);
	return 0;
}