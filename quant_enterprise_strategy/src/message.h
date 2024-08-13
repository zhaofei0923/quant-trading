#ifndef _MESSAGE_H
#define _MESSAGE_H
#include <string.h>
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

/***************************客户端和服务端通讯的通用报文头*********************/
struct Head
{
	int iFunctionId;  //功能号
	int iMsgtype;  //消息类型
	int iReqId; //请求id，唯一，不要重复
	int iRoletype;   //角色类型
	int iBodyLength;    //报文体长度
	bool bIsLast;  //是否最后一条数据
	int iErrorCode;     //错误码,响应消息使用
	char szErrorInfo[100];   //错误信息,响应消息使用

	Head()
	{
		memset(this, 0, sizeof(Head));
	}

	void set(Head* s1, Head* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(Head));
	}
	Head& operator=(const Head& s)//重载运算符
	{
		set(this, (Head*)&s);
		return *this;
	}
	Head(const Head& s)//复制构造函数
	{
		*this = s;
	}
};

/********************************客户端和服务端通讯的报文体**************************************/
//登录
struct LoginReqT
{
	char szUserId[20];
	char szPassword[20];

	LoginReqT()  //默认构造函数
	{
		memset(this, 0, sizeof(LoginReqT));
	}
	void set(LoginReqT* s1, LoginReqT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(LoginReqT));
	}
	LoginReqT& operator=(const LoginReqT& s)//重载运算符
	{
		set(this, (LoginReqT*)&s);
		return *this;
	}
	LoginReqT(const LoginReqT& s)//复制构造函数
	{
		*this = s;
	}
};
struct LoginResT
{};//无报文体

   //登出
struct LogoutReqT
{};//无报文体
struct LogoutResT
{};//无报文体

   //查询资金
struct QueryBalanceReqT
{};
struct QueryBalanceResT
{
	unsigned long long 	iUseMargin;///占用的保证金
	unsigned long long	iFrozenMargin; ///冻结的保证金
	unsigned long long	iFrozenCash; ///冻结的资金
	unsigned long long	iFrozenCommission; ///冻结的手续费
	unsigned long long	iCurrMargin; ///当前保证金总额
	unsigned long long	iCommission; ///手续费
	unsigned long long	iAvailable; ///可用资金

	unsigned long long  iStartMoney; ///期初的资金
	unsigned long long  iAddMoney; ///调仓增加的资金

	QueryBalanceResT()  //默认构造函数
	{
		memset(this, 0, sizeof(QueryBalanceResT));
	}

	void set(QueryBalanceResT* s1, QueryBalanceResT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(QueryBalanceResT));
	}
	QueryBalanceResT& operator=(const QueryBalanceResT& s)//重载运算符
	{
		set(this, (QueryBalanceResT*)&s);
		return *this;
	}
	QueryBalanceResT(const QueryBalanceResT& s)//复制构造函数
	{
		*this = s;
	}
};

//查询委托单
struct QueryOrderReqT
{
	int iFlag;//0-查当天全部，1-根据szClientOrderId查询
	char szClientOrderId[30]; //客户端委托id，一天内不要重复，用于区分委托单

	QueryOrderReqT()  //默认构造函数
	{
		memset(this, 0, sizeof(QueryOrderReqT));
	}
	void set(QueryOrderReqT* s1, QueryOrderReqT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(QueryOrderReqT));
	}
	QueryOrderReqT& operator=(const QueryOrderReqT& s)//重载运算符
	{
		set(this, (QueryOrderReqT*)&s);
		return *this;
	}
	QueryOrderReqT(const QueryOrderReqT& s)//复制构造函数
	{
		*this = s;
	}
};
struct OrderDetail
{
	int iInitDate; //本地新建订单日期
	int iInitTime; //本地新建订单时间 HHMMSSsss
	char szJysInsertDate[40]; //交易所报单日期
	char szJysInsertTime[40]; //交易所委托时间
	char szUserId[10]; //交易账号
	char szExchangeID[10]; //市场
	char szStockCode[40];  //代码
	char szOrderRef[40];  //报单引用
	char szOrderSysID[40]; // 报单编号,相当于委托编号
	char cAction;  //买卖操作
	unsigned long long    iPrice;  //价格,三位小数
	int iEntrustNum; //委托数量
	int iTradeNum; //成交数量
	char   cStatus;//报单状态

	OrderDetail()  //默认构造函数
	{
		memset(this, 0, sizeof(OrderDetail));
	}
	void set(OrderDetail* s1, OrderDetail* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(OrderDetail));
	}
	OrderDetail& operator=(const OrderDetail& s)//重载运算符
	{
		set(this, (OrderDetail*)&s);
		return *this;
	}
	OrderDetail(const OrderDetail& s)//复制构造函数
	{
		*this = s;
	}
};
struct QueryOrderResT
{
	OrderDetail OrderData;

	QueryOrderResT()  //默认构造函数
	{
		memset(this, 0, sizeof(QueryOrderResT));
	}
	void set(QueryOrderResT* s1, QueryOrderResT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(QueryOrderResT));
	}
	QueryOrderResT& operator=(const QueryOrderResT& s)//重载运算符
	{
		set(this, (QueryOrderResT*)&s);
		return *this;
	}
	QueryOrderResT(const QueryOrderResT& s)//复制构造函数
	{
		*this = s;
	}
};

//查询交易
struct QueryTradeReqT
{
	int iFlag;//0-查当天全部，1-根据szClientOrderId查询
	char szClientOrderId[30]; //客户端委托id，一天内不要重复，用于区分委托单

	QueryTradeReqT()  //默认构造函数
	{
		memset(this, 0, sizeof(QueryTradeReqT));
	}
	void set(QueryTradeReqT* s1, QueryTradeReqT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(QueryTradeReqT));
	}
	QueryTradeReqT& operator=(const QueryTradeReqT& s)//重载运算符
	{
		set(this, (QueryTradeReqT*)&s);
		return *this;
	}
	QueryTradeReqT(const QueryTradeReqT& s)//复制构造函数
	{
		*this = s;
	}
};
struct TradeDetail
{
	int iInitDate; //本地日期 yyyymmdd
	int iInitTime; //本地时间 HHMMSSsss
	char szJysTradeDate[40]; //交易所成交日期
	char szJysTradeTime[40]; //交易所成交时间
	char szUserId[10]; //交易账号
	char szExchangeID[10]; //市场
	char szStockCode[40];  //代码
	char szOrderRef[40];  //报单引用
	char szOrderSysID[40]; // 报单编号,相当于委托编号
	char szTradeID[40]; //成交编号
	char cAction;  //买卖操作
	unsigned long long    iPrice;  //价格,三位小数
	int iTradeNum; //成交数量

	TradeDetail()  //默认构造函数
	{
		memset(this, 0, sizeof(TradeDetail));
	}
	void set(TradeDetail* s1, TradeDetail* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(TradeDetail));
	}
	TradeDetail& operator=(const TradeDetail& s)//重载运算符
	{
		set(this, (TradeDetail*)&s);
		return *this;
	}
	TradeDetail(const TradeDetail& s)//复制构造函数
	{
		*this = s;
	}
};
struct QueryTradeResT
{
	TradeDetail TradeData;

	QueryTradeResT()  //默认构造函数
	{
		memset(this, 0, sizeof(QueryTradeResT));
	}
	void set(QueryTradeResT* s1, QueryTradeResT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(QueryTradeResT));
	}
	QueryTradeResT& operator=(const QueryTradeResT& s)//重载运算符
	{
		set(this, (QueryTradeResT*)&s);
		return *this;
	}
	QueryTradeResT(const QueryTradeResT& s)//复制构造函数
	{
		*this = s;
	}
};

//查询持仓
struct QueryPositionReqT
{};//无报文体
struct PositionDetail
{
	int iInitDate; //本地日期 yyyymmdd
	char szUserId[10]; //交易账户
	char szExchangeID[10]; //市场
	char szStockCode[40];  //代码
	char cDirection; //持仓方向
	char  cHedgeFlag;///投机套保标志
	int	iYdPosition;///昨日持仓
	int	iTodayPosition; ///今日持仓
	int	iPosition; ///总持仓
	unsigned long long 	iLongFrozen; ///多头冻结
	unsigned long long 	iShortFrozen; ///空头冻结
	unsigned long long 	iUseMargin; ///占用的保证金,三位小数
	unsigned long long 	iFrozenMargin; ///冻结的保证金,三位小数
	unsigned long long 	iFrozenCash; ///冻结的资金,三位小数
	unsigned long long 	iFrozenCommission;///冻结的手续费,三位小数
	unsigned long long 	iCommission; ///手续费,三位小数
	unsigned long long 	iPreSettlementPrice; ///昨仓结算价,三位小数
	unsigned long long 	iSettlementPrice; ///本次结算价,三位小数

	PositionDetail()  //默认构造函数
	{
		memset(this, 0, sizeof(PositionDetail));
	}
	void set(PositionDetail* s1, PositionDetail* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(PositionDetail));
	}
	PositionDetail& operator=(const PositionDetail& s)//重载运算符
	{
		set(this, (PositionDetail*)&s);
		return *this;
	}
	PositionDetail(const PositionDetail& s)//复制构造函数
	{
		*this = s;
	}
};
struct QueryPositionResT
{
	PositionDetail PositionData;

	QueryPositionResT()  //默认构造函数
	{
		memset(this, 0, sizeof(QueryPositionResT));
	}
	void set(QueryPositionResT* s1, QueryPositionResT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(QueryPositionResT));
	}
	QueryPositionResT& operator=(const QueryPositionResT& s)//重载运算符
	{
		set(this, (QueryPositionResT*)&s);
		return *this;
	}
	QueryPositionResT(const QueryPositionResT& s)//复制构造函数
	{
		*this = s;
	}
};

//委托
struct PlaceOrderReqT
{
	char szExchangeID[10]; //市场
	char szStockCode[40];
	char cAction;
	unsigned long long  iPrice; //小数点三位
	int iEntrustNum;
	char szClientOrderId[30]; //客户端委托id，一天内不要重复，用于区分委托单

	PlaceOrderReqT()  //默认构造函数
	{
		memset(this, 0, sizeof(PlaceOrderReqT));
	}
	void set(PlaceOrderReqT* s1, PlaceOrderReqT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(PlaceOrderReqT));
	}
	PlaceOrderReqT& operator=(const PlaceOrderReqT& s)//重载运算符
	{
		set(this, (PlaceOrderReqT*)&s);
		return *this;
	}
	PlaceOrderReqT(const PlaceOrderReqT& s)//复制构造函数
	{
		*this = s;
	}
};
struct PlaceOrderResT
{
	char szOrderRef[40];  //报单引用

	PlaceOrderResT()  //默认构造函数
	{
		memset(this, 0, sizeof(PlaceOrderResT));
	}
	void set(PlaceOrderResT* s1, PlaceOrderResT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(PlaceOrderResT));
	}
	PlaceOrderResT& operator=(const PlaceOrderResT& s)//重载运算符
	{
		set(this, (PlaceOrderResT*)&s);
		return *this;
	}
	PlaceOrderResT(const PlaceOrderResT& s)//复制构造函数
	{
		*this = s;
	}
};

//撤单
struct CancelOrderReqT
{
	char szClientOrderId[30]; //客户端委托id，一天内不要重复，用于区分委托单

	CancelOrderReqT()  //默认构造函数
	{
		memset(this, 0, sizeof(CancelOrderReqT));
	}
	void set(CancelOrderReqT* s1, CancelOrderReqT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(CancelOrderReqT));
	}
	CancelOrderReqT& operator=(const CancelOrderReqT& s)//重载运算符
	{
		set(this, (CancelOrderReqT*)&s);
		return *this;
	}
	CancelOrderReqT(const CancelOrderReqT& s)//复制构造函数
	{
		*this = s;
	}
};
struct CancelOrderResT
{};//无报文体

 //委托回报
struct OrderNotifyT
{
	OrderDetail OrderData;

	OrderNotifyT()  //默认构造函数
	{
		memset(this, 0, sizeof(OrderNotifyT));
	}
	void set(OrderNotifyT* s1, OrderNotifyT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(OrderNotifyT));
	}
	OrderNotifyT& operator=(const OrderNotifyT& s)//重载运算符
	{
		set(this, (OrderNotifyT*)&s);
		return *this;
	}
	OrderNotifyT(const OrderNotifyT& s)//复制构造函数
	{
		*this = s;
	}
};
//成交回报
struct TradeNotifyT
{
	TradeDetail TradeData;

	TradeNotifyT()  //默认构造函数
	{
		memset(this, 0, sizeof(TradeNotifyT));
	}
	void set(TradeNotifyT* s1, TradeNotifyT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(TradeNotifyT));
	}
	TradeNotifyT& operator=(const TradeNotifyT& s)//重载运算符
	{
		set(this, (TradeNotifyT*)&s);
		return *this;
	}
	TradeNotifyT(const TradeNotifyT& s)//复制构造函数
	{
		*this = s;
	}
};
//委托错误通知
struct PlaceOrderErrNotifyT
{
	char szUserId[10]; //交易账号
	char szClientOrderId[30]; //客户端委托id，一天内不要重复，用于区分委托单
	char szOrderRef[40];  //报单引用
	int iErrorCode;     //错误码,响应消息使用
	char szErrorInfo[100];   //错误信息,响应消息使用

	PlaceOrderErrNotifyT()
	{
		memset(this, 0, sizeof(PlaceOrderErrNotifyT));
	}
	void set(PlaceOrderErrNotifyT* s1, PlaceOrderErrNotifyT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(PlaceOrderErrNotifyT));
	}
	PlaceOrderErrNotifyT& operator=(const PlaceOrderErrNotifyT& s)//重载运算符
	{
		set(this, (PlaceOrderErrNotifyT*)&s);
		return *this;
	}
	PlaceOrderErrNotifyT(const PlaceOrderErrNotifyT& s)//复制构造函数
	{
		*this = s;
	}
};
//撤单错误通知
struct CancelOrderErrNotifyT
{
	char szUserId[10]; //交易账号
	char szClientOrderId[30]; //客户端委托id，一天内不要重复，用于区分委托单
	char szOrderRef[40];  //报单引用
	int iErrorCode;     //错误码,响应消息使用
	char szErrorInfo[100];   //错误信息,响应消息使用

	CancelOrderErrNotifyT()
	{
		memset(this, 0, sizeof(CancelOrderErrNotifyT));
	}
	void set(CancelOrderErrNotifyT* s1, CancelOrderErrNotifyT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(CancelOrderErrNotifyT));
	}
	CancelOrderErrNotifyT& operator=(const CancelOrderErrNotifyT& s)//重载运算符
	{
		set(this, (CancelOrderErrNotifyT*)&s);
		return *this;
	}
	CancelOrderErrNotifyT(const CancelOrderErrNotifyT& s)//复制构造函数
	{
		*this = s;
	}
};

//增加通讯的消息队列
struct AddMsgQueueReqT
{};//无报文体
struct AddMsgQueueResT
{
	int iReadKey; //服务端读的key，即客户端写的key
	int iWriteKey; //服务端写的key，即客户端读的key

	AddMsgQueueResT()
	{
		memset(this, 0, sizeof(AddMsgQueueResT));
	}
	void set(AddMsgQueueResT* s1, AddMsgQueueResT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(AddMsgQueueResT));
	}
	AddMsgQueueResT& operator=(const AddMsgQueueResT& s)//重载运算符
	{
		set(this, (AddMsgQueueResT*)&s);
		return *this;
	}
	AddMsgQueueResT(const AddMsgQueueResT& s)//复制构造函数
	{
		*this = s;
	}
};

//删除通讯的消息队列
struct DelMsgQueueReqT
{};//无报文体
struct DelMsgQueueResT
{};//无报文体

 //订阅行情
struct SubQuoteReqT
{
	int iNum;
	//iNum个SubItem

};
struct SubItem
{
	char szStockCode[40];

	SubItem()
	{
		memset(this, 0, sizeof(SubItem));
	}
	void set(SubItem* s1, SubItem* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(SubItem));
	}
	SubItem& operator=(const SubItem& s)//重载运算符
	{
		set(this, (SubItem*)&s);
		return *this;
	}
	SubItem(const SubItem& s)//复制构造函数
	{
		*this = s;
	}
};
struct SubQuoteResT
{
	int iClientNum; //客户端编号
	int iDataKey; //数据区的key

	SubQuoteResT()
	{
		memset(this, 0, sizeof(SubQuoteResT));
	}
	void set(SubQuoteResT* s1, SubQuoteResT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(SubQuoteResT));
	}
	SubQuoteResT& operator=(const SubQuoteResT& s)//重载运算符
	{
		set(this, (SubQuoteResT*)&s);
		return *this;
	}
	SubQuoteResT(const SubQuoteResT& s)//复制构造函数
	{
		*this = s;
	}
};

//取消订阅行情
struct CancelSubQuoteReqT
{
	int iClientNum; //客户端编号
};
struct CancelSubQuoteResT
{};//无报文体

//行情快照
//快照数据，价格为3位小数点
struct MarketData
{
	
	char szTradingDay[10]; //交易日
	char szActionDay[10]; //业务日期
	char szExchangeId[10]; //交易所代码
	char szStockCode[50]; //股票代码，合约代码
	char szStockName[100]; //股票名称，合约名称
	char szUpdateTime[20];

	uint64_t iLastPrice; //最新价
	uint64_t iPreSettlePrice; //上次结算价
	uint64_t iSettlePrice; //本次结算价
	uint64_t iPreClosePrice; //昨收盘价
	uint64_t iOpenPrice;//今开盘价
	uint64_t iLowPrice; //最低价
	uint64_t iHighPrice; //最高价
	uint64_t iClosePrice; //今收盘价
	uint64_t iUpperLimitPrice; ///涨停板价
	uint64_t iLowerLimitPrice; ///跌停板价
	uint64_t iVolume;//数量
	uint64_t iPreOpenInterest; //昨持仓量
	uint64_t iTurnOver; //成交金额
	uint64_t iOpenInterest; //持仓量
	uint64_t iPreDelta; //昨虚实度
	uint64_t iCurrDelta; //今虚实度
	uint64_t iUpdateMillisec; //最后修改毫秒
	uint64_t iAveragePrice; //当日均价

	uint64_t  iSellPrice1; ///卖1档价格
	uint64_t  iSellPrice2; ///卖2档价格
	uint64_t  iSellPrice3;
	uint64_t  iSellPrice4;
	uint64_t  iSellPrice5;

	uint64_t  iBuyPrice1; ///买1档价格
	uint64_t  iBuyPrice2;///买2档价格
	uint64_t  iBuyPrice3;
	uint64_t  iBuyPrice4;
	uint64_t  iBuyPrice5;

	uint64_t iSellQuantity1;///卖1档数量
	uint64_t iSellQuantity2;///卖2档数量
	uint64_t iSellQuantity3;
	uint64_t iSellQuantity4;
	uint64_t iSellQuantity5;

	uint64_t iBuyQuantity1;///买1档数量
	uint64_t iBuyQuantity2;///买2档数量
	uint64_t iBuyQuantity3;
	uint64_t iBuyQuantity4;
	uint64_t iBuyQuantity5;

	MarketData()
	{
		memset(this, 0, sizeof(MarketData));
	}
	void set(MarketData* s1, MarketData* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(MarketData));
	}
	MarketData& operator=(const MarketData& s)//重载运算符
	{
		set(this, (MarketData*)&s);
		return *this;
	}
	MarketData(const MarketData& s)//复制构造函数
	{
		*this = s;
	}

};

//增减资金
struct AddDelMoneyReqT
{
	char szUserId[20]; //交易账号
	int  iFlag;  //操作标志：0-增加资金，1-减少资金
	unsigned long long  iAmount; //资金的金额,小数点三位

	AddDelMoneyReqT()  //默认构造函数
	{
		memset(this, 0, sizeof(AddDelMoneyReqT));
	}
	void set(AddDelMoneyReqT* s1, AddDelMoneyReqT* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(AddDelMoneyReqT));
	}
	AddDelMoneyReqT& operator=(const AddDelMoneyReqT& s)//重载运算符
	{
		set(this, (AddDelMoneyReqT*)&s);
		return *this;
	}
	AddDelMoneyReqT(const AddDelMoneyReqT& s)//复制构造函数
	{
		*this = s;
	}

};
struct AddDelMoneyResT
{};//无报文体


#endif