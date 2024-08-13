#!/bin/sh

ProcessName=run_trade.sh
ProcessNum=`ps -ef | grep "${ProcessName}" | grep -v "grep" | wc -l`

#进程数量大于等于1
if [ $ProcessNum -ge 1 ];
then
    ########
    echo "run_trade process exist"
else
	echo "start run_trade process..."
    sh ${ProcessName}  &
fi

sleep 60

ProcessName=run_market.sh
ProcessNum=`ps -ef | grep "${ProcessName}" | grep -v "grep" | wc -l`

#进程数量大于等于1
if [ $ProcessNum -ge 1 ];
then
    ########
    echo "run_market process exist"
else
	echo "start run_market process..."
    sh ${ProcessName}  &
fi

sleep 60

ProcessName=run_strategy.sh
ProcessNum=`ps -ef | grep "${ProcessName}" | grep -v "grep" | wc -l`

#进程数量大于等于1
if [ $ProcessNum -ge 1 ];
then
    ########
    echo "run_strategy process exist"
else
	echo "start run_strategy process..."
    sh ${ProcessName}  &
fi
