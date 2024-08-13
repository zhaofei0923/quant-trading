#!/bin/sh

#删除共享内存
#ipcrm -a

#停止进程的时间下限
limtime="08:55:01"
#limStamp=`date -d "$limtime" +%s`
#停止进程的时间上限
limtimeup="08:55:50"
#limStampup=`date -d "$limtimeup" +%s`
limtimenight="20:54:01"
limtimenightup="20:54:50"

#配置项
ExeDir=/home/kevinzhao/quant/quant_enterprise/quant_enterprise_market/src/bin/
ProcessName=QuantMarketServer

#获取进程id
#Pid=`ps -ef | egrep ${ProcessName} |   egrep -v "grep|vi|tail" | sed -n 1p | awk '{print $2}'`
#计算进程数量
#ProcessNum=`ps -ef | grep "${ProcessName}" | grep -v "grep" | wc -l`

#日志文件
LogFile=${ExeDir}"market_run_log.txt";

#循环
while true;do
    curdate=`date`
    curtime=`date "+%H:%M:%S"`
    curStamp=`date -d "$curtime" +%s`
    limStamp=`date -d "$limtime" +%s`
    limStampup=`date -d "$limtimeup" +%s`
    limStampNight=`date -d "$limtimenight" +%s`
    limStampNightUp=`date -d "$limtimenightup" +%s`

    ProcessNum=`ps -ef | grep "${ProcessName}" | grep -v "grep" | wc -l`

    #进程数量大于等于1
    if [ $ProcessNum -ge 1 ];
    then
            echo $curdate  ":The process  exist." >>$LogFile
            echo  "curStamp:"$curStamp >>$LogFile
            echo  "limStamp:"$limStamp >>$LogFile
            echo  "limStampup:"$limStampup >>$LogFile
	    #在时间范围，重启进程
	    if [ $curStamp -ge $limStamp -a $curStamp -lt $limStampup ]; then
            Pid=`ps -ef | grep ${ProcessName} | grep -v 'grep' | awk '{print $2}'`
            kill -9  ${Pid}
            echo $curdate  ":kill process....." $Pid >>$LogFile
            echo $curdate  ":quit....." >>$LogFile
            sleep 120

			#删除共享内存
            ipcrm -a


            curdate=`date`

            cd ${ExeDir}
            NowDir=`pwd`
            echo $curdate": start process! NowDir is" $NowDir >>$LogFile
           ./${ProcessName}  &

	    fi

        if [ $curStamp -ge $limStampNight -a $curStamp -lt $limStampNightUp ]; then
            Pid=`ps -ef | grep ${ProcessName} | grep -v 'grep' | awk '{print $2}'`
            kill -9  ${Pid}
            echo $curdate  ":kill process....." $Pid >>$LogFile
            echo $curdate  ":quit....." >>$LogFile
            sleep 120

			#删除共享内存
            ipcrm -a


            curdate=`date`

            cd ${ExeDir}
            NowDir=`pwd`
            echo $curdate": start process! NowDir is" $NowDir >>$LogFile
           ./${ProcessName}  &

	    fi
    else
        echo $curdate  ":The process does not exist." >>$LogFile
        echo $curdate  ":start process....." >>$LogFile
        cd ${ExeDir}
        NowDir=`pwd`
        echo $curdate": NowDir is" $NowDir >>$LogFile
        ./${ProcessName}  &
    fi
    sleep 10
done

