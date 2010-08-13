#!/bin/bash

cd core/
make && sudo make install
cd ../modules
make && sudo make install
#cd ../projects/utils
#cd 
