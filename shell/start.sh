#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# 运行期依赖库路径（CTP/SQLite），供子进程继承
export LD_LIBRARY_PATH="$(cd "$SCRIPT_DIR/.." && pwd)/other_depend/ctp/linux64/lib:$(cd "$SCRIPT_DIR/.." && pwd)/other_depend/sqlite/lib:${LD_LIBRARY_PATH:-}"

ProcessName=run_trade.sh
ProcessNum=`ps -ef | grep "${ProcessName}" | grep -v "grep" | wc -l`

#进程数量大于等于1
if [ $ProcessNum -ge 1 ];
then
    ########
    echo "run_trade process exist"
else
	echo "start run_trade process..."
    bash "$SCRIPT_DIR/${ProcessName}"  &
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
    bash "$SCRIPT_DIR/${ProcessName}"  &
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
    bash "$SCRIPT_DIR/${ProcessName}"  &
fi
