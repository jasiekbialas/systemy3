#!/bin/bash

cd "$(dirname "$0")" # set current working directory to the directory of the script

rsync -av --exclude=".git" ./usr/include minix:/usr
rsync -av --exclude=".git" ./usr/src/include minix:/usr/src
rsync -av --exclude=".git" ./usr/src/lib minix:/usr/src
rsync -av --exclude=".git" ./usr/src/minix minix:/usr/src
