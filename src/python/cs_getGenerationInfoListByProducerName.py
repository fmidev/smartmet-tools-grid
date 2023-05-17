### Importing libraries. Set the "PYTHONPATH" environment value to point
### library directories if you get an error message about missing libraries. 

import sys
import os
from omniORB import CORBA
from FmiSmartMet import SmartMet


### Checking that the command line is valid:

if len(sys.argv) != 3:
  print("USAGE: python cs_getGenerationInfoListByProducerName.py <sessionId> <producerName>")
  sys.exit(-1)

  
### Initializing ORB (= Object Request Broker):

argv = ["-ORBgiopMaxMsgSize","500000000"]
orb = CORBA.ORB_init(argv, CORBA.ORB_ID)


### Connecting to the service interface. Assuming that the IOR (= International Object Reference)
### can be found from the "SMARTMET_CS_IOR" environment variable.

ior = os.environ['SMARTMET_CS_IOR']
obj = orb.string_to_object(ior)

contentServer = obj._narrow(SmartMet.ContentServer.Corba.ServiceInterface)

if contentServer is None:
   print("Object reference is not an SmartMet.ContentServer.Corba.ServiceInterface")
   sys.exit(-2)

   
### Service call:   
   
sessionId = int(sys.argv[1])
producerName = sys.argv[2]

res,generationList = contentServer.getGenerationInfoListByProducerName(sessionId,producerName)


### If the service call returns non-zero value, then the call has failed.
### Error codes are listed here: 
###    "https://github.com/fmidev/smartmet-library-grid-content/blob/master/src/contentServer/definition/ServiceResults.h"

if res != 0:
  print "ERROR: %d\n" %res
  sys.exit(-3)
  
  
### Printing the result:
  
for g in generationList:
  print("  GENERATION %d;%d;%d;%s;%s;%s;%d;%d;%d" % (g.generationId,g.generationType,g.producerId,g.name,g.description,g.analysisTime,g.flags,g.sourceId,g.status))

  