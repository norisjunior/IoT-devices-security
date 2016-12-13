#!/bin/bash
./udpserver_v3.py aaaa::212:7402:2:202 &
./udpserver_v3.py aaaa::212:7403:3:303 &
./udpserver_v3.py aaaa::212:7404:4:404 &
wait
exit

