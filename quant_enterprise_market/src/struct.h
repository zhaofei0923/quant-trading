#ifndef _STRUCT_H
#define _STRUCT_H
#include <string>
#include <string.h>
#include <vector>
#include "const.h"
using namespace std;

/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

///��Լ��֤����
struct InstrumentMarginRate
{
	//��Լ����
	string szInstrumentId;
	//����������
	string szExchangeId;
	///Ͷ���ױ���־
	char 	cHedgeFlag;
	///��ͷ��֤����
	double	dLongMarginRatioByMoney;
	///��ͷ��֤���
	double	dLongMarginRatioByVolume;
	///��ͷ��֤����
	double	dShortMarginRatioByMoney;
	///��ͷ��֤���
	double	dShortMarginRatioByVolume;
	///�Ƿ���Խ�������ȡ
	int	iIsRelative;

	InstrumentMarginRate()  //Ĭ�Ϲ��캯��
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

	void set(InstrumentMarginRate* s1, InstrumentMarginRate* s2)//��ֵ����
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

	InstrumentMarginRate& operator=(const InstrumentMarginRate& s)//���������
	{
		set(this, (InstrumentMarginRate*)&s);
		return *this;
	}

	InstrumentMarginRate(const InstrumentMarginRate& s)//���ƹ��캯��
	{
		*this = s;
	}
};

///��Լ��������
struct InstrumentCommissionRate
{
	//��Լ����
	string szInstrumentId;
	//����������
	string szExchangeId;
	///������������
	double	dOpenRatioByMoney;
	///����������
	double	dOpenRatioByVolume;
	///ƽ����������
	double	dCloseRatioByMoney;
	///ƽ��������
	double	dCloseRatioByVolume;
	///ƽ����������
	double	dCloseTodayRatioByMoney;
	///ƽ��������
	double	dCloseTodayRatioByVolume;
	///ҵ������
	char	cBizType;

	InstrumentCommissionRate()  //Ĭ�Ϲ��캯��
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

	void set(InstrumentCommissionRate* s1, InstrumentCommissionRate* s2)//��ֵ����
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

	InstrumentCommissionRate& operator=(const InstrumentCommissionRate& s)//���������
	{
		set(this, (InstrumentCommissionRate*)&s);
		return *this;
	}

	InstrumentCommissionRate(const InstrumentCommissionRate& s)//���ƹ��캯��
	{
		*this = s;
	}
};

///��ǰ���������ѵ���ϸ����
struct InstrumentOrderCommRate
{
	//��Լ����
	string szInstrumentId;
	//����������
	string szExchangeId;
	///Ͷ���ױ���־
	char	cHedgeFlag;
	///����������
	double	dOrderCommByVolume;
	///����������
	double	dOrderActionCommByVolume;

	InstrumentOrderCommRate()  //Ĭ�Ϲ��캯��
	{
		szInstrumentId = "";
		szExchangeId = "";
		cHedgeFlag = 0;
		dOrderCommByVolume = 0;
		dOrderActionCommByVolume = 0;
	}

	void set(InstrumentOrderCommRate* s1, InstrumentOrderCommRate* s2)//��ֵ����
	{
		s1->szInstrumentId = s2->szInstrumentId;
		s1->szExchangeId = s2->szExchangeId;
		s1->cHedgeFlag = s2->cHedgeFlag;
		s1->dOrderCommByVolume = s2->dOrderCommByVolume;
		s1->dOrderActionCommByVolume = s2->dOrderActionCommByVolume;
	}

	InstrumentOrderCommRate& operator=(const InstrumentOrderCommRate& s)//���������
	{
		set(this, (InstrumentOrderCommRate*)&s);
		return *this;
	}

	InstrumentOrderCommRate(const InstrumentOrderCommRate& s)//���ƹ��캯��
	{
		*this = s;
	}
};

//��Լ��Ϣ
struct InstrumentT
{
	string szInstrumentId; //��Լ����
	string szExchangeId; //����������
	string szInstrumentName;//��Լ����
	string szProductId;//��Ʒ����
	string szProductClass; //��Ʒ����
	int iDeliveryYear; //�������
	int iDeliveryMonth; //������
	int  iMaxMarketOrderVolume;//�м۵�����µ���
	int  iMinMarketOrderVolume;//�м۵���С�µ���
	int  iMaxLimitOrderVolume;//�޼۵�����µ���
	int  iMinLimitOrderVolume;//�޼۵���С�µ���
	int iVolumeMultiple;//��Լ��������
	double dPriceTick;//��С�䶯��λ
	string szExpireDate;//������
	int iIsTrading;//��ǰ�Ƿ���
	string szPositionType;//�ֲ����� 1-���ֲ� 2-�ۺϳֲ�
	string szPositionDateType;//�ֲ��������� 1-ʹ����ʷ�ֲ� 2-��ʹ����ʷ�ֲ�
	double dLongMarginRatio;//��ͷ��֤����
	double dShortMarginRatio;//��ͷ��֤����
	string szMaxMarginSideAlgorithm;//�Ƿ�ʹ�ô��߱�֤���㷨
	double  dStrikePrice;//ִ�м�
	char  cOptionsType;//��Ȩ����
	double dUnderlyingMultiple;//��Լ������Ʒ����

	InstrumentT()  //Ĭ�Ϲ��캯��
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

	void set(InstrumentT* s1, InstrumentT* s2)//��ֵ����
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

	InstrumentT& operator=(const InstrumentT& s)//���������
	{
		set(this, (InstrumentT*)&s);
		return *this;
	}

	InstrumentT(const InstrumentT& s)//���ƹ��캯��
	{
		*this = s;
	}
};

//�ʽ�
struct BalanceT
{
	string szUserId; //�˺�
	int iTdType; //�������ͣ�0-ctp
	double	dUseMargin;///ռ�õı�֤��
	double	dFrozenMargin; ///����ı�֤��
	double	dFrozenCash; ///������ʽ�
	double	dFrozenCommission; ///�����������
	double	dCurrMargin; ///��ǰ��֤���ܶ�
	double	dCommission; ///������
	double	dAvailable; ///�����ʽ�

	double  dStartMoney; ///�ڳ����ʽ�
	double  dAddMoney; ///�������ӵ��ʽ�

	int iSettleDate; //��������
	int iModifyDate; //�޸�����
	int iModifyTime; //�޸�ʱ��

	BalanceT()  //Ĭ�Ϲ��캯��
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

	void set(BalanceT* s1, BalanceT* s2)//��ֵ����
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

	BalanceT& operator=(const BalanceT& s)//���������
	{
		set(this, (BalanceT*)&s);
		return *this;
	}

	BalanceT(const BalanceT& s)//���ƹ��캯��
	{
		*this = s;
	}


};

/*
˵����
cAction:
������֣�ָͶ���߶���δ���۸����ƿ��Ƕ���ȡ�Ľ����ֶΣ�������п��Ǻ�Լ����ζ���ʻ��ʽ������Լ�����ᡣ
����ƽ�֣���ָͶ���߶�δ���۸����Ʋ����ö���ȡ�Ľ����ֶΣ�����ԭ������Ŀ��Ǻ�Լ������Ͷ�����ʽ��ʻ��ⶳ��
�������֣���ָͶ���߶�δ���۸����ƿ�������ȡ�Ľ����ֶΣ�����������Լ���������֣��ʻ��ʽ𶳽ᡣ
���ƽ�֣���ָͶ���߽����е�������Լ��δ�����鲻�ٿ�����������ǰ������Լ����ԭ����������Լ�Գ�����˳��г����ʻ��ʽ�ⶳ
*/

//ί��
struct OrderT
{
	int iInitDate; //��������
	int iInitTime; //����ʱ��
	int iModifyDate; //�޸�����
	int iModifyTime; //�޸�ʱ��
	int iTdType; //�������ͣ�0-ctp
	string szJysInsertDate; //��������������
	string szJysInsertTime; //������ί��ʱ��
	string szUserId; //�����˻�
	string szExchangeID; //�г�
	string szStockCode;  //����
	string szClientOrderId; //�ͻ���ί��id��Ψһ����Ҫ�ظ�
	string szOrderRef;  //��������  12λ�ַ�
	string szOrderSysID; // �������,�൱��ί�б��
	int   iFrontID;  //ǰ�ñ��
	int   iSessionID; //�Ự���
	char cAction;  //��������
	double  dPrice;  //�۸�
	int iEntrustNum; //ί������
	int iTradeNum; //�ɽ�����
	char   cStatus;//����״̬
	double	dFrozenMargin; ///����ı�֤��
	double	dFrozenCommission;///�����������
	int iLongShortFrozen; //��ն�������
	double dLongShortFrozenAmount; //��ն�����

	int iVolumeMultiple;//��Լ��������
	double	dMarginRatioByMoney;///��֤����
	double	dMarginRatioByVolume;///��֤���
	double	dRatioByMoney;///��ƽ�ֺ�Լ��������
	double	dRatioByVolume;///��ƽ�ֺ�Լ������
	double	dCloseTodayRatioByMoney;///ƽ����������
	double	dCloseTodayRatioByVolume;///ƽ��������
	double	dOrderCommByVolume;///����������
	double	dOrderActionCommByVolume;///����������
	double  dPreSettlementPrice; //������


	//��string���ͣ�������ʹ��memset�Խṹ���ʼ��
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

	void set(OrderT* s1, OrderT* s2)//��ֵ����
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
	OrderT& operator=(const OrderT& s)//���������
	{
		set(this, (OrderT*)&s);
		return *this;
	}
	OrderT(const OrderT& s)//���ƹ��캯��
	{
		*this = s;
	}

};

//�ɽ�
struct TradeT
{
	int iInitDate; //�������� yyyymmdd
	int iInitTime; //����ʱ�� HHMMSS
	int iTdType; //�������ͣ�0-ctp
	string szJysTradeDate;//�������ɽ�����
	string szJysTradeTime;//�������ɽ�ʱ��
	string szUserId; //�����˻�
	string szExchangeID; //�г�
	string szStockCode;  //����
	string szOrderRef;  //��������
	string szOrderSysID; // �������,�൱��ί�б��
	string szTradeID; //�ɽ����
	char cAction;  //��������
	double  dPrice;  //�۸�
	int iTradeNum; //�ɽ�����
	int iCloseNum; //����ƽ������

	TradeT()  //Ĭ�Ϲ��캯��
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

	void set(TradeT* s1, TradeT* s2)//��ֵ����
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
	TradeT& operator=(const TradeT& s)//���������
	{
		set(this, (TradeT*)&s);
		return *this;
	}
	TradeT(const TradeT& s)//���ƹ��캯��
	{
		*this = s;
	}


};

//�ֲ�
struct PositionT
{
	int iInitDate; //���� yyyymmdd
	int iInitTime; //����ʱ�� HHMMSS
	int iModifyDate; //�޸�����
	int iModifyTime; //�޸�ʱ��
	int iTdType; //�������ͣ�0-ctp
	string szUserId; //�˺�
	string szExchangeID; //�г�
	string szStockCode;  //����
	char  cDirection; //�ֲַ���
	char  cHedgeFlag;///Ͷ���ױ���־
	int	iYdPosition;///���ճֲ�
	int	iTodayPosition; ///���ճֲ�
	int	iPosition; ///�ֲܳ�
	int	iLongFrozen; ///��ͷ����
	int	iShortFrozen; ///��ͷ����
	double	dLongFrozenAmount;///��ͷ������
	double	dShortFrozenAmount;///��ͷ������
	double	dUseMargin; ///ռ�õı�֤��
	double	dFrozenMargin; ///����ı�֤��
	double	dFrozenCash; ///������ʽ�
	double	dFrozenCommission;///�����������
	double	dCommission; ///������
	double	dPreSettlementPrice; ///�ϴν����
	double	dSettlementPrice; ///���ν����
	double	dMarginRateByMoney; ///��֤����
	double	dMarginRateByVolume; ///��֤����(������)
	char    cOperType; //��������
	char    cAction;  //��������


	PositionT()  //Ĭ�Ϲ��캯��
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

	void set(PositionT* s1, PositionT* s2)//��ֵ����
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
	PositionT& operator=(const PositionT& s)//���������
	{
		set(this, (PositionT*)&s);
		return *this;
	}
	PositionT(const PositionT& s)//���ƹ��캯��
	{
		*this = s;
	}


};

//��������
struct OpenCountT
{
	int iInitDate; //��������
	int iTdType; //�������ͣ�0-ctp
	string szUserId; //�����˻�
	string szExchangeID; //�г�
	string szStockCode;  //����
	char  cDirection; //�ֲַ���
	int iOpenCount; //��������

	OpenCountT()  //Ĭ�Ϲ��캯��
	{
		iInitDate = 0;
		iTdType = 0;
		szUserId = "";
		szExchangeID = "";
		szStockCode = "";
		cDirection = '\0';
		iOpenCount = 0;
	}

	void set(OpenCountT* s1, OpenCountT* s2)//��ֵ����
	{
		s1->iInitDate = s2->iInitDate;
		s1->iTdType = s2->iTdType;
		s1->szUserId = s2->szUserId;
		s1->szExchangeID = s2->szExchangeID;
		s1->szStockCode = s2->szStockCode;
		s1->cDirection = s2->cDirection;
		s1->iOpenCount = s2->iOpenCount;
	}
	OpenCountT& operator=(const OpenCountT& s)//���������
	{
		set(this, (OpenCountT*)&s);
		return *this;
	}
	OpenCountT(const OpenCountT& s)//���ƹ��캯��
	{
		*this = s;
	}
};

//��Ϣ���е����ݽṹ
struct MsgQueueData
{
	char szData[SHARE_MEMORY_MSG_MAX_LENGTH];

	MsgQueueData()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(MsgQueueData));
	}

	void set(MsgQueueData* s1, MsgQueueData* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(MsgQueueData));
	}
	MsgQueueData& operator=(const MsgQueueData& s)//���������
	{
		set(this, (MsgQueueData*)&s);
		return *this;
	}
	MsgQueueData(const MsgQueueData& s)//���ƹ��캯��
	{
		*this = s;
	}
};

//ͨ����Ϣ
struct ServerMsg
{
	int iConnectId;  //����id
	char szUserId[20];  //�˺�
	int iFunctionId;  //���ܺ�
	int iRoletype;   //��ɫ����
	unsigned int iMsgIndex; //�������
	int iBodyLen; //�����峤��
	char*pBody;   //������ָ��
	bool   bIsLast;  //���һ����¼����Ӧ��Ϣʹ��
	int iErrorCode;     //������,��Ӧ��Ϣʹ��
	char szErrorInfo[100]; //������Ϣ,��Ӧ��Ϣʹ��

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

	void set(ServerMsg* s1, ServerMsg* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(ServerMsg));
	}
	ServerMsg& operator=(const ServerMsg& s)//���������
	{
		set(this, (ServerMsg*)&s);
		return *this;
	}
	ServerMsg(const ServerMsg& s)//���ƹ��캯��
	{
		*this = s;
	}

};


/************************************�����ļ��еĽṹ**************************************/
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

	CtpConfig()  //Ĭ�Ϲ��캯��
	{
		iTradePort = 0;
		iMarketPort = 0;
	}

	void set(CtpConfig* s1, CtpConfig* s2)//��ֵ����
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
	CtpConfig& operator=(const CtpConfig& s)//���������
	{
		set(this, (CtpConfig*)&s);
		return *this;
	}
	CtpConfig(const CtpConfig& s)//���ƹ��캯��
	{
		*this = s;
	}

};

struct User
{
	int iRoletype;
	string szUserId;
	string szPassword;

	User()  //Ĭ�Ϲ��캯��
	{
		iRoletype = 0;

	}
	void set(User* s1, User* s2)//��ֵ����
	{
		s1->iRoletype = s2->iRoletype;
		s1->szUserId = s2->szUserId;
		s1->szPassword = s2->szPassword;
	}
	User& operator=(const User& s)//���������
	{
		set(this, (User*)&s);
		return *this;
	}
	User(const User& s)//���ƹ��캯��
	{
		*this = s;
	}
};
struct TcpSvrInfo
{
	int iPort;

	TcpSvrInfo()  //Ĭ�Ϲ��캯��
	{
		iPort = 0;

	}
	void set(TcpSvrInfo* s1, TcpSvrInfo* s2)//��ֵ����
	{
		s1->iPort = s2->iPort;

	}
	TcpSvrInfo& operator=(const TcpSvrInfo& s)//���������
	{
		set(this, (TcpSvrInfo*)&s);
		return *this;
	}
	TcpSvrInfo(const TcpSvrInfo& s)//���ƹ��캯��
	{
		*this = s;
	}
};
struct LogInfo
{
	int iLoglevel;

	LogInfo()  //Ĭ�Ϲ��캯��
	{
		iLoglevel = 0;

	}
	void set(LogInfo* s1, LogInfo* s2)//��ֵ����
	{
		s1->iLoglevel = s2->iLoglevel;

	}
	LogInfo& operator=(const LogInfo& s)//���������
	{
		set(this, (LogInfo*)&s);
		return *this;
	}
	LogInfo(const LogInfo& s)//���ƹ��캯��
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

    Bar()//��ʼ������
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

    //��ֵ����
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

    //���ظ�ֵ�����
    Bar &operator=(const Bar &bar)
    {
        if (this == &bar)
        {
            return *this;
        }
        set(this, (Bar *)&bar);
        return *this;
    }

    //���ƹ��캯��
    Bar(const Bar &bar)
    {
        *this = bar;
    }

};

/*****************************************����ͨ�÷��ؽṹ***************************************/
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