/*
 * ProcessingThread.h
 *
 *  Created on: Apr 11, 2016
 *      Author: sri
 */

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
			QueueSizeCondition *_ptrCondtionLock,int _iThreadSingleContainerSize);
	static void * Run(void * _pProcessor);
	pthread_t StartProcessingThread(ProcessingThread *_ptrProcessingThread);
	void SetClientCount(int count);
	void setDeleteThreshold(int deleteThreshold);
	void StopProcessorThread() {
		m_bIsIinterrupt = true;
	}
private:
	int processQ();
	void UpdateHeartbeatTable(int _iPartitionId,unsigned long _iHeartbeat);
	unsigned long GetStableTimestamp(int _iPartitionId);
	void InsertBatchLabels(vector<Label*> _iLabels);
	std::list<int> DeletePossibleLabels(unsigned long StableTimestamp);
	void StartProcesser();
	QueueSizeCondition *m_ptrCondtionLock;
    int m_iSingleContainerSize;

    int m_iclientCount;
    long m_ideleteThreshold;

    EventQueueFrame *m_ptrComQ;
	pthread_t m_threadid;
	volatile bool m_bIsIinterrupt;
	struct timeval m_tPerformanceTime;
	long m_processed;
	long int m_oldTimerTick;
	std::map<int,unsigned long> m_heartbeat;
	std::multimap<unsigned long,int> m_storage;
};

#endif /* PROCESSINGTHREAD_H_ */
