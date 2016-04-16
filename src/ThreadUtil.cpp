/*
 * ThreadUtil.cpp
 *
 *  Created on: Apr 11, 2016
 *      Author: sri
 */


#include <stdio.h>
#include <sys/time.h>
#include "ThreadUtil.h"


using namespace utl::que;
using namespace utl;

#define SECONDS      1000000000
#define MILLISECONDS 1000000
#define MICROSECONDS 1000
#define NANOSECONDS  1


EventDataPacket::EventDataPacket() {
	m_ptrData = NULL;
	m_ptrNext = NULL;
}
EventDataPacket::EventDataPacket(void * _pData) {
	m_ptrData = _pData;
	m_ptrNext = NULL;
}

Queue::Queue() {
	m_ptrHead = NULL;
	m_ptrTail = NULL;
}
void Queue::PushToQueue(EventDataPacket * _pDataContainer) {
	if (m_ptrHead) {
		m_ptrTail->m_ptrNext = _pDataContainer;
	} else {
		m_ptrHead = _pDataContainer;
	}
	m_ptrTail = _pDataContainer;
}

EventQueueFrame::EventQueueFrame() {
	m_ptrProducerQueue = new Queue();
	m_ptrIntermediateQueue = new Queue();
	m_ptrConsumerQueue = new Queue();

	pthread_mutex_init(&m_mutexQueueHolder, NULL);

}
EventQueueFrame::~EventQueueFrame() {
	delete m_ptrProducerQueue;
	delete m_ptrIntermediateQueue;
	delete m_ptrConsumerQueue;
}
void EventQueueFrame::AddToProducerQueue(
		EventDataPacket * _pDataContainer) {
	m_ptrProducerQueue->PushToQueue(_pDataContainer);
}
void EventQueueFrame::AddToProducerQueueWithLock(
		EventDataPacket * _pDataContainer) {
	pthread_mutex_lock(&m_mutexQueueHolder);
	m_ptrProducerQueue->PushToQueue(_pDataContainer);
	pthread_mutex_unlock(&m_mutexQueueHolder);

}
void EventQueueFrame::PushToIntermediateQueue() {
	pthread_mutex_lock(&m_mutexQueueHolder);
	if (m_ptrProducerQueue->m_ptrHead) {
		if (m_ptrIntermediateQueue->m_ptrHead) {
			m_ptrIntermediateQueue->m_ptrTail->m_ptrNext =
					m_ptrProducerQueue->m_ptrHead;
		} else {
			m_ptrIntermediateQueue->m_ptrHead = m_ptrProducerQueue->m_ptrHead;
		}
		m_ptrIntermediateQueue->m_ptrTail = m_ptrProducerQueue->m_ptrTail;
		m_ptrProducerQueue->m_ptrHead = NULL;
		m_ptrProducerQueue->m_ptrTail = NULL;
	}

	pthread_mutex_unlock(&m_mutexQueueHolder);
}
void EventQueueFrame::PollFromIntermediateQueue() {
	pthread_mutex_lock(&m_mutexQueueHolder);
	if (m_ptrIntermediateQueue->m_ptrHead) {
		if (m_ptrConsumerQueue->m_ptrHead) {
			m_ptrConsumerQueue->m_ptrTail->m_ptrNext =
					m_ptrIntermediateQueue->m_ptrHead;
		} else {
			m_ptrConsumerQueue->m_ptrHead = m_ptrIntermediateQueue->m_ptrHead;
		}
		m_ptrConsumerQueue->m_ptrTail = m_ptrIntermediateQueue->m_ptrTail;
		m_ptrIntermediateQueue->m_ptrHead = NULL;
		m_ptrIntermediateQueue->m_ptrTail = NULL;
	}
	pthread_mutex_unlock(&m_mutexQueueHolder);
}
EventDataPacket* EventQueueFrame::PollFromConsumerQueue() {
	if (m_ptrConsumerQueue->m_ptrHead) {
		EventDataPacket* pCont = m_ptrConsumerQueue->m_ptrHead;
		m_ptrConsumerQueue->m_ptrHead =
				m_ptrConsumerQueue->m_ptrHead->m_ptrNext;
		pCont->m_ptrNext = NULL;
		return pCont;
	} else
		return NULL;
}

ThreadToken::ThreadToken() {
	pthread_mutex_init(&m_mutexThreadToken, NULL);
	pthread_cond_init(&m_condThreadToken, NULL);
	m_isPreviousPrcessed = false;
}
ThreadToken::~ThreadToken() {
	pthread_mutex_destroy(&m_mutexThreadToken);
	pthread_cond_destroy(&m_condThreadToken);
}
void ThreadToken::WaitForSignal() {
	pthread_mutex_lock(&m_mutexThreadToken);
	while (!m_isPreviousPrcessed) {
		pthread_cond_wait(&m_condThreadToken, &m_mutexThreadToken);
	}
	m_isPreviousPrcessed = false;
	pthread_mutex_unlock(&m_mutexThreadToken);
}
void ThreadToken::SendSignal() {
	pthread_mutex_lock(&m_mutexThreadToken);
	m_isPreviousPrcessed = true;
	pthread_cond_signal(&m_condThreadToken);
	pthread_mutex_unlock(&m_mutexThreadToken);
}

QueueSizeCondition::QueueSizeCondition() {
	pthread_mutex_init(&m_mutexQueueSize, NULL);
	pthread_cond_init(&m_condQueueSize, NULL);
	m_iObectSize = 0;
	m_iMaxQueueCapacity = 0;
}
QueueSizeCondition::~QueueSizeCondition() {
	pthread_cond_destroy(&m_condQueueSize);
	pthread_mutex_destroy(&m_mutexQueueSize);
}

void QueueSizeCondition::InitQueueSizeCondition(
		unsigned long long _iMaxCapcity) {
	m_iMaxQueueCapacity = _iMaxCapcity;
}

void QueueSizeCondition::IncreaseQueueSize(unsigned int _iValue) {
	pthread_mutex_lock(&m_mutexQueueSize);
	while (m_iObectSize >= m_iMaxQueueCapacity) {
		pthread_cond_wait(&m_condQueueSize, &m_mutexQueueSize);
	}
	m_iObectSize += _iValue;
	/* A woken thread must acquire the lock, so it will also have to wait until we call unlock*/
	pthread_cond_signal(&m_condQueueSize);
	pthread_mutex_unlock(&m_mutexQueueSize);

}
void QueueSizeCondition::DecreaseQueueSize(unsigned int _iValue) {
	pthread_mutex_lock(&m_mutexQueueSize);
	while (m_iObectSize == 0) {
		pthread_cond_wait(&m_condQueueSize, &m_mutexQueueSize);
	}
	m_iObectSize -= _iValue;
	pthread_cond_signal(&m_condQueueSize);
	pthread_mutex_unlock(&m_mutexQueueSize);

}

Timer_Q::Timer_Q() {
	m_iSleepInterval = 0;
}
Timer_Q::Timer_Q(int _iThreadSleepInterval,
		bool _isThisMilliSleep) {
	m_iSleepInterval = _iThreadSleepInterval;
	memset(&m_stNanoTimeSpec, 0, sizeof(m_stNanoTimeSpec));
	m_stNanoTimeSpec.tv_sec = 0;
	if (_isThisMilliSleep)
		m_stNanoTimeSpec.tv_nsec = m_iSleepInterval * 1000000L;
	else
		m_stNanoTimeSpec.tv_nsec = m_iSleepInterval;
}
Timer_Q::~Timer_Q() {

}
// calling function has to delete the buffer, otherwise, this would be memory leak
char * Timer_Q::GetCurrentTimeStamp() {
	time_t l_rawTime;
	struct tm * l_ptTimeInfo;
	char *l_ptrBuffer = new char[80];

	time(&l_rawTime);
	l_ptTimeInfo = localtime(&l_rawTime);

	strftime(l_ptrBuffer, 80, "%d-%m-%Y %I:%M:%S", l_ptTimeInfo);

	return l_ptrBuffer;

}

unsigned long long Timer_Q::GetEpochTime() {
	struct timeval l_stTimeVal;
	gettimeofday(&l_stTimeVal, NULL);
	unsigned long long l_llMillisecondsSinceEpoch =
			(unsigned long long) (l_stTimeVal.tv_sec) * 1000
					+ (unsigned long long) (l_stTimeVal.tv_usec) / 1000;
	return l_llMillisecondsSinceEpoch;
}
// calling function has to delete the buffer, otherwise, this would be a memory leak
// we can use this string for event name
char * Timer_Q::GetUniqString(int _iSuffix) {
	char *l_ptrBuffer = new char[80];
	struct timeval te;
	gettimeofday(&te, NULL); // get current time
	long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; //
	sprintf(l_ptrBuffer, "%lld_%s_%d", milliseconds, "SICS", _iSuffix);
	return l_ptrBuffer;
}
unsigned long long Timer_Q::GetMonotonicTime() {
	struct timespec l_stTimeSpec;
	int l_iReturnValue;
	l_iReturnValue = clock_gettime(CLOCK_MONOTONIC, &l_stTimeSpec);
	if (l_iReturnValue < 0) {
		printf(
				"[StreamingProcessor][ERROR] ############ Error in getting timestamp - %i\n",
				l_iReturnValue);
		exit(EXIT_FAILURE);
	}
	return (unsigned long long) l_stTimeSpec.tv_sec * SECONDS
			+ l_stTimeSpec.tv_nsec * NANOSECONDS;
}

void Timer_Q::NanoSleep() {
	nanosleep(&m_stNanoTimeSpec, (struct timespec *) NULL);
}


