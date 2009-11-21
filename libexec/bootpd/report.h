/* report.h */
/* $FreeBSD: src/libexec/bootpd/report.h,v 1.5.36.1.2.1 2009/10/25 01:10:29 kensmith Exp $ */

extern void report_init(int nolog);
extern void report(int, const char *, ...) __printflike(2, 3);
extern const char *get_errmsg(void);
