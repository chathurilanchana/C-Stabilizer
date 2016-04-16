
/*
 * ThreadData.cpp
 *
 *  Created on: Apr 11, 2016
 *      Author: sri
 */
using namespace std;
#include<vector>
#include "ReceivedMessage.h"

ReceivedMessage::ReceivedMessage(int _iPartionId, unsigned long _iHartbeatTimestamp, vector <Label> _iLabels) {

	m_iHartbeatTimestamp = _iHartbeatTimestamp;
	m_iPartionId = _iPartionId;
	m_iLabels = _iLabels;
}

ReceivedMessage::~ReceivedMessage() {
}

unsigned long ReceivedMessage::GetHeartbeat() {
	return m_iHartbeatTimestamp;
}
int ReceivedMessage::GetPartionId() {
	return m_iPartionId;
}
vector<Label> ReceivedMessage::GetLabels() {
	return m_iLabels;
}