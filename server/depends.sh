#!/bin/bash

# 1. 可执行程序的路径名
# 2. 目录名称 --- 将这个程序的依赖库拷贝到指定目录下
declare depends
get_depends() {
    depends=$(ldd $1 | awk '{if (match($3,"/")){print $3}}')
    #mkdir $2
    cp -Lr $depends $2
}

get_depends ./gateway/build/gateway_server ./gateway/depends
get_depends ./file/build/file_server ./file/depends
get_depends ./friend/build/friend_server ./friend/depends
get_depends ./message/build/message_server ./message/depends
get_depends ./speech/build/speech_server ./speech/depends
get_depends ./transmite/build/transmite_server ./transmite/depends
get_depends ./user/build/user_server ./user/depends

cp /bin/nc ./gateway/
cp /bin/nc ./file/
cp /bin/nc ./friend/
cp /bin/nc ./message/
cp /bin/nc ./speech/
cp /bin/nc ./transmit/
cp /bin/nc ./user/

get_depends /bin/nc ./gateway/depends
get_depends /bin/nc ./file/depends
get_depends /bin/nc ./friend/depends
get_depends /bin/nc ./message/depends
get_depends /bin/nc ./speech/depends
get_depends /bin/nc ./user/depends
get_depends /bin/nc ./transmit/depends