### Importing libraries. Set the "PYTHONPATH" environment value to point
### library directories if you get an error message about missing libraries. 

import sys
import os
from omniORB import CORBA
from FmiSmartMet import SmartMet


### Checking that the command line is valid:

if len(sys.argv) != 3:
  print("USAGE: python cs_getGenerationInfoById.py <sessionId> <generationId>")
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
generationId = int(sys.argv[2])

res,generationInfo  = contentServer.getGenerationInfoById(sessionId,generationId)


### If the service call returns non-zero value, then the call has failed.
### Error codes are listed here: 
###    "https://github.com/fmidev/smartmet-library-grid-content/blob/master/src/contentServer/definition/ServiceResults.h"

if res != 0:
  print "ERROR: %d\n" %res
  sys.exit(-3)
  
  
### Printing the result:
  
print("PRODUCER")
print("- generationId : %d" % generationInfo.producerId)
print("- producerId   : %d" % generationInfo.producerId)
print("- name         : %s" % generationInfo.name)
print("- description  : %s" % generationInfo.description)
print("- analysisTime : %s" % generationInfo.analysisTime)
print("- flags        : %d" % generationInfo.flags)
print("- sourceId     : %d" % generationInfo.sourceId)
print("- status       : %d" % generationInfo.status)

