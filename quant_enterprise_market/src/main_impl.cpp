#include "main_impl.h"
#include "config.h"
#include "sim_log.h"
#include "tcp_server.h"
#include "msg_queue_server.h"
#include "save_data.h"
#include "trade_data.h"
#include "mem_pool_mng.h"


int MainImpl::Init()
{

	ConfigData*pConfigData = ConfigData::GetInstance();
	if (false == pConfigData->LoadConfig())
	{
		return -1;
	}

	SimLog::Instance().InitSimLog(pConfigData->LogCfgData.iLoglevel);

	TradeData::Init();
	MemPoolMng::GetInstance()->Init();
	CTcpServer::GetInstance()->Init();
	CMsgQueueServer::GetInstance()->Init();
	CSaveData::GetInstance()->Init();

	m_bIsInit = true;
	return 0;
}
int MainImpl::Start()
{
	if (false == m_bIsInit)
	{
		return -1;
	}

	//启动tcp
	CTcpServer::GetInstance()->Start();
	//启动共享内存消息队列通讯
	CMsgQueueServer::GetInstance()->Start();
	//启动保存数据到文件
	CSaveData::GetInstance()->Start();

	std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3));

	m_pMdAdapterCtp = new MdAdapterCtp();
	ConfigData*pConfigData = ConfigData::GetInstance();
	m_pMdAdapterCtp->Connect(pConfigData->CtpCfgData);

	return 0;
}
int MainImpl::Stop()
{
	TradeData::Close();

	CTcpServer::GetInstance()->Stop();
	CMsgQueueServer::GetInstance()->Stop();
	CSaveData::GetInstance()->Stop();

	return 0;
}
int MainImpl::Release()
{
	return 0;
}