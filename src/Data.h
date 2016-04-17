/*
 * Data.h
 *
 *  Created on: Apr 15, 2016
 *      Author: sri
 */

#ifndef DATA_H_
#define DATA_H_

#include <string.h>


#define INIT_NORM_BUF_LEN		2097152   // 2048*1024
#define INC_UNIT_LEN			2048
#define INIT_MASS_BUF_LEN		4096
#define INIT_MASS_BUF_DEL_LEN	3072
#define PKT_SEND_SIZE_MAX		1200


using namespace std;

class Data {
public:
	Data(int);
	virtual ~Data();
	const char* GetData();
	int GetDataLen();
	void ResetData();
	void DeleteFromStart(int);
protected:
	char * m_ptrData;
	int m_iCurrentLen;
	int m_iMaxDataLen;
};

class ClientData: public Data {
public:
	ClientData(int _iInitBufLen = INIT_NORM_BUF_LEN,
			int _iIncLen = INC_UNIT_LEN);
	virtual ~ClientData();
	void Append(const char*, int);
	int m_iStepLen;
};
#endif /* DATA_H_ */
