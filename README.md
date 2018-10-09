Grid Tools
==========

This module contains several programs used by the SmartMet Server grid support. 

## Server programs
The main services of the grid support can be used as independent services, which means that they must be executed and configured as independent servers. For this purpose, this module contains few server programs that can be configured multiple ways. 

These implementations are more like demonstrations that show how easily we can implement our own servers, because all necessary components are available in the grid content library. The point is that we can easily construct a server that is complete for own purposes. I.e. we can merge, duplicate and arrange service components in such way that we get an optional solution to our own environment.


## Client programs

Client programs can be used for accessing remote Content Server APIs, Data Server APIs or Query Server APIs. Technically these programs uses CORBA- or HTTP-client implementations of these service APIs.

When Redis-implementation of the Content Server API is used, the current client programs can access information also in the Redis database, inspite of that the actual service functionality is implemented on the clients' side. In this case the current client programs use the remote Redis database as native Redis-clients, but the actual service logic is on the clients' side.

## File programs
These programs can be used for opening single grid files and finding out their structure and content.

## Utils
These programs are simple assisting programs.

## FMI programs
These programs are mainly used by FMI (Finnish Meteorological Institute). They are not necessary useful for other organizations, but they can be used as models when implementing own programs.


## Licence
The Grid Tools moduel is a part of the SmartMet Server (https://github.com/fmidev/smartmet-server), which is published with MIT-license.

## How to contribute
Found a bug? Want to implement a new feature? Your contribution is very welcome!

Small changes and bug fixes can be submitted via pull request. In larger contributions, premilinary plan is recommended (in GitHub wiki). 

CLA is required in order to contribute. Please contact us for more information!

## Documentation
SmartMet Server Grid support (https://github.com/fmidev/smartmet-tools-grid/raw/master/doc/grid-support.pdf)


## Communication and Resources
You may contact us from following channels:
* Email: beta@fmi.fi
* Facebook: https://www.facebook.com/fmibeta/
* GitHub: [issues](../../issues)

Other resources which may be useful:  
* Presentation about the server: http://www.slideshare.net/tervo/smartmet-server-providing-metocean-data  
* Our public web pages (in Finnish): http://ilmatieteenlaitos.fi/avoin-lahdekoodi   




