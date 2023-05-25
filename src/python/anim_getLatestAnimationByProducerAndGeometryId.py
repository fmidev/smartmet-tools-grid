import sys
import os
from omniORB import CORBA
from FmiSmartMet import SmartMet
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime

if len(sys.argv) != 4:
  print("USAGE: python anim_getLatestAnimationByProducerAndGeometryId.py <producerName>[,<producerName>] <geometryId> <outputDir>")
  sys.exit(-1)

modificationParameters = []
latest = "19700101T000000"  
width = 20
height = 20
producers = sys.argv[1].split(",")
requestedGeometryId = sys.argv[2]
geometryId = 1007

pdir = "/usr/bin/clients/python"
mdir = "/usr/bin/utils"
outDir = sys.argv[3]
    
outputFile = "{}/{}_{}_{}_{}.png" 

argv = ["-ORBgiopMaxMsgSize","500000000"]
orb = CORBA.ORB_init(argv, CORBA.ORB_ID)
ior = os.environ['SMARTMET_CS_IOR']
obj = orb.string_to_object(ior)
orb2 = CORBA.ORB_init(argv, CORBA.ORB_ID)
ior2 = os.environ['SMARTMET_DS_IOR']
obj2 = orb.string_to_object(ior2)

# Connecting to the content service:
contentServer = obj._narrow(SmartMet.ContentServer.Corba.ServiceInterface)
if contentServer is None:
   sys.exit(-2)

# Connecting to the data service:
dataServer = obj2._narrow(SmartMet.DataServer.Corba.ServiceInterface)
if dataServer is None:
   sys.exit(-3)

i = 0
for p in producers:
  res,generation = contentServer.getLastGenerationInfoByProducerNameAndStatus(0,p,1);
  if i == 0:
    res,contentList = contentServer.getContentListByGenerationId(0,generation.generationId,0,0,2000000000,0)
    res,timeList = contentServer.getContentTimeListByGenerationId(0,generation.generationId);
  else:  
    res,cList = contentServer.getContentListByGenerationId(0,generation.generationId,0,0,2000000000,0)
    contentList = contentList + cList
  
  i = i +1  
  
i = 0
ti = 0
for t in timeList:
  print("echo %s" % t)
  file_T_K = 0
  message_T_K = 0

  file_P_PA = 0
  message_P_PA = 0

  file_U_MS = 0
  message_U_MS = 0

  file_V_MS = 0
  message_V_MS = 0
  
  for c in contentList:
 
    if i == 0:  
      if geometryId == 0:
        geometryId = c.geometryId

      fname_landBorder1 = outputFile.format(outDir,producers[0],geometryId,"landBorder","1") 
      fname_landBorder2 = outputFile.format(outDir,producers[0],geometryId,"landBorder","2") 
      fname_T_K_isobands = outputFile.format(outDir,producers[0],geometryId,t,"T-K_isobands") 
      fname_T_K_numbers = outputFile.format(outDir,producers[0],geometryId,t,"T-K_numbers") 
      fname_P_PA_isolines = outputFile.format(outDir,producers[0],geometryId,t,"P-PA_isolines") 
      fname_wind_arrows = outputFile.format(outDir,producers[0],geometryId,t,"windArrows") 
        
      print("python3 %s/graph_getCoverBordersByGeometryId.py %s 240 \"#FFFFFF\" 1.0 %s %d %d" % (pdir,requestedGeometryId,fname_landBorder1,width,height))
      print("python3 %s/graph_getCoverBordersByGeometryId.py %s 240 \"#000000\" 0.0 %s %d %d" % (pdir,requestedGeometryId,fname_landBorder2,width,height))
      
    if t == c.forecastTime  and  c.geometryId == geometryId:

      if c.fmiParameterName == "T-K" and c.fmiParameterLevelId == 1 and c.parameterLevel == 0:
        file_T_K = c.fileId
        message_T_K = c.messageIndex
      
      if c.fmiParameterName == "U-MS" and c.fmiParameterLevelId == 1 and c.parameterLevel == 0:
        file_U_MS = c.fileId
        message_U_MS = c.messageIndex

      if c.fmiParameterName == "V-MS" and c.fmiParameterLevelId == 1 and c.parameterLevel == 0:
        file_V_MS = c.fileId
        message_V_MS = c.messageIndex
        
      if c.fmiParameterName == "P-PA" and c.parameterLevel == 0:
        file_P_PA = c.fileId
        message_P_PA = c.messageIndex
        

    i = i + 1
                  
  if file_T_K > 0:
    print("python3 %s/graph_getGridByGeometryId.py %d %d %s 0 1.0 jet 1 0 1.0 -0.01 0.01 %s %d %d" % (pdir,file_T_K,message_T_K,requestedGeometryId,fname_T_K_isobands,width,height))
    print("python3 %s/graph_getNumbersByGeometryId.py %d %d %s -273.15 1.0 17 10 \"000000\" 1 0 \"{:0.0f}\" %s %d %d" % (pdir,file_T_K,message_T_K,requestedGeometryId,fname_T_K_numbers,width,height))
    
  if file_U_MS > 0  and file_V_MS > 0:
    print("python3 %s/graph_getArrowsByGeometryId.py %d %d %d %d %s 10 \"#8080A0\" 0.7 %s %d %d" % (pdir,file_U_MS,message_U_MS,file_V_MS,message_V_MS,requestedGeometryId,fname_wind_arrows,width,height))  

  if file_P_PA > 0:    
    print("python3 %s/graph_getIsolinesByGeometryId.py %d %d %s 40 \"#000040\" 0.4 %s %d %d" % (pdir,file_P_PA,message_P_PA,requestedGeometryId,fname_P_PA_isolines,width,height))
    
  if file_T_K > 0 and  file_U_MS > 0  and file_V_MS > 0 and file_P_PA > 0:
    print("%s/gu_mergePngFiles %s/%s_%d_%s_merge_%04d.png FFE0E0FF %s %s %s %s %s %s" % (mdir,outDir,producers[0],geometryId,requestedGeometryId,ti,fname_T_K_isobands,fname_P_PA_isolines,fname_wind_arrows,fname_landBorder1,fname_landBorder2,fname_T_K_numbers))
    
  ti = ti + 1
