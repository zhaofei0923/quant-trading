#ifndef _TYPEDEF_H
#define _TYPEDEF_H

#include <memory>
#include <map>
#include "struct.h"
#include "message.h"
using namespace std;
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

typedef map<string, MarketData>  MarketDataInfo;  //ExchangeID+InstrumentID,行情数据
typedef map<string, string>  InstrumentExchange; //InstrumentID,ExchangeID
typedef map<string, shared_ptr<InstrumentT>>  InstrumentInfo; //ExchangeID+InstrumentID,合约
typedef map<string, shared_ptr<InstrumentMarginRate>> InstrumentMarginRateInfo; //ExchangeID+InstrumentID,合约保证金率
typedef map<string, shared_ptr<InstrumentCommissionRate>> InstrumentCommissionRateInfo; //ExchangeID+InstrumentID,合约手续费率
typedef map<string, shared_ptr<InstrumentOrderCommRate>>  InstrumentOrderCommRateInfo; //ExchangeID+InstrumentID,报单手续费

#endif 
