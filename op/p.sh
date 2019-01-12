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

cs_server_check()
{
    count=`ps -eLf | grep cs_server | grep -v grep | wc -l`
    if [[ ${count} -ne "0" ]]; then
        pid=`ps -eLf | grep cs_server | grep -v grep | awk '{print $2}'`
        echo "cs_server is running, pid: "$pid
    else
        echo "cs_server is not running;"
    fi
}
cs_client_check
cs_server_check
