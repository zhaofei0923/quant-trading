#include "main_impl.h"
#include "config.h"
#include "sim_log.h"
#include "print.h"
#include "execute_engine.h"
#include "tcp_server.h"
#include "msg_queue_server.h"
#include "trade_data.h"
#include "mem_pool_mng.h"
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

int MainImpl::Init()
{

	ConfigData*pConfigData = ConfigData::GetInstance();
	if (false == pConfigData->LoadConfig())
	{
		return -1;
	}

	SimLog::Instance().InitSimLog(pConfigData->LogCfgData.iLoglevel);
	PrintData::PrintConfigData(*pConfigData);

	TradeData::Init();
	MemPoolMng::GetInstance()->Init();
	ExecuteEngine::GetInstance()->Init();
	CTcpServer::GetInstance()->Init();
	CMsgQueueServer::GetInstance()->Init();
	m_bIsInit = true;
	return 0;
}
int MainImpl::Start()
{
	if (false == m_bIsInit)
	{
		return -1;
	}

	ExecuteEngine::GetInstance()->Start();
	CTcpServer::GetInstance()->Start();
	CMsgQueueServer::GetInstance()->Start();
	return 0;
}
int MainImpl::Stop()
{
	ExecuteEngine::GetInstance()->Stop();
	CTcpServer::GetInstance()->Stop();
	CMsgQueueServer::GetInstance()->Stop();

	return 0;
}
int MainImpl::Release()
{
	MemPoolMng::GetInstance()->Release();
	return 0;
}