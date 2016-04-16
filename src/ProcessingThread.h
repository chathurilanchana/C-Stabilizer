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
	void StopProcessorThread() {
		m_bIsIinterrupt = true;
	}
private:
	int processQ();
	void StartProcesser();
	QueueSizeCondition *m_ptrCondtionLock;
    int m_iSingleContainerSize;
    EventQueueFrame *m_ptrComQ;
	pthread_t m_threadid;
	volatile bool m_bIsIinterrupt;
	struct timeval m_tPerformanceTime;
};

#endif /* PROCESSINGTHREAD_H_ */
