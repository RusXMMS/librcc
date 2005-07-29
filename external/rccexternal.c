#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#include <glib/gthread.h>

#include "../src/rccexternal.h"
#include "rcclibtranslate.h"


int main() {
    int s, sd;
    char addr[376];
    const char *rcc_home_dir;
    struct sockaddr_un mysock, clisock;
    socklen_t socksize;
    pid_t mypid;
    
    unsigned char loopflag = 1;
    
    rcc_external_info info;
    ssize_t readed;
    unsigned char cmd;

#ifdef HAVE_PWD_H
    struct passwd *pw;
#endif /* HAVE_PWD_H */

    mypid = getpid();

    rcc_home_dir = getenv ("HOME");
#ifdef HAVE_PWD_H
    if (!rcc_home_dir) {
	setpwent ();
	pw = getpwuid(getuid ());
	endpwent ();
	if ((pw)&&(pw->pw_dir)) rcc_home_dir = pw->pw_dir;
    }
#endif /* HAVE_PWD_H */
    if (strlen(rcc_home_dir)>256) return -1;
    if (!rcc_home_dir) rcc_home_dir = "/";

    rccLibTranslateInit(rcc_home_dir);

    sprintf(addr,"%s/.rcc/comm/",rcc_home_dir);
    mkdir(addr, 00600);
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

    while (loopflag) {
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
    
    return 0;
}
