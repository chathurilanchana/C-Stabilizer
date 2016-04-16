/*
 * ServerDef.h
 *
 *  Created on: Apr 15, 2016
 *      Author: sri
 */

#ifndef SERVERDEF_H_
#define SERVERDEF_H_

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <list>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <pthread.h>
#include <queue>
#include <set>
#include <signal.h>
#include <sstream>
#include <stack>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
using namespace std;
#ifndef EPOLLRDHUP
#define EPOLLRDHUP							0x2000
#endif
#define EPOLL_SVR_EVENT_FLAGS			EPOLLET | EPOLLPRI | EPOLLIN
#define EPOLL_CLI_EVENT_FLAGS			EPOLLET | EPOLLPRI | EPOLLIN | EPOLLRDHUP | EPOLLERR

#define GET_CURR_TIME(VAR,RETVAL,RETMICRO)	gettimeofday(&VAR,NULL); RETMICRO = VAR.tv_usec ; RETVAL = VAR.tv_sec * 1000 + RETMICRO / 1000;

#define MAX_MAX_CLIENT_COUNT 	65000
#define MAX_ARRAY_SIZE 		 	1024
#define MAX_TIMERS_LIMIT		128
#define MAX_TIMERS_INCREASE_INTERVAL		128
#define MAX_PULSE_TIMEOUT		30

#define MAX_EPOL_EVENTS			128
#define MAX_EPOL_WAIT	 		5
#define CLI_READ_SIZE 15000

enum TYPES
{
	E_EVT_CLIENT = 0,
	E_EVT_SERVER,
	E_EVT_TIMER,
	E_EVT_TYPES_COUNT
};

class ServerType
{
public:
	ServerType(TYPES _etEType)
	{
		et_EType = _etEType;
	}
	virtual ~ServerType(){};
	inline TYPES getEvtType(){return et_EType ; };


private:
	TYPES et_EType;
};



#endif /* SERVERDEF_H_ */
