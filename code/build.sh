#!/bin/bash

CODE_HOME="$PWD"
OPTS=-g
cd ../../build > /dev/null
g++ $OPTS $CODE_HOME/win32_renderer.cpp -o renderer.exe
cd $CODE_HOME > /dev/null
