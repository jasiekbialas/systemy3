#!/bin/bash

cd "$(dirname "$0")" # set current working directory to the directory of the script

rsync -av --exclude=".git" ./usr/include/ minix:/usr/include/
rsync -av --exclude=".git" ./usr/src/include minix:/usr/src/include
rsync -av --exclude=".git" ./usr/src/lib minix:/usr/src/lib
rsync -av --exclude=".git" ./usr/src/minix minix:/usr/src/minix
