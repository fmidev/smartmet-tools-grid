import sys
import os
from omniORB import CORBA
from FmiSmartMet import SmartMet
from array import *
import matplotlib.pyplot as plt
import numpy as np
import matplotlib as mpl

ParamValueMissing = -16777216

argv = ["-ORBgiopMaxMsgSize","50000000"]
orb = CORBA.ORB_init(argv, CORBA.ORB_ID)
ior = os.environ['SMARTMET_DS_IOR']
obj = orb.string_to_object(ior)
dataServer = obj._narrow(SmartMet.DataServer.Corba.ServiceInterface)

if dataServer is None:
   print("Object reference is not an SmartMet.DataServer.Corba.ServiceInterface")
   sys.exit(-2)

if len(sys.argv) != 15:
  print("USAGE: python graph_getNumbersByGeometryId.py <fileId> <messageIndex> <geometryId> <add> <mul> <step> <size> <color> <seaMask> <landMask> <format> <outputFile> <width> <height>")
  sys.exit(-1)

fileId = int(sys.argv[1])
messageIndex = int(sys.argv[2])
geometryId = sys.argv[3]
add = float(sys.argv[4])
mul = float(sys.argv[5])
step = int(sys.argv[6])
sz = float(sys.argv[7])
color = sys.argv[8]
seaMask = int(sys.argv[9])
landMask = int(sys.argv[10])
fmt = sys.argv[11]
outputFile = sys.argv[12]
width = int(sys.argv[13])
height = int(sys.argv[14])

                         
# Fetching grid point coordinates according to geometry id
a1 = SmartMet.DataServer.Corba.CorbaAttribute("grid.geometryId",geometryId) 
attributeList = [a1]
res0,attributes,coordinates = dataServer.getGridLatlonCoordinatesByGeometry(0,attributeList);

# Fetching grid data according to coordinates
modificationParameters = [0] 
res,values  = dataServer.getGridValueVectorByCoordinateList(0,fileId,messageIndex,1,coordinates.coordinateList,1,0,modificationParameters)
        
columns = coordinates.columns
rows = coordinates.rows
reverseY = coordinates.reverseYDirection

# print("graph_getNumbers %d %d" %(coordinates.columns,coordinates.rows))
   
# Fetching land cover values for grid coordinates
if landMask == 1 or seaMask == 1:
  res3,cover = dataServer.getPropertyValuesByCoordinates(0,"covertype",coordinates.coordinateList)


# Converting value lists to matrixes (columns x rows)
VALUES = [[0]*columns for i in range(rows)]
alphas = [[0]*columns for i in range(rows)]

         
i = 0;
for row in range(rows):
  r = row     
  if reverseY == 0:
    r = rows-row-1
       
  for col in range(columns):              
    VALUES[r][col] = (values[i] + add) * mul
  
    if landMask == 1 or seaMask == 1:
      a = cover[i]

      if a == 240:
        if seaMask == 1:
          VALUES[r][col] = ParamValueMissing
      else:
        if landMask == 1:
          VALUES[r][col] = ParamValueMissing

    alphas[r][col] = 0.0
    i = i + 1

# Arrow positions and data
acols = int(columns / step)
arows = int(rows / step)

fig, ax = plt.subplots(frameon=False)
fig.set_figwidth(width)
fig.set_figheight(height)
im = ax.imshow(VALUES, alpha=alphas)

for xx in range(acols):
  for yy in range(arows):
    x = xx*step + step/2
    y = yy*step + step/2
    z = VALUES[int(y)][int(x)]
                                              
    if z != ParamValueMissing:
      ax.text(x,y,fmt.format(z), ha='center', va='center', color=color,size=sz,clip_on=1)  
      # ax.text(x,y,fmt.format(z), ha='center', va='center', color=color,size=sz,bbox=dict(facecolor='black', alpha=0.2))

plt.xticks([])
plt.yticks([])
plt.subplots_adjust(0,0,1,1,0,0)
for ax in fig.axes:
  ax.axis('off')
  ax.margins(0,0)
  ax.xaxis.set_major_locator(plt.NullLocator())
  ax.yaxis.set_major_locator(plt.NullLocator())
  
plt.savefig(outputFile, transparent=True, pad_inches = 0, bbox_inches='tight')



