/*
  LibRCC - module comunicating with rcc-external helper application

  Copyright (C) 2005-2008 Suren A. Chilingaryan <csa@dside.dyndns.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License version 2.1 or later
  as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
  for more details.

  You should have received a copy of the GNU Lesser General Public License 
  along with this program; if not, write to the Free Software Foundation, Inc.,
  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "../config.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif /* HAVE_SIGNAL_H */

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */
#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif /* HAVE_SYS_UN_H */
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif /* HAVE_SYS_WAIT_H */

#include "rcchome.h"
#include "rccexternal.h"
#include "internal.h"

#define RCC_EXT_PROG_NAME "rccexternal"
#define RCC_EXTERNAL_TIMEOUT			250 /* 100us */

#ifdef HAVE_UNISTD_H
static pid_t pid = (pid_t)-1;
static char *addr = NULL;
#endif /* HAVE_UNISTD_H */

int rccExternalInit() {
#ifdef HAVE_UNISTD_H
# ifdef HAVE_SIGNAL_H
    struct sigaction act;
# endif /* HAVE_SIGNAL_H */
# ifdef HAVE_SYS_STAT_H
    struct stat st;
# endif /* HAVE_SYS_STAT_H */

    if (pid != (pid_t)-1) return 0;

    if (!addr) {
	addr = (char*)malloc(strlen(rcc_home_dir)+32);
	if (!addr) return -1;
    }

    pid = fork();
    if (pid) {
	if (pid == (pid_t)-1) return -1;
	sprintf(addr,"%s/.rcc/comm/%lu.sock", rcc_home_dir, (unsigned long)pid);

# ifdef HAVE_SIGNAL_H
	act.sa_handler = SIG_IGN;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGPIPE,&act,NULL);
# endif /* HAVE_SIGNAL_H */

	return 0;
    }

    /*if ((!stat("../external/" RCC_EXT_PROG_NAME, &st))&&(st.st_mode&S_IXOTH)) {
	execl ("../external/" RCC_EXT_PROG_NAME, RCC_EXT_PROG_NAME, NULL);
    } else*/ 
# ifdef HAVE_SYS_STAT_H
    if ((!stat(LIBRCC_DATA_DIR "/" RCC_EXT_PROG_NAME, &st))&&(st.st_mode&S_IXOTH)) {
# endif /* HAVE_SYS_STAT_H */
	execl(LIBRCC_DATA_DIR "/" RCC_EXT_PROG_NAME, RCC_EXT_PROG_NAME, NULL);
# ifdef HAVE_SYS_STAT_H
    }
# endif /* HAVE_SYS_STAT_H */
#endif /* HAVE_UNISTD_H */
    exit(1);
}

void rccExternalFree() {
#ifdef HAVE_UNISTD_H
    int retry;
    pid_t res;
    struct timespec timeout = { 0, 5000000 };

    if (pid == (pid_t)-1) return;

    for (retry = 0; retry < 3; retry++) {
	rccExternalConnect(0);    
	
	nanosleep(&timeout, NULL);
	
	res = waitpid(pid, NULL, WNOHANG);
	if (res) break;
	else timeout.tv_nsec*=10;
    }

    pid = (pid_t)-1;
    if (addr) free(addr);
#endif /* HAVE_UNISTD_H */
}

#ifdef HAVE_SYS_SELECT_H
static int rccExternalSetDeadline(struct timeval *tv, unsigned long timeout) {
/*
    gettimeofday(tv, NULL);
    tv->tv_sec += (tv->tv_usec + timeout + RCC_EXTERNAL_TIMEOUT) / 1000000;
    tv->tv_usec = (tv->tv_usec + timeout + RCC_EXTERNAL_TIMEOUT) % 1000000;
*/
    tv->tv_sec = (timeout + RCC_EXTERNAL_TIMEOUT) / 1000000;
    tv->tv_usec = (timeout + RCC_EXTERNAL_TIMEOUT) % 1000000;
    return 0;
}
#endif /* HAVE_SYS_SELECT_H */

size_t rccExternalWrite(int s, const char *buffer, ssize_t size, unsigned long timeout) {
#ifdef HAVE_SYS_SELECT_H
    int err;
    unsigned char connected = 1;
    ssize_t writed, res = 0;
    struct timeval tv;
    fd_set fdcon;

    if (s == -1) return -1;
    
    for (writed = 0; ((writed < size)&&(connected)); writed += connected?res:0) {
	FD_ZERO(&fdcon);
	FD_SET(s, &fdcon);
	rccExternalSetDeadline(&tv, timeout);
	err = select(s+1,NULL,&fdcon,NULL,&tv);
	if (err<=0) connected = 0;
	else {
	    res = write(s, buffer + writed, size - writed);
	    if (res<=0) connected = 0;
	}
    }

    return size - writed;
#else /* HAVE_SYS_SELECT_H */
    return -1;
#endif /* HAVE_SYS_SELECT_H */
}

size_t rccExternalRead(int s, char *buffer, ssize_t size, unsigned long timeout) {
#ifdef HAVE_SYS_SELECT_H
    int err;
    unsigned char connected = 1;
    ssize_t readed, res = 0;
    struct timeval tv;
    fd_set fdcon;
    
    if (s == -1) return -1;
    
    for (readed = 0; ((readed < size)&&(connected)); readed += connected?res:0) {
	FD_ZERO(&fdcon);
	FD_SET(s, &fdcon);
	rccExternalSetDeadline(&tv, timeout);
	err = select(s+1,&fdcon,NULL,NULL,&tv);
	if (err<=0) connected = 0;
	else {
	    res = read(s, buffer + readed, size - readed);
	    if (res<=0) connected = 0;
	}
    }

    return size - readed;
#else /* HAVE_SYS_SELECT_H */
    return -1;
#endif /* HAVE_SYS_SELECT_H */
}

int rccExternalConnect(unsigned char module) {
#ifdef HAVE_SYS_SOCKET_H
    int err;
    int retries = 10;
    int sock;
    int flags;
    struct sockaddr_un mysock;
    struct timeval tv;
    struct timespec ts;
    fd_set fdcon;
    
    if (pid == (pid_t)-1) return -1;
    
    sock = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sock<=0) return -1;

    flags = fcntl(sock,F_GETFL,0);
    if (flags<0) flags = 0;
    if (fcntl(sock,F_SETFL,flags|O_NONBLOCK)<0) {
	close(sock);
	return -1;
    }
        
    memset(&mysock, 0, sizeof(mysock));
    mysock.sun_family=AF_UNIX;
    strncpy(mysock.sun_path,addr,sizeof(mysock.sun_path));
    mysock.sun_path[sizeof(mysock.sun_path)-1]=0;

again:
    if (connect(sock,(struct sockaddr*)&mysock,sizeof(mysock))<0) {
        if (errno == EINPROGRESS) {
	    FD_ZERO(&fdcon);
	    FD_SET(sock, &fdcon);

	    rccExternalSetDeadline(&tv, 0);
	    err = select(sock+1,&fdcon,NULL,NULL,&tv);
	    if (err<=0) {
		close(sock);
		return -1;
	    }
	} else if ((errno == ENOENT)&&(retries)) {
	    ts.tv_sec = (RCC_EXTERNAL_TIMEOUT/10) / 1000000;
	    ts.tv_nsec = ((RCC_EXTERNAL_TIMEOUT/10) % 1000000)*1000;
	    nanosleep(&ts, NULL);
	    retries--;
	    goto again;
	} else {
	    close(sock);
	    return -1;
	}
    }
    
    if (rccExternalWrite(sock, (char*)&module, 1, 0)) {
	close(sock);
	return -1;
    }

    return sock;
#else /* HAVE_SYS_SOCKET_H */
    return -1;
#endif /* HAVE_SYS_SOCKET_H */
}

void rccExternalClose(int s) {
#ifdef HAVE_SYS_SOCKET_H
    unsigned char cmd = 0;
    if (s != -1) {
	write(s, &cmd, 1);
	close(s);
    }
#endif /* HAVE_SYS_SOCKET_H */
}

int rccExternalAllowOfflineMode() {
    int sock;
    int err;
    rcc_external_option opt = RCC_EXTERNAL_OPTION_OFFLINE; 
    unsigned long opt_value = 1;
    
    sock = rccExternalConnect(RCC_EXTERNAL_MODULE_OPTIONS);
    if (sock) {
	err = rccExternalWrite(sock, (char*)&opt, sizeof(rcc_external_option), 0);
	if (!err) err = rccExternalWrite(sock, (char*)&opt_value, sizeof(unsigned long), 0);
	rccExternalClose(sock);
	return err;
    }
    return -1;
}
