#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../config.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif /* HAVE_PWD_H */

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
#endif /* JAVE_SYS_TIME_H */

#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif /* HAVE_SIGNAL_H */

#include <glib/gthread.h>

#include "../src/rcchome.h"
#include "../src/rccexternal.h"
#include "rcclibtranslate.h"

#define RCC_EXIT_CHECK_TIMEOUT 10 /* seconds */


char rcc_external_offline = 0;

int main() {
#ifdef HAVE_SIGNAL_H
    struct sigaction act;
#endif /* HAVE_PWD_H */
    int err;
    struct timeval tv;
    fd_set fdcon;

    int s, sd;
    char addr[376];
    struct sockaddr_un mysock, clisock;
    socklen_t socksize;
    
    pid_t parentpid;
    pid_t mypid;
    
    unsigned char loopflag = 1;
    
    rcc_external_info info;
    rcc_external_option option;
    unsigned long option_value_long;
    
    ssize_t readed;
    unsigned char cmd;

    parentpid = getppid();    
    mypid = getpid();
    
    rccHomeSet();
    rccLibTranslateInit(rcc_home_dir);

    sprintf(addr,"%s/.rcc/comm/",rcc_home_dir);
    mkdir(addr, 00700);
    sprintf(addr,"%s/.rcc/comm/%lu.sock", rcc_home_dir, (unsigned long)mypid);
    
    s = socket(PF_UNIX, SOCK_STREAM, 0);
    if (!s) return -1;
    
    memset(&mysock, 0, sizeof(mysock));
    mysock.sun_family=AF_UNIX;
    strncpy(mysock.sun_path,addr,sizeof(mysock.sun_path));
    mysock.sun_path[sizeof(mysock.sun_path)-1]=0;

    unlink(addr);

    if (bind(s,(struct sockaddr*)&mysock,sizeof(mysock))==-1) return -1;
    if (listen(s,1)<0) {
	unlink(addr);
	return -1;
    }

#ifdef HAVE_SIGNAL_H
    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGPIPE,&act,NULL);
    sigaction(SIGINT,&act,NULL);
#endif /* HAVE_SIGNAL_H */

    while (loopflag) {
	tv.tv_sec = RCC_EXIT_CHECK_TIMEOUT;
	tv.tv_usec = 0;

	FD_ZERO(&fdcon);
	FD_SET(s, &fdcon);
	
	err = select(s+1, &fdcon, NULL, NULL, &tv);
	if (err<=0) {
	    if (getppid() != parentpid) break;
	    continue;
	}

	sd = accept(s,(struct sockaddr*)&clisock,&socksize);
	if (sd < 0) continue;
	
	readed = recv(sd,&cmd,1,0);
	if (readed<=0) {
	    close(sd);
	    continue;
	}
	
	switch (cmd) {
	    case RCC_EXTERNAL_MODULE_CONTROL:
		loopflag = 0;
	    break;
	    case RCC_EXTERNAL_MODULE_OPTIONS:
		readed = recv(sd,&option,sizeof(rcc_external_option),0);
		if (readed < sizeof(rcc_external_option)) break;
		switch(option) {
		    case RCC_EXTERNAL_OPTION_OFFLINE:
			readed = recv (sd, &option_value_long, sizeof(unsigned long), 0);
			if (readed < sizeof(unsigned long)) break;
			puts("got an offline option");
			rcc_external_offline = option_value_long?1:0;
		    break;
		    default:
			break;
		}
	    break;
	    case RCC_EXTERNAL_MODULE_LIBRTRANSLATE:
		info = (rcc_external_info)malloc(sizeof(rcc_external_info_s));
		if (info) info->s = sd;
		else break;
		if (g_thread_create(rccLibTranslate, info, FALSE, NULL)) continue;
	    break;
	}
	close(sd);
    }

    close(s);
    unlink(addr);  

    rccLibTranslateFree();
    rccHomeFree();
        
    return 0;
}
