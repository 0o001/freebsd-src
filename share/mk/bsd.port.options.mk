# $FreeBSD: src/share/mk/bsd.port.options.mk,v 1.1.14.1.4.1 2010/06/14 02:09:06 kensmith Exp $

USEOPTIONSMK=	yes
INOPTIONSMK=	yes

.include <bsd.port.mk>

.undef INOPTIONSMK
