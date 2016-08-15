

#include "Data.h"
using namespace std;

Data::Data(int _iCurMax) {
	m_iCurrentLen = 0;
	m_iMaxDataLen = _iCurMax;
	m_ptrData = new char[m_iMaxDataLen + 1];
	m_ptrData[0] = 0;
}
Data::~Data() {
	delete[] m_ptrData;
	m_ptrData = NULL;

}
const char* Data::GetData() {
	return m_ptrData;
}
int Data::GetDataLen() {
	return m_iCurrentLen;
}
void Data::ResetData() {
	m_ptrData[0] = 0;
	m_iCurrentLen = 0;
}

ClientData::ClientData(int _iInitBufLen, int _iIncLen) :
		Data(_iInitBufLen) {
	m_iStepLen = _iIncLen;
}

ClientData::~ClientData() {

}

void ClientData::Append(const char*_zBuffer, int _iBufLen) {
	int iTotalLen = m_iCurrentLen + _iBufLen;
	if (iTotalLen > m_iMaxDataLen) {
		m_iMaxDataLen = iTotalLen + m_iStepLen;
		char * zTBuf = new char[m_iMaxDataLen + 1];

		memcpy(zTBuf, m_ptrData, m_iCurrentLen);
		memcpy(zTBuf + m_iCurrentLen, _zBuffer, _iBufLen);
		m_iCurrentLen = iTotalLen;
		char * zTDel = m_ptrData;
		m_ptrData = zTBuf;
		delete[] zTDel;
	} else {
		memcpy(m_ptrData + m_iCurrentLen, _zBuffer, _iBufLen);
		m_iCurrentLen += _iBufLen;
	}
	m_ptrData[m_iCurrentLen] = 0;
}

void Data::DeleteFromStart(int _iDelLen) {
	memmove(m_ptrData, m_ptrData + _iDelLen, m_iCurrentLen - _iDelLen);
	m_iCurrentLen = m_iCurrentLen - _iDelLen;
	m_ptrData[m_iCurrentLen] = 0;
}

