#!/bin/sh
ROOT_PW=$1
FILE=$2

(echo $ROOT_PW | sudo -kS chmod +x $FILE > /dev/null 2>&1) || true
(echo $ROOT_PW | sudo -kS setcap cap_net_raw,cap_net_admin=ep $FILE > /dev/null 2>&1) || true


