
#ifndef THREADUTIL_H_
#define THREADUTIL_H_

#include <iostream>
#include <list>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

namespace utl {
namespace que {
class EventDataPacket {
public:
	EventDataPacket(void *);
	EventDataPacket();
	virtual ~EventDataPacket() {
	}
	void * m_ptrData;
	EventDataPacket* m_ptrNext;
};
class Queue {
public:
	EventDataPacket * m_ptrHead, *m_ptrTail;
	Queue();
	void PushToQueue(EventDataPacket *);
};
class EventQueueFrame {
public:
	EventQueueFrame();
	virtual ~EventQueueFrame();
	void AddToProducerQueue(EventDataPacket*);
	void AddToProducerQueueWithLock(EventDataPacket*);
	EventDataPacket* PollFromConsumerQueue();
	void PushToIntermediateQueue();
	void PollFromIntermediateQueue();
public:
	Queue * m_ptrProducerQueue;
	Queue * m_ptrIntermediateQueue;
	Queue *m_ptrConsumerQueue;
	pthread_mutex_t m_mutexQueueHolder;

};

class QueueSizeCondition {
public:
	QueueSizeCondition();
	~QueueSizeCondition();
	int getQueueTotalSize() {
		return m_iObectSize;
	}
	void IncreaseQueueSize(unsigned int _iValue);
	void DecreaseQueueSize(unsigned int _iValue);
	void InitQueueSizeCondition(unsigned long long _iMaxCapcity);
private:
	pthread_mutex_t m_mutexQueueSize;
	pthread_cond_t m_condQueueSize;
	unsigned long long m_iObectSize;
	unsigned long long m_iMaxQueueCapacity;

};
class ThreadToken {
public:
	ThreadToken();
	~ThreadToken();
	void SendSignal();
	void WaitForSignal();
	void TerminateIgnition();
private:
	bool m_isPreviousPrcessed;
	pthread_mutex_t m_mutexThreadToken;
	pthread_cond_t m_condThreadToken;

};

}
class Timer_Q {
public:
	Timer_Q();
	Timer_Q(int _iThreadSleepInterval, bool _isThisMilliSleep);
	unsigned long long GetMonotonicTime();
	char * GetUniqString(int _iSuffix);
	unsigned long long GetEpochTime();
	char * GetCurrentTimeStamp();
	~Timer_Q();
	void NanoSleep();
private:
	int m_iSleepInterval;
	struct timespec m_stNanoTimeSpec;
};

}
#endif /* THREADUTIL_H_ */
