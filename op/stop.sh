cs_server_stop()
{
    count=`ps -ef | grep cs_server | grep -v grep | wc -l`
    if [[ ${count} -ne "0" ]]; then
        pid=`ps -ef | grep cs_server  | grep -v grep | awk '{print $2}'`
        ps -ef | grep cs_server  | grep -v grep | awk '{print $2}' | xargs kill -USR1 
	    echo "stop success"
    fi
}

cs_client_stop()
{
    count=`ps -ef | grep cs_client | grep -v grep | wc -l`
    if [[ ${count} -ne "0" ]]; then
        ps -ef | grep cs_client  | grep -v grep | awk '{print $2}' | xargs kill -USR1 
    fi
}

cs_server_stop
cs_client_stop
