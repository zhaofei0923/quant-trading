/*
作者：赵志根
微信：401985690
qq群号：450286917
*/
-- ----------------------------
-- Table structure for td_balance
-- 资金表
-- ----------------------------
DROP TABLE IF EXISTS td_balance;
CREATE TABLE td_balance (
  user_id text,
  td_type int(11) DEFAULT NULL /* '交易类型：0-ctp'*/,
  use_margin double DEFAULT NULL /* '占用的保证金'*/,
  frozen_margin double DEFAULT NULL /* '冻结的保证金'*/,
  frozen_cash double DEFAULT NULL /* '冻结的资金'*/,
  frozen_commission double DEFAULT NULL /* '冻结的手续费'*/,
  curr_margin double DEFAULT NULL /* '当前保证金总额'*/,
  commission double DEFAULT NULL /* '手续费'*/,
  available double DEFAULT NULL /* '可用资金'*/,
  start_money double DEFAULT NULL /* '期初的资金'*/,
  add_money double DEFAULT NULL /* '调仓增加的资金'*/,
  settle_date int(11) DEFAULT NULL /* '结算日期'*/,
  modify_date int(11) DEFAULT NULL /* '修改日期'*/,
  modify_time int(11) DEFAULT NULL /* '修改时间'*/
) ;

-- ----------------------------
-- Records of td_balance
-- ----------------------------

-- ----------------------------
-- Table structure for td_instrument
-- 期货合约表
-- ----------------------------
DROP TABLE IF EXISTS td_instrument;
CREATE TABLE td_instrument (
  instrument_id text /* '合约代码'*/,
  exchange_id text /* '交易所代码'*/,
  instrument_name text /* '合约名称'*/,
  product_id text /* '产品代码'*/,
  product_class text /* '产品类型'*/,
  delivery_year int /* '交割年份'*/,
  delivery_month int /* '交割月'*/,
  max_market_order_volume int(11) DEFAULT NULL /* '市价单最大下单量'*/,
  min_market_order_volume int(11) DEFAULT NULL /* '市价单最小下单量'*/,
  max_limit_order_volume int(11) DEFAULT NULL /* '限价单最大下单量'*/,
  min_limit_order_volume int(11) DEFAULT NULL /* '限价单最小下单量'*/,
  volume_multiple int(11) DEFAULT NULL /* '合约数量乘数'*/,
  price_tick double DEFAULT NULL /* '最小变动价位'*/,
  expire_date text /* '到期日'*/,
  is_trading int(11) DEFAULT NULL /* '当前是否交易'*/,
  position_type text /* '持仓类型 1-净持仓 2-综合持仓'*/,
  position_date_type text /* '持仓日期类型 1-使用历史持仓 2-不使用历史持仓'*/, 
  long_margin_ratio double DEFAULT NULL /* '多头保证金率'*/,
  short_margin_ratio double DEFAULT NULL /* '空头保证金率'*/,
  max_margin_side_algorithm text /* '是否使用大额单边保证金算法'*/,
  strike_price double DEFAULT NULL /* '执行价'*/,
  options_type text /* '期权类型'*/,
  underlying_multiple double DEFAULT NULL /* '合约基础商品乘数'*/
) ;

-- ----------------------------
-- Records of td_instrument
-- ----------------------------

-- ----------------------------
-- Table structure for td_order
-- 委托表
-- ----------------------------
DROP TABLE IF EXISTS td_order;
CREATE TABLE td_order (
  init_date int(11) DEFAULT NULL /* '本地新建订单日期'*/,
  init_time int(11) DEFAULT NULL /* '本地新建订单时间'*/,
  modify_date int(11) DEFAULT NULL /* '修改日期'*/,
  modify_time int(11) DEFAULT NULL /* '修改时间'*/,
  td_type int(11) DEFAULT NULL /* '交易类型：0-ctp'*/,
  jys_insert_date text /* '交易所报单日期'*/,
  jys_insert_time text /* '交易所委托时间'*/,
  user_id text,
  exchange_id text /* '市场'*/,
  stock_code text /* '代码'*/,
  client_order_id text /* '客户端委托id，唯一，不要重复	'*/,
  order_ref text /* '报单引用'*/,
  order_sys_id text /* '报单编号,相当于委托编号'*/,
  front_id int(11) DEFAULT NULL /* '前置编号'*/,
  session_id int(11) DEFAULT NULL /* '会话编号'*/,
  action text /* '买卖操作'*/,
  price double DEFAULT NULL /* '价格'*/,
  entrust_num int(11) DEFAULT NULL /* '委托数量'*/,
  trade_num int(11) DEFAULT NULL /* '成交数量'*/,
  status text /* '报单状态'*/
) ;

-- ----------------------------
-- Records of td_order
-- ----------------------------

-- ----------------------------
-- Table structure for td_position
-- 持仓表
-- ----------------------------
DROP TABLE IF EXISTS td_position;
CREATE TABLE td_position (
  init_date int(11) DEFAULT NULL /* '本地新建订单日期'*/,
  init_time int(11) DEFAULT NULL /* '本地新建订单时间'*/,
  modify_date int(11) DEFAULT NULL /* '修改日期'*/,
  modify_time int(11) DEFAULT NULL /* '修改时间'*/,
  td_type int(11) DEFAULT NULL /* '交易类型：0-ctp'*/,
  user_id text,
  exchange_id text /* '市场'*/,
  stock_code text /* '代码'*/,
  direction text /* '持仓方向'*/,
  hedge_flag text /* '投机套保标志'*/,
  yd_position int(11) DEFAULT NULL /* '昨日持仓'*/,
  today_position int(11) DEFAULT NULL /* '今日持仓'*/,
  position int(11) DEFAULT NULL /* '总持仓'*/,
  long_frozen int(11) DEFAULT NULL /* '多头冻结'*/,
  short_frozen int(11) DEFAULT NULL /* '空头冻结'*/, 
  long_frozen_amount double DEFAULT NULL /* '多头冻结金额'*/,
  short_frozen_amount double DEFAULT NULL /* '空头冻结金额'*/,
  use_margin double DEFAULT NULL /* '占用的保证金'*/,
  frozen_margin double DEFAULT NULL /* '冻结的保证金'*/,
  frozen_cash double DEFAULT NULL /* '冻结的资金'*/,
  frozen_commission double DEFAULT NULL /* '冻结的手续费'*/,
  commission double DEFAULT NULL /* '手续费'*/,
  pre_settlement_price double DEFAULT NULL /* '上次结算价'*/,
  settlement_price double DEFAULT NULL /* '本次结算价'*/,
  margin_rate_by_money double DEFAULT NULL /* '保证金率'*/,
  margin_rate_by_volume double DEFAULT NULL /* '保证金率(按手数)'*/

) ;

-- ----------------------------
-- Records of td_position
-- ----------------------------

-- ----------------------------
-- Table structure for td_trade
-- 成交表
-- ----------------------------
DROP TABLE IF EXISTS td_trade;
CREATE TABLE td_trade (
  init_date int(11) DEFAULT NULL /* '本地日期'*/,
  init_time int(11) DEFAULT NULL /* '本地时间'*/,
  td_type int(11) DEFAULT NULL /* '交易类型：0-ctp'*/,
  jys_trade_date text /* '交易所成交日期'*/,
  jys_trade_time text /* '交易所成交时间'*/,
  user_id text,
  exchange_id text /* '市场'*/,
  stock_code text /* '代码'*/,
  order_ref text /* '报单引用'*/,
  order_sys_id text /* '报单编号'*/,
  trade_id text /* '成交编号'*/,
  action text /* '买卖操作'*/,
  price double DEFAULT NULL /* '价格'*/,
  trade_num int(11) DEFAULT NULL /* '成交数量'*/,
  close_num int(11) DEFAULT NULL /* '平仓数量'*/
) ;

-- ----------------------------
-- Records of td_trade
-- ----------------------------
-- ----------------------------
-- Table structure for td_instrument_margin_rate
-- 期货保证金费率表
-- ----------------------------
DROP TABLE IF EXISTS td_instrument_margin_rate;
CREATE TABLE td_instrument_margin_rate (
  instrument_id text  /* '合约代码'*/,
  exchange_id text /* '交易所代码'*/,
  hedge_flag text /* '投机套保标志'*/,
  long_margin_ratio_by_money double DEFAULT NULL /* '多头保证金率'*/,
  long_margin_ratio_by_volume double DEFAULT NULL /* '多头保证金费'*/,
  short_margin_ratio_by_money double DEFAULT NULL /* '空头保证金率'*/,
  short_margin_ratio_by_volume double DEFAULT NULL /* '空头保证金费'*/,
  is_relative int(11) DEFAULT NULL /* '是否相对交易所收取'*/
) ;

-- ----------------------------
-- Table structure for td_instrument_commission_ratey
-- 期货手续费费率表
-- ----------------------------
DROP TABLE IF EXISTS td_instrument_commission_rate;
CREATE TABLE td_instrument_commission_rate (
  instrument_id text  /* '合约代码'*/,
  exchange_id text /* '交易所代码'*/,
  open_ratio_by_money double DEFAULT NULL /* '开仓手续费率'*/,
  open_ratio_by_volume double DEFAULT NULL /* '开仓手续费'*/,
  close_ratio_by_money double DEFAULT NULL /* '平仓手续费率'*/,
  close_ratio_by_volume double DEFAULT NULL /* '平仓手续费'*/,
  close_today_ratio_by_money double DEFAULT NULL /* '平今手续费率'*/,
  close_today_ratio_by_volume double DEFAULT NULL /* '平今手续费'*/,
  biz_type text /* '业务类型'*/
) ;

-- ----------------------------
-- Table structure for td_instrument_order_comm_rate
-- 期货报撤单费率表
-- ----------------------------
DROP TABLE IF EXISTS td_instrument_order_comm_rate;
CREATE TABLE td_instrument_order_comm_rate (
  instrument_id text  /* '合约代码'*/,
  exchange_id text /* '交易所代码'*/,
  hedge_flag text /* '投机套保标志'*/,
  order_comm_by_volume double DEFAULT NULL /* '报单手续费'*/,
  order_action_comm_by_volume double DEFAULT NULL /* '撤单手续费'*/
  
) ;
-- ----------------------------
-- Table structure for td_commission_open_count
-- 手续费开仓数量表。计算手续费时，记录开仓数量。
-- ----------------------------
DROP TABLE IF EXISTS td_commission_open_count;
CREATE TABLE td_commission_open_count (
  init_date int(11) DEFAULT NULL /* '本地日期'*/,
  td_type int(11) DEFAULT NULL /* '交易类型：0-ctp'*/,
  user_id text,
  exchange_id text /* '市场'*/,
  stock_code text /* '代码'*/,
  direction text /* '持仓方向'*/,
  open_count int(11) DEFAULT NULL /* '开仓数量'*/
  
) ;