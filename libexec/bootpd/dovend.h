/* dovend.h */
/* $FreeBSD: src/libexec/bootpd/dovend.h,v 1.2.32.1 2008/11/25 02:59:29 kensmith Exp $ */

extern int dovend_rfc1497(struct host *hp, u_char *buf, int len);
extern int insert_ip(int, struct in_addr_list *, u_char **, int *);
extern void insert_u_long(u_int32, u_char **);
