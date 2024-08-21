#ifndef _MESSAGE_H
#define _MESSAGE_H
#include <string.h>
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

/***************************�ͻ��˺ͷ����ͨѶ��ͨ�ñ���ͷ*********************/
struct Head
{
	int iFunctionId;  //���ܺ�
	int iMsgtype;  //��Ϣ����
	int iReqId; //����id��Ψһ����Ҫ�ظ�
	int iRoletype;   //��ɫ����
	int iBodyLength;    //�����峤��
	bool bIsLast;  //�Ƿ����һ������
	int iErrorCode;     //������,��Ӧ��Ϣʹ��
	char szErrorInfo[100];   //������Ϣ,��Ӧ��Ϣʹ��

	Head()
	{
		memset(this, 0, sizeof(Head));
	}

	void set(Head* s1, Head* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(Head));
	}
	Head& operator=(const Head& s)//���������
	{
		set(this, (Head*)&s);
		return *this;
	}
	Head(const Head& s)//���ƹ��캯��
	{
		*this = s;
	}
};

/********************************�ͻ��˺ͷ����ͨѶ�ı�����**************************************/
//��¼
struct LoginReqT
{
	char szUserId[20];
	char szPassword[20];

	LoginReqT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(LoginReqT));
	}
	void set(LoginReqT* s1, LoginReqT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(LoginReqT));
	}
	LoginReqT& operator=(const LoginReqT& s)//���������
	{
		set(this, (LoginReqT*)&s);
		return *this;
	}
	LoginReqT(const LoginReqT& s)//���ƹ��캯��
	{
		*this = s;
	}
};
struct LoginResT
{};//�ޱ�����

   //�ǳ�
struct LogoutReqT
{};//�ޱ�����
struct LogoutResT
{};//�ޱ�����

   //��ѯ�ʽ�
struct QueryBalanceReqT
{};
struct QueryBalanceResT
{
	unsigned long long 	iUseMargin;///ռ�õı�֤��
	unsigned long long	iFrozenMargin; ///����ı�֤��
	unsigned long long	iFrozenCash; ///������ʽ�
	unsigned long long	iFrozenCommission; ///�����������
	unsigned long long	iCurrMargin; ///��ǰ��֤���ܶ�
	unsigned long long	iCommission; ///������
	unsigned long long	iAvailable; ///�����ʽ�

	unsigned long long  iStartMoney; ///�ڳ����ʽ�
	unsigned long long  iAddMoney; ///�������ӵ��ʽ�

	QueryBalanceResT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(QueryBalanceResT));
	}

	void set(QueryBalanceResT* s1, QueryBalanceResT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(QueryBalanceResT));
	}
	QueryBalanceResT& operator=(const QueryBalanceResT& s)//���������
	{
		set(this, (QueryBalanceResT*)&s);
		return *this;
	}
	QueryBalanceResT(const QueryBalanceResT& s)//���ƹ��캯��
	{
		*this = s;
	}
};

//��ѯί�е�
struct QueryOrderReqT
{
	int iFlag;//0-�鵱��ȫ����1-����szClientOrderId��ѯ
	char szClientOrderId[30]; //�ͻ���ί��id��һ���ڲ�Ҫ�ظ�����������ί�е�

	QueryOrderReqT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(QueryOrderReqT));
	}
	void set(QueryOrderReqT* s1, QueryOrderReqT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(QueryOrderReqT));
	}
	QueryOrderReqT& operator=(const QueryOrderReqT& s)//���������
	{
		set(this, (QueryOrderReqT*)&s);
		return *this;
	}
	QueryOrderReqT(const QueryOrderReqT& s)//���ƹ��캯��
	{
		*this = s;
	}
};
struct OrderDetail
{
	int iInitDate; //�����½���������
	int iInitTime; //�����½�����ʱ�� HHMMSSsss
	char szJysInsertDate[40]; //��������������
	char szJysInsertTime[40]; //������ί��ʱ��
	char szUserId[10]; //�����˺�
	char szExchangeID[10]; //�г�
	char szStockCode[40];  //����
	char szOrderRef[40];  //��������
	char szOrderSysID[40]; // �������,�൱��ί�б��
	char cAction;  //��������
	unsigned long long    iPrice;  //�۸�,��λС��
	int iEntrustNum; //ί������
	int iTradeNum; //�ɽ�����
	char   cStatus;//����״̬

	OrderDetail()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(OrderDetail));
	}
	void set(OrderDetail* s1, OrderDetail* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(OrderDetail));
	}
	OrderDetail& operator=(const OrderDetail& s)//���������
	{
		set(this, (OrderDetail*)&s);
		return *this;
	}
	OrderDetail(const OrderDetail& s)//���ƹ��캯��
	{
		*this = s;
	}
};
struct QueryOrderResT
{
	OrderDetail OrderData;

	QueryOrderResT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(QueryOrderResT));
	}
	void set(QueryOrderResT* s1, QueryOrderResT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(QueryOrderResT));
	}
	QueryOrderResT& operator=(const QueryOrderResT& s)//���������
	{
		set(this, (QueryOrderResT*)&s);
		return *this;
	}
	QueryOrderResT(const QueryOrderResT& s)//���ƹ��캯��
	{
		*this = s;
	}
};

//��ѯ����
struct QueryTradeReqT
{
	int iFlag;//0-�鵱��ȫ����1-����szClientOrderId��ѯ
	char szClientOrderId[30]; //�ͻ���ί��id��һ���ڲ�Ҫ�ظ�����������ί�е�

	QueryTradeReqT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(QueryTradeReqT));
	}
	void set(QueryTradeReqT* s1, QueryTradeReqT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(QueryTradeReqT));
	}
	QueryTradeReqT& operator=(const QueryTradeReqT& s)//���������
	{
		set(this, (QueryTradeReqT*)&s);
		return *this;
	}
	QueryTradeReqT(const QueryTradeReqT& s)//���ƹ��캯��
	{
		*this = s;
	}
};
struct TradeDetail
{
	int iInitDate; //�������� yyyymmdd
	int iInitTime; //����ʱ�� HHMMSSsss
	char szJysTradeDate[40]; //�������ɽ�����
	char szJysTradeTime[40]; //�������ɽ�ʱ��
	char szUserId[10]; //�����˺�
	char szExchangeID[10]; //�г�
	char szStockCode[40];  //����
	char szOrderRef[40];  //��������
	char szOrderSysID[40]; // �������,�൱��ί�б��
	char szTradeID[40]; //�ɽ����
	char cAction;  //��������
	unsigned long long    iPrice;  //�۸�,��λС��
	int iTradeNum; //�ɽ�����

	TradeDetail()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(TradeDetail));
	}
	void set(TradeDetail* s1, TradeDetail* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(TradeDetail));
	}
	TradeDetail& operator=(const TradeDetail& s)//���������
	{
		set(this, (TradeDetail*)&s);
		return *this;
	}
	TradeDetail(const TradeDetail& s)//���ƹ��캯��
	{
		*this = s;
	}
};
struct QueryTradeResT
{
	TradeDetail TradeData;

	QueryTradeResT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(QueryTradeResT));
	}
	void set(QueryTradeResT* s1, QueryTradeResT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(QueryTradeResT));
	}
	QueryTradeResT& operator=(const QueryTradeResT& s)//���������
	{
		set(this, (QueryTradeResT*)&s);
		return *this;
	}
	QueryTradeResT(const QueryTradeResT& s)//���ƹ��캯��
	{
		*this = s;
	}
};

//��ѯ�ֲ�
struct QueryPositionReqT
{};//�ޱ�����
struct PositionDetail
{
	int iInitDate; //�������� yyyymmdd
	char szUserId[10]; //�����˻�
	char szExchangeID[10]; //�г�
	char szStockCode[40];  //����
	char cDirection; //�ֲַ���
	char  cHedgeFlag;///Ͷ���ױ���־
	int	iYdPosition;///���ճֲ�
	int	iTodayPosition; ///���ճֲ�
	int	iPosition; ///�ֲܳ�
	unsigned long long 	iLongFrozen; ///��ͷ����
	unsigned long long 	iShortFrozen; ///��ͷ����
	unsigned long long 	iUseMargin; ///ռ�õı�֤��,��λС��
	unsigned long long 	iFrozenMargin; ///����ı�֤��,��λС��
	unsigned long long 	iFrozenCash; ///������ʽ�,��λС��
	unsigned long long 	iFrozenCommission;///�����������,��λС��
	unsigned long long 	iCommission; ///������,��λС��
	unsigned long long 	iPreSettlementPrice; ///��ֽ����,��λС��
	unsigned long long 	iSettlementPrice; ///���ν����,��λС��

	PositionDetail()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(PositionDetail));
	}
	void set(PositionDetail* s1, PositionDetail* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(PositionDetail));
	}
	PositionDetail& operator=(const PositionDetail& s)//���������
	{
		set(this, (PositionDetail*)&s);
		return *this;
	}
	PositionDetail(const PositionDetail& s)//���ƹ��캯��
	{
		*this = s;
	}
};
struct QueryPositionResT
{
	PositionDetail PositionData;

	QueryPositionResT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(QueryPositionResT));
	}
	void set(QueryPositionResT* s1, QueryPositionResT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(QueryPositionResT));
	}
	QueryPositionResT& operator=(const QueryPositionResT& s)//���������
	{
		set(this, (QueryPositionResT*)&s);
		return *this;
	}
	QueryPositionResT(const QueryPositionResT& s)//���ƹ��캯��
	{
		*this = s;
	}
};

//ί��
struct PlaceOrderReqT
{
	char szExchangeID[10]; //�г�
	char szStockCode[40];
	char cAction;
	unsigned long long  iPrice; //С������λ
	int iEntrustNum;
	char szClientOrderId[30]; //�ͻ���ί��id��һ���ڲ�Ҫ�ظ�����������ί�е�

	PlaceOrderReqT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(PlaceOrderReqT));
	}
	void set(PlaceOrderReqT* s1, PlaceOrderReqT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(PlaceOrderReqT));
	}
	PlaceOrderReqT& operator=(const PlaceOrderReqT& s)//���������
	{
		set(this, (PlaceOrderReqT*)&s);
		return *this;
	}
	PlaceOrderReqT(const PlaceOrderReqT& s)//���ƹ��캯��
	{
		*this = s;
	}
};
struct PlaceOrderResT
{
	char szOrderRef[40];  //��������

	PlaceOrderResT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(PlaceOrderResT));
	}
	void set(PlaceOrderResT* s1, PlaceOrderResT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(PlaceOrderResT));
	}
	PlaceOrderResT& operator=(const PlaceOrderResT& s)//���������
	{
		set(this, (PlaceOrderResT*)&s);
		return *this;
	}
	PlaceOrderResT(const PlaceOrderResT& s)//���ƹ��캯��
	{
		*this = s;
	}
};

//����
struct CancelOrderReqT
{
	char szClientOrderId[30]; //�ͻ���ί��id��һ���ڲ�Ҫ�ظ�����������ί�е�

	CancelOrderReqT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(CancelOrderReqT));
	}
	void set(CancelOrderReqT* s1, CancelOrderReqT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(CancelOrderReqT));
	}
	CancelOrderReqT& operator=(const CancelOrderReqT& s)//���������
	{
		set(this, (CancelOrderReqT*)&s);
		return *this;
	}
	CancelOrderReqT(const CancelOrderReqT& s)//���ƹ��캯��
	{
		*this = s;
	}
};
struct CancelOrderResT
{};//�ޱ�����

 //ί�лر�
struct OrderNotifyT
{
	OrderDetail OrderData;

	OrderNotifyT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(OrderNotifyT));
	}
	void set(OrderNotifyT* s1, OrderNotifyT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(OrderNotifyT));
	}
	OrderNotifyT& operator=(const OrderNotifyT& s)//���������
	{
		set(this, (OrderNotifyT*)&s);
		return *this;
	}
	OrderNotifyT(const OrderNotifyT& s)//���ƹ��캯��
	{
		*this = s;
	}
};
//�ɽ��ر�
struct TradeNotifyT
{
	TradeDetail TradeData;

	TradeNotifyT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(TradeNotifyT));
	}
	void set(TradeNotifyT* s1, TradeNotifyT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(TradeNotifyT));
	}
	TradeNotifyT& operator=(const TradeNotifyT& s)//���������
	{
		set(this, (TradeNotifyT*)&s);
		return *this;
	}
	TradeNotifyT(const TradeNotifyT& s)//���ƹ��캯��
	{
		*this = s;
	}
};
//ί�д���֪ͨ
struct PlaceOrderErrNotifyT
{
	char szUserId[10]; //�����˺�
	char szClientOrderId[30]; //�ͻ���ί��id��һ���ڲ�Ҫ�ظ�����������ί�е�
	char szOrderRef[40];  //��������
	int iErrorCode;     //������,��Ӧ��Ϣʹ��
	char szErrorInfo[100];   //������Ϣ,��Ӧ��Ϣʹ��

	PlaceOrderErrNotifyT()
	{
		memset(this, 0, sizeof(PlaceOrderErrNotifyT));
	}
	void set(PlaceOrderErrNotifyT* s1, PlaceOrderErrNotifyT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(PlaceOrderErrNotifyT));
	}
	PlaceOrderErrNotifyT& operator=(const PlaceOrderErrNotifyT& s)//���������
	{
		set(this, (PlaceOrderErrNotifyT*)&s);
		return *this;
	}
	PlaceOrderErrNotifyT(const PlaceOrderErrNotifyT& s)//���ƹ��캯��
	{
		*this = s;
	}
};
//��������֪ͨ
struct CancelOrderErrNotifyT
{
	char szUserId[10]; //�����˺�
	char szClientOrderId[30]; //�ͻ���ί��id��һ���ڲ�Ҫ�ظ�����������ί�е�
	char szOrderRef[40];  //��������
	int iErrorCode;     //������,��Ӧ��Ϣʹ��
	char szErrorInfo[100];   //������Ϣ,��Ӧ��Ϣʹ��

	CancelOrderErrNotifyT()
	{
		memset(this, 0, sizeof(CancelOrderErrNotifyT));
	}
	void set(CancelOrderErrNotifyT* s1, CancelOrderErrNotifyT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(CancelOrderErrNotifyT));
	}
	CancelOrderErrNotifyT& operator=(const CancelOrderErrNotifyT& s)//���������
	{
		set(this, (CancelOrderErrNotifyT*)&s);
		return *this;
	}
	CancelOrderErrNotifyT(const CancelOrderErrNotifyT& s)//���ƹ��캯��
	{
		*this = s;
	}
};

//����ͨѶ����Ϣ����
struct AddMsgQueueReqT
{};//�ޱ�����
struct AddMsgQueueResT
{
	int iReadKey; //����˶���key�����ͻ���д��key
	int iWriteKey; //�����д��key�����ͻ��˶���key

	AddMsgQueueResT()
	{
		memset(this, 0, sizeof(AddMsgQueueResT));
	}
	void set(AddMsgQueueResT* s1, AddMsgQueueResT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(AddMsgQueueResT));
	}
	AddMsgQueueResT& operator=(const AddMsgQueueResT& s)//���������
	{
		set(this, (AddMsgQueueResT*)&s);
		return *this;
	}
	AddMsgQueueResT(const AddMsgQueueResT& s)//���ƹ��캯��
	{
		*this = s;
	}
};

//ɾ��ͨѶ����Ϣ����
struct DelMsgQueueReqT
{};//�ޱ�����
struct DelMsgQueueResT
{};//�ޱ�����

 //��������
struct SubQuoteReqT
{
	int iNum;
	//iNum��SubItem

};
struct SubItem
{
	char szStockCode[40];

	SubItem()
	{
		memset(this, 0, sizeof(SubItem));
	}
	void set(SubItem* s1, SubItem* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(SubItem));
	}
	SubItem& operator=(const SubItem& s)//���������
	{
		set(this, (SubItem*)&s);
		return *this;
	}
	SubItem(const SubItem& s)//���ƹ��캯��
	{
		*this = s;
	}
};
struct SubQuoteResT
{
	int iClientNum; //�ͻ��˱��
	int iDataKey; //��������key

	SubQuoteResT()
	{
		memset(this, 0, sizeof(SubQuoteResT));
	}
	void set(SubQuoteResT* s1, SubQuoteResT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(SubQuoteResT));
	}
	SubQuoteResT& operator=(const SubQuoteResT& s)//���������
	{
		set(this, (SubQuoteResT*)&s);
		return *this;
	}
	SubQuoteResT(const SubQuoteResT& s)//���ƹ��캯��
	{
		*this = s;
	}
};

//ȡ����������
struct CancelSubQuoteReqT
{
	int iClientNum; //�ͻ��˱��
};
struct CancelSubQuoteResT
{};//�ޱ�����

//�������
//�������ݣ��۸�Ϊ3λС����
struct MarketData
{
	
	char szTradingDay[10]; //������
	char szActionDay[10]; //ҵ������
	char szExchangeId[10]; //����������
	char szStockCode[50]; //��Ʊ���룬��Լ����
	char szStockName[100]; //��Ʊ���ƣ���Լ����
	char szUpdateTime[20];

	uint64_t iLastPrice; //���¼�
	uint64_t iPreSettlePrice; //�ϴν����
	uint64_t iSettlePrice; //���ν����
	uint64_t iPreClosePrice; //�����̼�
	uint64_t iOpenPrice;//���̼�
	uint64_t iLowPrice; //��ͼ�
	uint64_t iHighPrice; //��߼�
	uint64_t iClosePrice; //�����̼�
	uint64_t iUpperLimitPrice; ///��ͣ���
	uint64_t iLowerLimitPrice; ///��ͣ���
	uint64_t iVolume;//����
	uint64_t iPreOpenInterest; //��ֲ���
	uint64_t iTurnOver; //�ɽ����
	uint64_t iOpenInterest; //�ֲ���
	uint64_t iPreDelta; //����ʵ��
	uint64_t iCurrDelta; //����ʵ��
	uint64_t iUpdateMillisec; //����޸ĺ���
	uint64_t iAveragePrice; //���վ���

	uint64_t  iSellPrice1; ///��1���۸�
	uint64_t  iSellPrice2; ///��2���۸�
	uint64_t  iSellPrice3;
	uint64_t  iSellPrice4;
	uint64_t  iSellPrice5;

	uint64_t  iBuyPrice1; ///��1���۸�
	uint64_t  iBuyPrice2;///��2���۸�
	uint64_t  iBuyPrice3;
	uint64_t  iBuyPrice4;
	uint64_t  iBuyPrice5;

	uint64_t iSellQuantity1;///��1������
	uint64_t iSellQuantity2;///��2������
	uint64_t iSellQuantity3;
	uint64_t iSellQuantity4;
	uint64_t iSellQuantity5;

	uint64_t iBuyQuantity1;///��1������
	uint64_t iBuyQuantity2;///��2������
	uint64_t iBuyQuantity3;
	uint64_t iBuyQuantity4;
	uint64_t iBuyQuantity5;

	MarketData()
	{
		memset(this, 0, sizeof(MarketData));
	}
	void set(MarketData* s1, MarketData* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(MarketData));
	}
	MarketData& operator=(const MarketData& s)//���������
	{
		set(this, (MarketData*)&s);
		return *this;
	}
	MarketData(const MarketData& s)//���ƹ��캯��
	{
		*this = s;
	}

};

//�����ʽ�
struct AddDelMoneyReqT
{
	char szUserId[20]; //�����˺�
	int  iFlag;  //������־��0-�����ʽ�1-�����ʽ�
	unsigned long long  iAmount; //�ʽ�Ľ��,С������λ

	AddDelMoneyReqT()  //Ĭ�Ϲ��캯��
	{
		memset(this, 0, sizeof(AddDelMoneyReqT));
	}
	void set(AddDelMoneyReqT* s1, AddDelMoneyReqT* s2)//��ֵ����
	{
		memcpy(s1, s2, sizeof(AddDelMoneyReqT));
	}
	AddDelMoneyReqT& operator=(const AddDelMoneyReqT& s)//���������
	{
		set(this, (AddDelMoneyReqT*)&s);
		return *this;
	}
	AddDelMoneyReqT(const AddDelMoneyReqT& s)//���ƹ��캯��
	{
		*this = s;
	}

};
struct AddDelMoneyResT
{};//�ޱ�����


#endif