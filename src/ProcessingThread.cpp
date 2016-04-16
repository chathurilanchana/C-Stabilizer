/*
 * ProcessingThread.cpp
 *
 *  Created on: Apr 11, 2016
 *      Author: sri
 */

using namespace std;
#include "ProcessingThread.h"
#include "Label.h"
#include<vector>
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <algorithm>


#define SECONDS      1000000000
#define MILLISECONDS 1000000
#define MICROSECONDS 1000
#define NANOSECONDS  1

using namespace utl;


ProcessingThread::ProcessingThread() {
	m_ptrCondtionLock = NULL;
	m_ptrComQ=NULL;
	m_bIsIinterrupt = false;
	m_threadid = 0;
	m_iSingleContainerSize=0;
	m_processed=0;

}

ProcessingThread::~ProcessingThread() {
	std::cout << "[ProcessingThread] Deallocating the memory now "
			<< std::endl;
}
void ProcessingThread::InitProcessingThread(
		EventQueueFrame *_ptrJavaObjectDispatcher,
		QueueSizeCondition *_ptrCondtionLock, int _iThreadSingleContainerSize) {

	m_ptrComQ = _ptrJavaObjectDispatcher;
	m_iSingleContainerSize = _iThreadSingleContainerSize;
	m_ptrCondtionLock = _ptrCondtionLock;

}
void *ProcessingThread::Run(void * _pLHandler) {
	((ProcessingThread*) _pLHandler)->StartProcesser();
	return NULL;
}


pthread_t ProcessingThread::StartProcessingThread(
		ProcessingThread *_ptrProcessingThread) {

	pthread_create(&m_threadid, NULL, (void*(*)(void*)) ProcessingThread::Run, (void*) _ptrProcessingThread);
	printf(
	"[ProcessingThread][INFO] ############  ProcessingThread  id -  %li \n",
	m_threadid);

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	return m_threadid;
}

void ProcessingThread::StartProcesser() {

	while (true) {
		int l_iPrcoeedSize = processQ();
		m_ptrCondtionLock->DecreaseQueueSize(l_iPrcoeedSize);
	}
}

int ProcessingThread::processQ() {

	EventDataPacket * pCont = NULL;
	int l_iProcessedMsg = 0;

	m_ptrComQ->PollFromIntermediateQueue();
	while ((pCont = m_ptrComQ->PollFromConsumerQueue())) {
		ReceivedMessage * _pMsg = (ReceivedMessage*) pCont->m_ptrData;
		vector<Label> _pLabels=_pMsg->GetLabels();
		/*printf(
					"The processed message is  %ld %i %ld \n",
					 _pMsg->GetHeartbeat(),_pMsg->GetPartionId(), _pLabels.size());*/
		m_processed=m_processed+1;
		if ((m_processed% 10)==0){
			printf("count is %ld \n",m_processed);
		}
		delete _pMsg;
		delete pCont;
		pCont = NULL;
		vector<Label>().swap( _pLabels );
	}
	return l_iProcessedMsg * m_iSingleContainerSize;

}
