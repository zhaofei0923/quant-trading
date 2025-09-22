#ifndef _TYPEDEF_H
#define _TYPEDEF_H

#include <memory>
#include <map>
#include "struct.h"
#include "message.h"
using namespace std;
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

typedef map<string, MarketData>  MarketDataInfo;  //ExchangeID+InstrumentID,��������
typedef map<string, string>  InstrumentExchange; //InstrumentID,ExchangeID
typedef map<string, shared_ptr<InstrumentT>>  InstrumentInfo; //ExchangeID+InstrumentID,��Լ
typedef map<string, shared_ptr<InstrumentMarginRate>> InstrumentMarginRateInfo; //ExchangeID+InstrumentID,��Լ��֤����
typedef map<string, shared_ptr<InstrumentCommissionRate>> InstrumentCommissionRateInfo; //ExchangeID+InstrumentID,��Լ��������
typedef map<string, shared_ptr<InstrumentOrderCommRate>>  InstrumentOrderCommRateInfo; //ExchangeID+InstrumentID,����������

#endif 
