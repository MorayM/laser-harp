#!/bin/zsh

# Permissions get reset if you unplug the board
sudo chmod  a+rw /dev/ttyACM0
# Compile and upload the sketch
arduino-cli compile --fqbn arduino:avr:mega $1
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:mega $1