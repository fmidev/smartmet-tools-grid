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

if len(sys.argv) != 11:
  print("USAGE: python graph_getArrows.py <ums_fileId> <ums_messageIndex> <vms_fileId> <vms_messageIndex> <step> <color> <lineWidth> <outputFile> <width> <height>")
  sys.exit(-1)

fileId1 = int(sys.argv[1])
messageIndex1 = int(sys.argv[2])
fileId2 = int(sys.argv[3])
messageIndex2 = int(sys.argv[4])
step = int(sys.argv[5])
color = sys.argv[6]
lineWidth = float(sys.argv[7])/1000
outputFile = sys.argv[8]
awidth = int(sys.argv[9])
aheight = int(sys.argv[10])

                         
# Fetching grid data from the Data Server    
res1,values1  = dataServer.getGridData(0,fileId1,messageIndex1)
res2,values2  = dataServer.getGridData(0,fileId2,messageIndex2)
        
columns = values1.columns
rows = values1.rows
reverseY = values1.reverseYDirection

# Converting value lists to matrixes (columns x rows)
ums = [[0]*columns for i in range(rows)]
vms = [[0]*columns for i in range(rows)]
alphas = [[0]*columns for i in range(rows)]

i = 0;
for row in range(rows):
  r = row     
  if reverseY == 0:
    r = rows-row-1
       
  for col in range(columns):
    ums[r][col] = values1.valueList[i]
    vms[r][col] = values2.valueList[i]
    alphas[r][col] = 0.0
    i = i + 1

# Arrow positions and data
acols = int(columns / step)
arows = int(rows / step)
UMS = [[0]*acols for i in range(arows)]
VMS = [[0]*acols for i in range(arows)]
X = [[0]*acols for i in range(arows)]
Y = [[0]*acols for i in range(arows)]

for xx in range(acols):
  for yy in range(arows):
    UMS[yy][xx] = ums[yy*step][xx*step]
    VMS[yy][xx] = vms[yy*step][xx*step]
    X[yy][xx] = xx*step + step/2
    Y[yy][xx] = yy*step + step/2

fig, ax = plt.subplots(frameon=False)
fig.set_figwidth(awidth)
fig.set_figheight(aheight)
im = ax.imshow(ums, alpha=alphas)
ax.quiver(X,Y,UMS,VMS,color=color,width=lineWidth)

plt.xticks([])
plt.yticks([])
plt.subplots_adjust(0,0,1,1,0,0)
for ax in fig.axes:
  ax.axis('off')
  ax.margins(0,0)
  ax.xaxis.set_major_locator(plt.NullLocator())
  ax.yaxis.set_major_locator(plt.NullLocator())
  
plt.savefig(outputFile, transparent=True, pad_inches = 0, bbox_inches='tight')



