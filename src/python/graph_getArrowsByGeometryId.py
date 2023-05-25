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

if len(sys.argv) != 12:
  print("USAGE: python graph_getArrows.py <ums_fileId> <ums_messageIndex> <vms_fileId> <vms_messageIndex> <geometryId> <step> <color> <lineWidth> <outputFile> <width> <height>")
  sys.exit(-1)

fileId1 = int(sys.argv[1])
messageIndex1 = int(sys.argv[2])
fileId2 = int(sys.argv[3])
messageIndex2 = int(sys.argv[4])
geometryId = sys.argv[5]
step = int(sys.argv[6])
color = sys.argv[7]
lineWidth = float(sys.argv[8])/1000
outputFile = sys.argv[9]
awidth = int(sys.argv[10])
aheight = int(sys.argv[11])
                        

# Fetching grid point coordinates according to geometry id
a1 = SmartMet.DataServer.Corba.CorbaAttribute("grid.geometryId",geometryId) 
attributeList = [a1]
res0,attributes,coordinates = dataServer.getGridLatlonCoordinatesByGeometry(0,attributeList);

   
# Fetching grid data from the Data Server    
   
# Fetching grid data according to coordinates
modificationParameters = [0] 
res1,values1  = dataServer.getGridValueVectorByCoordinateList(0,fileId1,messageIndex1,1,coordinates.coordinateList,1,0,modificationParameters)
res1,values2  = dataServer.getGridValueVectorByCoordinateList(0,fileId2,messageIndex2,1,coordinates.coordinateList,1,0,modificationParameters)
        
columns = coordinates.columns
rows = coordinates.rows
reverseY = coordinates.reverseYDirection

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
    ums[r][col] = values1[i]
    vms[r][col] = values2[i]
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



