# $FreeBSD: src/share/skel/dot.login,v 1.11.2.1 1999/08/29 16:47:50 peter Exp $
#
# .login - csh login script, read by login shell, 
#	   after `.cshrc' at login.
#
# see also csh(1), environ(7).
#

set path = (/sbin /bin /usr/sbin /usr/bin /usr/games /usr/local/bin /usr/X11R6/bin $HOME/bin)
setenv MANPATH "/usr/share/man:/usr/X11R6/man:/usr/local/man"

# Interviews settings
#setenv CPU "FREEBSD"
#set path = ($path /usr/local/interviews/bin/$CPU)
#setenv MANPATH "${MANPATH}:/usr/local/interviews/man"

# A rightous umask
umask 22

[ -x /usr/games/fortune ] && /usr/games/fortune
