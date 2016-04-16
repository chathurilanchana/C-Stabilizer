/*
 * Server.cpp
 *
 *  Created on: Apr 15, 2016
 *      Author: sri
 */

#include "SocketManager.h"

Server::Server(unsigned int _uiPort, unsigned int _uiServerID, ServerCallback * _pSCB,
		SocketManager * _pSocketMan) :
		ServerType(E_EVT_SERVER) {
	ui_ServerPort = _uiPort;
	ui_ServerID = _uiServerID;
	p_ServerCB = _pSCB;
	p_SocketMan = _pSocketMan;
}

Server::~Server() {
	p_SocketMan->AddToDeletedServerSet(ui_ServerID);
}
void Server::CloseServerSocket() {
	close(ui_ServerID);
}
unsigned int Server::GetServerID() {
	return ui_ServerID;
}
unsigned int Server::GetServerPort() {
	return ui_ServerPort;
}
ServerCallback * Server::GetServerCB() {
	return p_ServerCB;
}
void Server::AddToClientSet(unsigned int _uiClientID) {
	set_ActiveClients.insert(_uiClientID);
}
void Server::RemoveFromClientSet(unsigned int _uiClientID) {
	set_ActiveClients.erase(_uiClientID);
}
void ServerCallback::OnData(Server * _pServer, Client * _pClient, char * _zData,
		int _iLen) {
}
void ServerCallback::OnConnect(Server * _pServer, Client * _pClient) {
}
void ServerCallback::OnDisconnect(Server * _pServer, Client * _pClient,
		ErrorMsgTag _Err) {
}
void ServerCallback::OnReadyToSend(Server * _pServer, Client * _pClient) {
}

void Server::AddtoConnectedClientsList(Client* _pClient) {
	lst_ConnectedClients.push_back(_pClient);
}

void Server::RemoveFromConnectedClientsList(Client* _pClient) {
	LISTOFCLIENTS::iterator ite = lst_ConnectedClients.begin();
	while (ite != lst_ConnectedClients.end()) {
		if ((*ite) == _pClient) {
			lst_ConnectedClients.erase(ite);
			break;
		}
		++ite;
	}
}

void Server::SendToAllClients(const char * _zBuf, unsigned int _uiLen) {
	LISTOFCLIENTS::iterator ite = lst_ConnectedClients.begin();
	while (ite != lst_ConnectedClients.end()) {
		(*ite)->Send(_zBuf, _uiLen);
		++ite;
	}
}



