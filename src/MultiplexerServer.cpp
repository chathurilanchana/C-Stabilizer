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

using namespace std;

#define SIZE_OF_EVENTPACKET 24

int main(int argc, char **argv) {

	// start the server, this call is blocking call, never return!!!
	//ServerAPI *ptrServer = new ServerAPI();
    //delete ptrServer;
	printf("arg1:clientcount , arg2:delete log threshold, arg3:labelReceiverPort, arg4:deliveryLabelSize \n");

    int m_clientCount=atoi(argv[1]);
    long m_deleteThreshold=atol(argv[2]);//after how many deletes we should print
    int m_labelReceiverPort=atoi(argv[3]);
    int m_labelDeliverySize=atoi(argv[4]);// size when we deliver the stable labels

    printf("client count; %i delete threshold:%ld port: %i :batch del: %i \n",m_clientCount,m_deleteThreshold
    		,m_labelReceiverPort,m_labelDeliverySize);

	QueueSizeCondition *m_ptrCondtionLock = new QueueSizeCondition();
	m_ptrCondtionLock->InitQueueSizeCondition(50000);

	EventQueueFrame *l_ptrQ = new EventQueueFrame();
	ProcessingThread *l_ptrProcThread = new ProcessingThread();
	l_ptrProcThread->InitProcessingThread(l_ptrQ, m_ptrCondtionLock, SIZE_OF_EVENTPACKET);
	l_ptrProcThread->StartProcessingThread(l_ptrProcThread);
    l_ptrProcThread->SetClientCount(m_clientCount);
    l_ptrProcThread->setDeleteThreshold(m_deleteThreshold);//after how many deletes we print
    bool m_receiverStatus=l_ptrProcThread->openConnectionToLabelReceiver(m_labelReceiverPort);
    l_ptrProcThread->setLabelDeliverySize(m_labelDeliverySize);

    printf("receiver status is %d \n",m_receiverStatus);

	ServerAPI *l_ptrRec = new ServerAPI();
	l_ptrRec->InintReceiverThread(l_ptrQ, m_ptrCondtionLock, SIZE_OF_EVENTPACKET);
	l_ptrRec->StartReceiverThread(l_ptrRec);

	while (true) {
		sleep(5);
	}

	delete m_ptrCondtionLock;
	delete l_ptrQ;
	//delete l_ptrProcThread;
	delete l_ptrRec;
	return 0;
	 return 0;
}
