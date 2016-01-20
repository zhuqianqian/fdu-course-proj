/*
* proto.h:  Definition of network protocal.
*			This file contains the definition of IP, TCP, UDP.
*	
*	Copyright (c) Zhu Qianqian <zhuqianqian.299@gmail.com>
*	All Rights Reserved.
*/

#ifndef PROTO_H
#define PROTO_H

#pragma pack(push, 1)

/* IPv4 address */
typedef struct _ipaddr
{
	union {
		struct {
			unsigned char b1, b2, b3, b4;
		} ip_b;
		struct
		{
			unsigned short w1, w2;
		} ip_w;
		unsigned long ip_l;
	} un;
} ipaddr;

typedef struct _iphdr
{
#if defined (LITTLE_ENDIAN)
	unsigned char hdrlen : 4;
	unsigned char version : 4;
#else
	unsigned char version : 4;
	unsigned char hdrlen : 4;
#endif /* LITTLE_ENDIAN */
	unsigned char svctype;
	unsigned short datalen;
	unsigned short identifier;
#if defined (LITTLE_ENDIAN)
	unsigned short flag : 3;
	unsigned short offset : 13;
#else
	unsigned short offset : 13;
	unsigned short flag : 3;
#endif /* LITTLE_ENDIAN */
	unsigned char	ttl;
	unsigned char	protocal;
	unsigned short	checksum;
	ipaddr			srcip;
	ipaddr			destip;
} iphdr, * iphdr_ptr;

typedef struct _tcpflag
{
	union {
		unsigned char val;
		struct {
#if defined (LITTLE_ENDIAN)
			unsigned char fin : 1;
			unsigned char syn : 1;
			unsigned char rst : 1;
			unsigned char psh : 1;
			unsigned char ack : 1;
			unsigned char urg : 1;
			unsigned char ecn_echo : 1;
			unsigned char cwr : 1;
#else
			unsigned char cwr : 1;
			unsigned char ecn_echo : 1;
			unsigned char urg : 1;
			unsigned char ack : 1;
			unsigned char psh : 1;
			unsigned char rst : 1;
			unsigned char syn : 1;
			unsigned char fin : 1;
#endif /* LITTLE_ENDIAN */
		} flags;
	}un;
} tcpflag;

typedef struct _tcphdr
{
	unsigned short	srcport;
	unsigned short	destport;
	unsigned long	seqno;
	unsigned long	ackno;
#if defined (LITTLE_ENDIAN)
	unsigned char reserved : 4;
	unsigned char hdrlen : 4;
#else
	unsigned char hdrlen : 4;
	unsigned char reserved : 4;
#endif /* LITTLE_ENDIAN */
	tcpflag	flag;
	unsigned short rcvwnd;
	unsigned short checksum;
	unsigned short urgptr;
} tcphdr, *tcphdr_ptr;

typedef struct _udphdr
{
	unsigned short	srcport;
	unsigned short	destport;
	unsigned short	length;
	unsigned short	checksum;
} udphdr, * udphdr_ptr;

typedef struct _icmphdr
{
	union {
		struct {
			u_char type;
			u_char code;
		} tc;
		u_short s_tc;
	}tc_u;
	u_short checksum;
	u_short identifier;
	u_short sequence;
}icmphdr, *icmphdr_ptr;

#define PF_ICMP	(0x01)
#define PF_TCP	(0x06)
#define PF_UDP	(0x11)

#pragma pack(pop)

#endif /* PROTO_H */