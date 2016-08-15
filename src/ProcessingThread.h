

#ifndef PROCESSINGTHREAD_H_
#define PROCESSINGTHREAD_H_

#include <pthread.h>
#include <vector>
#include <sstream>
#include <fstream>
#include "ThreadUtil.h"
#include "Label.h"
#include "ReceivedMessage.h"
#include <iostream>
#include <map>
using namespace utl::que;
using namespace utl;

class ProcessingThread {
public:
	ProcessingThread();  // Private so that it can  not be called
	virtual ~ProcessingThread();
	void InitProcessingThread(EventQueueFrame *_ptrThreadQ,
			QueueSizeCondition *_ptrCondtionLock,
			int _iThreadSingleContainerSize);
	static void * Run(void * _pProcessor);
	pthread_t StartProcessingThread(ProcessingThread *_ptrProcessingThread);
	void SetClientCount(int count);
	void setDeleteThreshold(int deleteThreshold);
	bool openConnectionToLabelReceiver(int port);
	void setLabelDeliverySize(int batchSize);
	void StopProcessorThread() {
		m_bIsIinterrupt = true;
	}
private:
	int processQ();
	void UpdateHeartbeatTable(int _iPartitionId, unsigned long _iHeartbeat);
	unsigned long GetStableTimestamp(int _iPartitionId);
	void InsertBatchLabels(vector<Label*> _iLabels);
	void DeletePossibleLabels(unsigned long StableTimestamp);
	void DoPossibleBatchDelivery();
	void StartProcesser();
	QueueSizeCondition *m_ptrCondtionLock;
	int m_iSingleContainerSize;

	int m_iclientCount;
	long m_ideleteThreshold;
	int m_socketfd;
	int m_labelDeliverySize;

	EventQueueFrame *m_ptrComQ;
	pthread_t m_threadid;
	volatile bool m_bIsIinterrupt;
	struct timeval m_tPerformanceTime;
	long m_processed;
	long int m_oldTimerTick;
	std::map<int, unsigned long> m_heartbeat;
	std::multimap<unsigned long, int> m_storage;
	std::vector<int> deletedList;

};

#endif /* PROCESSINGTHREAD_H_ */
