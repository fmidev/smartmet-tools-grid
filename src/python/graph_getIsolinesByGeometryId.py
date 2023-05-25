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

if len(sys.argv) != 10:
  print("USAGE: python graph_getIsolinesByGeometryId.py <fileId> <messageIndex> <geometryId> <levels> <color> <lineWidth> <outputFile> <width> <height>")
  sys.exit(-1)

fileId = int(sys.argv[1])
messageIndex = int(sys.argv[2])
geometryId = sys.argv[3]
levelCount = int(sys.argv[4])
color = sys.argv[5]
lineWidth = float(sys.argv[6])
outputFile = sys.argv[7]
width = int(sys.argv[8])
height = int(sys.argv[9])

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

# print("graph_getIsolines %d %d" %(columns,rows))

   
# Converting value lists to matrixes (columns x rows)
VALUES = [[0]*columns for i in range(rows)]
alphas = [[0]*columns for i in range(rows)]

i = 0;
for row in range(rows):
  r = row     
  if reverseY == 0:
    r = rows-row-1
    
  for col in range(columns):
    VALUES[r][col] = values[i]
    alphas[r][col] = 0.0
    i = i + 1

# Levels
levels = np.linspace(np.min(VALUES), np.max(VALUES), levelCount)

fig, ax = plt.subplots(frameon=False)
fig.set_figwidth(width)
fig.set_figheight(height)
im = ax.imshow(VALUES, alpha=alphas)
ax.contour(VALUES,levels=levels, linewidths=[lineWidth], colors=[color])
plt.xticks([])
plt.yticks([])
plt.subplots_adjust(0,0,1,1,0,0)
for ax in fig.axes:
  ax.axis('off')
  ax.margins(0,0)
  ax.xaxis.set_major_locator(plt.NullLocator())
  ax.yaxis.set_major_locator(plt.NullLocator())
  
plt.savefig(outputFile, transparent=True, pad_inches = 0, bbox_inches='tight')

