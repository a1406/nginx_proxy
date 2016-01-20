#!/bin/sh
 ./configure --with-debug --with-stream --with-cc-opt="-g -O0"  --without-http --with-ld-opt="-pthread -lcrypt" --add-module="./nginx_proxy"
