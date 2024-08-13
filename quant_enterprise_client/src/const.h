#ifndef _CONST_H
#define _CONST_H
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

//参数配置
#define PRICE_MULTIPLE 1000.00  //价格倍数,用于价格的int和double之间的转换,客户端和服务端之间的价格转换为int类型进行传输
#define VIR_ACCOUNT_LENGTH 3 //交易员账户的位数
#define PART_ORDER_REF_MAX 999 //OrderRef后3位的最大值
#define VIR_ACCOUNT_INIT "000" //交易员账号初始值
#define SHARE_MEMORY_MSG_MAX_LENGTH  1024  //共享内存报文最大长度
#define NO_VALUE_START  100000 //未赋值变量的最小值

//标志
#define FLAG_ADD  1  //增加
#define FLAG_DEL  0  //减少

//风险控制
#define RISK_BALANCE_RATE  0.8  //低于此比率的资金则不能开仓

//市场代码
#define  EXCHANGE_SHSE  "SHSE"  //上交所，市场代码 SHSE
#define  EXCHANGE_SZSE  "SZSE"//深交所，市场代码 SZSE
#define  EXCHANGE_CFFEX  "CFFEX"//中金所，市场代码 CFFEX
#define  EXCHANGE_SHFE  "SHFE"//上期所，市场代码 SHFE
#define  EXCHANGE_DCE  "DCE"//大商所，市场代码 DCE
#define  EXCHANGE_CZCE  "CZCE"//郑商所，市场代码 CZCE
#define  EXCHANGE_CMX  "CMX"//纽约商品交易所， 市场代码 CMX(GLN, SLN)
#define  EXCHANGE_IPE  "IPE"//伦敦国际石油交易所， 市场代码 IPE(OIL, GAL)
#define  EXCHANGE_NYM  "NYM"//纽约商业交易所， 市场代码 NYM(CON, HON)
#define  EXCHANGE_CBT  "CBT"//芝加哥商品期货交易所，市场代码 CBT(SOC, SBC, SMC, CRC)
#define  EXCHANGE_NYB  "NYB"//纽约期货交易所，市场代码 NYB(SGN)


//order action  买卖方向
#define ORDER_ACTION_BUY_OPEN    '0' //0-买进开仓
#define ORDER_ACTION_SELL_CLOSE   '1'//1-卖出平仓
#define ORDER_ACTION_SELL_OPEN    '2'// 2-卖出开仓
#define ORDER_ACTION_BUY_CLOSE    '3'// 3-买进平仓
#define ORDER_ACTION_BUY_UNKNOWN  'z' //未知

//order  status  报单状态
#define ORDER_STATUS_NO_SEND   '0'//未发送
#define ORDER_STATUS_SEND   '1'//已发送
#define ORDER_STATUS_SUBMIT   '2'//已提交,即未报
#define ORDER_STATUS_ACCEPT   '3'//已接受，即已报
#define ORDER_STATUS_CANCEL_REFUSE  '4'//撤单被拒绝
#define ORDER_STATUS_INSERT_REFUSE  '5'//报单被拒绝
#define ORDER_STATUS_PART_TRADE '6'//部分成交
#define ORDER_STATUS_ALL_TRADE  '7'// 全部成交
#define ORDER_STATUS_CANCEL   '8'//已撤单
#define ORDER_STATUS_SEND_FAIL   '9'//发送失败
#define ORDER_STATUS_UNKNOWN  'z' //未知

//交易买卖方向
#define TRADE_DIRECTION_BUY  '0'  ///买
#define TRADE_DIRECTION_SELL '1'  ///卖

//Position cOperType 操作类型
#define POSITION_OPER_FROZEN  '1'//冻结
#define POSITION_OPER_PART_UN_FROZEN  '2'//部分解冻
#define POSITION_OPER_REMAIN_UN_FROZEN  '3'//剩余部分解冻

//Position  Direction 持仓方向
#define POSITION_DIRECTION_NET '1'  //净
#define POSITION_DIRECTION_LONG '2' //多头  即买开仓，卖平仓
#define POSITION_DIRECTION_SHORT '3' //空头  即卖开仓，买平仓

///投机套保标志
#define HF_Speculation '1'  ///投机
#define HF_Arbitrage '2'   ///套利

//消息类型
#define	MSG_TYPE_REQ  0 //请求
#define	MSG_TYPE_RES  1  //响应

//功能号
#define	FUNC_LOGIN 1001  //登录
#define	FUNC_LOGOUT  1002  //退出
#define	FUNC_QUERY_BALANCE 1003  //查询资金
#define	FUNC_QUERY_ORDER  1004   //查询委托
#define	FUNC_QUERY_TRADE 1005   //查询成交
#define	FUNC_QUERY_POSITION 1006  //查询持仓
#define	FUNC_PLACE_ORDER 1007   //委托下单
#define	FUNC_CANCEL_ORDER 1008  //撤单
#define	FUNC_ORDER_NOTIFY 1009  //委托回报
#define	FUNC_TRADE_NOTIFY 1010  //成交回报
#define	FUNC_PLACE_ORDER_ERR_NOTIFY 1011  //委托错误通知
#define	FUNC_CANCEL_ORDER_ERR_NOTIFY 1012  //撤单错误通知
#define	FUNC_ADD_MSG_QUEUE 1013  //增加用于发送委托和撤单的消息队列
#define	FUNC_DEL_MSG_QUEUE 1014  //删除用于发送委托和撤单的消息队列
#define	FUNC_SUB_QUOTE 1015  //订阅行情
#define	FUNC_CANCEL_SUB_QUOTE 1016  //取消订阅行情
#define	FUNC_MARKET_QUOTE 1017  //行情快照
#define FUNC_ADD_DEL_MONEY  1018   //为交易账号增减资金

//角色类型
#define  ROLE_TYPE_TRADER 0  //交易员,策略使用的账户
#define  ROLE_TYPE_ADMIN  1  //管理员

//结果码，错误码
#define RET_OK   0  //成功
#define RET_FAIL   -1  //失败
#define RET_ACCOUNT_NOT_EXIST   -2001  //账号不存在
#define RET_ACCOUNT_PASSWORD_ERROR   -2002  //密码错误
#define RET_JYS_ERROR   -2003  //交易所错误
#define RET_ORDER_REF_EXIST   -2004  //OrderRef已经存在
#define RET_ORDER_REF_NOT_EXIST   -2005  //OrderRef不存在
#define RET_SEND_JYS_FAIL   -2006  //发送到交易所失败
#define RET_CLIENT_ORDER_ID_EXIST  -2007 // ClientOrderId已经存在
#define RET_CLIENT_ORDER_ID_NOT_EXIST  -2008 // ClientOrderId不存在
#define RET_CLIENT_MSG_ID_EXIST  -2009// 客户端ReqId已经存在
#define RET_RECORD_NOT_EXIST  -2010  //记录不存在
#define RET_PASSWORD_ERROR  -2011 //密码错误
#define RET_ROLE_ERROR   -2012  //角色类型错误
#define RET_CREATE_MSG_QUEUE_FAIL   -2013  //创建消息队列失败
#define RET_MSG_QUEUE_EXIST   -2014  //消息队列已经存在
#define RET_MSG_QUEUE_NOT_EXIST   -2015  //消息队列不存在
#define RET_NO_RIGHT   -2016  //没有权限
#define RET_NO_RCV_FRONTID_SESSIONID   -2017  //没有收到FrontID，SessionID
#define RET_UP_HIGH_PRICE   -2018  //高于涨停价
#define RET_DOWN_LOW_PRICE   -2019  //低于跌停价
#define RET_NO_POSITION   -2020  //没有持仓
#define RET_NO_BALANCE   -2021  //没有资金
#define RET_NO_STOCK_CODE   -2022  //没有股票代码
#define RET_ENTRUST_NUM_ERROR   -2023  //委托数量错误
#define RET_ONESELF_TRADE_ERROR   -2024  //自成交错误

#endif // !_CONST_H
