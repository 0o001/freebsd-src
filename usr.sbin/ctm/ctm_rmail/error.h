/* $FreeBSD: src/usr.sbin/ctm/ctm_rmail/error.h,v 1.2.32.1 2008/11/25 02:59:29 kensmith Exp $ */

extern	void	err_set_log(char *log_file);
extern	void	err_prog_name(char *name);
extern	void	err(const char *fmt, ...) __printflike(1, 2);
