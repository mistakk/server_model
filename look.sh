#! /bin/sh
ps -ef |grep -E 'server_|client' |grep -v grep |grep -v centos
echo
lsof -i:2222 
