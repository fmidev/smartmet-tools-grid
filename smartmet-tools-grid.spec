%define DIRNAME grid
%define SPECNAME smartmet-tools-%{DIRNAME}
Summary: SmartMet tools for grid support
Name: %{SPECNAME}
Version: 18.6.14
Release: 1%{?dist}.fmi
License: MIT
Group: SmartMet/Plugins
URL: https://github.com/fmidev/smartmet-plugin-grid-gui
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: rpm-build
BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: boost-devel
BuildRequires: libconfig-devel
BuildRequires: smartmet-library-spine-devel
BuildRequires: smartmet-library-grid-files-devel
BuildRequires: smartmet-library-grid-content-devel
BuildRequires: gdal-devel
BuildRequires: omniORB-devel
BuildRequires: libpqxx-devel
BuildRequires: postgresql-devel
BuildRequires: libmicrohttpd-devel
BuildRequires: libjpeg-devel
BuildRequires: libpng-devel
Requires: libconfig
#Requires: smartmet-library-macgyver >= 18.2.6
#Requires: smartmet-library-spine >= 18.1.15
#Requires: smartmet-server >= 17.11.10
#Requires: smartmet-engine-grid >= 18.2.8
Requires: boost-date-time
Requires: smartmet-library-grid-files
Requires: smartmet-library-grid-content
#Requires: smartmet-engine-grid

Provides: radon2smartmet = %{Version}
Provides: filesys2smartmet = %{Version}
Provides: radon2config = %{Version}
Provides: radon2csv = %{Version}
Provides: grid_query = %{Version}
Provides: grid_info = %{Version}
Provides: grid_maps = %{Version}
Provides: grid_submaps = %{Version}
Provides: grid_images = %{Version}
Provides: grid_dump = %{Version}
Provides: grid_add = %{Version}
Provides: cs_getCsvFiles = %{Version}
Provides: cs_deleteGenerationInfoByName = %{Version}
Provides: cs_getFileInfoCountByGenerationId = %{Version}
Provides: cs_getContentCount = %{Version}
Provides: cs_addProducerInfo = %{Version}
Provides: cs_getFileInfoListByGenerationId = %{Version}
Provides: cs_getContentListByParameterGenerationIdAndForecastTime = %{Version}
Provides: cs_addGenerationInfo = %{Version}
Provides: cs_getContentListByGenerationIdAndTimeRange = %{Version}
Provides: cs_getFileInfoCountsByProducers = %{Version}
Provides: cs_getContentTimeListByGenerationAndGeometryId = %{Version}
Provides: cs_deleteDataServerInfoById = %{Version}
Provides: cs_getFileInfoList = %{Version}
Provides: cs_getProducerInfoList = %{Version}
Provides: cs_getContentListByParameter = %{Version}
Provides: cs_getFileInfoListByProducerId = %{Version}
Provides: ds_getGridData = %{Version}
Provides: cs_getFileInfoListByGenerationName = %{Version}
Provides: cs_deleteFileInfoById = %{Version}
Provides: cs_getProducerInfoCount = %{Version}
Provides: cs_getContentListBySourceId = %{Version}
Provides: ds_getGridImage = %{Version}
Provides: cs_getLastGenerationInfoByProducerIdAndStatus = %{Version}
Provides: ds_getGridValueListByRectangle = %{Version}
Provides: cs_getProducerParameterList = %{Version}
Provides: cs_deleteProducerInfoById = %{Version}
Provides: cs_getFileInfoCountByProducerId = %{Version}
Provides: cs_getContentListOfInvalidIntegrity = %{Version}
Provides: cs_deleteGenerationInfoListByProducerId = %{Version}
Provides: cs_getContentList = %{Version}
Provides: cs_syncContentFromCsvToRedis = %{Version}
Provides: cs_deleteFileInfoListByProducerId = %{Version}
Provides: ds_getGridValueListByCircle = %{Version}
Provides: cs_getLastGenerationInfoByProducerNameAndStatus = %{Version}
Provides: cs_addFileAndContentListFromFile = %{Version}
Provides: cs_deleteGenerationInfoListBySourceId = %{Version}
Provides: cs_getContentListByServerId = %{Version}
Provides: cs_getFileInfoById = %{Version}
Provides: cs_deleteFileInfoListByGenerationIdAndForecastTime = %{Version}
Provides: cs_deleteFileInfoListByFileIdList = %{Version}
Provides: cs_deleteContentInfo = %{Version}
Provides: cs_getFileInfoListByProducerName = %{Version}
Provides: ds_getGridValueVectorByPoint = %{Version}
Provides: cs_getFileInfoByName = %{Version}
Provides: cs_getFileInfoListBySourceId = %{Version}
Provides: cs_getEventInfoList = %{Version}
Provides: cs_deleteFileInfoByName = %{Version}
Provides: cs_getLevelInfoList = %{Version}
Provides: cs_getProducerInfoListByParameter = %{Version}
Provides: ds_getGridValueList = %{Version}
Provides: cs_getEventInfoCount = %{Version}
Provides: cs_deleteProducerInfoListBySourceId = %{Version}
Provides: cs_getGenerationInfoListByGeometryId = %{Version}
Provides: cs_getContentListByFileId = %{Version}
Provides: cs_getLastEventInfo = %{Version}
Provides: cs_getContentListByGroupFlags = %{Version}
Provides: cs_getGenerationInfoById = %{Version}
Provides: ds_getGridValueVectorByRectangle = %{Version}
Provides: cs_getGenerationInfoListByProducerName = %{Version}
Provides: cs_syncContentFromCsvToHttp = %{Version}
Provides: cs_deleteFileInfoListBySourceId = %{Version}
Provides: cs_getContentListByProducerName = %{Version}
Provides: qs_getProducerList = %{Version}
Provides: cs_deleteFileInfoListByGenerationId = %{Version}
Provides: cs_syncContentFromCsv = %{Version}
Provides: ds_getGridValueByPoint = %{Version}
Provides: cs_setGenerationInfoStatusById = %{Version}
Provides: cs_deleteContentListByProducerId = %{Version}
Provides: cs_getContentListByGenerationName = %{Version}
Provides: cs_deleteContentListBySourceId = %{Version}
Provides: cs_deleteFileInfoListByGroupFlags = %{Version}
Provides: ds_getGridAttributeList = %{Version}
Provides: cs_getGenerationInfoCount = %{Version}
Provides: cs_getProducerInfoByName = %{Version}
Provides: cs_getProducerNameAndGeometryList = %{Version}
Provides: cs_deleteContentListByGroupFlags = %{Version}
Provides: cs_deleteContentListByGenerationName = %{Version}
Provides: cs_deleteContentListByFileId = %{Version}
Provides: cs_addContentInfo = %{Version}
Provides: cs_getFileInfoCountsByGenerations = %{Version}
Provides: cs_reload = %{Version}
Provides: cs_deleteFileInfoListByGenerationName = %{Version}
Provides: cs_getDataServerInfoList = %{Version}
Provides: cs_getContentTimeListByGenerationId = %{Version}
Provides: cs_getGenerationInfoByName = %{Version}
Provides: cs_setGenerationInfoStatusByName = %{Version}
Provides: cs_getContentGeometryIdListByGenerationId = %{Version}
Provides: ds_getGridFileCount = %{Version}
Provides: cs_deleteFileInfoListByProducerName = %{Version}
Provides: cs_getContentListByFileName = %{Version}
Provides: cs_getProducerInfoById = %{Version}
Provides: ds_getGridCoordinates = %{Version}
Provides: cs_addFileInfoListFromFile = %{Version}
Provides: cs_addFileInfo = %{Version}
Provides: cs_deleteGenerationInfoListByProducerName = %{Version}
Provides: cs_saveData = %{Version}
Provides: cs_getContentListByGenerationId = %{Version}
Provides: cs_getGenerationInfoListByProducerId = %{Version}
Provides: cs_addFileInfoWithContentList = %{Version}
Provides: cs_getFileInfoCountBySourceId = %{Version}
Provides: qs_getParameterValues = %{Version}
Provides: cs_deleteVirtualContent = %{Version}
Provides: cs_unregisterContentListByServerId = %{Version}
Provides: cs_deleteContentListByFileName = %{Version}
Provides: cs_getContentParamKeyListByGenerationId = %{Version}
Provides: cs_deleteProducerInfoByName = %{Version}
Provides: cs_getContentInfo = %{Version}
Provides: cs_addDataServerInfo = %{Version}
Provides: ds_getGridImageByArea = %{Version}
Provides: cs_getGenerationInfoList = %{Version}
Provides: ds_getGridValueListByPolygon = %{Version}
Provides: cs_syncContent = %{Version}
Provides: cs_deleteContentListByGenerationId = %{Version}
Provides: cs_getDataServerInfoCount = %{Version}
Provides: cs_getGenerationIdGeometryIdAndForecastTimeList = %{Version}
Provides: cs_deleteContentListByProducerName = %{Version}
Provides: cs_getContentTimeListByProducerId = %{Version}
Provides: cs_deleteGenerationInfoById = %{Version}
Provides: cs_getDataServerInfoById = %{Version}
Provides: cs_clear = %{Version}
Provides: ds_getGridMap = %{Version}
Provides: cs_getContentListByProducerId = %{Version}
Provides: cs_getParameterPreloadList = %{Version}
Provides: cs_getContentListByParameterAndGenerationId = %{Version}
Provides: cs_getGenerationInfoListBySourceId = %{Version}
Provides: cs_getFileInfoCount = %{Version}
Provides: cs_getProducerInfoListBySourceId = %{Version}
Provides: cs_getContentParamListByGenerationId = %{Version}
Provides: cs_updateVirtualContent = %{Version}
Provides: corbaDataServer = %{Version}
Provides: corbaQueryServer = %{Version}
Provides: corbaContentServer = %{Version}
Provides: httpContentServer = %{Version}
Provides: gu_getLatLonDistance = %{Version}
Provides: gu_executeLuaFunction = %{Version}
Provides: gu_newbase2fmi = %{Version}
Provides: gu_getEnlargedPolygon = %{Version}
Provides: gu_getGridGeometryIdListByLatLon = %{Version}
Provides: gu_getGridPointsInsidePolygon = %{Version}
Provides: gu_showConfigurationAttributes = %{Version}
Provides: gu_getGridPointsInsideCircle = %{Version}
Provides: gu_getLuaFunctions = %{Version}
Provides: gu_getGridPointsInsidePolygonPath = %{Version}
Provides: gu_replaceConfigurationAttributes = %{Version}
Provides: gu_getGridLatLonCoordinatesByGeometryId = %{Version}

%description
SmartMet tools for grid support

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n %{SPECNAME}

%build -q -n %{SPECNAME}
make %{_smp_mflags}

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(0775,root,root,0775)
%{_bindir}/fmi/radon2smartmet
%{_bindir}/fmi/filesys2smartmet
%{_bindir}/fmi/radon2config
%{_bindir}/fmi/radon2csv
%{_bindir}/files/grid_query
%{_bindir}/files/grid_info
%{_bindir}/files/grid_maps
%{_bindir}/files/grid_submaps
%{_bindir}/files/grid_images
%{_bindir}/files/grid_dump
%{_bindir}/files/grid_add
%{_bindir}/clients/cs_getCsvFiles
%{_bindir}/clients/cs_deleteGenerationInfoByName
%{_bindir}/clients/cs_getFileInfoCountByGenerationId
%{_bindir}/clients/cs_getContentCount
%{_bindir}/clients/cs_addProducerInfo
%{_bindir}/clients/cs_getFileInfoListByGenerationId
%{_bindir}/clients/cs_getContentListByParameterGenerationIdAndForecastTime
%{_bindir}/clients/cs_addGenerationInfo
%{_bindir}/clients/cs_getContentListByGenerationIdAndTimeRange
%{_bindir}/clients/cs_getFileInfoCountsByProducers
%{_bindir}/clients/cs_getContentTimeListByGenerationAndGeometryId
%{_bindir}/clients/cs_deleteDataServerInfoById
%{_bindir}/clients/cs_getFileInfoList
%{_bindir}/clients/cs_getProducerInfoList
%{_bindir}/clients/cs_getContentListByParameter
%{_bindir}/clients/cs_getFileInfoListByProducerId
%{_bindir}/clients/ds_getGridData
%{_bindir}/clients/cs_getFileInfoListByGenerationName
%{_bindir}/clients/cs_deleteFileInfoById
%{_bindir}/clients/cs_getProducerInfoCount
%{_bindir}/clients/cs_getContentListBySourceId
%{_bindir}/clients/ds_getGridImage
%{_bindir}/clients/cs_getLastGenerationInfoByProducerIdAndStatus
%{_bindir}/clients/ds_getGridValueListByRectangle
%{_bindir}/clients/cs_getProducerParameterList
%{_bindir}/clients/cs_deleteProducerInfoById
%{_bindir}/clients/cs_getFileInfoCountByProducerId
%{_bindir}/clients/cs_getContentListOfInvalidIntegrity
%{_bindir}/clients/cs_deleteGenerationInfoListByProducerId
%{_bindir}/clients/cs_getContentList
%{_bindir}/clients/cs_syncContentFromCsvToRedis
%{_bindir}/clients/cs_deleteFileInfoListByProducerId
%{_bindir}/clients/ds_getGridValueListByCircle
%{_bindir}/clients/cs_getLastGenerationInfoByProducerNameAndStatus
%{_bindir}/clients/cs_addFileAndContentListFromFile
%{_bindir}/clients/cs_deleteGenerationInfoListBySourceId
%{_bindir}/clients/cs_getContentListByServerId
%{_bindir}/clients/cs_getFileInfoById
%{_bindir}/clients/cs_deleteFileInfoListByGenerationIdAndForecastTime
%{_bindir}/clients/cs_deleteFileInfoListByFileIdList
%{_bindir}/clients/cs_deleteContentInfo
%{_bindir}/clients/cs_getFileInfoListByProducerName
%{_bindir}/clients/ds_getGridValueVectorByPoint
%{_bindir}/clients/cs_getFileInfoByName
%{_bindir}/clients/cs_getFileInfoListBySourceId
%{_bindir}/clients/cs_getEventInfoList
%{_bindir}/clients/cs_deleteFileInfoByName
%{_bindir}/clients/cs_getLevelInfoList
%{_bindir}/clients/cs_getProducerInfoListByParameter
%{_bindir}/clients/ds_getGridValueList
%{_bindir}/clients/cs_getEventInfoCount
%{_bindir}/clients/cs_deleteProducerInfoListBySourceId
%{_bindir}/clients/cs_getGenerationInfoListByGeometryId
%{_bindir}/clients/cs_getContentListByFileId
%{_bindir}/clients/cs_getLastEventInfo
%{_bindir}/clients/cs_getContentListByGroupFlags
%{_bindir}/clients/cs_getGenerationInfoById
%{_bindir}/clients/ds_getGridValueVectorByRectangle
%{_bindir}/clients/cs_getGenerationInfoListByProducerName
%{_bindir}/clients/cs_syncContentFromCsvToHttp
%{_bindir}/clients/cs_deleteFileInfoListBySourceId
%{_bindir}/clients/cs_getContentListByProducerName
%{_bindir}/clients/qs_getProducerList
%{_bindir}/clients/cs_deleteFileInfoListByGenerationId
%{_bindir}/clients/cs_syncContentFromCsv
%{_bindir}/clients/ds_getGridValueByPoint
%{_bindir}/clients/cs_setGenerationInfoStatusById
%{_bindir}/clients/cs_deleteContentListByProducerId
%{_bindir}/clients/cs_getContentListByGenerationName
%{_bindir}/clients/cs_deleteContentListBySourceId
%{_bindir}/clients/cs_deleteFileInfoListByGroupFlags
%{_bindir}/clients/ds_getGridAttributeList
%{_bindir}/clients/cs_getGenerationInfoCount
%{_bindir}/clients/cs_getProducerInfoByName
%{_bindir}/clients/cs_getProducerNameAndGeometryList
%{_bindir}/clients/cs_deleteContentListByGroupFlags
%{_bindir}/clients/cs_deleteContentListByGenerationName
%{_bindir}/clients/cs_deleteContentListByFileId
%{_bindir}/clients/cs_addContentInfo
%{_bindir}/clients/cs_getFileInfoCountsByGenerations
%{_bindir}/clients/cs_reload
%{_bindir}/clients/cs_deleteFileInfoListByGenerationName
%{_bindir}/clients/cs_getDataServerInfoList
%{_bindir}/clients/cs_getContentTimeListByGenerationId
%{_bindir}/clients/cs_getGenerationInfoByName
%{_bindir}/clients/cs_setGenerationInfoStatusByName
%{_bindir}/clients/cs_getContentGeometryIdListByGenerationId
%{_bindir}/clients/ds_getGridFileCount
%{_bindir}/clients/cs_deleteFileInfoListByProducerName
%{_bindir}/clients/cs_getContentListByFileName
%{_bindir}/clients/cs_getProducerInfoById
%{_bindir}/clients/ds_getGridCoordinates
%{_bindir}/clients/cs_addFileInfoListFromFile
%{_bindir}/clients/cs_addFileInfo
%{_bindir}/clients/cs_deleteGenerationInfoListByProducerName
%{_bindir}/clients/cs_saveData
%{_bindir}/clients/cs_getContentListByGenerationId
%{_bindir}/clients/cs_getGenerationInfoListByProducerId
%{_bindir}/clients/cs_addFileInfoWithContentList
%{_bindir}/clients/cs_getFileInfoCountBySourceId
%{_bindir}/clients/qs_getParameterValues
%{_bindir}/clients/cs_deleteVirtualContent
%{_bindir}/clients/cs_unregisterContentListByServerId
%{_bindir}/clients/cs_deleteContentListByFileName
%{_bindir}/clients/cs_getContentParamKeyListByGenerationId
%{_bindir}/clients/cs_deleteProducerInfoByName
%{_bindir}/clients/cs_getContentInfo
%{_bindir}/clients/cs_addDataServerInfo
%{_bindir}/clients/ds_getGridImageByArea
%{_bindir}/clients/cs_getGenerationInfoList
%{_bindir}/clients/ds_getGridValueListByPolygon
%{_bindir}/clients/cs_syncContent
%{_bindir}/clients/cs_deleteContentListByGenerationId
%{_bindir}/clients/cs_getDataServerInfoCount
%{_bindir}/clients/cs_getGenerationIdGeometryIdAndForecastTimeList
%{_bindir}/clients/cs_deleteContentListByProducerName
%{_bindir}/clients/cs_getContentTimeListByProducerId
%{_bindir}/clients/cs_deleteGenerationInfoById
%{_bindir}/clients/cs_getDataServerInfoById
%{_bindir}/clients/cs_clear
%{_bindir}/clients/ds_getGridMap
%{_bindir}/clients/cs_getContentListByProducerId
%{_bindir}/clients/cs_getParameterPreloadList
%{_bindir}/clients/cs_getContentListByParameterAndGenerationId
%{_bindir}/clients/cs_getGenerationInfoListBySourceId
%{_bindir}/clients/cs_getFileInfoCount
%{_bindir}/clients/cs_getProducerInfoListBySourceId
%{_bindir}/clients/cs_getContentParamListByGenerationId
%{_bindir}/clients/cs_updateVirtualContent
%{_bindir}/servers/corbaDataServer
%{_bindir}/servers/corbaQueryServer
%{_bindir}/servers/corbaContentServer
%{_bindir}/servers/httpContentServer
%{_bindir}/utils/gu_getLatLonDistance
%{_bindir}/utils/gu_executeLuaFunction
%{_bindir}/utils/gu_newbase2fmi
%{_bindir}/utils/gu_getEnlargedPolygon
%{_bindir}/utils/gu_getGridGeometryIdListByLatLon
%{_bindir}/utils/gu_getGridPointsInsidePolygon
%{_bindir}/utils/gu_showConfigurationAttributes
%{_bindir}/utils/gu_getGridPointsInsideCircle
%{_bindir}/utils/gu_getLuaFunctions
%{_bindir}/utils/gu_getGridPointsInsidePolygonPath
%{_bindir}/utils/gu_replaceConfigurationAttributes
%{_bindir}/utils/gu_getGridLatLonCoordinatesByGeometryId

%changelog
* Thu Jun 14 2018 Roope Tervo <roope.tervo@fmi.fi> - 18.6.14-1.fmi
- Initial build