#ifndef CONTACT_STATUS_H
#define CONTACT_STATUS_H

#include <QMetaType>

enum ContactStatus
{
	CS_Online=10,
	CS_Away=30,
	CS_Hidden=40,
	CS_Busy=50,
	CS_CallMe=60,
	CS_Silent=70,
	CS_Offline=100
};

Q_DECLARE_METATYPE(ContactStatus)

enum ContactClientType
{
	CCT_Pc=1,
	CCT_Phone=21,
	CCT_Iphone=24,
	CCT_Web=41,
	CCT_Ipad
};

Q_DECLARE_METATYPE(ContactClientType)

#endif //CONTACT_STATUS_H
