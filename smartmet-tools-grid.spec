%define DIRNAME grid
%define SPECNAME smartmet-tools-%{DIRNAME}
Summary: SmartMet tools for grid support
Name: %{SPECNAME}
Version: 21.1.14
Release: 1%{?dist}.fmi
License: MIT
Group: SmartMet/Plugins
URL: https://github.com/fmidev/smartmet-plugin-grid-gui
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: rpm-build
BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: boost169-devel
BuildRequires: libconfig-devel
BuildRequires: smartmet-library-spine-devel >= 21.1.14
BuildRequires: smartmet-library-grid-files-devel >= 21.1.14
BuildRequires: smartmet-library-grid-content-devel >= 21.1.14
BuildRequires: gdal32-devel
BuildRequires: omniORB-devel
BuildRequires: libpqxx-devel
BuildRequires: libmicrohttpd-devel
BuildRequires: libjpeg-devel
BuildRequires: libpng-devel
BuildRequires: openldap-devel
BuildRequires: openssl-devel
BuildRequires: krb5-devel
Requires: libconfig
#Requires: smartmet-library-macgyver >= 18.2.6
#Requires: smartmet-library-spine >= 18.1.15
#Requires: smartmet-server >= 17.11.10
Requires: boost169-date-time
Requires: smartmet-library-grid-files >= 21.1.14
Requires: smartmet-library-grid-content >= 21.1.14
Requires: openldap
Requires: openssl-libs
Requires: krb5-devel

Provides: corbaContentServer = %{Version}
Provides: corbaDataServer = %{Version}
Provides: corbaGridServer = %{Version}
Provides: corbaQueryServer = %{Version}
Provides: cs_addContentInfo = %{Version}
Provides: cs_addFileAndContentListFromFile = %{Version}
Provides: cs_addFileInfo = %{Version}
Provides: cs_addFileInfoListFromFile = %{Version}
Provides: cs_addFileInfoWithContentList = %{Version}
Provides: cs_addGenerationInfo = %{Version}
Provides: cs_addProducerInfo = %{Version}
Provides: cs_clear = %{Version}
Provides: cs_deleteContentInfo = %{Version}
Provides: cs_deleteContentListByFileId = %{Version}
Provides: cs_deleteContentListByFileName = %{Version}
Provides: cs_deleteContentListByGenerationId = %{Version}
Provides: cs_deleteContentListByGenerationName = %{Version}
Provides: cs_deleteContentListByGroupFlags = %{Version}
Provides: cs_deleteContentListByProducerId = %{Version}
Provides: cs_deleteContentListByProducerName = %{Version}
Provides: cs_deleteContentListBySourceId = %{Version}
Provides: cs_deleteFileInfoById = %{Version}
Provides: cs_deleteFileInfoByName = %{Version}
Provides: cs_deleteFileInfoListByFileIdList = %{Version}
Provides: cs_deleteFileInfoListByGenerationId = %{Version}
Provides: cs_deleteFileInfoListByGenerationIdAndForecastTime = %{Version}
Provides: cs_deleteFileInfoListByGenerationName = %{Version}
Provides: cs_deleteFileInfoListByGroupFlags = %{Version}
Provides: cs_deleteFileInfoListByProducerId = %{Version}
Provides: cs_deleteFileInfoListByProducerName = %{Version}
Provides: cs_deleteFileInfoListBySourceId = %{Version}
Provides: cs_deleteGenerationInfoById = %{Version}
Provides: cs_deleteGenerationInfoByName = %{Version}
Provides: cs_deleteGenerationInfoListByProducerId = %{Version}
Provides: cs_deleteGenerationInfoListByProducerName = %{Version}
Provides: cs_deleteGenerationInfoListBySourceId = %{Version}
Provides: cs_deleteProducerInfoById = %{Version}
Provides: cs_deleteProducerInfoByName = %{Version}
Provides: cs_deleteProducerInfoListBySourceId = %{Version}
Provides: cs_deleteVirtualContent = %{Version}
Provides: cs_getContentCount = %{Version}
Provides: cs_getContentGeometryIdListByGenerationId = %{Version}
Provides: cs_getContentInfo = %{Version}
Provides: cs_getContentList = %{Version}
Provides: cs_getContentListByFileId = %{Version}
Provides: cs_getContentListByFileIdList = %{Version}
Provides: cs_getContentListByFileName = %{Version}
Provides: cs_getContentListByGenerationId = %{Version}
Provides: cs_getContentListByGenerationIdAndTimeRange = %{Version}
Provides: cs_getContentListByGenerationName = %{Version}
Provides: cs_getContentListByGroupFlags = %{Version}
Provides: cs_getContentListByParameter = %{Version}
Provides: cs_getContentListByParameterAndGenerationId = %{Version}
Provides: cs_getContentListByParameterAndProducerId = %{Version}
Provides: cs_getContentListByParameterGenerationIdAndForecastTime = %{Version}
Provides: cs_getContentListByProducerId = %{Version}
Provides: cs_getContentListByProducerName = %{Version}
Provides: cs_getContentListBySourceId = %{Version}
Provides: cs_getContentListOfInvalidIntegrity = %{Version}
Provides: cs_getContentParamKeyListByGenerationId = %{Version}
Provides: cs_getContentParamListByGenerationId = %{Version}
Provides: cs_getContentTimeListByGenerationAndGeometryId = %{Version}
Provides: cs_getContentTimeListByGenerationId = %{Version}
Provides: cs_getContentTimeListByProducerId = %{Version}
Provides: cs_getCsvFiles = %{Version}
Provides: cs_getEventInfoCount = %{Version}
Provides: cs_getEventInfoList = %{Version}
Provides: cs_getFileInfoById = %{Version}
Provides: cs_getFileInfoByName = %{Version}
Provides: cs_getFileInfoCount = %{Version}
Provides: cs_getFileInfoCountByGenerationId = %{Version}
Provides: cs_getFileInfoCountByProducerId = %{Version}
Provides: cs_getFileInfoCountBySourceId = %{Version}
Provides: cs_getFileInfoCountsByGenerations = %{Version}
Provides: cs_getFileInfoCountsByProducers = %{Version}
Provides: cs_getFileInfoList = %{Version}
Provides: cs_getFileInfoListByFileIdList = %{Version}
Provides: cs_getFileInfoListByGenerationId = %{Version}
Provides: cs_getFileInfoListByGenerationName = %{Version}
Provides: cs_getFileInfoListByProducerId = %{Version}
Provides: cs_getFileInfoListByProducerName = %{Version}
Provides: cs_getFileInfoListBySourceId = %{Version}
Provides: cs_getGenerationIdGeometryIdAndForecastTimeList = %{Version}
Provides: cs_getGenerationInfoById = %{Version}
Provides: cs_getGenerationInfoByName = %{Version}
Provides: cs_getGenerationInfoCount = %{Version}
Provides: cs_getGenerationInfoList = %{Version}
Provides: cs_getGenerationInfoListByGeometryId = %{Version}
Provides: cs_getGenerationInfoListByProducerId = %{Version}
Provides: cs_getGenerationInfoListByProducerName = %{Version}
Provides: cs_getGenerationInfoListBySourceId = %{Version}
Provides: cs_getHashByProducerId = %{Version}
Provides: cs_getLastEventInfo = %{Version}
Provides: cs_getLastGenerationInfoByProducerIdAndStatus = %{Version}
Provides: cs_getLastGenerationInfoByProducerNameAndStatus = %{Version}
Provides: cs_getLevelInfoList = %{Version}
Provides: cs_getParameterPreloadList = %{Version}
Provides: cs_getProducerInfoById = %{Version}
Provides: cs_getProducerInfoByName = %{Version}
Provides: cs_getProducerInfoCount = %{Version}
Provides: cs_getProducerInfoList = %{Version}
Provides: cs_getProducerInfoListByParameter = %{Version}
Provides: cs_getProducerInfoListBySourceId = %{Version}
Provides: cs_getProducerNameAndGeometryList = %{Version}
Provides: cs_getProducerParameterList = %{Version}
Provides: cs_getProducerParameterListByProducerId = %{Version}
Provides: cs_reload = %{Version}
Provides: cs_saveData = %{Version}
Provides: cs_setGenerationInfoStatusById = %{Version}
Provides: cs_setGenerationInfoStatusByName = %{Version}
Provides: cs_syncContent = %{Version}
Provides: cs_syncContentFromCsv = %{Version}
Provides: cs_syncContentFromCsvToHttp = %{Version}
Provides: cs_syncContentFromCsvToRedis = %{Version}
Provides: cs_updateVirtualContent = %{Version}
Provides: ds_getGridAttributeList = %{Version}
Provides: ds_getGridCoordinates = %{Version}
Provides: ds_getGridData = %{Version}
Provides: ds_getGridFileCount = %{Version}
Provides: ds_getGridImage = %{Version}
Provides: ds_getGridImageByArea = %{Version}
Provides: ds_getGridIsobandImage = %{Version}
Provides: ds_getGridIsobandImageByGeometry = %{Version}
Provides: ds_getGridIsobandImageByTime = %{Version}
Provides: ds_getGridIsobandImageByTimeAndGeometry = %{Version}
Provides: ds_getGridIsobandMap = %{Version}
Provides: ds_getGridIsolineImage = %{Version}
Provides: ds_getGridIsolineImageByGeometry = %{Version}
Provides: ds_getGridIsolineImageByTime = %{Version}
Provides: ds_getGridIsolineImageByTimeAndGeometry = %{Version}
Provides: ds_getGridIsolineMap = %{Version}
Provides: ds_getGridMap = %{Version}
Provides: ds_getGridValueByLevelAndPoint = %{Version}
Provides: ds_getGridValueByPoint = %{Version}
Provides: ds_getGridValueByTimeAndPoint = %{Version}
Provides: ds_getGridValueByTimeLevelAndPoint = %{Version}
Provides: ds_getGridValueList = %{Version}
Provides: ds_getGridValueListByCircle = %{Version}
Provides: ds_getGridValueListByPolygon = %{Version}
Provides: ds_getGridValueListByRectangle = %{Version}
Provides: ds_getGridValueListByTimeAndCircle = %{Version}
Provides: ds_getGridValueListByTimeAndPolygon = %{Version}
Provides: ds_getGridValueVector = %{Version}
Provides: ds_getGridValueVectorByPoint = %{Version}
Provides: ds_getGridValueVectorByRectangle = %{Version}
Provides: ds_getGridValueVectorByTime = %{Version}
Provides: filesys2smartmet = %{Version}
Provides: generateFmigFiles = %{Version}
Provides: grid_add = %{Version}
Provides: grid_create = %{Version}
Provides: grid_dump = %{Version}
Provides: grid_getIsobandImage = %{Version}
Provides: grid_getIsolineImage = %{Version}
Provides: grid_images = %{Version}
Provides: grid_info = %{Version}
Provides: grid_maps = %{Version}
Provides: grid_query = %{Version}
Provides: grid_submaps = %{Version}
Provides: gu_executeLuaFunction = %{Version}
Provides: gu_getEnlargedPolygon = %{Version}
Provides: gu_getGridGeometryIdListByLatLon = %{Version}
Provides: gu_getGridLatLonCoordinatesByGeometryId = %{Version}
Provides: gu_getGridPointsInsideCircle = %{Version}
Provides: gu_getGridPointsInsidePolygon = %{Version}
Provides: gu_getGridPointsInsidePolygonPath = %{Version}
Provides: gu_getLatLonDistance = %{Version}
Provides: gu_getLuaFunctions = %{Version}
Provides: gu_mergePngFiles = %{Version}
Provides: gu_mergePngFilesSeq = %{Version}
Provides: gu_newbase2fmi = %{Version}
Provides: gu_replaceConfigurationAttributes = %{Version}
Provides: gu_showConfigurationAttributes = %{Version}
Provides: httpContentServer = %{Version}
Provides: qs_getParameterIsobands = %{Version}
Provides: qs_getParameterIsolines = %{Version}
Provides: qs_getParameterValueByPointAndTimeList = %{Version}
Provides: qs_getParameterValues = %{Version}
Provides: qs_getParameterValuesByPointAndTimeList = %{Version}
Provides: qs_getProducerList = %{Version}
Provides: radon2config = %{Version}
Provides: radon2csv = %{Version}
Provides: radon2smartmet = %{Version}

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
%{_bindir}/clients/cs_addContentInfo
%{_bindir}/clients/cs_addFileAndContentListFromFile
%{_bindir}/clients/cs_addFileInfo
%{_bindir}/clients/cs_addFileInfoListFromFile
%{_bindir}/clients/cs_addFileInfoWithContentList
%{_bindir}/clients/cs_addGenerationInfo
%{_bindir}/clients/cs_addProducerInfo
%{_bindir}/clients/cs_clear
%{_bindir}/clients/cs_deleteContentInfo
%{_bindir}/clients/cs_deleteContentListByFileId
%{_bindir}/clients/cs_deleteContentListByFileName
%{_bindir}/clients/cs_deleteContentListByGenerationId
%{_bindir}/clients/cs_deleteContentListByGenerationName
%{_bindir}/clients/cs_deleteContentListByGroupFlags
%{_bindir}/clients/cs_deleteContentListByProducerId
%{_bindir}/clients/cs_deleteContentListByProducerName
%{_bindir}/clients/cs_deleteContentListBySourceId
%{_bindir}/clients/cs_deleteFileInfoById
%{_bindir}/clients/cs_deleteFileInfoByName
%{_bindir}/clients/cs_deleteFileInfoListByFileIdList
%{_bindir}/clients/cs_deleteFileInfoListByGenerationId
%{_bindir}/clients/cs_deleteFileInfoListByGenerationIdAndForecastTime
%{_bindir}/clients/cs_deleteFileInfoListByGenerationName
%{_bindir}/clients/cs_deleteFileInfoListByGroupFlags
%{_bindir}/clients/cs_deleteFileInfoListByProducerId
%{_bindir}/clients/cs_deleteFileInfoListByProducerName
%{_bindir}/clients/cs_deleteFileInfoListBySourceId
%{_bindir}/clients/cs_deleteGenerationInfoById
%{_bindir}/clients/cs_deleteGenerationInfoByName
%{_bindir}/clients/cs_deleteGenerationInfoListByProducerId
%{_bindir}/clients/cs_deleteGenerationInfoListByProducerName
%{_bindir}/clients/cs_deleteGenerationInfoListBySourceId
%{_bindir}/clients/cs_deleteProducerInfoById
%{_bindir}/clients/cs_deleteProducerInfoByName
%{_bindir}/clients/cs_deleteProducerInfoListBySourceId
%{_bindir}/clients/cs_deleteVirtualContent
%{_bindir}/clients/cs_getContentCount
%{_bindir}/clients/cs_getContentGeometryIdListByGenerationId
%{_bindir}/clients/cs_getContentInfo
%{_bindir}/clients/cs_getContentList
%{_bindir}/clients/cs_getContentListByFileId
%{_bindir}/clients/cs_getContentListByFileIdList
%{_bindir}/clients/cs_getContentListByFileName
%{_bindir}/clients/cs_getContentListByGenerationId
%{_bindir}/clients/cs_getContentListByGenerationIdAndTimeRange
%{_bindir}/clients/cs_getContentListByGenerationName
%{_bindir}/clients/cs_getContentListByGroupFlags
%{_bindir}/clients/cs_getContentListByParameter
%{_bindir}/clients/cs_getContentListByParameterAndGenerationId
%{_bindir}/clients/cs_getContentListByParameterAndProducerId
%{_bindir}/clients/cs_getContentListByParameterGenerationIdAndForecastTime
%{_bindir}/clients/cs_getContentListByProducerId
%{_bindir}/clients/cs_getContentListByProducerName
%{_bindir}/clients/cs_getContentListBySourceId
%{_bindir}/clients/cs_getContentListOfInvalidIntegrity
%{_bindir}/clients/cs_getContentParamKeyListByGenerationId
%{_bindir}/clients/cs_getContentParamListByGenerationId
%{_bindir}/clients/cs_getContentTimeListByGenerationAndGeometryId
%{_bindir}/clients/cs_getContentTimeListByGenerationId
%{_bindir}/clients/cs_getContentTimeListByProducerId
%{_bindir}/clients/cs_getCsvFiles
%{_bindir}/clients/cs_getEventInfoCount
%{_bindir}/clients/cs_getEventInfoList
%{_bindir}/clients/cs_getFileInfoById
%{_bindir}/clients/cs_getFileInfoByName
%{_bindir}/clients/cs_getFileInfoCount
%{_bindir}/clients/cs_getFileInfoCountByGenerationId
%{_bindir}/clients/cs_getFileInfoCountByProducerId
%{_bindir}/clients/cs_getFileInfoCountBySourceId
%{_bindir}/clients/cs_getFileInfoCountsByGenerations
%{_bindir}/clients/cs_getFileInfoCountsByProducers
%{_bindir}/clients/cs_getFileInfoList
%{_bindir}/clients/cs_getFileInfoListByFileIdList
%{_bindir}/clients/cs_getFileInfoListByGenerationId
%{_bindir}/clients/cs_getFileInfoListByGenerationName
%{_bindir}/clients/cs_getFileInfoListByProducerId
%{_bindir}/clients/cs_getFileInfoListByProducerName
%{_bindir}/clients/cs_getFileInfoListBySourceId
%{_bindir}/clients/cs_getGenerationIdGeometryIdAndForecastTimeList
%{_bindir}/clients/cs_getGenerationInfoById
%{_bindir}/clients/cs_getGenerationInfoByName
%{_bindir}/clients/cs_getGenerationInfoCount
%{_bindir}/clients/cs_getGenerationInfoList
%{_bindir}/clients/cs_getGenerationInfoListByGeometryId
%{_bindir}/clients/cs_getGenerationInfoListByProducerId
%{_bindir}/clients/cs_getGenerationInfoListByProducerName
%{_bindir}/clients/cs_getGenerationInfoListBySourceId
%{_bindir}/clients/cs_getHashByProducerId
%{_bindir}/clients/cs_getLastEventInfo
%{_bindir}/clients/cs_getLastGenerationInfoByProducerIdAndStatus
%{_bindir}/clients/cs_getLastGenerationInfoByProducerNameAndStatus
%{_bindir}/clients/cs_getLevelInfoList
%{_bindir}/clients/cs_getParameterPreloadList
%{_bindir}/clients/cs_getProducerInfoById
%{_bindir}/clients/cs_getProducerInfoByName
%{_bindir}/clients/cs_getProducerInfoCount
%{_bindir}/clients/cs_getProducerInfoList
%{_bindir}/clients/cs_getProducerInfoListByParameter
%{_bindir}/clients/cs_getProducerInfoListBySourceId
%{_bindir}/clients/cs_getProducerNameAndGeometryList
%{_bindir}/clients/cs_getProducerParameterList
%{_bindir}/clients/cs_getProducerParameterListByProducerId
%{_bindir}/clients/cs_reload
%{_bindir}/clients/cs_saveData
%{_bindir}/clients/cs_setGenerationInfoStatusById
%{_bindir}/clients/cs_setGenerationInfoStatusByName
%{_bindir}/clients/cs_syncContent
%{_bindir}/clients/cs_syncContentFromCsv
%{_bindir}/clients/cs_syncContentFromCsvToHttp
%{_bindir}/clients/cs_syncContentFromCsvToRedis
%{_bindir}/clients/cs_updateVirtualContent
%{_bindir}/clients/ds_getGridAttributeList
%{_bindir}/clients/ds_getGridCoordinates
%{_bindir}/clients/ds_getGridData
%{_bindir}/clients/ds_getGridFileCount
%{_bindir}/clients/ds_getGridImage
%{_bindir}/clients/ds_getGridImageByArea
%{_bindir}/clients/ds_getGridIsobandImage
%{_bindir}/clients/ds_getGridIsobandImageByGeometry
%{_bindir}/clients/ds_getGridIsobandImageByTime
%{_bindir}/clients/ds_getGridIsobandImageByTimeAndGeometry
%{_bindir}/clients/ds_getGridIsobandMap
%{_bindir}/clients/ds_getGridIsolineImage
%{_bindir}/clients/ds_getGridIsolineImageByGeometry
%{_bindir}/clients/ds_getGridIsolineImageByTime
%{_bindir}/clients/ds_getGridIsolineImageByTimeAndGeometry
%{_bindir}/clients/ds_getGridIsolineMap
%{_bindir}/clients/ds_getGridMap
%{_bindir}/clients/ds_getGridValueByLevelAndPoint
%{_bindir}/clients/ds_getGridValueByPoint
%{_bindir}/clients/ds_getGridValueByTimeAndPoint
%{_bindir}/clients/ds_getGridValueByTimeLevelAndPoint
%{_bindir}/clients/ds_getGridValueList
%{_bindir}/clients/ds_getGridValueListByCircle
%{_bindir}/clients/ds_getGridValueListByPolygon
%{_bindir}/clients/ds_getGridValueListByRectangle
%{_bindir}/clients/ds_getGridValueListByTimeAndCircle
%{_bindir}/clients/ds_getGridValueListByTimeAndPolygon
%{_bindir}/clients/ds_getGridValueVector
%{_bindir}/clients/ds_getGridValueVectorByPoint
%{_bindir}/clients/ds_getGridValueVectorByRectangle
%{_bindir}/clients/ds_getGridValueVectorByTime
%{_bindir}/clients/qs_getParameterIsobands
%{_bindir}/clients/qs_getParameterIsolines
%{_bindir}/clients/qs_getParameterValueByPointAndTime
%{_bindir}/clients/qs_getParameterValues
%{_bindir}/clients/qs_getParameterValuesByPointAndTimeList
%{_bindir}/clients/qs_getProducerList
%{_bindir}/files/grid_add
%{_bindir}/files/grid_create
%{_bindir}/files/grid_dump
%{_bindir}/files/grid_getIsobandImage
%{_bindir}/files/grid_getIsolineImage
%{_bindir}/files/grid_images
%{_bindir}/files/grid_info
%{_bindir}/files/grid_maps
%{_bindir}/files/grid_query
%{_bindir}/files/grid_submaps
%{_bindir}/fmi/filesys2smartmet
%{_bindir}/fmi/generateFmigFiles
%{_bindir}/fmi/radon2config
%{_bindir}/fmi/radon2csv
%{_bindir}/fmi/radon2smartmet
%{_bindir}/servers/corbaContentServer
%{_bindir}/servers/corbaDataServer
%{_bindir}/servers/corbaGridServer
%{_bindir}/servers/corbaQueryServer
%{_bindir}/servers/httpContentServer
%{_bindir}/utils/gu_executeLuaFunction
%{_bindir}/utils/gu_getEnlargedPolygon
%{_bindir}/utils/gu_getGridGeometryIdListByLatLon
%{_bindir}/utils/gu_getGridLatLonCoordinatesByGeometryId
%{_bindir}/utils/gu_getGridPointsInsideCircle
%{_bindir}/utils/gu_getGridPointsInsidePolygon
%{_bindir}/utils/gu_getGridPointsInsidePolygonPath
%{_bindir}/utils/gu_getLatLonDistance
%{_bindir}/utils/gu_getLuaFunctions
%{_bindir}/utils/gu_mergePngFiles
%{_bindir}/utils/gu_mergePngFilesSeq
%{_bindir}/utils/gu_newbase2fmi
%{_bindir}/utils/gu_replaceConfigurationAttributes
%{_bindir}/utils/gu_showConfigurationAttributes


%defattr(0644,root,root,0755)
%config(noreplace) %{_unitdir}/radon2smartmet.service

# We do not enable the service by default, it must be done manually

%preun
if [ $1 -eq 0 ]; then
   systemctl stop radon2smartmet
   systemctl disable radon2smartmet
fi

%changelog
* Thu Jan 14 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.14-1.fmi
- Repackaged smartmet to resolve debuginfo issues
* Mon Jan 11 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.11-1.fmi
- Repackaged due to grid-files API changes
* Mon Jan  4 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.4-1.fmi
- Repackaged due to base library API changes
* Thu Dec  3 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.12.3-1.fmi
- Added a setting to control redis locking
* Mon Nov 30 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.11.30-1.fmi
- Repackaged due to grid-content library API changes
* Tue Nov 24 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.11.24-1.fmi
- Fixed problem related to different 'time_period' formats in different tables
* Thu Oct 22 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.22-1.fmi
- Minor improvements
- Removed cs_getContentListByRequestCounterKey
* Thu Oct 15 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.15-1.fmi
- Repackaged due to library ABI changes
* Wed Oct  7 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.7-1.fmi
- Removed cs_addDataServerInfo, cs_deleteDataServerInfoById, cs_getContentListByServerId, cs_getDataServerInfoById, cs_getDataServerInfoCount, cs_getDataServerInfoList and cs_unregisterContentListByServerId
* Thu Oct  1 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.1-1.fmi
- Repackaged due to library ABI changes
* Wed Sep 23 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.23-1.fmi
- Use Fmi::Exception instead of Spine::Exception
* Fri Sep 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.18-1.fmi
- Minor improvements
* Tue Sep 15 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.15-1.fmi
- Minor improvements, code refactoring
* Mon Sep 14 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.14-1.fmi
- Repackaged due to library ABI changes
* Mon Sep  7 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.7-1.fmi
- Updated grid coordinate processing
* Mon Aug 31 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.31-1.fmi
- Repackaged due to library ABI changes
* Fri Aug 21 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.21-1.fmi
- Upgrade to fmt 6.2
* Tue Aug 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.18-1.fmi
- Repackaged due to grid library ABI changes
* Fri Aug 14 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.14-1.fmi
- Repackaged due to grid-libraries ABI changes
* Mon Jun  8 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.6.8-1.fmi
- Added forecast number handling
* Fri May 15 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.5.15-1.fmi
- Added memoryMapCheckEnabled settings
* Thu Apr 30 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.30-1.fmi
- Added integrated corba-server
* Sat Apr 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.18-1.fmi
- Upgraded to Boost 1.69
* Fri Apr  3 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.3-1.fmi
- Created a program for generating fast grid files suitable for timeseries queries
- New configuration parameters
* Tue Mar 10 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.3.10-1.fmi
- Preloading is now configurable
* Thu Mar  5 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.3.5-1.fmi
- Minor fixes
* Tue Feb 25 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.2.25-1.fmi
- Added new configuration attributes
- Setting generation status to ready when its content addition is complete
* Wed Feb 19 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.2.19-1.fmi
- Added preloading functionality
* Wed Jan 29 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.1.29-1.fmi
- Reduced memory use of radon2smartmet
* Tue Jan 21 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.1.21-1.fmi
- Reduced memory consumption
* Thu Jan 16 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.1.16-1.fmi
- Added event list size configuration parameter
* Thu Jan  2 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.1.2-1.fmi
- radon2smartmet fixes
* Wed Dec 11 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.12.11-1.fmi
- Recompiled due to API changes in grid libraries
* Wed Dec  4 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.12.4-1.fmi
- Repackaged due to base library changes
* Fri Nov 22 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.22-1.fmi
- Repackaged due to API changes in grid-content library
* Wed Nov 20 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.20-1.fmi
- Several improvements to isoband, isoline and timeseries commands
* Thu Nov  7 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.7-1.fmi
- Repackaged due to library changes
* Fri Oct 25 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.10.25-1.fmi
- Added message location and size information
* Tue Oct  1 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.10.1-1.fmi
- Repackaged due to SmartMet library ABI changes
* Thu Sep 19 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.9.19-1.fmi
- New release version
* Mon Aug 12 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.8.12-1.fmi
- Added modification time to data content
* Fri Aug  9 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.8.9-1.fmi
- Numerous improvements
* Mon May  6 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.5.6-1.fmi
- Support for GRIB downloads
* Tue Apr  2 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.4.2-1.fmi
- New release version
* Tue Mar 19 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.3.19-1.fmi
- Repackaged due to grid-files header changes
* Fri Mar 15 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.3.15-1.fmi
- Various improvements
* Fri Feb 15 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.15-1.fmi
- Various improvements
* Thu Jan 17 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.1.17-1.fmi
- Various improvements
* Mon Nov 12 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.11.12-1.fmi
- Added a systemd script
* Thu Oct 25 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.10.25-1.fmi
- Repackaged due to library API changes
* Mon Oct 15 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.10.15-1.fmi
- Small improvements
* Mon Oct  1 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.10.1-1.fmi
- Version update
* Wed Sep 26 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.9.26-1.fmi
- Version update
* Mon Sep 10 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.9.10-1.fmi
- Version update
* Thu Aug 30 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.30-1.fmi
- Silenced CodeChecker warnings
* Tue Aug 28 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.28-1.fmi
- Packaged latest version
* Mon Aug 27 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.27-1.fmi
- Packaged latest version
* Thu Jun 14 2018 Roope Tervo <roope.tervo@fmi.fi> - 18.6.14-1.fmi
- Initial build
