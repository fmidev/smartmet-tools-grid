%define DIRNAME grid
%define SPECNAME smartmet-tools-%{DIRNAME}
Summary: SmartMet tools for grid support
Name: %{SPECNAME}
Version: 21.12.2
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
BuildRequires: smartmet-library-spine-devel >= 21.12.2
BuildRequires: smartmet-library-grid-files-devel >= 21.12.2
BuildRequires: smartmet-library-grid-content-devel >= 21.12.2
BuildRequires: gdal33-devel
BuildRequires: postgresql13-devel
BuildRequires: omniORB-devel
BuildRequires: libpqxx-devel
BuildRequires: libmicrohttpd-devel
BuildRequires: libjpeg-devel
BuildRequires: libpng-devel
BuildRequires: openldap-devel
BuildRequires: openssl-devel
BuildRequires: krb5-devel
#Requires: smartmet-library-macgyver >= 18.12.1
#Requires: smartmet-library-spine >= 18.12.2
#Requires: smartmet-server >= 17.11.10
Requires: boost169-date-time
Requires: smartmet-library-grid-files >= 21.12.2
Requires: smartmet-library-grid-content >= 21.12.2
Requires: openldap
Requires: openssl-libs
Requires: krb5-devel
#TestRequires: smartmet-utils-devel >= 21.11.23

Provides: corbaContentServer = %{version}
Provides: corbaDataServer = %{version}
Provides: corbaGridServer = %{version}
Provides: corbaQueryServer = %{version}
Provides: cs_addContentInfo = %{version}
Provides: cs_addFileAndContentListFromFile = %{version}
Provides: cs_addFileInfo = %{version}
Provides: cs_addFileInfoListFromFile = %{version}
Provides: cs_addFileInfoWithContentList = %{version}
Provides: cs_addGenerationInfo = %{version}
Provides: cs_addProducerInfo = %{version}
Provides: cs_clear = %{version}
Provides: cs_deleteContentInfo = %{version}
Provides: cs_deleteContentListByFileId = %{version}
Provides: cs_deleteContentListByFileName = %{version}
Provides: cs_deleteContentListByGenerationId = %{version}
Provides: cs_deleteContentListByGenerationName = %{version}
Provides: cs_deleteContentListByProducerId = %{version}
Provides: cs_deleteContentListByProducerName = %{version}
Provides: cs_deleteContentListBySourceId = %{version}
Provides: cs_deleteFileInfoById = %{version}
Provides: cs_deleteFileInfoByName = %{version}
Provides: cs_deleteFileInfoListByFileIdList = %{version}
Provides: cs_deleteFileInfoListByGenerationId = %{version}
Provides: cs_deleteFileInfoListByGenerationIdAndForecastTime = %{version}
Provides: cs_deleteFileInfoListByGenerationName = %{version}
Provides: cs_deleteFileInfoListByProducerId = %{version}
Provides: cs_deleteFileInfoListByProducerName = %{version}
Provides: cs_deleteFileInfoListBySourceId = %{version}
Provides: cs_deleteGenerationInfoById = %{version}
Provides: cs_deleteGenerationInfoByName = %{version}
Provides: cs_deleteGenerationInfoListByProducerId = %{version}
Provides: cs_deleteGenerationInfoListByProducerName = %{version}
Provides: cs_deleteGenerationInfoListBySourceId = %{version}
Provides: cs_deleteProducerInfoById = %{version}
Provides: cs_deleteProducerInfoByName = %{version}
Provides: cs_deleteProducerInfoListBySourceId = %{version}
Provides: cs_deleteVirtualContent = %{version}
Provides: cs_getContentCount = %{version}
Provides: cs_getContentGeometryIdListByGenerationId = %{version}
Provides: cs_getContentInfo = %{version}
Provides: cs_getContentList = %{version}
Provides: cs_getContentListByFileId = %{version}
Provides: cs_getContentListByFileIdList = %{version}
Provides: cs_getContentListByFileName = %{version}
Provides: cs_getContentListByGenerationId = %{version}
Provides: cs_getContentListByGenerationIdAndTimeRange = %{version}
Provides: cs_getContentListByGenerationName = %{version}
Provides: cs_getContentListByParameter = %{version}
Provides: cs_getContentListByParameterAndGenerationId = %{version}
Provides: cs_getContentListByParameterAndProducerId = %{version}
Provides: cs_getContentListByParameterGenerationIdAndForecastTime = %{version}
Provides: cs_getContentListByProducerId = %{version}
Provides: cs_getContentListByProducerName = %{version}
Provides: cs_getContentListBySourceId = %{version}
Provides: cs_getContentListOfInvalidIntegrity = %{version}
Provides: cs_getContentParamKeyListByGenerationId = %{version}
Provides: cs_getContentParamListByGenerationId = %{version}
Provides: cs_getContentTimeListByGenerationAndGeometryId = %{version}
Provides: cs_getContentTimeListByGenerationId = %{version}
Provides: cs_getContentTimeListByProducerId = %{version}
Provides: cs_getCsvFiles = %{version}
Provides: cs_getEventInfoCount = %{version}
Provides: cs_getEventInfoList = %{version}
Provides: cs_getFileInfoById = %{version}
Provides: cs_getFileInfoByName = %{version}
Provides: cs_getFileInfoCount = %{version}
Provides: cs_getFileInfoCountByGenerationId = %{version}
Provides: cs_getFileInfoCountByProducerId = %{version}
Provides: cs_getFileInfoCountBySourceId = %{version}
Provides: cs_getFileInfoCountsByGenerations = %{version}
Provides: cs_getFileInfoCountsByProducers = %{version}
Provides: cs_getFileInfoList = %{version}
Provides: cs_getFileInfoListByFileIdList = %{version}
Provides: cs_getFileInfoListByGenerationId = %{version}
Provides: cs_getFileInfoListByGenerationName = %{version}
Provides: cs_getFileInfoListByProducerId = %{version}
Provides: cs_getFileInfoListByProducerName = %{version}
Provides: cs_getFileInfoListBySourceId = %{version}
Provides: cs_getGenerationIdGeometryIdAndForecastTimeList = %{version}
Provides: cs_getGenerationInfoById = %{version}
Provides: cs_getGenerationInfoByName = %{version}
Provides: cs_getGenerationInfoCount = %{version}
Provides: cs_getGenerationInfoList = %{version}
Provides: cs_getGenerationInfoListByGeometryId = %{version}
Provides: cs_getGenerationInfoListByProducerId = %{version}
Provides: cs_getGenerationInfoListByProducerName = %{version}
Provides: cs_getGenerationInfoListBySourceId = %{version}
Provides: cs_getHashByProducerId = %{version}
Provides: cs_getLastEventInfo = %{version}
Provides: cs_getLastGenerationInfoByProducerIdAndStatus = %{version}
Provides: cs_getLastGenerationInfoByProducerNameAndStatus = %{version}
Provides: cs_getLevelInfoList = %{version}
Provides: cs_getParameterPreloadList = %{version}
Provides: cs_getProducerInfoById = %{version}
Provides: cs_getProducerInfoByName = %{version}
Provides: cs_getProducerInfoCount = %{version}
Provides: cs_getProducerInfoList = %{version}
Provides: cs_getProducerInfoListByParameter = %{version}
Provides: cs_getProducerInfoListBySourceId = %{version}
Provides: cs_getProducerNameAndGeometryList = %{version}
Provides: cs_getProducerParameterList = %{version}
Provides: cs_getProducerParameterListByProducerId = %{version}
Provides: cs_reload = %{version}
Provides: cs_saveData = %{version}
Provides: cs_setGenerationInfoStatusById = %{version}
Provides: cs_setGenerationInfoStatusByName = %{version}
Provides: cs_syncContent = %{version}
Provides: cs_syncContentFromCsv = %{version}
Provides: cs_syncContentFromCsvToHttp = %{version}
Provides: cs_syncContentFromCsvToRedis = %{version}
Provides: cs_updateVirtualContent = %{version}
Provides: ds_getGridAttributeList = %{version}
Provides: ds_getGridCoordinates = %{version}
Provides: ds_getGridData = %{version}
Provides: ds_getGridFileCount = %{version}
Provides: ds_getGridImage = %{version}
Provides: ds_getGridImageByArea = %{version}
Provides: ds_getGridIsobandImage = %{version}
Provides: ds_getGridIsobandImageByGeometry = %{version}
Provides: ds_getGridIsobandImageByTime = %{version}
Provides: ds_getGridIsobandImageByTimeAndGeometry = %{version}
Provides: ds_getGridIsobandMap = %{version}
Provides: ds_getGridIsolineImage = %{version}
Provides: ds_getGridIsolineImageByGeometry = %{version}
Provides: ds_getGridIsolineImageByTime = %{version}
Provides: ds_getGridIsolineImageByTimeAndGeometry = %{version}
Provides: ds_getGridIsolineMap = %{version}
Provides: ds_getGridMap = %{version}
Provides: ds_getGridValueByLevelAndPoint = %{version}
Provides: ds_getGridValueByPoint = %{version}
Provides: ds_getGridValueByTimeAndPoint = %{version}
Provides: ds_getGridValueByTimeLevelAndPoint = %{version}
Provides: ds_getGridValueList = %{version}
Provides: ds_getGridValueListByCircle = %{version}
Provides: ds_getGridValueListByPolygon = %{version}
Provides: ds_getGridValueListByRectangle = %{version}
Provides: ds_getGridValueListByTimeAndCircle = %{version}
Provides: ds_getGridValueListByTimeAndPolygon = %{version}
Provides: ds_getGridValueVector = %{version}
Provides: ds_getGridValueVectorByPoint = %{version}
Provides: ds_getGridValueVectorByRectangle = %{version}
Provides: ds_getGridValueVectorByTime = %{version}
Provides: filesys2smartmet = %{version}
Provides: generateFmigFiles = %{version}
Provides: grid_add = %{version}
Provides: grid_create = %{version}
Provides: grid_dump = %{version}
Provides: grid_getIsobandImage = %{version}
Provides: grid_getIsolineImage = %{version}
Provides: grid_images = %{version}
Provides: grid_info = %{version}
Provides: grid_maps = %{version}
Provides: grid_query = %{version}
Provides: grid_submaps = %{version}
Provides: gu_executeLuaFunction = %{version}
Provides: gu_getEnlargedPolygon = %{version}
Provides: gu_getGridGeometryIdListByLatLon = %{version}
Provides: gu_getGridLatLonCoordinatesByGeometryId = %{version}
Provides: gu_getGridPointsInsideCircle = %{version}
Provides: gu_getGridPointsInsidePolygon = %{version}
Provides: gu_getGridPointsInsidePolygonPath = %{version}
Provides: gu_getLatLonDistance = %{version}
Provides: gu_getLuaFunctions = %{version}
Provides: gu_mergePngFiles = %{version}
Provides: gu_mergePngFilesSeq = %{version}
Provides: gu_newbase2fmi = %{version}
Provides: gu_replaceConfigurationAttributes = %{version}
Provides: gu_showConfigurationAttributes = %{version}
Provides: httpContentServer = %{version}
Provides: qs_getParameterIsobands = %{version}
Provides: qs_getParameterIsolines = %{version}
Provides: qs_getParameterValueByPointAndTimeList = %{version}
Provides: qs_getParameterValues = %{version}
Provides: qs_getParameterValuesByPointAndTimeList = %{version}
Provides: qs_getProducerList = %{version}
Provides: radon2config = %{version}
Provides: radon2csv = %{version}
Provides: radon2smartmet = %{version}

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
%{_bindir}/clients/cs_deleteContentListByProducerId
%{_bindir}/clients/cs_deleteContentListByProducerName
%{_bindir}/clients/cs_deleteContentListBySourceId
%{_bindir}/clients/cs_deleteFileInfoById
%{_bindir}/clients/cs_deleteFileInfoByName
%{_bindir}/clients/cs_deleteFileInfoListByFileIdList
%{_bindir}/clients/cs_deleteFileInfoListByGenerationId
%{_bindir}/clients/cs_deleteFileInfoListByGenerationIdAndForecastTime
%{_bindir}/clients/cs_deleteFileInfoListByGenerationName
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
* Mon Nov 15 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.11.15-1.fmi
- Performance improvements to redis tools
* Fri Oct 29 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.29-1.fmi
- Minor fixes
* Tue Oct 19 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.19-1.fmi
- Added low level filtering capability
* Mon Oct 11 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.11-1.fmi
- Simplified grid storage structures
* Mon Oct  4 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.4-1.fmi
- Minor improvements
* Wed Sep 15 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.9.15-1.fmi
- NetCDF support
* Mon Sep 13 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.9.13-1.fmi
- Build update
* Thu Sep  9 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.9.9-1.fmi
- Removed obsolete libconfig dependency
* Tue Aug 31 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.8.31-1.fmi
- Repackaged due to Spine ABI changes
* Tue Jul 27 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.7.27-1.fmi
- Fixed string conversion bug in radon2csv
* Tue Jun  8 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.6.8-1.fmi
- Repackaged due to memory use reducing ABI changes in base libraries
* Tue Jun  1 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.6.1-1.fmi
- Minor updates
* Tue May 25 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.5.25-1.fmi
- Minor improvements to image rendering and printouts
* Thu Apr  1 2021 Pertti Kinnia <pertti.kinnia@fmi.fi> - 21.4.1-1.fmi
- Repackaged due to grid-files API changes
* Wed Mar  3 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.3.3-1.fmi
- Repackaged due to grid-files API changes
* Wed Feb  3 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.3-1.fmi
- Prefer time_t over std::string for speed
* Wed Jan 27 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.27-1.fmi
- Repackaged due to ABI changes in base libraries
* Tue Jan 19 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.19-1.fmi
- Repackaged due to base library ABI changes
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
