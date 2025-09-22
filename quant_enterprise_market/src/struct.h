#ifndef _STRUCT_H
#define _STRUCT_H
#include <string>
#include <string.h>
#include <vector>
#include "const.h"
using namespace std;

/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

///合约保证金率
struct InstrumentMarginRate
{
	//合约代码
	string szInstrumentId;
	//交易所代码
	string szExchangeId;
	///投机套保标志
	char 	cHedgeFlag;
	///多头保证金率
	double	dLongMarginRatioByMoney;
	///多头保证金费
	double	dLongMarginRatioByVolume;
	///空头保证金率
	double	dShortMarginRatioByMoney;
	///空头保证金费
	double	dShortMarginRatioByVolume;
	///是否相对交易所收取
	int	iIsRelative;

	InstrumentMarginRate()  //默认构造函数
	{
		szInstrumentId = "";
		szExchangeId = "";
		cHedgeFlag = 0;
		dLongMarginRatioByMoney = 0;
		dLongMarginRatioByVolume = 0;
		dShortMarginRatioByMoney = 0;
		dShortMarginRatioByVolume = 0;
		iIsRelative = 0;
	}

	void set(InstrumentMarginRate* s1, InstrumentMarginRate* s2)//赋值函数
	{
		s1->szInstrumentId = s2->szInstrumentId;
		s1->szExchangeId = s2->szExchangeId;
		s1->cHedgeFlag = s2->cHedgeFlag;
		s1->dLongMarginRatioByMoney = s2->dLongMarginRatioByMoney;
		s1->dLongMarginRatioByVolume = s2->dLongMarginRatioByVolume;
		s1->dShortMarginRatioByMoney = s2->dShortMarginRatioByMoney;
		s1->dShortMarginRatioByVolume = s2->dShortMarginRatioByVolume;
		s1->iIsRelative = s2->iIsRelative;
	}

	InstrumentMarginRate& operator=(const InstrumentMarginRate& s)//重载运算符
	{
		set(this, (InstrumentMarginRate*)&s);
		return *this;
	}

	InstrumentMarginRate(const InstrumentMarginRate& s)//复制构造函数
	{
		*this = s;
	}
};

///合约手续费率
struct InstrumentCommissionRate
{
	//合约代码
	string szInstrumentId;
	//交易所代码
	string szExchangeId;
	///开仓手续费率
	double	dOpenRatioByMoney;
	///开仓手续费
	double	dOpenRatioByVolume;
	///平仓手续费率
	double	dCloseRatioByMoney;
	///平仓手续费
	double	dCloseRatioByVolume;
	///平今手续费率
	double	dCloseTodayRatioByMoney;
	///平今手续费
	double	dCloseTodayRatioByVolume;
	///业务类型
	char	cBizType;

	InstrumentCommissionRate()  //默认构造函数
	{
		szInstrumentId = "";
		szExchangeId = "";
		dOpenRatioByMoney = 0;
		dOpenRatioByVolume = 0;
		dCloseRatioByMoney = 0;
		dCloseRatioByVolume = 0;
		dCloseTodayRatioByMoney = 0;
		dCloseTodayRatioByVolume = 0;
		cBizType = 0;
	}

	void set(InstrumentCommissionRate* s1, InstrumentCommissionRate* s2)//赋值函数
	{
		s1->szInstrumentId = s2->szInstrumentId;
		s1->szExchangeId = s2->szExchangeId;
		s1->dOpenRatioByMoney = s2->dOpenRatioByMoney;
		s1->dOpenRatioByVolume = s2->dOpenRatioByVolume;
		s1->dCloseRatioByMoney = s2->dCloseRatioByMoney;
		s1->dCloseRatioByVolume = s2->dCloseRatioByVolume;
		s1->dCloseTodayRatioByMoney = s2->dCloseTodayRatioByMoney;
		s1->dCloseTodayRatioByVolume = s2->dCloseTodayRatioByVolume;
		s1->cBizType = s2->cBizType;
	}

	InstrumentCommissionRate& operator=(const InstrumentCommissionRate& s)//重载运算符
	{
		set(this, (InstrumentCommissionRate*)&s);
		return *this;
	}

	InstrumentCommissionRate(const InstrumentCommissionRate& s)//复制构造函数
	{
		*this = s;
	}
};

///当前报单手续费的详细内容
struct InstrumentOrderCommRate
{
	//合约代码
	string szInstrumentId;
	//交易所代码
	string szExchangeId;
	///投机套保标志
	char	cHedgeFlag;
	///报单手续费
	double	dOrderCommByVolume;
	///撤单手续费
	double	dOrderActionCommByVolume;

	InstrumentOrderCommRate()  //默认构造函数
	{
		szInstrumentId = "";
		szExchangeId = "";
		cHedgeFlag = 0;
		dOrderCommByVolume = 0;
		dOrderActionCommByVolume = 0;
	}

	void set(InstrumentOrderCommRate* s1, InstrumentOrderCommRate* s2)//赋值函数
	{
		s1->szInstrumentId = s2->szInstrumentId;
		s1->szExchangeId = s2->szExchangeId;
		s1->cHedgeFlag = s2->cHedgeFlag;
		s1->dOrderCommByVolume = s2->dOrderCommByVolume;
		s1->dOrderActionCommByVolume = s2->dOrderActionCommByVolume;
	}

	InstrumentOrderCommRate& operator=(const InstrumentOrderCommRate& s)//重载运算符
	{
		set(this, (InstrumentOrderCommRate*)&s);
		return *this;
	}

	InstrumentOrderCommRate(const InstrumentOrderCommRate& s)//复制构造函数
	{
		*this = s;
	}
};

//合约信息
struct InstrumentT
{
	string szInstrumentId; //合约代码
	string szExchangeId; //交易所代码
	string szInstrumentName;//合约名称
	string szProductId;//产品代码
	string szProductClass; //产品类型
	int iDeliveryYear; //交割年份
	int iDeliveryMonth; //交割月
	int  iMaxMarketOrderVolume;//市价单最大下单量
	int  iMinMarketOrderVolume;//市价单最小下单量
	int  iMaxLimitOrderVolume;//限价单最大下单量
	int  iMinLimitOrderVolume;//限价单最小下单量
	int iVolumeMultiple;//合约数量乘数
	double dPriceTick;//最小变动价位
	string szExpireDate;//到期日
	int iIsTrading;//当前是否交易
	string szPositionType;//持仓类型 1-净持仓 2-综合持仓
	string szPositionDateType;//持仓日期类型 1-使用历史持仓 2-不使用历史持仓
	double dLongMarginRatio;//多头保证金率
	double dShortMarginRatio;//空头保证金率
	string szMaxMarginSideAlgorithm;//是否使用大额单边保证金算法
	double  dStrikePrice;//执行价
	char  cOptionsType;//期权类型
	double dUnderlyingMultiple;//合约基础商品乘数

	InstrumentT()  //默认构造函数
	{
		szInstrumentId = "";
		szExchangeId = "";
		szInstrumentName = "";
		szProductId = "";
		szProductClass = "";
		iDeliveryYear = 0;
		iDeliveryMonth = 0;
		iMaxMarketOrderVolume = 0;
		iMinMarketOrderVolume = 0;
		iMaxLimitOrderVolume = 0;
		iMinLimitOrderVolume = 0;
		iVolumeMultiple = 0;
		dPriceTick = 0;
		szExpireDate = "";
		iIsTrading = 0;
		szPositionType = "";
		szPositionDateType = "";
		dLongMarginRatio = 0;
		dShortMarginRatio = 0;
		szMaxMarginSideAlgorithm = "";
		dStrikePrice = 0;
		cOptionsType = 0;
		dUnderlyingMultiple = 0;
	}

	void set(InstrumentT* s1, InstrumentT* s2)//赋值函数
	{
		s1->szInstrumentId = s2->szInstrumentId;
		s1->szExchangeId = s2->szExchangeId;
		s1->szInstrumentName = s2->szInstrumentName;
		s1->szProductId = s2->szProductId;
		s1->szProductClass = s2->szProductClass;
		s1->iDeliveryYear = s2->iDeliveryYear;
		s1->iDeliveryMonth = s2->iDeliveryMonth;
		s1->iMaxMarketOrderVolume = s2->iMaxMarketOrderVolume;
		s1->iMinMarketOrderVolume = s2->iMinMarketOrderVolume;
		s1->iMaxLimitOrderVolume = s2->iMaxLimitOrderVolume;
		s1->iMinLimitOrderVolume = s2->iMinLimitOrderVolume;
		s1->iVolumeMultiple = s2->iVolumeMultiple;
		s1->dPriceTick = s2->dPriceTick;
		s1->szExpireDate = s2->szExpireDate;
		s1->iIsTrading = s2->iIsTrading;
		s1->szPositionType = s2->szPositionType;
		s1->szPositionDateType = s2->szPositionDateType;
		s1->dLongMarginRatio = s2->dLongMarginRatio;
		s1->dShortMarginRatio = s2->dShortMarginRatio;
		s1->szMaxMarginSideAlgorithm = s2->szMaxMarginSideAlgorithm;
		s1->dStrikePrice = s2->dStrikePrice;
		s1->cOptionsType = s2->cOptionsType;
		s1->dUnderlyingMultiple = s2->dUnderlyingMultiple;
	}

	InstrumentT& operator=(const InstrumentT& s)//重载运算符
	{
		set(this, (InstrumentT*)&s);
		return *this;
	}

	InstrumentT(const InstrumentT& s)//复制构造函数
	{
		*this = s;
	}
};

//资金
struct BalanceT
{
	string szUserId; //账号
	int iTdType; //交易类型：0-ctp
	double	dUseMargin;///占用的保证金
	double	dFrozenMargin; ///冻结的保证金
	double	dFrozenCash; ///冻结的资金
	double	dFrozenCommission; ///冻结的手续费
	double	dCurrMargin; ///当前保证金总额
	double	dCommission; ///手续费
	double	dAvailable; ///可用资金

	double  dStartMoney; ///期初的资金
	double  dAddMoney; ///调仓增加的资金

	int iSettleDate; //结算日期
	int iModifyDate; //修改日期
	int iModifyTime; //修改时间

	BalanceT()  //默认构造函数
	{
		szUserId = "";
		iTdType = 0;
		dUseMargin = 0;
		dFrozenMargin = 0;
		dFrozenCash = 0;
		dFrozenCommission = 0;
		dCurrMargin = 0;
		dCommission = 0;
		dAvailable = 0;
		dStartMoney = 0;
		dAddMoney = 0;
		iModifyDate = 0;
		iModifyTime = 0;
	}

	void set(BalanceT* s1, BalanceT* s2)//赋值函数
	{
		s1->szUserId = s2->szUserId;
		s1->iTdType = s2->iTdType;
		s1->dUseMargin = s2->dUseMargin;
		s1->dFrozenMargin = s2->dFrozenMargin;
		s1->dFrozenCash = s2->dFrozenCash;
		s1->dFrozenCommission = s2->dFrozenCommission;
		s1->dCurrMargin = s2->dCurrMargin;
		s1->dCommission = s2->dCommission;
		s1->dAvailable = s2->dAvailable;
		s1->dStartMoney = s2->dStartMoney;
		s1->dAddMoney = s2->dAddMoney;
		s1->iModifyDate = s2->iModifyDate;
		s1->iModifyTime = s2->iModifyTime;
	}

	BalanceT& operator=(const BalanceT& s)//重载运算符
	{
		set(this, (BalanceT*)&s);
		return *this;
	}

	BalanceT(const BalanceT& s)//复制构造函数
	{
		*this = s;
	}


};

/*
说明：
cAction:
买进开仓：指投资者对于未来价格趋势看涨而采取的交易手段，买进持有看涨合约，意味着帐户资金买进合约而冻结。
卖出平仓：是指投资者对未来价格趋势不看好而采取的交易手段，而将原来买进的看涨合约卖出，投资者资金帐户解冻。
卖出开仓：是指投资者对未来价格趋势看跌而采取的交易手段，卖出看跌合约。卖出开仓，帐户资金冻结。
买进平仓：是指投资者将持有的卖出合约对未来行情不再看跌而补回以前卖出合约，与原来的卖出合约对冲抵消退出市场，帐户资金解冻
*/

//委托
struct OrderT
{
	int iInitDate; //本地日期
	int iInitTime; //本地时间
	int iModifyDate; //修改日期
	int iModifyTime; //修改时间
	int iTdType; //交易类型：0-ctp
	string szJysInsertDate; //交易所报单日期
	string szJysInsertTime; //交易所委托时间
	string szUserId; //交易账户
	string szExchangeID; //市场
	string szStockCode;  //代码
	string szClientOrderId; //客户端委托id，唯一，不要重复
	string szOrderRef;  //报单引用  12位字符
	string szOrderSysID; // 报单编号,相当于委托编号
	int   iFrontID;  //前置编号
	int   iSessionID; //会话编号
	char cAction;  //买卖操作
	double  dPrice;  //价格
	int iEntrustNum; //委托数量
	int iTradeNum; //成交数量
	char   cStatus;//报单状态
	double	dFrozenMargin; ///冻结的保证金
	double	dFrozenCommission;///冻结的手续费
	int iLongShortFrozen; //多空冻结数量
	double dLongShortFrozenAmount; //多空冻结金额

	int iVolumeMultiple;//合约数量乘数
	double	dMarginRatioByMoney;///保证金率
	double	dMarginRatioByVolume;///保证金费
	double	dRatioByMoney;///开平仓合约手续费率
	double	dRatioByVolume;///开平仓合约手续费
	double	dCloseTodayRatioByMoney;///平今手续费率
	double	dCloseTodayRatioByVolume;///平今手续费
	double	dOrderCommByVolume;///报单手续费
	double	dOrderActionCommByVolume;///撤单手续费
	double  dPreSettlementPrice; //昨结算价


	//有string类型，不可以使用memset对结构体初始化
	OrderT()
	{
		iInitDate = 0;
		iInitTime = 0;
		iModifyDate = 0;
		iModifyTime = 0;
		iTdType = 0;
		szJysInsertDate = "";
		szJysInsertTime = "";
		szUserId = "";
		szExchangeID = "";
		szStockCode = "";
		szClientOrderId = "";
		szOrderRef = "";
		szOrderSysID = "";
		iFrontID = 0;
		iSessionID = 0;
		cAction = '\0';
		dPrice = 0;
		iEntrustNum = 0;
		iTradeNum = 0;
		cStatus = '0';
		dFrozenMargin = 0;
		dFrozenCommission = 0;
		iLongShortFrozen = 0;
		dLongShortFrozenAmount = 0;
		iVolumeMultiple = 0;
		dMarginRatioByMoney = 0;
		dMarginRatioByVolume = 0;
		dRatioByMoney = 0;
		dRatioByVolume = 0;
		dCloseTodayRatioByMoney = 0;
		dCloseTodayRatioByVolume = 0;
		dOrderCommByVolume = 0;
		dOrderActionCommByVolume = 0;
		dPreSettlementPrice = 0;
	}

	void set(OrderT* s1, OrderT* s2)//赋值函数
	{
		s1->iInitDate = s2->iInitDate;
		s1->iInitTime = s2->iInitTime;
		s1->iModifyDate = s2->iModifyDate;
		s1->iModifyTime = s2->iModifyTime;
		s1->iTdType = s2->iTdType;
		s1->szJysInsertDate = s2->szJysInsertDate;
		s1->szJysInsertTime = s2->szJysInsertTime;
		s1->szUserId = s2->szUserId;
		s1->szExchangeID = s2->szExchangeID;
		s1->szStockCode = s2->szStockCode;
		s1->szClientOrderId = s2->szClientOrderId;
		s1->szOrderRef = s2->szOrderRef;
		s1->szOrderSysID = s2->szOrderSysID;
		s1->iFrontID = s2->iFrontID;
		s1->iSessionID = s2->iSessionID;
		s1->cAction = s2->cAction;
		s1->dPrice = s2->dPrice;
		s1->iEntrustNum = s2->iEntrustNum;
		s1->iTradeNum = s2->iTradeNum;
		s1->cStatus = s2->cStatus;
		s1->dFrozenMargin = s2->dFrozenMargin;
		s1->dFrozenCommission = s2->dFrozenCommission;
		s1->iLongShortFrozen = s2->iLongShortFrozen;
		s1->dLongShortFrozenAmount = s2->dLongShortFrozenAmount;
		s1->iVolumeMultiple = s2->iVolumeMultiple;
		s1->dMarginRatioByMoney = s2->dMarginRatioByMoney;
		s1->dMarginRatioByVolume = s2->dMarginRatioByVolume;
		s1->dRatioByMoney = s2->dRatioByMoney;
		s1->dRatioByVolume = s2->dRatioByVolume;
		s1->dCloseTodayRatioByMoney = s2->dCloseTodayRatioByMoney;
		s1->dCloseTodayRatioByVolume = s2->dCloseTodayRatioByVolume;
		s1->dOrderCommByVolume = s2->dOrderCommByVolume;
		s1->dOrderActionCommByVolume = s2->dOrderActionCommByVolume;
		s1->dPreSettlementPrice = s2->dPreSettlementPrice;
	}
	OrderT& operator=(const OrderT& s)//重载运算符
	{
		set(this, (OrderT*)&s);
		return *this;
	}
	OrderT(const OrderT& s)//复制构造函数
	{
		*this = s;
	}

};

//成交
struct TradeT
{
	int iInitDate; //本地日期 yyyymmdd
	int iInitTime; //本地时间 HHMMSS
	int iTdType; //交易类型：0-ctp
	string szJysTradeDate;//交易所成交日期
	string szJysTradeTime;//交易所成交时间
	string szUserId; //交易账户
	string szExchangeID; //市场
	string szStockCode;  //代码
	string szOrderRef;  //报单引用
	string szOrderSysID; // 报单编号,相当于委托编号
	string szTradeID; //成交编号
	char cAction;  //买卖操作
	double  dPrice;  //价格
	int iTradeNum; //成交数量
	int iCloseNum; //当天平仓数量

	TradeT()  //默认构造函数
	{
		iInitDate = 0;
		iInitTime = 0;
		iTdType = 0;
		szJysTradeDate = "";
		szJysTradeTime = "";
		szUserId = "";
		szExchangeID = "";
		szStockCode = "";
		szOrderRef = "";
		szOrderSysID = "";
		szTradeID = "";
		cAction = '\0';
		dPrice = 0;
		iTradeNum = 0;
		iCloseNum = 0;
	}

	void set(TradeT* s1, TradeT* s2)//赋值函数
	{

		s1->iInitDate = s2->iInitDate;
		s1->iInitTime = s2->iInitTime;
		s1->iTdType = s2->iTdType;
		s1->szJysTradeDate = s2->szJysTradeDate;
		s1->szJysTradeTime = s2->szJysTradeTime;
		s1->szUserId = s2->szUserId;
		s1->szExchangeID = s2->szExchangeID;
		s1->szStockCode = s2->szStockCode;
		s1->szOrderRef = s2->szOrderRef;
		s1->szOrderSysID = s2->szOrderSysID;
		s1->szTradeID = s2->szTradeID;
		s1->cAction = s2->cAction;
		s1->dPrice = s2->dPrice;
		s1->iTradeNum = s2->iTradeNum;
		s1->iCloseNum = s2->iCloseNum;
	}
	TradeT& operator=(const TradeT& s)//重载运算符
	{
		set(this, (TradeT*)&s);
		return *this;
	}
	TradeT(const TradeT& s)//复制构造函数
	{
		*this = s;
	}


};

//持仓
struct PositionT
{
	int iInitDate; //日期 yyyymmdd
	int iInitTime; //本地时间 HHMMSS
	int iModifyDate; //修改日期
	int iModifyTime; //修改时间
	int iTdType; //交易类型：0-ctp
	string szUserId; //账号
	string szExchangeID; //市场
	string szStockCode;  //代码
	char  cDirection; //持仓方向
	char  cHedgeFlag;///投机套保标志
	int	iYdPosition;///昨日持仓
	int	iTodayPosition; ///今日持仓
	int	iPosition; ///总持仓
	int	iLongFrozen; ///多头冻结
	int	iShortFrozen; ///空头冻结
	double	dLongFrozenAmount;///多头冻结金额
	double	dShortFrozenAmount;///空头冻结金额
	double	dUseMargin; ///占用的保证金
	double	dFrozenMargin; ///冻结的保证金
	double	dFrozenCash; ///冻结的资金
	double	dFrozenCommission;///冻结的手续费
	double	dCommission; ///手续费
	double	dPreSettlementPrice; ///上次结算价
	double	dSettlementPrice; ///本次结算价
	double	dMarginRateByMoney; ///保证金率
	double	dMarginRateByVolume; ///保证金率(按手数)
	char    cOperType; //操作类型
	char    cAction;  //买卖操作


	PositionT()  //默认构造函数
	{
		iInitDate = 0;
		iInitTime = 0;
		iModifyDate = 0;
		iModifyTime = 0;
		iTdType = 0;
		szUserId = "";
		szExchangeID = "";
		szStockCode = "";
		cDirection = '\0';
		cHedgeFlag = '\0';
		iYdPosition = 0;
		iTodayPosition = 0;
		iPosition = 0;
		iLongFrozen = 0;
		iShortFrozen = 0;
		dLongFrozenAmount = 0;
		dShortFrozenAmount = 0;
		dUseMargin = 0;
		dFrozenMargin = 0;
		dFrozenCash = 0;
		dFrozenCommission = 0;
		dCommission = 0;
		dPreSettlementPrice = 0;
		dSettlementPrice = 0;
		dMarginRateByMoney = 0;
		dMarginRateByVolume = 0;
		cOperType = '\0';
		cAction = '\0';
	}

	void set(PositionT* s1, PositionT* s2)//赋值函数
	{
		s1->iInitDate = s2->iInitDate;
		s1->iInitTime = s2->iInitTime;
		s1->iModifyDate = s2->iModifyDate;
		s1->iModifyTime = s2->iModifyTime;
		s1->iTdType = s2->iTdType;
		s1->szUserId = s2->szUserId;
		s1->szExchangeID = s2->szExchangeID;
		s1->szStockCode = s2->szStockCode;
		s1->cDirection = s2->cDirection;
		s1->cHedgeFlag = s2->cHedgeFlag;
		s1->iYdPosition = s2->iYdPosition;
		s1->iTodayPosition = s2->iTodayPosition;
		s1->iPosition = s2->iPosition;
		s1->iLongFrozen = s2->iLongFrozen;
		s1->iShortFrozen = s2->iShortFrozen;
		s1->dLongFrozenAmount = s2->dLongFrozenAmount;
		s1->dShortFrozenAmount = s2->dShortFrozenAmount;
		s1->dUseMargin = s2->dUseMargin;
		s1->dFrozenMargin = s2->dFrozenMargin;
		s1->dFrozenCash = s2->dFrozenCash;
		s1->dFrozenCommission = s2->dFrozenCommission;
		s1->dCommission = s2->dCommission;
		s1->dPreSettlementPrice = s2->dPreSettlementPrice;
		s1->dSettlementPrice = s2->dSettlementPrice;
		s1->dMarginRateByMoney = s2->dMarginRateByMoney;
		s1->dMarginRateByVolume = s2->dMarginRateByVolume;
		s1->cOperType = s2->cOperType;
		s1->cAction = s2->cAction;
	}
	PositionT& operator=(const PositionT& s)//重载运算符
	{
		set(this, (PositionT*)&s);
		return *this;
	}
	PositionT(const PositionT& s)//复制构造函数
	{
		*this = s;
	}


};

//开仓数量
struct OpenCountT
{
	int iInitDate; //本地日期
	int iTdType; //交易类型：0-ctp
	string szUserId; //交易账户
	string szExchangeID; //市场
	string szStockCode;  //代码
	char  cDirection; //持仓方向
	int iOpenCount; //开仓数量

	OpenCountT()  //默认构造函数
	{
		iInitDate = 0;
		iTdType = 0;
		szUserId = "";
		szExchangeID = "";
		szStockCode = "";
		cDirection = '\0';
		iOpenCount = 0;
	}

	void set(OpenCountT* s1, OpenCountT* s2)//赋值函数
	{
		s1->iInitDate = s2->iInitDate;
		s1->iTdType = s2->iTdType;
		s1->szUserId = s2->szUserId;
		s1->szExchangeID = s2->szExchangeID;
		s1->szStockCode = s2->szStockCode;
		s1->cDirection = s2->cDirection;
		s1->iOpenCount = s2->iOpenCount;
	}
	OpenCountT& operator=(const OpenCountT& s)//重载运算符
	{
		set(this, (OpenCountT*)&s);
		return *this;
	}
	OpenCountT(const OpenCountT& s)//复制构造函数
	{
		*this = s;
	}
};

//消息队列的数据结构
struct MsgQueueData
{
	char szData[SHARE_MEMORY_MSG_MAX_LENGTH];

	MsgQueueData()  //默认构造函数
	{
		memset(this, 0, sizeof(MsgQueueData));
	}

	void set(MsgQueueData* s1, MsgQueueData* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(MsgQueueData));
	}
	MsgQueueData& operator=(const MsgQueueData& s)//重载运算符
	{
		set(this, (MsgQueueData*)&s);
		return *this;
	}
	MsgQueueData(const MsgQueueData& s)//复制构造函数
	{
		*this = s;
	}
};

//通用消息
struct ServerMsg
{
	int iConnectId;  //连接id
	char szUserId[20];  //账号
	int iFunctionId;  //功能号
	int iRoletype;   //角色类型
	unsigned int iMsgIndex; //报文序号
	int iBodyLen; //报文体长度
	char*pBody;   //报文体指针
	bool   bIsLast;  //最后一条记录，响应消息使用
	int iErrorCode;     //错误码,响应消息使用
	char szErrorInfo[100]; //错误信息,响应消息使用

	ServerMsg()
	{
		iConnectId = 0;
		memset(szUserId, 0, sizeof(szUserId));
		iFunctionId = 0;
		iRoletype = ROLE_TYPE_TRADER;
		iMsgIndex = 0;
		iBodyLen = 0;
		pBody = NULL;
		bIsLast = true;
		iErrorCode = RET_OK;
		memset(szErrorInfo, 0, sizeof(szErrorInfo));
		snprintf(szErrorInfo, sizeof(szErrorInfo), "OK");
	}

	void set(ServerMsg* s1, ServerMsg* s2)//赋值函数
	{
		memcpy(s1, s2, sizeof(ServerMsg));
	}
	ServerMsg& operator=(const ServerMsg& s)//重载运算符
	{
		set(this, (ServerMsg*)&s);
		return *this;
	}
	ServerMsg(const ServerMsg& s)//复制构造函数
	{
		*this = s;
	}

};


/************************************配置文件中的结构**************************************/
struct CtpConfig
{
	string szTradeIp;
	int iTradePort;
	string szMarketIp;
	int iMarketPort;
	string szBrokerId;
	string szUserId;
	string szPassword;
	string szAuthCode;
	string szAppID;

	CtpConfig()  //默认构造函数
	{
		iTradePort = 0;
		iMarketPort = 0;
	}

	void set(CtpConfig* s1, CtpConfig* s2)//赋值函数
	{
		s1->szTradeIp = s2->szTradeIp;
		s1->iTradePort = s2->iTradePort;
		s1->szMarketIp = s2->szMarketIp;
		s1->iMarketPort = s2->iMarketPort;
		s1->szBrokerId = s2->szBrokerId;
		s1->szUserId = s2->szUserId;
		s1->szPassword = s2->szPassword;
		s1->szAuthCode = s2->szAuthCode;
		s1->szAppID = s2->szAppID;
	}
	CtpConfig& operator=(const CtpConfig& s)//重载运算符
	{
		set(this, (CtpConfig*)&s);
		return *this;
	}
	CtpConfig(const CtpConfig& s)//复制构造函数
	{
		*this = s;
	}

};

struct User
{
	int iRoletype;
	string szUserId;
	string szPassword;

	User()  //默认构造函数
	{
		iRoletype = 0;

	}
	void set(User* s1, User* s2)//赋值函数
	{
		s1->iRoletype = s2->iRoletype;
		s1->szUserId = s2->szUserId;
		s1->szPassword = s2->szPassword;
	}
	User& operator=(const User& s)//重载运算符
	{
		set(this, (User*)&s);
		return *this;
	}
	User(const User& s)//复制构造函数
	{
		*this = s;
	}
};
struct TcpSvrInfo
{
	int iPort;

	TcpSvrInfo()  //默认构造函数
	{
		iPort = 0;

	}
	void set(TcpSvrInfo* s1, TcpSvrInfo* s2)//赋值函数
	{
		s1->iPort = s2->iPort;

	}
	TcpSvrInfo& operator=(const TcpSvrInfo& s)//重载运算符
	{
		set(this, (TcpSvrInfo*)&s);
		return *this;
	}
	TcpSvrInfo(const TcpSvrInfo& s)//复制构造函数
	{
		*this = s;
	}
};
struct LogInfo
{
	int iLoglevel;

	LogInfo()  //默认构造函数
	{
		iLoglevel = 0;

	}
	void set(LogInfo* s1, LogInfo* s2)//赋值函数
	{
		s1->iLoglevel = s2->iLoglevel;

	}
	LogInfo& operator=(const LogInfo& s)//重载运算符
	{
		set(this, (LogInfo*)&s);
		return *this;
	}
	LogInfo(const LogInfo& s)//复制构造函数
	{
		*this = s;
	}
};

struct Bar
{
    string sDate;
    string sTime;
    string sInstrumentID;
    double dOpen;
    double dHigh;
    double dLow;
    double dClose;
    double dVolume;
    double dOpenInterest;

    Bar()//初始化函数
    {
        sDate = "";
        sTime = "";
        sInstrumentID = "";
        dOpen = 0.0;
        dHigh = 0.0;
        dLow = 0.0;
        dClose = 0.0;
        dVolume = 0.0;
        dOpenInterest = 0.0;
    }

    //赋值函数
    void set(Bar *bar1, Bar *bar2)
    {
        bar1->sDate = bar2->sDate;
        bar1->sTime = bar2->sTime;
        bar1->sInstrumentID = bar2->sInstrumentID;
        bar1->dOpen = bar2->dOpen;
        bar1->dHigh = bar2->dHigh;
        bar1->dLow = bar2->dLow;
        bar1->dClose = bar2->dClose;
        bar1->dVolume = bar2->dVolume;
        bar1->dOpenInterest = bar2->dOpenInterest;

    }

    //重载赋值运算符
    Bar &operator=(const Bar &bar)
    {
        if (this == &bar)
        {
            return *this;
        }
        set(this, (Bar *)&bar);
        return *this;
    }

    //复制构造函数
    Bar(const Bar &bar)
    {
        *this = bar;
    }

};

/*****************************************函数通用返回结构***************************************/
template<typename T>
struct TdResponse
{
	T RspResult;
	int iRequestId;
	bool bIsLast;
	int iErrCode;
	string szMsg;
};

template<typename T>
struct MdResponse
{
	T RspResult;
	int iErrCode;
	string szMsg;
};

#endif // !_STRUCT_H