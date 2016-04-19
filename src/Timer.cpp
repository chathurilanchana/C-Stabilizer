/*
 * Timer.cpp
 *
 *  Created on: Apr 15, 2016
 *      Author: sri
 */

#include "SocketManager.h"
Timer::Timer(unsigned int _uiInterval, TimerCallback * _pTCB, bool _bIsRepeated,
		unsigned int _uiTimerID, SocketManager * _pSocketMan, int _iTimerType) :
		ServerType(E_EVT_TIMER) {
	ui_Interval = _uiInterval;
	ui_NextFire = 0;
	p_TimerCB = _pTCB;
	p_SocketMan = _pSocketMan;
	b_IsRepeated = _bIsRepeated;
	ui_TimerID = _uiTimerID;
	b_IsRemovedTimer = false;
	p_Data = NULL;
	i_TimerType = _iTimerType;
}

unsigned int Timer::GetInterval() {
	return ui_Interval;
}

unsigned int Timer::GetNextFire() {
	return ui_NextFire;
}

unsigned int Timer::GetTimerID() {
	return ui_TimerID;
}

void Timer::SetNextFire(unsigned int _uiFireTime) {
	ui_NextFire = _uiFireTime;
}

TimerCallback * Timer::GetTimerCB() {
	return p_TimerCB;
}

void Timer::ResetTimerId() {
	ui_TimerID = 0;
}

bool Timer::IsRepeated() {
	return b_IsRepeated;
}

void Timer::SetIsRemovedTimer(bool _State) {
	b_IsRemovedTimer = _State;
}

TimerCallback::~TimerCallback() {
}

Timer::~Timer() {
	if (!b_IsRemovedTimer)
		p_SocketMan->AddToDeletedTimerSet(ui_TimerID);
}

