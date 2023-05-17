import sys
import os
from omniORB import CORBA
from FmiSmartMet import SmartMet
from array import *
import matplotlib.pyplot as plt
import numpy as np
import matplotlib as mpl

argv = ["-ORBgiopMaxMsgSize","50000000"]
orb = CORBA.ORB_init(argv, CORBA.ORB_ID)
ior = os.environ['SMARTMET_DS_IOR']
obj = orb.string_to_object(ior)
dataServer = obj._narrow(SmartMet.DataServer.Corba.ServiceInterface)

if dataServer is None:
   print("Object reference is not an SmartMet.DataServer.Corba.ServiceInterface")
   sys.exit(-2)


if len(sys.argv) != 14:
  print("USAGE: python graph_getGrid.py <fileId> <messageIndex> <add> <mul> <colorMap> <seaMask> <landMask> <transparency> <missingValueStart> <missingValueEnd> <outputFile> <width> <height>")
  sys.exit(-1)

fileId = int(sys.argv[1])
messageIndex = int(sys.argv[2])
add = float(sys.argv[3])
mul = float(sys.argv[4])
colormap = sys.argv[5]
seaMask = int(sys.argv[6])
landMask = int(sys.argv[7])
transparency = float(sys.argv[8])
missingValueStart = float(sys.argv[9])
missingValueEnd = float(sys.argv[10])
outputFile = sys.argv[11]
width = int(sys.argv[12])
height = int(sys.argv[13])
  
  
# Fetching grid data from the Data Server    
res1,values  = dataServer.getGridData(0,fileId,messageIndex)

columns = values.columns
rows = values.rows
reverseY = values.reverseYDirection


# Converting the value list to a value matrix (columns x rows)

# Fetching grid point coordinates             
res2,coordinates = dataServer.getGridCoordinates(0,fileId,messageIndex,1);

# print("graph_getGrid %d %d" %(coordinates.columns,coordinates.rows))

# Fetching land cover values for grid coordinates
if landMask == 1 or seaMask == 1:
  res4,cover = dataServer.getPropertyValuesByCoordinates(0,"covertype",coordinates.coordinateList)


# Converting the value list to a value matrix (columns x rows)
VALUES = [[0]*columns for i in range(rows)]
alphas = [[0]*columns for i in range(rows)]
COVER = [[0]*columns for i in range(rows)]

i = 0;
for row in range(rows):
  r = row     
  if reverseY == 0:
    r = rows-row-1
    
  for col in range(columns):
    v = (values.valueList[i] + add) * mul
    VALUES[r][col] = v
    mask = 1.0
    
    if landMask == 1 or seaMask == 1:    
      a = cover[i]
              
      if seaMask == 1 and a == 240:
        mask = 0.0

      if landMask == 1 and a != 240:
        mask = 0.0
           
    if transparency < 1.0:
      mask = transparency*v
      
    if v >= missingValueStart and v <= missingValueEnd:
      mask = 0
      
    alphas[r][col] = mask      
    i = i + 1

fig, ax = plt.subplots(frameon=False)
fig.set_figwidth(width)
fig.set_figheight(height)
im = ax.imshow(VALUES, alpha=alphas, cmap=mpl.colormaps[colormap])
plt.xticks([])
plt.yticks([])
plt.subplots_adjust(0,0,1,1,0,0)
for ax in fig.axes:
  ax.axis('off')
  ax.margins(0,0)
  ax.xaxis.set_major_locator(plt.NullLocator())
  ax.yaxis.set_major_locator(plt.NullLocator())
  
plt.savefig(outputFile, transparent=True, pad_inches = 0, bbox_inches='tight')

