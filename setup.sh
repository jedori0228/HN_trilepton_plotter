#!/bin/bash
export PLOTTER_WORKING_DIR=`pwd`
export FILE_PATH=$PLOTTER_WORKING_DIR/rootfiles/
export PLOT_PATH=$PLOTTER_WORKING_DIR/plots/
export SCRIPT_DIR=$PLOTTER_WORKING_DIR/script/
export ROOT_INCLUDE_PATH=$ROOT_INCLUDE_PATH:$PLOTTER_WORKING_DIR/include/

alias run="bash "$SCRIPT_DIR"/run.sh"
