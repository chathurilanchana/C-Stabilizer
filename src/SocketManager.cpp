/*
 * SocketManager.cpp
 *
 *  Created on: Apr 15, 2016
 *      Author: sri
 */

#include "SocketManager.h"

using namespace std;

SocketManager::SocketManager() {
	i_ReadTurns = 1024;
	i_SockLogFd = -1;
	i_TotMicroSec = 0;
	i_LocalSeconds = -1;
	i_MaxTimers = MAX_TIMERS_LIMIT;
	m_ptrTimers = new Timer*[i_MaxTimers];
	for (int i = 1; i < MAX_TIMERS_LIMIT; ++i) {
		m_ptrTimers[i] = NULL;
		stack_FreeTimerIDs.push(i);
	}

	ui_TotalDeviation = 0;
	ui_CurrentPos = 1;

	i_EpollFd = epoll_create(MAX_EPOL_EVENTS);
	if (i_EpollFd < 0) {
		cout << "[ERROR] Couln't able to create epoll, Exit now!!!" << endl;
		exit(1);
	}
	i_CliAddrSize = sizeof(struct sockaddr_in);
	pstEpollRetEvents = new struct epoll_event[MAX_EPOL_EVENTS];

	m_ptrServerTypes = new ServerType *[MAX_MAX_CLIENT_COUNT];
	for (int i = 0; i < MAX_MAX_CLIENT_COUNT; ++i)
		m_ptrServerTypes[i] = NULL;

}

SocketManager::~SocketManager() {
}

int SocketManager::SetLimits(unsigned int _uiSoftLimit,
		unsigned int _uiHardLimit) {
	rlimit limits;

	limits.rlim_cur = _uiSoftLimit;
	limits.rlim_max = _uiHardLimit;

	setrlimit(RLIMIT_NOFILE, &limits);
	return 0;
}

void SocketManager::Run() {
	p_HBTimer = CreateTimer(MAX_PULSE_TIMEOUT, true, this);

	unsigned int uiPollInterval = 1;
	GET_CURR_TIME(tvnow, ui_LastTime, ui_MicroSec);

	while (true) {
		int iLPos = ui_CurrentPos % MAX_ARRAY_SIZE;

		TIMER_LIST::iterator ite = set_Timers[iLPos].begin();
		while (ite != set_Timers[iLPos].end()) {
			st_TimerBox * pTimeBox = *ite;
			unsigned int uiTimerId = pTimeBox->ui_TimerID;

			TIMER_LIST::iterator iteTemp = ite;
			++ite;
			set_Timers[iLPos].erase(iteTemp);

			if (IsTimerLive(uiTimerId)) {
				Timer * pTimer = pTimeBox->p_Timer;
				unsigned int uiNestFire = pTimer->GetNextFire();

				if (uiNestFire == 0) {
					TimerCallback * pTCB = pTimer->GetTimerCB();
					pTCB->OnTimer(pTimer);

					if (IsTimerLive(uiTimerId)) {
						if (pTimer->IsRepeated()) {
							pTimer->SetNextFire(pTimer->GetInterval());
							AddToCorrectSlot(pTimeBox);
						} else {
							pTimer->ResetTimerId();
							delete pTimeBox;
							stack_FreeTimerIDs.push(uiTimerId);
						}

					} else {
						delete pTimeBox;
						stack_FreeTimerIDs.push(uiTimerId);
					}

				} else {
					AddToCorrectSlot(pTimeBox);
				}
			} else {
				delete pTimeBox;
				stack_FreeTimerIDs.push(uiTimerId);
			}
		}
		++ui_CurrentPos;

		GET_CURR_TIME(tvnow, ui_CurrTime, ui_MicroSec);
		ui_TotalDeviation += (ui_CurrTime - ui_LastTime) / 2;

		ui_LastTime = ui_CurrTime;

		if (ui_TotalDeviation > 0) {
			unsigned int uiFRet = FireAll();
			ui_TotalDeviation += uiFRet / 2;
		}

		uiPollInterval = MAX_EPOL_WAIT;
		unsigned int uiMax = ui_CurrentPos + MAX_EPOL_WAIT;
		for (unsigned int ui = ui_CurrentPos; ui < uiMax; ++ui) {
			int iTemPos = ui % MAX_ARRAY_SIZE;
			if (set_Timers[iTemPos].size() > 0) {
				uiPollInterval = ui - ui_CurrentPos + 1;
				break;
			}
		}

		unsigned int uiRet;

		uiRet = ReadSockets(uiPollInterval);
		if (uiPollInterval < uiRet) {
			ui_TotalDeviation += (uiRet - uiPollInterval) + 1;
			ui_CurrentPos += uiPollInterval - 1;
		} else {
			if (uiRet > 0)
				ui_CurrentPos += uiRet - 1;
		}

	}
}

unsigned int SocketManager::FireAll() {
	unsigned int uiTemp = 0;

	for (unsigned int ui = ui_CurrentPos;
			ui < (ui_CurrentPos + ui_TotalDeviation); ++ui) {
		++uiTemp;
		if (uiTemp == MAX_ARRAY_SIZE) {
			ui_TotalDeviation = uiTemp - 1;
			break;
		}
		int iLPos = ui % MAX_ARRAY_SIZE;
		TIMER_LIST::iterator ite = set_Timers[iLPos].begin();
		while (ite != set_Timers[iLPos].end()) {
			st_TimerBox * pTimeBox = *ite;
			unsigned int uiTimerId = pTimeBox->ui_TimerID;
			if (IsTimerLive(uiTimerId)) {
				lst_FireAll.push_back(pTimeBox);
			} else {
				stack_FreeTimerIDs.push(uiTimerId);
			}

			++ite;
		}
		set_Timers[iLPos].clear();
	}

	ui_CurrentPos += ui_TotalDeviation;
	ui_TotalDeviation = 0;

	TIMER_LIST::iterator iteAll = lst_FireAll.begin();
	while (iteAll != lst_FireAll.end()) {
		st_TimerBox * pTimeBox = *iteAll;
		unsigned int uiTimerId = pTimeBox->ui_TimerID;

		if (IsTimerLive(uiTimerId)) {
			Timer * pTimer = pTimeBox->p_Timer;

			unsigned int uiNestFire = pTimer->GetNextFire();
			if (uiNestFire == 0) {
				pTimer->SetNextFire(pTimer->GetInterval());
				TimerCallback * pTCB = pTimer->GetTimerCB();
				pTCB->OnTimer(pTimer);

				if (IsTimerLive(uiTimerId)) {
					if (pTimer->IsRepeated()) {
						pTimer->SetNextFire(pTimer->GetInterval());
						AddToCorrectSlot(pTimeBox);
					} else {
						pTimer->ResetTimerId();
						delete pTimeBox;
						stack_FreeTimerIDs.push(uiTimerId);
					}

				} else {
					delete pTimeBox;
					stack_FreeTimerIDs.push(uiTimerId);
				}

			} else {
				AddToCorrectSlot(pTimeBox);
			}
		} else {
			delete pTimeBox;
			stack_FreeTimerIDs.push(uiTimerId);
		}
		++iteAll;
	}

	lst_FireAll.clear();
	GET_CURR_TIME(tvnow, ui_CurrTime, ui_MicroSec);
	unsigned int uiDeleay = ui_CurrTime - ui_LastTime;
	ui_LastTime = ui_CurrTime;
	return uiDeleay;
}

Timer * SocketManager::CreateTimer(double _dIntaval, bool _bIsRepeated,
		TimerCallback * _pTCB, int _iTimerType) {
	unsigned int uiTimerID = GetTimerIDFromStack();
	unsigned int uiInt = (unsigned int) (_dIntaval * 1000);

	Timer * pTimer = new Timer(uiInt, _pTCB, _bIsRepeated, uiTimerID, this,
			_iTimerType);
	m_ptrTimers[uiTimerID] = pTimer;
	AddToTimerList(pTimer);
	return pTimer;
}

Client * SocketManager::CreateClient(const char * _zServerIP,
		unsigned int _uiServerPort, ClientCallback * _pCCB, bool _bHB) {
	int hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket < 0) {
		return NULL;
	}

	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(_zServerIP);

	if (sockAddr.sin_addr.s_addr == (in_addr_t) -1) {
		hostent* lphost;
		lphost = gethostbyname(_zServerIP);

		if (lphost != NULL)
			sockAddr.sin_addr.s_addr = ((in_addr*) lphost->h_addr)->s_addr;
			else
			{
				close(hSocket);
				return NULL;
			}
		}
	in_port_t inpl = 0;
	char zBuf[2];
	memcpy(zBuf, &_uiServerPort, 2);
	char zBufR[2];
	zBufR[0] = zBuf[1];
	zBufR[1] = zBuf[0];
	memcpy(&inpl, zBufR, 2);
	sockAddr.sin_port = inpl;

	if (connect(hSocket, (sockaddr*) &sockAddr, sizeof(sockAddr)) < 0) {
		int err = errno;
		if ((err != EWOULDBLOCK) && (err != EINPROGRESS)) {

			close(hSocket);
			return NULL;
		}
	}
	SetNonBlocking(hSocket);
	SetSocketOptions(hSocket);

	int iSocket = hSocket;
	struct epoll_event stEpollEvent;
	memset(&stEpollEvent, 0, sizeof(stEpollEvent));
	stEpollEvent.events = EPOLL_CLI_EVENT_FLAGS;
	stEpollEvent.data.fd = iSocket;
	if (epoll_ctl(i_EpollFd, EPOLL_CTL_ADD, iSocket, &stEpollEvent) < 0) {
		cout << "[ERROR ]epoll_ctl creation failed .  IP " << _zServerIP
				<< " Port " << _uiServerPort << endl;
		return NULL;
	}
	Client * pClient = new Client(_zServerIP, _uiServerPort, iSocket, _pCCB,
			this, NULL);
	m_ptrServerTypes[iSocket] = pClient;
	return pClient;

}

Server * SocketManager::CreateServer(unsigned int _uiPort,
		ServerCallback * _pSCB, bool _bHB) {
	int iSocket = -1;
	struct sockaddr_in stAddr;

	if ((iSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		cout << "[ERROR]Socket creation failed. Port = " << _uiPort << endl;
		return NULL;
	}

	stAddr.sin_family = AF_INET;
	stAddr.sin_port = htons((unsigned int16_t) _uiPort);
	stAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	memset(&(stAddr.sin_zero), '\0', 8);
	SetNonBlocking(iSocket);
	SetSocketOptions(iSocket);

	if (bind(iSocket, (struct sockaddr *) &stAddr, sizeof(struct sockaddr))
			== -1) {
		cout << "[ERROR] Socket bind failed. Port = " << _uiPort << endl;
		return NULL;
	}

	if (listen(iSocket, 2048) == -1) {
		cout << "[ERROR] Socket listen failed. Port = " << _uiPort << endl;
		return NULL;
	}
	struct epoll_event stEpollEvent;
	memset(&stEpollEvent, 0, sizeof(stEpollEvent));
	stEpollEvent.events = EPOLL_SVR_EVENT_FLAGS; // only care about in
	stEpollEvent.data.fd = iSocket;
	if (epoll_ctl(i_EpollFd, EPOLL_CTL_ADD, iSocket, &stEpollEvent) < 0) {
		cout << "[ERROR] epoll_ctl creation failed. Port = " << _uiPort << endl;
		return NULL;
	}
	Server * pServer = new Server(_uiPort, iSocket, _pSCB, this);
	m_ptrServerTypes[iSocket] = pServer;
	cout << "[INFO] ===============================================" << endl;
	cout << "[INFO] Server is started on : " << _uiPort << endl;
	cout << "[INFO] ===============================================" << endl;
	return pServer;
}

void SocketManager::AddToTimerList(Timer * _pTimer) {
	unsigned int uiIntaval = _pTimer->GetInterval();
	int iPos, iCurrPos;
	unsigned int uiNext = 0;
	iCurrPos = ui_CurrentPos % MAX_ARRAY_SIZE;

	if (uiIntaval < MAX_ARRAY_SIZE) {
		uiNext = ui_CurrentPos + uiIntaval;
		iPos = uiNext % MAX_ARRAY_SIZE;
	} else {
		uiNext = ui_CurrentPos + MAX_ARRAY_SIZE - 1;
		iPos = uiNext % MAX_ARRAY_SIZE;
		_pTimer->SetNextFire(uiIntaval - MAX_ARRAY_SIZE + 1);
	}
	st_TimerBox * pTimeBox = new st_TimerBox(_pTimer->GetTimerID(), _pTimer);
	set_Timers[iPos].push_back(pTimeBox);

}

void SocketManager::AddToCorrectSlot(st_TimerBox * _pstTimeBox) {
	Timer * pTimer = _pstTimeBox->p_Timer;
	unsigned int uiNextFire = pTimer->GetNextFire();
	int iPos, iCurrPos;
	unsigned int uiNext = 0;
	iCurrPos = ui_CurrentPos % MAX_ARRAY_SIZE;

	if (uiNextFire < MAX_ARRAY_SIZE) // less than max time wait intawal
	{
		uiNext = ui_CurrentPos + uiNextFire;
		iPos = uiNext % MAX_ARRAY_SIZE;
		pTimer->SetNextFire(0);
	} else {
		uiNext = ui_CurrentPos + MAX_ARRAY_SIZE - 1;
		iPos = uiNext % MAX_ARRAY_SIZE;
		pTimer->SetNextFire(uiNextFire - MAX_ARRAY_SIZE + 1);
	}
	set_Timers[iPos].push_back(_pstTimeBox);

}

void SocketManager::AddToDeletedTimerSet(unsigned int _uiTimerID) {
	m_ptrTimers[_uiTimerID] = NULL;
}
void SocketManager::AddToDeletedClientSet(unsigned int _uiClientID) {
	m_ptrServerTypes[_uiClientID] = NULL;
	if (epoll_ctl(i_EpollFd, EPOLL_CTL_DEL, _uiClientID, NULL) < 0) {
		cout << "[ERROR] Removing a Client from epoll failed.  Client = "
				<< _uiClientID << endl;
	}
	close(_uiClientID);
}
void SocketManager::AddToDeletedServerSet(unsigned int _uiServerID) {
	m_ptrServerTypes[_uiServerID] = NULL; //currently not deleting the server object. Wt to do with connected clients ????
	if (epoll_ctl(i_EpollFd, EPOLL_CTL_DEL, _uiServerID, NULL) < 0) {
		cout << "[ERROR] Removing a server from epoll failed.  Server = "
				<< _uiServerID << endl;
	}
	close(_uiServerID);
}

bool SocketManager::IsTimerLive(unsigned int _uiTimerID) {
	if (m_ptrTimers[_uiTimerID]) {
		return true;
	}
	return false;
}

void SocketManager::SetNonBlocking(int _iSockID) {
	int iOldFlags = fcntl(_iSockID, F_GETFL, 0);
	if (!(iOldFlags & O_NONBLOCK)) {
		iOldFlags |= O_NONBLOCK;
	}
	fcntl(_iSockID, F_SETFL, iOldFlags);
}

void SocketManager::SetSocketOptions(int _iSockID) {
	unsigned int uiMask = 1;

	setsockopt(_iSockID, SOL_SOCKET, SO_REUSEADDR, &uiMask, sizeof(uiMask));

	linger ll;
	ll.l_onoff = 1;
	ll.l_linger = 1;

	setsockopt(_iSockID, SOL_SOCKET, SO_LINGER, &ll, sizeof(linger));
	uiMask = 250000;
	setsockopt(_iSockID, SOL_SOCKET, SO_SNDBUF, &uiMask, sizeof(int));
	uiMask = 250000;
	setsockopt(_iSockID, SOL_SOCKET, SO_RCVBUF, &uiMask, sizeof(int));
	uiMask = 1;
	setsockopt(_iSockID, SOL_SOCKET, SO_KEEPALIVE, &uiMask, sizeof(int));
	uiMask = 1;
	setsockopt(_iSockID, IPPROTO_TCP, TCP_NODELAY, &uiMask, sizeof(int));
}

unsigned int SocketManager::ReadSockets(unsigned int _uiInterval) {

	if (_uiInterval > MAX_EPOL_WAIT)
		_uiInterval = MAX_EPOL_WAIT;
	int iWaitFds = epoll_wait(i_EpollFd, pstEpollRetEvents, MAX_EPOL_EVENTS,
			_uiInterval);

	if (iWaitFds > 0) {
		for (int i = 0; i < iWaitFds; ++i) {
			int iReadySock = pstEpollRetEvents[i].data.fd;

			ServerType * pEItem = m_ptrServerTypes[iReadySock];
			if (pEItem) {
				if (pEItem->getEvtType() == E_EVT_SERVER) {
					Server * pTemServer = (Server *) pEItem;
					while (true) {
						int iCliSock = accept(iReadySock,
								(struct sockaddr *) &stCliAddr,
								(unsigned int *) &i_CliAddrSize);
						if (iCliSock == -1) {
							break;
						}
						struct epoll_event stEpEvent;
						memset(&stEpEvent, 0, sizeof(stEpEvent));
						SetNonBlocking(iCliSock);
						SetSocketOptions(iCliSock);
						stEpEvent.events = EPOLL_CLI_EVENT_FLAGS;
						stEpEvent.data.fd = iCliSock;

						if (epoll_ctl(i_EpollFd, EPOLL_CTL_ADD, iCliSock,
								&stEpEvent) < 0) {
							cout
									<< " [ERROR] Adding a client to epoll failed. Client = "
									<< iCliSock << " Server = " << iReadySock
									<< endl;
							continue;
						}

						// print client details, IP address
						sockaddr_in addr;
						memset(&addr, 0, sizeof(addr));
						unsigned int addr_len = sizeof(addr);
						const char * zCliIp = NULL;
						int iPort;

						Client * pClient = NULL;

						if (getpeername(iCliSock, (sockaddr *) &addr,
								(socklen_t *) &addr_len) < 0) {
							pClient = new Client("UnknownIP", 0,
									(unsigned int) iCliSock, NULL, this,
									pTemServer);
							m_ptrServerTypes[iCliSock] = pClient;
							ServerCallback * pSCB = pTemServer->GetServerCB();
							pSCB->OnConnect(pTemServer, pClient);
							pTemServer->AddtoConnectedClientsList(pClient);
						} else {
							zCliIp = inet_ntoa(addr.sin_addr);
							iPort = addr.sin_port;
							pClient = new Client(zCliIp, (unsigned int) iPort,
									(unsigned int) iCliSock, NULL, this,
									pTemServer);
							m_ptrServerTypes[iCliSock] = pClient;
							ServerCallback * pSCB = pTemServer->GetServerCB();
							pSCB->OnConnect(pTemServer, pClient);
							pTemServer->AddtoConnectedClientsList(pClient);
						}
					}
				} else {
					int iCliId = iReadySock;
					Client * pClient = (Client *) pEItem;
					if (pstEpollRetEvents[i].events & EPOLLRDHUP) {

						Server * pServer = pClient->GetServer();
						if (!pServer) {
							ClientCallback * pCCB = pClient->GetClientCB();
							m_ptrServerTypes[iCliId] = NULL;
							pCCB->OnDisconnect(pClient, ERR_NONE);
						} else {
							ServerCallback * pSCB = pServer->GetServerCB();
							m_ptrServerTypes[iCliId] = NULL;
							pSCB->OnDisconnect(pServer, pClient, ERR_NONE);
							pServer->RemoveFromConnectedClientsList(pClient);
						}

						continue;
					}
					if (pstEpollRetEvents[i].events & EPOLLIN) {
						int iReadByteCount = 0;

						for (int t = 0; t < i_ReadTurns; ++t) {
							pClient = (Client *) m_ptrServerTypes[iCliId];
							if (pClient) {
								if ((iReadByteCount = read(iCliId, z_ReadBuff,
										CLI_READ_SIZE)) > 0) {
									Server * pServer = pClient->GetServer();
									if (!pServer) {
										ClientCallback * pCCB =
												pClient->GetClientCB();
										pCCB->OnData(pClient, z_ReadBuff,
												iReadByteCount);
									} else {
										ServerCallback * pSCB =
												pServer->GetServerCB();
										pSCB->OnData(pServer, pClient,
												z_ReadBuff, iReadByteCount);
									}

								} else if (iReadByteCount == 0) {
									Server * pServer = pClient->GetServer();
									if (!pServer) {
										ClientCallback * pCCB =
												pClient->GetClientCB();
										m_ptrServerTypes[iCliId] = NULL;
										pCCB->OnDisconnect(pClient, ERR_NONE);
									} else {
										ServerCallback * pSCB =
												pServer->GetServerCB();
										m_ptrServerTypes[iCliId] = NULL;
										pSCB->OnDisconnect(pServer, pClient,
												ERR_NONE);
									}
									break;
								} else {
									if (errno != EAGAIN) {
										Server * pServer = pClient->GetServer();
										if (!pServer) {
											ClientCallback * pCCB =
													pClient->GetClientCB();
											m_ptrServerTypes[iCliId] = NULL;
											pCCB->OnDisconnect(pClient,
													ERR_NONE);
										} else {
											ServerCallback * pSCB =
													pServer->GetServerCB();
											m_ptrServerTypes[iCliId] = NULL;
											pSCB->OnDisconnect(pServer, pClient,
													ERR_NONE);
										}
									}
									break;
								}
							} else {
								break;
							}
						}

					}
					if (pstEpollRetEvents[i].events & EPOLLOUT) //can write to this client
					{
						set_OnReadyToSendClients.insert(iCliId);
						pClient->SetBufferStatus(false);
						int iBufLen = pClient->m_SendBuffer.GetDataLen();
						int iSendLen = SendToClient(iCliId);
						if (iBufLen == iSendLen) {
							struct epoll_event stEpEvent;
							memset(&stEpEvent, 0, sizeof(stEpEvent));
							stEpEvent.events = EPOLL_CLI_EVENT_FLAGS;
							stEpEvent.data.fd = iCliId;

							if (epoll_ctl(i_EpollFd, EPOLL_CTL_MOD, iCliId,
									&stEpEvent) < 0) {
								cout
										<< "[ERROR] Modifying epoll failed. Client = "
										<< iCliId << endl;
							}

						}
					}

				}
			}

		}
	}
	if (set_OnReadyToSendClients.size() > 0) {
		ONR2S_CLIENTS::iterator ite = set_OnReadyToSendClients.begin();
		while (ite != set_OnReadyToSendClients.end()) {
			int iClientId = *ite;
			Client * pClient = (Client *) m_ptrServerTypes[iClientId];
			if (pClient) {

				Server * pServer = pClient->GetServer();
				if (!pServer) {
					ClientCallback * pCCB = pClient->GetClientCB();
					pCCB->OnReadyToSend(pClient);
				} else {
					ServerCallback * pSCB = pServer->GetServerCB();
					pSCB->OnReadyToSend(pServer, pClient);
				}
			}
			++ite;
		}
		set_OnReadyToSendClients.clear();
	}
	if (set_DiconClients.size() > 0) {
		WRITE_DESCON_CLIENTS::iterator ite = set_DiconClients.begin();
		while (ite != set_DiconClients.end()) {
			int iClientId = *ite;
			Client * pClient = (Client *) m_ptrServerTypes[iClientId];

			if (pClient) {
				Server * pServer = pClient->GetServer();
				if (!pServer) {
					ClientCallback * pCCB = pClient->GetClientCB();
					m_ptrServerTypes[iClientId] = NULL;
					pCCB->OnDisconnect(pClient, ERR_NONE);
				} else {
					ServerCallback * pSCB = pServer->GetServerCB();
					m_ptrServerTypes[iClientId] = NULL;
					pSCB->OnDisconnect(pServer, pClient, ERR_NONE);
				}
			} else {
				if (epoll_ctl(i_EpollFd, EPOLL_CTL_DEL, iClientId, NULL) < 0) {
					cout
							<< "[ERROR] Removing a client from epoll failed.  Client = "
							<< iClientId << endl;
				}
			}
			++ite;
		}
		set_DiconClients.clear();
	}
	GET_CURR_TIME(tvnow, ui_CurrTime, ui_MicroSec);
	unsigned int uiDeleay = ui_CurrTime - ui_LastTime;
	ui_LastTime = ui_CurrTime;
	return uiDeleay;
}

int SocketManager::SendToClient(unsigned int _uiClid) {
	int iClientId = _uiClid;

	Client * pClient = (Client *) m_ptrServerTypes[iClientId];
	if (pClient) {
		int iCurSend = 0, iSendLen = 0;
		int iBufLen = pClient->m_SendBuffer.GetDataLen();
		const char * zBuf = pClient->m_SendBuffer.GetData();

		int iTotLen = iBufLen;
		while (iSendLen < iTotLen) {
			if ((iCurSend = write(iClientId, zBuf + iSendLen, iBufLen)) < 0) {
				if (errno == EAGAIN && iBufLen > 0) {
					pClient->SetBufferStatus(true);
					pClient->m_SendBuffer.DeleteFromStart(iSendLen);

					struct epoll_event stEpEvent;
					stEpEvent.events = EPOLL_CLI_EVENT_FLAGS | EPOLLOUT;
					stEpEvent.data.fd = iClientId;

					if (epoll_ctl(i_EpollFd, EPOLL_CTL_MOD, iClientId,
							&stEpEvent) < 0) {
						cout << "[ERROR] Modifying epoll failed. Client = "
								<< iClientId << endl;
					}
					return iSendLen;
				} else {
					pClient->m_SendBuffer.DeleteFromStart(iSendLen);
					set_DiconClients.insert(iClientId);
					return 0;
				}
			}

			iBufLen -= iCurSend;
			iSendLen += iCurSend;
		}

		pClient->m_SendBuffer.ResetData();
		return iSendLen;

	}
	return -1;

}
int SocketManager::GetTimerIDFromStack() {
	if (!stack_FreeTimerIDs.empty()) {
		int iID = stack_FreeTimerIDs.top();
		stack_FreeTimerIDs.pop();
		return iID;
	} else {
		Timer ** apAldTimers = m_ptrTimers;
		m_ptrTimers = new Timer*[i_MaxTimers + MAX_TIMERS_INCREASE_INTERVAL];
		for (int i = 0; i < i_MaxTimers; ++i) {
			m_ptrTimers[i] = apAldTimers[i];
		}
		int iPreMax = i_MaxTimers;
		i_MaxTimers += MAX_TIMERS_INCREASE_INTERVAL;
		for (int i = iPreMax; i < i_MaxTimers; ++i) {
			m_ptrTimers[i] = NULL;
			stack_FreeTimerIDs.push(i);
		}
		delete[] apAldTimers;
		int iID = stack_FreeTimerIDs.top();
		stack_FreeTimerIDs.pop();
		return iID;
	}
}
void SocketManager::OnTimer(Timer * _pTimer) {

}
void SocketManager::Detach(Client * _pClient) {
	int iClid = _pClient->GetClientID();
	if (epoll_ctl(i_EpollFd, EPOLL_CTL_DEL, iClid, NULL) < 0) {
		cout << "[ERROR] Removing a Client from epoll failed.  Client = "
				<< iClid << " Error:[" << errno << "] " << strerror(errno)
				<< endl;
	}
	m_ptrServerTypes[iClid] = NULL;
	_pClient->SetSocketManager(NULL);
	_pClient->SetCCB(NULL);
	_pClient->SetServer(NULL);
}
void SocketManager::AttachClient(Client * _pClient, ClientCallback * _pCCB) {
	int iClid = _pClient->GetClientID();
	m_ptrServerTypes[iClid] = _pClient;
	_pClient->SetCCB(_pCCB);
	_pClient->SetSocketManager(this);
	struct epoll_event stEpollEvent;
	memset(&stEpollEvent, 0, sizeof(stEpollEvent));
	if (_pClient->IsBuffered())
		stEpollEvent.events = EPOLL_CLI_EVENT_FLAGS | EPOLLOUT;
	else
		stEpollEvent.events = EPOLL_CLI_EVENT_FLAGS;
	stEpollEvent.data.fd = iClid;
	if (epoll_ctl(i_EpollFd, EPOLL_CTL_ADD, iClid, &stEpollEvent) < 0) {
		cout << "[ERROR] epoll_ctl failed. CLID " << iClid << " Error:["
				<< errno << "] " << strerror(errno) << endl;
	}
}
