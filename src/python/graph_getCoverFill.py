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

if len(sys.argv) != 9:
  print("USAGE: python graph_getCoverFill.py <fileId> <messageIndex> <coverType> <reverse> <color> <outputFile> <width> <height>")
  sys.exit(-1)

fileId = int(sys.argv[1])
messageIndex = int(sys.argv[2])
coverType = int(sys.argv[3])
reverse = int(sys.argv[4])
color = sys.argv[5]
outputFile = sys.argv[6]
width = int(sys.argv[7])
height = int(sys.argv[8])
                      
# Fetching grid point coordinates             
res2,coordinates = dataServer.getGridCoordinates(0,fileId,messageIndex,1);

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

