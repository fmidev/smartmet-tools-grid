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

if len(sys.argv) != 8:
  print("USAGE: python graph_getCoverBorderByGeometryId.py <geometryId> <coverType> <color> <lineWidth> <outputFile> <width> <height>")
  sys.exit(-1)

geometryId = sys.argv[1]
coverType = int(sys.argv[2])
color = sys.argv[3]
lineWidth = float(sys.argv[4])
outputFile = sys.argv[5]
width = int(sys.argv[6])
height = int(sys.argv[7])

                      
# Fetching grid point coordinates according to geometry id
a1 = SmartMet.DataServer.Corba.CorbaAttribute("grid.geometryId",geometryId) 
attributeList = [a1]
res0,attributes,coordinates = dataServer.getGridLatlonCoordinatesByGeometry(0,attributeList);

columns = coordinates.columns
rows = coordinates.rows
reverseY = coordinates.reverseYDirection

   
# Fetching land cover values for grid coordinates
res3,cover = dataServer.getPropertyValuesByCoordinates(0,"covertype",coordinates.coordinateList)

#print("graph_getCoverBorders %d %d" %(columns,rows))

# Converting value lists to matrixes (columns x rows)
VALUES = [[0]*columns for i in range(rows)]
alphas = [[0]*columns for i in range(rows)]

i = 0
for row in range(rows):
  r = row     
  if reverseY == 0:
    r = rows-row-1
    
  for col in range(columns):
    a = cover[i]

    if a == coverType:
      VALUES[r][col] = 0.0
    else:
      VALUES[r][col] = 1.0
                                        
    alphas[r][col] = 0.0
    i = i + 1

# Levels
levels = np.linspace(np.min(VALUES), np.max(VALUES), 1)

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

