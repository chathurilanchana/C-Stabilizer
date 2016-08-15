

#include "Label.h"

Label::Label(unsigned long _iLabelTimestamp, int _iLabelValue) {
	// TODO Auto-generated constructor stub
	m_iLabelTimeStamp = _iLabelTimestamp;
	m_iLabelValue = _iLabelValue;
}

Label::~Label() {
	// TODO Auto-generated destructor stub
}
unsigned long Label::GetTimestamp() {
	return m_iLabelTimeStamp;
}

int Label::GetValue() {
	return m_iLabelValue;
}
