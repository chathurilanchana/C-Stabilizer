/*
 * ServerAPI.cpp
 *
 *  Created on: Apr 15, 2016
 *      Author: sri
 */

#include "ServerAPI.h"
#include <unistd.h>
#include <sys/utsname.h>
using namespace std;
pthread_t ServerAPI::m_threadid;
#define PAYLOAD_SIZE 8
#define CARRIAGE_RETURN 13

ServerAPI::ServerAPI() {
	m_ptrTimer = new Timer_Q();
	p_SocketMan = new SocketManager();
	//p_CurProvider = p_SocketMan->CreateServer(50201, this, false);
	//p_SocketMan->Run();
}

void ServerAPI::InintReceiverThread(EventQueueFrame *_ptrQHolder,
		QueueSizeCondition * _ptrCondtionLock,
		int _iThreadSingleContainerSize) {
	m_pQHolder = _ptrQHolder;
	m_ptrQueueSizeCondition = _ptrCondtionLock;
	m_iSingleContainerSize = _iThreadSingleContainerSize;
}

ServerAPI::~ServerAPI() {
	delete p_SocketMan;
	std::cout << "[EventThread] destructor is calling now " << std::endl;

}

pthread_t ServerAPI::StartReceiverThread(ServerAPI *_ptrEventThread) {
	pthread_attr_t l_pthreadAttr;
	size_t l_pthreadStackSize;
	pthread_attr_init(&l_pthreadAttr);
	pthread_attr_getstacksize(&l_pthreadAttr, &l_pthreadStackSize);
	struct utsname l_utsName;
	uname(&l_utsName);
	printf(
			"[ReceiverThread][INFO] ########### Starting ReceiverThread ################# \n");
	printf("######## System architecture      : %s\n", l_utsName.machine);
	printf("######## Node name 		  : %s\n", l_utsName.nodename);
	printf("######## System name              : %s\n", l_utsName.sysname);
	printf("######## Kernel release           : %s\n", l_utsName.release);
	printf("######## Version                  : %s\n", l_utsName.version);
	printf("######## System stack size        : %li\n", l_pthreadStackSize);
	pthread_create(&m_threadid, NULL, (void*(*)(void*)) ServerAPI::Run, (void*)_ptrEventThread);
	printf("[ReceiverThread] ################### Starting thread id  : %li",
			m_threadid);

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	return m_threadid;
}

void *ServerAPI::Run(void * _pLHandler) {
	((ServerAPI*) _pLHandler)->StartThread();
	return NULL;
}

void ServerAPI::StartThread() {
	p_CurProvider = p_SocketMan->CreateServer(50201, this, false);
    p_SocketMan->Run();
}

void ServerAPI::OnTimer(Timer * _pTimer) {

}

void ServerAPI::ProcessedSocketData(unsigned int _uClientId, char *_ptrData) {
	/*cout << "[INFO] [ClientID - " << _uClientId << "]" << " Actual Data:  " << _ptrData
			<< endl;*/
	vector<Label> labels; //because we receive a list of labels
		std::vector<char*> v;
		char* chars_array = strtok(_ptrData, "|");
		while (chars_array) {
			v.push_back(chars_array);
			chars_array = strtok(NULL, "|");
		}
		int partitionId = atoi(v[0]);
		unsigned long heartbeat = strtoul(v[1], NULL, 0);

		std::vector<char*> lbls;
		char* lblstr = strtok(v[2], ";");
		while (lblstr) {
			lbls.push_back(lblstr);
			lblstr = strtok(NULL, ";");
		}

		for (size_t n = 0; n < lbls.size(); ++n) {
			char* lblparam = strtok(lbls[n], ":");
			std::vector<char*> lbl;
			while (lblparam) {
				lbl.push_back(lblparam);
				lblparam = strtok(NULL, ":");
			}

			unsigned long heartbeat = strtoul(lbl[0], NULL, 0);
			int value = atoi(lbl[1]);
			Label *pLabel1 = new Label(heartbeat, value);
			labels.push_back(*pLabel1);
		}
		//printf("partition id: %i heartbeat %ld \n", partitionId, heartbeat);
		ReceivedMessage *pEventData = new ReceivedMessage(partitionId, heartbeat, labels);

		m_ptrQueueSizeCondition->IncreaseQueueSize(m_iSingleContainerSize);
		EventDataPacket *pPacket = new EventDataPacket(pEventData);
		m_pQHolder->AddToProducerQueue(pPacket);
	    m_pQHolder->PushToIntermediateQueue();

	    delete [] _ptrData;
	    delete [] chars_array;
	    delete [] lblstr;

	// Note : Once we processed the data, we should delete the _ptrData by calling delete [] _ptrData.
	// Otherwise there will be a memory leak
	// 2. Put all the queue communication process here ( token, create vector labels and push)
}

void ServerAPI::Decode(Client *_pClient) {

	const char *pData = _pClient->m_RcvBuffer.GetData();
	int iRemain = _pClient->m_RcvBuffer.GetDataLen();
	//cout << "Total length of the message : " << iRemain << " Data : " << pData
	//	<< endl;
	char z_PayLoadSize[10];
	memset(z_PayLoadSize, 0, sizeof(z_PayLoadSize));
	memcpy(z_PayLoadSize, pData, PAYLOAD_SIZE);
	int l_iPayLoadLength = atoi(z_PayLoadSize);

	while (l_iPayLoadLength <= iRemain) {

		_pClient->m_RcvBuffer.DeleteFromStart(PAYLOAD_SIZE + 1);
		pData = _pClient->m_RcvBuffer.GetData();

		if (pData[l_iPayLoadLength] == '#') {
			//there is one message we can process
			char *ptrData = new char[l_iPayLoadLength];
			memset(ptrData, 0, l_iPayLoadLength);
			memcpy(ptrData, pData, (l_iPayLoadLength - 1));
			_pClient->m_RcvBuffer.DeleteFromStart(l_iPayLoadLength + 1);
			pData = _pClient->m_RcvBuffer.GetData();

			iRemain = _pClient->m_RcvBuffer.GetDataLen();

			memset(z_PayLoadSize, 0, sizeof(z_PayLoadSize));
			memcpy(z_PayLoadSize, pData, PAYLOAD_SIZE);
			l_iPayLoadLength = atoi(z_PayLoadSize);
			if (iRemain == 0)
				break;
		} else {
			break;
		}

	}

}
void ServerAPI::OnData(Server * _pServer, Client * _pClient, char * _zData,
		int _iLen) {

	// Note : Convention. Follow this way
	// We are assuming that incoming data had following structure
	// Header(8bytes)|Heart beat timestamp| partion id| label ts1:value1; label ts2:value2

	// This is will be received by telnet client, remove CF before proceed
	if ((int) _zData[_iLen - 2] == CARRIAGE_RETURN) {
		_zData[_iLen - 2] = '\n';
		_zData[_iLen - 1] = '\0';
		_pClient->m_RcvBuffer.Append(_zData, _iLen - 2);
	} else {
		_pClient->m_RcvBuffer.Append(_zData, _iLen);
	}

	Decode(_pClient);

}
void ServerAPI::OnConnect(Server * _pServer, Client * _pClient) {
	cout << "[INFO] Client connected, IP : " << _pClient->GetClientAddress()
			<< endl;
}
void ServerAPI::OnDisconnect(Server * _pServer, Client * _pClient,
		ErrorMsgTag _Err) {
	cout << "[INFO] Client disconnected, IP : " << _pClient->GetClientAddress()
			<< endl;
}
void ServerAPI::OnReadyToSend(Server * _pServer, Client * _pClient) {
}
