/*
 * SocketManager.h
 *
 *  Created on: Apr 15, 2016
 *      Author: sri
 */

#ifndef SOCKETMANAGER_H_
#define SOCKETMANAGER_H_

#include "ServerDef.h"
#include "Data.h"
typedef std::set<unsigned int> ONR2S_CLIENTS;
typedef std::set<unsigned int> DELETED_TIMERS;
typedef std::set<unsigned int> DELETED_CLIENTS;
typedef std::set<unsigned int> DELETED_SERVERS;
typedef std::set<unsigned int> WRITE_DESCON_CLIENTS;
typedef std::stack<unsigned int> FREE_TIMER_IDS;
typedef std::set<unsigned int> ACTIVE_CLIENTS;

class SocketManager;
class Client;
class Server;
class Timer;

typedef struct _st_TimerBox {
	unsigned int ui_TimerID;
	Timer * p_Timer;

	_st_TimerBox(unsigned int _uiID, Timer * _pTimer) {
		ui_TimerID = _uiID;
		p_Timer = _pTimer;
	}
} st_TimerBox;
typedef std::list<st_TimerBox *> TIMER_LIST;
typedef std::list<Client*> LISTOFCLIENTS;
typedef enum ErrMsgTag {
	ERR_NONE = 1, // No errors
	ERR_CANT_INIT_SOCKETS,
	ERR_SOCKET, // Error report by sockets
	ERR_TIMEOUT, // Connection timeout
	ERR_CLOSED, // connection closed
	ERR_BUFFERED,
	ERR_INVALID_PARAMETERS,

} ErrorMsgTag;

class TimerCallback {
public:
	TimerCallback() {
	}
	;
	virtual ~TimerCallback();
	virtual void OnTimer(Timer * _pTimer) = 0;
};

class ServerCallback {
public:
	ServerCallback() {
	}
	;
	virtual ~ServerCallback() {
	}
	;
	virtual void OnData(Server * _pServer, Client * _pClient, char * _zData,
			int _iLen) = 0;
	virtual void OnConnect(Server * _pServer, Client * _pClient) = 0;
	virtual void OnDisconnect(Server * _pServer, Client * _pClient,
			ErrorMsgTag _Err) = 0;
	virtual void OnReadyToSend(Server * _pServer, Client * _pClient) = 0;
};

class ClientCallback {
public:
	ClientCallback() {
	}
	;
	virtual ~ClientCallback() {
	}
	;
	virtual void OnData(Server * _pServer, Client * _pClient, char * _zData,
			int _iLen);
	virtual void OnData(Client * _pClient, char * _zData, int _iLen);
	virtual void OnDisconnect(Server * _pServer, Client * _pClient,
			ErrorMsgTag _Err);
	virtual void OnDisconnect(Client * _pClient, ErrorMsgTag _Err);
	virtual void OnReadyToSend(Client * _pClient);
};

class Timer: public ServerType {
public:
	Timer(unsigned int _uiInterval, TimerCallback * _pTCB, bool _bIsRepeated,
			unsigned int _uiTimerID, SocketManager * _pSocketMan,
			int _iTimerType = -1);
	virtual ~Timer();
	unsigned int GetInterval();
	unsigned int GetNextFire();
	unsigned int GetTimerID();

	void SetNextFire(unsigned int _uiFireTime);
	TimerCallback * GetTimerCB();

	void ResetTimerId();
	bool IsRepeated();
	void SetIsRemovedTimer(bool _State);
	void * p_Data;
	int i_TimerType;
private:
	bool b_IsRepeated;
	bool b_IsRemovedTimer;
	unsigned int ui_Interval;
	TimerCallback * p_TimerCB;
	SocketManager * p_SocketMan;
	unsigned int ui_NextFire;
	unsigned int ui_TimerID;

};

class Client: public ServerType {
public:
	Client(const char * _zIP, unsigned int _uiPort, unsigned int _uiClientID,
			ClientCallback * _pCCB, SocketManager * _pSocketMan,
			Server * _pServer);
	virtual ~Client();

	bool Send(const char * _zData, int _iLen);
	void SendBufferedData();
	void Detach();

	unsigned int GetClientID();
	ClientCallback * GetClientCB();
	Server * GetServer();
	void SetServer(Server * _pServer);
	void SetSocketManager(SocketManager * _pSockManager);
	const char * GetClientAddress() const;
	unsigned int GetClientPort() const;
	void SetCCB(ClientCallback * _pCCB);
	bool IsBuffered();
	void SetBufferStatus(bool _bStatus);

	void SetSocketSendBufferSize(unsigned int _uiSize);
	void SetSocketRecvBufferSize(unsigned int _uiSize);

	ClientData m_SendBuffer;
	ClientData m_RcvBuffer;
	void * p_Data;

protected:

	unsigned int ui_ClientID;
	bool b_IsBuffered;
	unsigned int ui_ServerPort;
	char z_ServerIP[100];
	ClientCallback * p_ClientCB;
	SocketManager * p_SocketMan;
	Server * p_Server;

};

class Server: public ServerType {
public:
	Server(unsigned int _uiPort, unsigned int _uiServerID,
			ServerCallback * _pSCB, SocketManager * _pSocketMan);
	virtual ~Server();
	unsigned int GetServerID();
	unsigned int GetServerPort();
	ServerCallback * GetServerCB();
	void AddToClientSet(unsigned int _uiClientID);
	void RemoveFromClientSet(unsigned int _uiClientID);
	void CloseServerSocket();
	void AddtoConnectedClientsList(Client* _pClient);
	void RemoveFromConnectedClientsList(Client* _pClient);
	void SendToAllClients(const char * _zBuf, unsigned int _uiLen);

	void * p_Data;

private:
	unsigned int ui_ServerPort;
	unsigned int ui_ServerID; //socket descriptor
	ServerCallback * p_ServerCB;
	SocketManager * p_SocketMan;
	ACTIVE_CLIENTS set_ActiveClients;
	LISTOFCLIENTS lst_ConnectedClients; //Set by application layer
};

class SocketManager: public TimerCallback {
public:
	SocketManager();
	virtual ~SocketManager();
	void Run();
	Timer * CreateTimer(double _dIntaval, bool _bIsRepeated,
			TimerCallback * _pTCB, int _iTimerType = -1);
	Server * CreateServer(unsigned int _uiPort, ServerCallback * _pSCB,
			bool _bHB = false);
	Client * CreateClient(const char * _zServerIP, unsigned int _uiServerPort,
			ClientCallback * _pCCB, bool _bHB);

	void AddToDeletedTimerSet(unsigned int _uiTimerID);
	void AddToDeletedClientSet(unsigned int _uiClientID);
	void AddToDeletedServerSet(unsigned int _uiServerID);
	int SendToClient(unsigned int _uiClid);

	void Detach(Client * _pClient);
	void AttachClient(Client * _pClient, ClientCallback * _pCCB);

	static int SetLimits(unsigned int _uiSoftLimit, unsigned int _uiHardLimit);

private:
	void OnTimer(Timer * _pTimer);
	void AddToTimerList(Timer * _pTimer);
	void AddToCorrectSlot(st_TimerBox * _pstTimeBox);

	void SetSocketOptions(int _iSockID);
	void SetNonBlocking(int _iSockID);

	unsigned int ReadSockets(unsigned int _uiInterval);
	int GetTimerIDFromStack();

	bool IsTimerLive(unsigned int _uiTimerID);
	unsigned int FireAll();

	unsigned int ui_CurrentPos;
	unsigned int ui_LastTime;
	unsigned int ui_CurrTime;

	unsigned int ui_TotalDeviation;
	unsigned int ui_MicroSec;
	int i_TotMicroSec;
	int i_ReadTurns;

	struct timeval tvnow;
	int i_MaxTimers;
	Timer ** m_ptrTimers;
	FREE_TIMER_IDS stack_FreeTimerIDs;
	TIMER_LIST set_Timers[MAX_ARRAY_SIZE];
	TIMER_LIST lst_FireAll;
	Timer * p_HBTimer;

	int i_EpollFd;
	int i_CliAddrSize;
	struct sockaddr_in stCliAddr;

	struct epoll_event * pstEpollRetEvents;
	ServerType ** m_ptrServerTypes;
	char z_ReadBuff[CLI_READ_SIZE];
	WRITE_DESCON_CLIENTS set_DiconClients;
	ONR2S_CLIENTS set_OnReadyToSendClients;

	int i_SockLogFd;
	int i_LocalSeconds;
	char z_LocalTimeStamp[25];
};
#endif /* SOCKETMANAGER_H_ */
