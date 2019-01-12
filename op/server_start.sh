cs_server_check()
{
    count=`ps -eLf | grep cs_server | grep -v grep | wc -l`
    if [[ ${count} -ne "0" ]]; then
        pid=`ps -eLf | grep cs_server  | grep -v grep | awk '{print $2}'`
        echo "cs_server is running, pid: "$pid
    else
        echo "cs_server is not running;"
    fi
}

cs_server_start()
{
    count=`ps -ef | grep multiserver | grep -v grep | wc -l`
    if [[ ${count} -le "0" ]]; then
        /root/self_study/client_server/server/server_select/cs_server $1 > /dev/null &
    fi
    cs_server_check
}

cs_server_start $1
