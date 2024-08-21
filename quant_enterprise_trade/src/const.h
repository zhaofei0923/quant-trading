#ifndef _CONST_H
#define _CONST_H
/*
���ߣ���־��
΢�ţ�401985690
qqȺ�ţ�450286917
*/

//��������
#define PRICE_MULTIPLE 1000.00  //�۸���,���ڼ۸��int��double֮���ת��,�ͻ��˺ͷ����֮��ļ۸�ת��Ϊint���ͽ��д���
#define VIR_ACCOUNT_LENGTH 3 //����Ա�˻���λ��
#define PART_ORDER_REF_MAX 999 //OrderRef��3λ�����ֵ
#define VIR_ACCOUNT_INIT "000" //����Ա�˺ų�ʼֵ
#define SHARE_MEMORY_MSG_MAX_LENGTH  1024  //�����ڴ汨����󳤶�
#define NO_VALUE_START  100000 //δ��ֵ��������Сֵ

//��־
#define FLAG_ADD  1  //����
#define FLAG_DEL  0  //����

//���տ���
#define RISK_BALANCE_RATE  0.8  //���ڴ˱��ʵ��ʽ����ܿ���

//�г�����
#define  EXCHANGE_SHSE  "SHSE"  //�Ͻ������г����� SHSE
#define  EXCHANGE_SZSE  "SZSE"//������г����� SZSE
#define  EXCHANGE_CFFEX  "CFFEX"//�н������г����� CFFEX
#define  EXCHANGE_SHFE  "SHFE"//���������г����� SHFE
#define  EXCHANGE_DCE  "DCE"//���������г����� DCE
#define  EXCHANGE_CZCE  "CZCE"//֣�������г����� CZCE
#define  EXCHANGE_CMX  "CMX"//ŦԼ��Ʒ�������� �г����� CMX(GLN, SLN)
#define  EXCHANGE_IPE  "IPE"//�׶ع���ʯ�ͽ������� �г����� IPE(OIL, GAL)
#define  EXCHANGE_NYM  "NYM"//ŦԼ��ҵ�������� �г����� NYM(CON, HON)
#define  EXCHANGE_CBT  "CBT"//֥�Ӹ���Ʒ�ڻ����������г����� CBT(SOC, SBC, SMC, CRC)
#define  EXCHANGE_NYB  "NYB"//ŦԼ�ڻ����������г����� NYB(SGN)


//order action  ��������
#define ORDER_ACTION_BUY_OPEN    '0' //0-�������
#define ORDER_ACTION_SELL_CLOSE   '1'//1-����ƽ��
#define ORDER_ACTION_SELL_OPEN    '2'// 2-��������
#define ORDER_ACTION_BUY_CLOSE    '3'// 3-���ƽ��
#define ORDER_ACTION_BUY_UNKNOWN  'z' //δ֪

//order  status  ����״̬
#define ORDER_STATUS_NO_SEND   '0'//δ����
#define ORDER_STATUS_SEND   '1'//�ѷ���
#define ORDER_STATUS_SUBMIT   '2'//���ύ,��δ��
#define ORDER_STATUS_ACCEPT   '3'//�ѽ��ܣ����ѱ�
#define ORDER_STATUS_CANCEL_REFUSE  '4'//�������ܾ�
#define ORDER_STATUS_INSERT_REFUSE  '5'//�������ܾ�
#define ORDER_STATUS_PART_TRADE '6'//���ֳɽ�
#define ORDER_STATUS_ALL_TRADE  '7'// ȫ���ɽ�
#define ORDER_STATUS_CANCEL   '8'//�ѳ���
#define ORDER_STATUS_SEND_FAIL   '9'//����ʧ��
#define ORDER_STATUS_UNKNOWN  'z' //δ֪

//������������
#define TRADE_DIRECTION_BUY  '0'  ///��
#define TRADE_DIRECTION_SELL '1'  ///��

//Position cOperType ��������
#define POSITION_OPER_FROZEN  '1'//����
#define POSITION_OPER_PART_UN_FROZEN  '2'//���ֽⶳ
#define POSITION_OPER_REMAIN_UN_FROZEN  '3'//ʣ�ಿ�ֽⶳ

//Position  Direction �ֲַ���
#define POSITION_DIRECTION_NET '1'  //��
#define POSITION_DIRECTION_LONG '2' //��ͷ  ���򿪲֣���ƽ��
#define POSITION_DIRECTION_SHORT '3' //��ͷ  �������֣���ƽ��

///Ͷ���ױ���־
#define HF_Speculation '1'  ///Ͷ��
#define HF_Arbitrage '2'   ///����

//��Ϣ����
#define	MSG_TYPE_REQ  0 //����
#define	MSG_TYPE_RES  1  //��Ӧ

//���ܺ�
#define	FUNC_LOGIN 1001  //��¼
#define	FUNC_LOGOUT  1002  //�˳�
#define	FUNC_QUERY_BALANCE 1003  //��ѯ�ʽ�
#define	FUNC_QUERY_ORDER  1004   //��ѯί��
#define	FUNC_QUERY_TRADE 1005   //��ѯ�ɽ�
#define	FUNC_QUERY_POSITION 1006  //��ѯ�ֲ�
#define	FUNC_PLACE_ORDER 1007   //ί���µ�
#define	FUNC_CANCEL_ORDER 1008  //����
#define	FUNC_ORDER_NOTIFY 1009  //ί�лر�
#define	FUNC_TRADE_NOTIFY 1010  //�ɽ��ر�
#define	FUNC_PLACE_ORDER_ERR_NOTIFY 1011  //ί�д���֪ͨ
#define	FUNC_CANCEL_ORDER_ERR_NOTIFY 1012  //��������֪ͨ
#define	FUNC_ADD_MSG_QUEUE 1013  //�������ڷ���ί�кͳ�������Ϣ����
#define	FUNC_DEL_MSG_QUEUE 1014  //ɾ�����ڷ���ί�кͳ�������Ϣ����
#define	FUNC_SUB_QUOTE 1015  //��������
#define	FUNC_CANCEL_SUB_QUOTE 1016  //ȡ����������
#define	FUNC_MARKET_QUOTE 1017  //�������
#define FUNC_ADD_DEL_MONEY  1018   //Ϊ�����˺������ʽ�

//��ɫ����
#define  ROLE_TYPE_TRADER 0  //����Ա,����ʹ�õ��˻�
#define  ROLE_TYPE_ADMIN  1  //����Ա

//����룬������
#define RET_OK   0  //�ɹ�
#define RET_FAIL   -1  //ʧ��
#define RET_ACCOUNT_NOT_EXIST   -2001  //�˺Ų�����
#define RET_ACCOUNT_PASSWORD_ERROR   -2002  //�������
#define RET_JYS_ERROR   -2003  //����������
#define RET_ORDER_REF_EXIST   -2004  //OrderRef�Ѿ�����
#define RET_ORDER_REF_NOT_EXIST   -2005  //OrderRef������
#define RET_SEND_JYS_FAIL   -2006  //���͵�������ʧ��
#define RET_CLIENT_ORDER_ID_EXIST  -2007 // ClientOrderId�Ѿ�����
#define RET_CLIENT_ORDER_ID_NOT_EXIST  -2008 // ClientOrderId������
#define RET_CLIENT_MSG_ID_EXIST  -2009// �ͻ���ReqId�Ѿ�����
#define RET_RECORD_NOT_EXIST  -2010  //��¼������
#define RET_PASSWORD_ERROR  -2011 //�������
#define RET_ROLE_ERROR   -2012  //��ɫ���ʹ���
#define RET_CREATE_MSG_QUEUE_FAIL   -2013  //������Ϣ����ʧ��
#define RET_MSG_QUEUE_EXIST   -2014  //��Ϣ�����Ѿ�����
#define RET_MSG_QUEUE_NOT_EXIST   -2015  //��Ϣ���в�����
#define RET_NO_RIGHT   -2016  //û��Ȩ��
#define RET_NO_RCV_FRONTID_SESSIONID   -2017  //û���յ�FrontID��SessionID
#define RET_UP_HIGH_PRICE   -2018  //������ͣ��
#define RET_DOWN_LOW_PRICE   -2019  //���ڵ�ͣ��
#define RET_NO_POSITION   -2020  //û�гֲ�
#define RET_NO_BALANCE   -2021  //û���ʽ�
#define RET_NO_STOCK_CODE   -2022  //û�й�Ʊ����
#define RET_ENTRUST_NUM_ERROR   -2023  //ί����������
#define RET_ONESELF_TRADE_ERROR   -2024  //�Գɽ�����

#endif // !_CONST_H
