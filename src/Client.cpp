

#include "SocketManager.h"
Client::Client(const char * _zIP, unsigned int _uiPort,
		unsigned int _uiClientID, ClientCallback * _pCCB,
		SocketManager * _pSocketMan, Server * _pServer) :
		ServerType(E_EVT_CLIENT) {
	ui_ClientID = _uiClientID;
	p_ClientCB = _pCCB;
	p_SocketMan = _pSocketMan;
	ui_ServerPort = _uiPort;
	p_Server = _pServer;
	strncpy(z_ServerIP, _zIP, 100);
	b_IsBuffered = false;
	p_Data = NULL;
}

Client::~Client() {
	p_SocketMan->AddToDeletedClientSet(ui_ClientID);
}
unsigned int Client::GetClientID() {
	return ui_ClientID;
}
ClientCallback * Client::GetClientCB() {
	return p_ClientCB;
}
Server * Client::GetServer() {
	return p_Server;
}
void Client::SetServer(Server * _pServer) {
	p_Server = _pServer;
}
void Client::SetSocketManager(SocketManager * _pSockManager) {
	p_SocketMan = _pSockManager;
}
const char * Client::GetClientAddress() const {
	return z_ServerIP;
}
unsigned int Client::GetClientPort() const {
	return ui_ServerPort;
}
void Client::SetCCB(ClientCallback * _pCCB) {
	p_ClientCB = _pCCB;
}
bool Client::IsBuffered() {
	return b_IsBuffered;
}
void Client::SetBufferStatus(bool _bStatus) {
	b_IsBuffered = _bStatus;
}

bool Client::Send(const char * _zData, int _iLen) {
	m_SendBuffer.Append(_zData, _iLen);
	p_SocketMan->SendToClient(ui_ClientID);
	return !b_IsBuffered;
}
void Client::SendBufferedData() {
	p_SocketMan->SendToClient(ui_ClientID);
}
void Client::Detach() {
	p_SocketMan->Detach(this);
}

void Client::SetSocketSendBufferSize(unsigned int _uiSize) {
	unsigned int uiMask = _uiSize;
	setsockopt(ui_ClientID, SOL_SOCKET, SO_SNDBUF, &uiMask,
			sizeof(unsigned int));
}
void Client::SetSocketRecvBufferSize(unsigned int _uiSize) {
	unsigned int uiMask = _uiSize;
	setsockopt(ui_ClientID, SOL_SOCKET, SO_RCVBUF, &uiMask,
			sizeof(unsigned int));
}

void ClientCallback::OnData(Server * _pServer, Client * _pClient, char * _zData,
		int _iLen) {
}
void ClientCallback::OnData(Client * _pClient, char * _zData, int _iLen) {
}

void ClientCallback::OnDisconnect(Server * _pServer, Client * _pClient,
		ErrorMsgTag _Err) {
}
void ClientCallback::OnDisconnect(Client * _pClient, ErrorMsgTag _Err) {
}

void ClientCallback::OnReadyToSend(Client * _pClient) {
}

