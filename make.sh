#!/bin/bash

cd core/ && make && sudo make install &&
cd ../modules && make && sudo make install &&
cd ../projects/utils/matrix && make && sudo make install
cd ../velgrid && make && sudo make install
cd ../vvhdplot && sudo make install
cd ../data2gnumeric && make && sudo make install

sleep 20s
#cd ../projects/utils
#cd 
