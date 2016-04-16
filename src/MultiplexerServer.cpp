//============================================================================
// Name        : MultiplexerServer.cpp
// Author      : Sri
// Version     :
// Copyright   : Your copyright notice
// Description : MultiplexerServer in C++, Ansi-style
//============================================================================

#include <iostream>

#include "ServerAPI.h"
#include "ProcessingThread.h"
#include "ThreadUtil.h"

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;



int main() {

	// start the server, this call is blocking call, never return!!!
	//ServerAPI *ptrServer = new ServerAPI();
    //delete ptrServer;

	QueueSizeCondition *m_ptrCondtionLock = new QueueSizeCondition();
	m_ptrCondtionLock->InitQueueSizeCondition(50000);

	EventQueueFrame *l_ptrQ = new EventQueueFrame();
	ProcessingThread *l_ptrProcThread = new ProcessingThread();
	l_ptrProcThread->InitProcessingThread(l_ptrQ, m_ptrCondtionLock, 60);
	l_ptrProcThread->StartProcessingThread(l_ptrProcThread);

	ServerAPI *l_ptrRec = new ServerAPI();
	l_ptrRec->InintReceiverThread(l_ptrQ, m_ptrCondtionLock, 60);
	l_ptrRec->StartReceiverThread(l_ptrRec);

	while (true) {
		sleep(1);
	}

	delete m_ptrCondtionLock;
	delete l_ptrQ;
	delete l_ptrProcThread;
	delete l_ptrRec;
	return 0;
	 return 0;
}
