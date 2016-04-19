/*
 * ThreadData.h
 *
 *  Created on: Apr 11, 2016
 *      Author: sri
 */
using namespace std;
#include "Label.h"
#include<vector>
#ifndef THREADDATA_H_
#define THREADDATA_H_

class ReceivedMessage {
public:
	ReceivedMessage(int _partitionId, unsigned long _heartbeatTimestamp,
			vector<Label*> _iLabel);
	virtual ~ReceivedMessage();
	unsigned long GetHeartbeat();
	int GetPartionId();
	vector<Label*> GetLabels();
private:
	unsigned long m_iHartbeatTimestamp; //to be used for vector to calcutale the min_stable
	int m_iPartionId;
	vector<Label*> m_iLabels; //{Label.timestamp,Label.partitionId}=Key, Value=Label in the datastructure
};

#endif /* THREADDATA_H_ */
