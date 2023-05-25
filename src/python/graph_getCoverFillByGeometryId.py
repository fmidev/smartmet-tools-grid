import sys
import os
from omniORB import CORBA
from FmiSmartMet import SmartMet
from array import *
import matplotlib.pyplot as plt
import numpy as np
import matplotlib as mpl
from matplotlib.colors import ListedColormap, LinearSegmentedColormap
import PIL

def hex_to_rgb(hex):
  rgb = []
  for i in (0, 2, 4):
    decimal = int(hex[i:i+2], 16)
    rgb.append(decimal)
  
  return tuple(rgb)



argv = ["-ORBgiopMaxMsgSize","50000000"]
orb = CORBA.ORB_init(argv, CORBA.ORB_ID)
ior = os.environ['SMARTMET_DS_IOR']
obj = orb.string_to_object(ior)
dataServer = obj._narrow(SmartMet.DataServer.Corba.ServiceInterface)

if dataServer is None:
   print("Object reference is not an SmartMet.DataServer.Corba.ServiceInterface")
   sys.exit(-2)

if len(sys.argv) != 8:
  print("USAGE: python graph_getCoverFillByGeometryId.py <geometryId> <coverType> <reverse> <color> <outputFile> <width> <height>")
  sys.exit(-1)

geomeryId = sys.argv[1]
coverType = int(sys.argv[2])
reverse = int(sys.argv[3])
color = sys.argv[4]
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

# print("graph_getCoverFill %d %d" %(columns,rows))
          
im = PIL.Image.new(mode="RGBA", size=(columns, rows))

cc = hex_to_rgb(color)
r = cc[0]
g = cc[1]
b = cc[2]

i = 0
for row in range(rows):
  rw = row     
  if reverseY == 0:
    rw = rows-row-1
    
  for col in range(columns):
    a = cover[i]
    mask = 255
    im.putpixel((col, rw), (r, g, b, 0))
    if a == coverType:
      if reverse == 0:
        mask = 255
      else:
        mask = 0      
    else:
      if reverse == 1:
        mask = 255      
      else:
        mask = 0
                                        
    im.putpixel((col, rw), (r, g, b, mask))
    i = i + 1


fig, ax = plt.subplots(frameon=False)
fig.set_figwidth(width)
fig.set_figheight(height)
im = ax.imshow(im)
plt.xticks([])
plt.yticks([])
plt.subplots_adjust(0,0,1,1,0,0)
for ax in fig.axes:
  ax.axis('off')
  ax.margins(0,0)
  ax.xaxis.set_major_locator(plt.NullLocator())
  ax.yaxis.set_major_locator(plt.NullLocator())
  
plt.savefig(outputFile, transparent=True, pad_inches = 0, bbox_inches='tight')

