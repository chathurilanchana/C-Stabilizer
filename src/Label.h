

#ifndef LABEL_H_
#define LABEL_H_

class Label {
public:
	Label(unsigned long _iTimestamp, int _iValue);
	Label();
	virtual ~Label();
	unsigned long GetTimestamp();
	int GetValue();
private:
	unsigned long m_iLabelTimeStamp;
	int m_iLabelValue;
};
#endif /* LABEL_H_ */
