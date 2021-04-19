#!/usr/bin/env python
# coding: utf-8

# # Arduino serial port reading and logging

# In[ ]:


import serial
import numpy as np
import matplotlib.pyplot as plt
#from drawnow import *
import sys
from datetime import datetime
import os.path

# enable live update
plt.ion()

# initialization of state variable for recording data
recording_flag = False

# Opens serial port
try:
    arduinoData = serial.Serial('COM4',9600) #Creating our serial object named arduinoData #'
except Exception as e:
    print("Error opening serial port. Close the Serial Monitor (if open) and retry")
    sys.exit()


arduinoData.flush() #clears buffers for serial port
while (arduinoData.inWaiting() == 0): #Wait here until there is data
    #print("no data")
    pass

firstString = arduinoData.readline() #first line is not used for issues of truncated messages

def listToString(s):

    # initialize an empty string
    str1 = ""

    # traverse in the string
    for ele in s:
        str1 += str(ele)+ ","

    # return string
    return str1


for i in range(0,100):
    filename = "prova"+str(int(i/10))+str(i%10)+".txt"
    #create if does not exist, do not open existing, write, sync after write
    if (not os.path.isfile(filename)):
        break;

file  = open(filename, "a+")
seq = []

while True:

    while (arduinoData.inWaiting() == 0): #Wait here until there is data
        #print("no data")
        pass

    try:
        # read line from serial and decode it
        arduinoString = arduinoData.readline()
        arduinoString = arduinoString.decode(encoding = 'utf-8')
        serialMessage = arduinoString.split(',')
        serialMessage[-1] = serialMessage[-1].strip("\r\n")
        #print(serialMessage)

    except:
        print("Problem reading Serial message. In case of crash try again")
        file.close()
        arduinoData.close()

    # identifies if message is "button_pressed"
    if serialMessage[0] == "Starting_recording":
        recording_flag = not recording_flag
        if recording_flag:
            experimentData = [["ID","time","L1","W1","L2","W2"]] # numpy array for experiment data
            print("Recording experiment")

    elif serialMessage[0] == "Ending_recording":
        file.close()
        arduinoData.close()
        recording_flag = not recording_flag
        print("Experiment data saved")
        break

    elif serialMessage[0] == "ID":
        try:
            data = serialMessage[1:-1]

        except:
            data = ""

        if recording_flag:
            file.write(listToString(data) + "\n")


# In[ ]:
