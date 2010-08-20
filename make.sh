#!/bin/bash

cd core/ && make && sudo make install &&
cd ../modules && make && sudo make install

sleep 20s
#cd ../projects/utils
#cd 
