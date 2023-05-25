import sys
import os
from omniORB import CORBA
from FmiSmartMet import SmartMet
from array import *
import matplotlib.pyplot as plt
import numpy as np
import matplotlib as mpl

argv = ["-ORBgiopMaxMsgSize","200000000"]
orb = CORBA.ORB_init(argv, CORBA.ORB_ID)
ior = os.environ['SMARTMET_DS_IOR']
obj = orb.string_to_object(ior)
dataServer = obj._narrow(SmartMet.DataServer.Corba.ServiceInterface)

if dataServer is None:
   print("Object reference is not an SmartMet.DataServer.Corba.ServiceInterface")
   sys.exit(-2)


if len(sys.argv) != 15:
  print("USAGE: python graph_getGridByGeometryId.py <fileId> <messageIndex> <geometryId> <add> <mul> <colorMap> <seaMask> <landMask> <transparency> <missingValueStart> <missingValueEnd> <outputFile> <width> <height>")
  sys.exit(-1)


fileId = int(sys.argv[1])
messageIndex = int(sys.argv[2])
geometryId = sys.argv[3]
add = float(sys.argv[4])
mul = float(sys.argv[5])
colormap = sys.argv[6]
seaMask = int(sys.argv[7])
landMask = int(sys.argv[8])
transparency = float(sys.argv[9])
missingValueStart = float(sys.argv[10])
missingValueEnd = float(sys.argv[11])
outputFile = sys.argv[12]
width = int(sys.argv[13])
height = int(sys.argv[14])
  
ParamValueMissing = -16777216    
  

# Fetching grid point coordinates according to geometry id
a1 = SmartMet.DataServer.Corba.CorbaAttribute("grid.geometryId",geometryId) 
attributeList = [a1]
res1,attributes,coordinates = dataServer.getGridLatlonCoordinatesByGeometry(0,attributeList);

# Fetching grid point coordinates according to geometry definition
#a1 = SmartMet.DataServer.Corba.CorbaAttribute("grid.crs",'GEOGCS["UNKNOWN",DATUM["UNKNOWN",SPHEROID["UNKNOWN",6367470,0]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]]')

#a1 = SmartMet.DataServer.Corba.CorbaAttribute("grid.crs",'EPSG:4326')
#a2 = SmartMet.DataServer.Corba.CorbaAttribute("grid.width","900") 
#a3 = SmartMet.DataServer.Corba.CorbaAttribute("grid.height","450") 
#a4 = SmartMet.DataServer.Corba.CorbaAttribute("grid.llbox","-179,-89,179,89") 
#a4 = SmartMet.DataServer.Corba.CorbaAttribute("grid.llbox","-15,30,50,75") 
#attributeList = [a1,a2,a3,a4]
#res1,attributes,coordinates = dataServer.getGridLatlonCoordinatesByGeometry(0,attributeList);

# Fetching grid data according to coordinates
modificationParameters = [0] 
res2,values  = dataServer.getGridValueVectorByCoordinateList(0,fileId,messageIndex,1,coordinates.coordinateList,1,0,modificationParameters)

# Fetching land cover values for grid coordinates
if landMask == 1 or seaMask == 1:
  res4,cover = dataServer.getPropertyValuesByCoordinates(0,"covertype",coordinates.coordinateList)

columns = coordinates.columns
rows = coordinates.rows
reverseY = coordinates.reverseYDirection

# Converting the value list to a value matrix (columns x rows)
VALUES = [[0]*columns for i in range(rows)]

i = 0;
for row in range(rows):
  r = row     
  if reverseY == 0:
    r = rows-row-1
    
  for col in range(columns):
    v = values[i]    

    VALUES[r][col] =  (v + add) * mul
    
    if landMask == 1 or seaMask == 1:    
      a = cover[i]
              
      if seaMask == 1 and a == 240:
        VALUES[r][col] = np.nan

      if landMask == 1 and a != 240:
        VALUES[r][col] = np.nan
                
    if values[i] == ParamValueMissing or (v >= missingValueStart and v <= missingValueEnd):
      #mask = 0
      VALUES[r][col] = np.nan
      
    i = i + 1
    
    
fig, ax = plt.subplots(frameon=False)
fig.set_figwidth(width)
fig.set_figheight(height)
im = ax.imshow(VALUES, cmap=mpl.colormaps[colormap])
plt.xticks([])
plt.yticks([])
plt.subplots_adjust(0,0,1,1,0,0)

for ax in fig.axes:
  ax.axis('off')
  ax.margins(0,0)
  ax.xaxis.set_major_locator(plt.NullLocator())
  ax.yaxis.set_major_locator(plt.NullLocator())
  
plt.savefig(outputFile, transparent=True, pad_inches = 0, bbox_inches='tight')

