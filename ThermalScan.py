# -*- coding: utf-8 -*-
"""
Created on Sat Jan 31 21:54:21 2015

@author: arnaudlina
"""
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg
import matplotlib.figure as mfig
import PyQt4.QtGui as gui
import numpy as np
import time

import os
os.chdir("/Users/arnaudlina/Documents/TECHNO/PROJECTS/PYTHON/LibDeComArduino")
from LibComArduino import *

# Scan parameters
ScanSizeX  = 10
ScanSizeY  = 10
PitchDelta = 40
YawDelta   = 20

print("")
print("==================")
print("OPEN CONNECTION...")
print("==================")
com = ComArduino(DEFAULT_DEVICE_NAME, "/dev/tty.usbmodem1451")
com.Open(True)
print("DONE!")

print("")
time.sleep(2.0);

print("")
print("==================")
print("SEND PARAMETERS...")
print("==================")
print(" Send Scan size X..."+str(ScanSizeX))
com.Write(ScanSizeX, True)
print(" Send Scan size Y..."+str(ScanSizeY))
com.Write(ScanSizeY, True)
print(" Send Yaw start..."+str(-YawDelta))
com.Write(-YawDelta, True)
print(" Send Yaw end..."+str(+YawDelta))
com.Write(+YawDelta, True);
print(" Send Pitch start..."+str(-PitchDelta))
com.Write(-PitchDelta, True)
print(" Send Pitch end..."+str(+PitchDelta))
com.Write(+PitchDelta, True)
print("DONE!")
time.sleep(5.0);

print("")
print("==================")
print("RECEIVE DATA...")
print("==================")
ScanData = [[0.0 for x in range(ScanSizeX)] for x in range(ScanSizeY)] 

count = 0;
while count<ScanSizeY:
    s = com.Read(True)
    if(len(s)>0):
       print(" READ SCAN #"+str(count)+" ["+str(len(s))+"bytes]")
       s = s.split()
       y = [0 for x in range(ScanSizeX)]
       for i in range(ScanSizeX):
           y[i]=float(s[i])
       ScanData[count]=y
       count=count+1
print("DONE!")

print("")
print("==================")
print("GENERATE VIEW...")
print("==================")
data_im = np.array(ScanData, dtype=float)
app    = gui.QApplication([])
fig    = mfig.Figure()
canvas = FigureCanvasQTAgg(fig)
ax = fig.add_subplot(111)
ax.imshow(data_im, interpolation = 'none')
canvas.show()
print("DONE!")