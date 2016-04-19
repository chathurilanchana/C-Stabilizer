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
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SECONDS      1000000000
#define MILLISECONDS 1000000
#define MICROSECONDS 1000
#define NANOSECONDS  1

using namespace utl;

ProcessingThread::ProcessingThread() {
	m_ptrCondtionLock = NULL;
	m_ptrComQ = NULL;
	m_bIsIinterrupt = false;
	m_threadid = 0;
	m_iSingleContainerSize = 0;
	m_processed = 0;
	m_oldTimerTick = 0;

}

ProcessingThread::~ProcessingThread() {
	std::cout << "[ProcessingThread] Deallocating the memory now " << std::endl;
}
void ProcessingThread::InitProcessingThread(
		EventQueueFrame *_ptrJavaObjectDispatcher,
		QueueSizeCondition *_ptrCondtionLock, int _iThreadSingleContainerSize) {

	m_ptrComQ = _ptrJavaObjectDispatcher;
	m_iSingleContainerSize = _iThreadSingleContainerSize;
	m_ptrCondtionLock = _ptrCondtionLock;

}

void ProcessingThread::SetClientCount(int count) {
	m_iclientCount = count;
	for (int n = 1; n < count; ++n) {
		m_heartbeat[n] = 0; //intilalize heartbeat
	}
}

bool ProcessingThread::openConnectionToLabelReceiver(int port) {
	struct sockaddr_in serv_addr;
	struct hostent *server;

	m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socketfd < 0) {
		printf("ERROR WHILE OPENING THE SOCKET \n");
		return false;
	}
	server = gethostbyname("127.0.0.1"); //assuming receiver on same node
	if (server == NULL) {
		printf("NO SUCH HOST EXISTS \n");
		return false;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr,
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
	serv_addr.sin_port = htons(port);
	if (connect(m_socketfd, (const sockaddr*) &serv_addr, sizeof(serv_addr))
			< 0) {
		printf("ERROR WHILE CONNECTING \n");
		return false;
	}

	return true;
}

void ProcessingThread::setLabelDeliverySize(int batchSize) {
	m_labelDeliverySize = batchSize;
}

void ProcessingThread::setDeleteThreshold(int deleteThreshold) {
	m_ideleteThreshold = deleteThreshold;
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

void ProcessingThread::UpdateHeartbeatTable(int _iPartitionId,
		unsigned long _iHeartbeat) {
	m_heartbeat[_iPartitionId] = _iHeartbeat;
}

unsigned long ProcessingThread::GetStableTimestamp(int _iPartitionId) {
	unsigned long l_minStableTimestamp = m_heartbeat[_iPartitionId];
	std::map<int, unsigned long>::iterator it;
	for (it = m_heartbeat.begin(); it != m_heartbeat.end(); ++it) {
		if (it->second < l_minStableTimestamp) {
			l_minStableTimestamp = it->second;
		}
	}
	return l_minStableTimestamp;
}

void ProcessingThread::InsertBatchLabels(vector<Label*> _iLabels) {
	for (size_t n = 0; n < _iLabels.size(); ++n) {
		Label *pLabel1 = _iLabels[n];
		m_storage.insert(
				std::pair<unsigned long, int>(pLabel1->GetTimestamp(),
						pLabel1->GetValue()));
	}
}

void ProcessingThread::DeletePossibleLabels(unsigned long StableTimestamp) {
	std::multimap<unsigned long, int>::iterator it, itup;

	it = m_storage.begin();
	while (true) {
		{
			if ((*it).first > StableTimestamp){
							break;
			}
			deletedList.push_back((*it).second);
			m_processed = m_processed + 1;
			it++;
		}
	}
	itup = it--;
	m_storage.erase(m_storage.begin(), itup);

	/*itup = m_storage.upper_bound(StableTimestamp);
	 for (it = m_storage.begin(); it != itup; ++it) {
	 deletedList.push_back((*it).second);
	 m_processed = m_processed + 1;
	 }
	 m_storage.erase(m_storage.begin(), itup);*/

	//printf("after delete contains %ld \n", m_storage.size());
}

void ProcessingThread::DoPossibleBatchDelivery() {
	int deletedCount = deletedList.size();
	if (deletedCount > m_labelDeliverySize) {
		write(m_socketfd, &deletedCount, sizeof(int));
		int n = write(m_socketfd, &deletedList[0], deletedCount * sizeof(int));
		if (n < 0)
			printf("********ERROR delivering labels********* \n");
		deletedList.clear();
	}
}

int ProcessingThread::processQ() {

	EventDataPacket * pCont = NULL;
	int l_iProcessedMsg = 0;
	m_ptrComQ->PollFromIntermediateQueue();
	while ((pCont = m_ptrComQ->PollFromConsumerQueue())) {
		++l_iProcessedMsg;
		ReceivedMessage * _pMsg = (ReceivedMessage*) pCont->m_ptrData;
		//vector<Label*> _pLabels=_pMsg->GetLabels();
		/*printf(
		 "The processed message is  %ld %i %ld \n",
		 _pMsg->GetHeartbeat(),_pMsg->GetPartionId(), _pLabels.size());*/
		int l_ipartitionId = _pMsg->GetPartionId();

		InsertBatchLabels(_pMsg->GetLabels());

		UpdateHeartbeatTable(l_ipartitionId, _pMsg->GetHeartbeat());
		unsigned long l_iStable = GetStableTimestamp(l_ipartitionId);
		DeletePossibleLabels(l_iStable);
		DoPossibleBatchDelivery();

		if (m_processed >= m_ideleteThreshold) {
			struct timeval tp;
			gettimeofday(&tp, NULL);
			long int l_newTick = tp.tv_sec * 1000 + tp.tv_usec / 1000;
			long int duration = l_newTick - m_oldTimerTick;
			std::cout << "Duration in ms: " << duration << " Delete count: "
					<< m_processed << "\n";
			m_oldTimerTick = l_newTick;
			m_processed = 0;
		}
		delete _pMsg;
		delete pCont;
		pCont = NULL;
	}
	return l_iProcessedMsg * 24;

}
