/*
 * HighPerformanceServer.h
 *
 *  Created on: Apr 15, 2016
 *      Author: sri
 */

#ifndef SERVERAPI_H_
#define SERVERAPI_H_

#include "SocketManager.h"
#include "ReceivedMessage.h"
#include "ThreadUtil.h"
using namespace utl::que;
using namespace utl;

class ServerAPI: public TimerCallback,
				public ServerCallback {
public:
	ServerAPI();
	virtual ~ServerAPI();
	void InintReceiverThread(EventQueueFrame *_ptrQHolder, QueueSizeCondition * _ptrCondtionLock, int _iThreadSingleContainerSize);
		void StartThread();
	static void * Run(void * _pProcessor);
	static pthread_t StartReceiverThread(ServerAPI *_ptrReceiverThread);


	void OnTimer(Timer * _pTimer);
	void OnData(Server * _pServer, Client * _pClient, char * _zData,
			int _iLen);
	void OnConnect(Server * _pServer, Client * _pClient);
	void OnDisconnect(Server * _pServer, Client * _pClient, ErrorMsgTag _Err);
	void OnReadyToSend(Server * _pServer, Client * _pClient);
	void Decode(Client *_pClient);
	ReceivedMessage* ProcessedSocketData(unsigned int _uClientId, char *_ptrData);

private:
	static pthread_t m_threadid;
		EventQueueFrame * m_pQHolder;
		int m_iSingleContainerSize;
		QueueSizeCondition * m_ptrQueueSizeCondition;
		int m_iTotalProcessingThreads;
		Timer_Q *m_ptrTimer;

	SocketManager * p_SocketMan;
	Server * p_CurProvider;
};

#endif /* SERVERAPI_H_ */
