#!/bin/bash

cd "$(dirname "$0")" # set current working directory to the directory of the script

rsync -av --exclude=".git" ./usr minix:/
rsync -av --exclude=".git" ./Documents minix:/root