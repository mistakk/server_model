cs_client_check()
{
    count=`ps -eLf | grep cs_client | grep -v grep | wc -l`
    if [[ ${count} -ge "10" ]]; then
        echo "cs_client has "$count" thread"
        return
    fi
    if [[ ${count} -ne "0" ]]; then
        pid=`ps -eLf | grep cs_client  | grep -v grep | awk '{print $2}'`
        echo "cs_client is running, pid: "$pid
    else
        echo "cs_client is not running;"
    fi
}

cs_client_start()
{
    count=`ps -ef | grep multiserver | grep -v grep | wc -l`
    if [[ ${count} -le "0" ]]; then
        /root/self_study/client_server/client/client_multi/cs_client $1 $2> /dev/null &
    fi
    cs_client_check
}

cs_client_start $1 $2
