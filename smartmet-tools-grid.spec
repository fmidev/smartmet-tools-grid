%define DIRNAME grid
%define SPECNAME smartmet-tools-%{DIRNAME}
Summary: SmartMet tools for grid support
Name: %{SPECNAME}
Version: 25.2.18
Release: 1%{?dist}.fmi
License: MIT
Group: SmartMet/Plugins
URL: https://github.com/fmidev/smartmet-plugin-grid-gui
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%if 0%{?rhel} && 0%{rhel} < 9
%define smartmet_boost boost169
%else
%define smartmet_boost boost
%endif

BuildRequires: rpm-build
BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: %{smartmet_boost}-devel
BuildRequires: smartmet-library-spine-devel >= 25.2.18
BuildRequires: smartmet-library-grid-files-devel >= 25.2.18
BuildRequires: smartmet-library-grid-content-devel >= 25.2.18
BuildRequires: gdal310-devel
BuildRequires: postgresql15-devel
BuildRequires: omniORB-devel >= 4.3.0
BuildRequires: libpqxx-devel
BuildRequires: libmicrohttpd-devel
BuildRequires: libjpeg-devel
BuildRequires: libpng-devel
BuildRequires: openldap-devel
BuildRequires: openssl-devel
BuildRequires: krb5-devel
#Requires: smartmet-library-macgyver >= 25.2.18
#Requires: smartmet-library-spine >= 25.2.18
#Requires: smartmet-server >= 25.2.18
Requires: smartmet-library-grid-files >= 25.2.18
Requires: smartmet-library-grid-content >= 25.2.18
Requires: openldap
Requires: openssl-libs
Requires: krb5-devel
#TestRequires: smartmet-utils-devel >= 25.2.18

Provides: corbaContentServer = %{version}
Provides: corbaDataServer = %{version}
Provides: corbaGridServer = %{version}
Provides: corbaQueryServer = %{version}
Provides: createLandSeaMap = %{version}
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
Provides: cs_deleteGeometryInfoById = %{version}
Provides: cs_deleteGeometryInfoListByGenerationId = %{version}
Provides: cs_deleteGeometryInfoListByProducerId = %{version}
Provides: cs_deleteGeometryInfoListBySourceId = %{version}
Provides: cs_deleteProducerInfoById = %{version}
Provides: cs_deleteProducerInfoByName = %{version}
Provides: cs_deleteProducerInfoListBySourceId = %{version}
Provides: cs_deleteVirtualContent = %{version}
Provides: cs_getContentCount = %{version}
Provides: cs_getContentGeometryIdListByGenerationId = %{version}
Provides: cs_getContentInfo = %{version}
Provides: cs_getContentLevelListByGenerationGeometryAndLevelId = %{version}
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
Provides: cs_getContentParamKeyListByGenerationAndGeometryId = %{version}
Provides: cs_getContentParamKeyListByGenerationGeometryAndLevelId = %{version}
Provides: cs_getContentParamKeyListByGenerationId = %{version}
Provides: cs_getContentParamListByGenerationId = %{version}
Provides: cs_getContentTimeListByGenerationAndGeometryId = %{version}
Provides: cs_getContentTimeListByGenerationGeometryAndLevelId = %{version}
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
Provides: cs_getGeometryInfoById = %{version}
Provides: cs_getGeometryInfoList = %{version}
Provides: cs_getGeometryInfoListByGenerationId = %{version}
Provides: cs_getGeometryInfoListByProducerId = %{version}
Provides: cs_getGeometryInfoListBySourceId = %{version}
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
Provides: cs_setGeometryInfoStatusById = %{version}
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
Provides: httpServer = %{version}

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
%{_bindir}/clients/*
%{_bindir}/files/*
%{_bindir}/fmi/*
%{_bindir}/servers/*
%{_bindir}/utils/*

%defattr(0644,root,root,0755)
%config(noreplace) %{_unitdir}/radon2smartmet.service

# We do not enable the service by default, it must be done manually

%preun
if [ $1 -eq 0 ]; then
   systemctl stop radon2smartmet
   systemctl disable radon2smartmet
fi

%changelog
* Tue Feb 18 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.2.18-1.fmi
- Update to gdal-3.10, geos-3.13 and proj-9.5

* Thu Jan  9 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.1.9-1.fmi
- Added content info extension for parameter details
- Reading aggregation and processing information from Radon
- Fix new[]/delete mismatch

* Thu Oct 31 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.10.31-1.fmi
- Repackaged due to ABI changes

* Wed Oct 16 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.10.16-1.fmi
- Added a possiblity to mark grid files as cacheable if they contain given level types

* Tue Sep  3 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.9.3-1.fmi
- New version due to smartmet-library-grid-files changes. Also code cleanup and simplification

* Wed Aug 21 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.8.21-1.fmi
- Fixed radon2smartmet update interval counter bug

* Wed Aug  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.8.7-1.fmi
- Update to gdal-3.8, geos-3.12, proj-94 and fmt-11

* Wed Jul 17 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.7.17-1.fmi
- Do not link with libboost_filesystem

* Fri Jul 12 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.7.12-1.fmi
- Replace many boost library types with C++ standard library ones

* Tue Jun  4 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.6.4-1.fmi
- Fetching unit information for different level types

* Thu May 16 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.5.16-1.fmi
- Clean up boost date-time uses

* Fri May  3 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.5.3-1.fmi
- Added level type information

* Wed Apr  3 2024 Mika Heiskanen <mheiskan@rhel8.dev.fmi.fi> - 24.4.3-1.fmi
- Fixed systemd paths

* Fri Feb 23 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> 24.2.23-1.fmi
- Full repackaging

* Tue Feb 20 2024 Mika Heiskanen <mheiskan@rhel8.dev.fmi.fi> - 24.2.20-1.fmi
- Repackaged due to ABI changes in grid libraries

* Mon Feb  5 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> 24.2.5-1.fmi
- Repackaged due to grid-files ABI changes

* Thu Jan  4 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.1.4-1.fmi
- Repackaged due to ABI changes in grid-files

* Fri Dec 22 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.12.22-1.fmi
- Repackaged due to ThreadLock ABI changes

* Fri Nov 17 2023 Pertti Kinnia <pertti.kinnia@fmi.fi> - 23.11.17-1.fmi
- Repackaged due to API changes in grid-files and grid-content

* Fri Sep 29 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.9.29-1.fmi
- Added createLandSeaMap

* Mon Sep 11 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.9.11-1.fmi
- Repackaged due to ABI changes in grid-files

* Fri Jul 28 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.7.28-1.fmi
- Repackage due to bulk ABI changes in macgyver/newbase/spine

* Wed Jul 12 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.7.12-1.fmi
- Use postgresql 15, gdal 3.5, geos 3.11 and proj-9.0

* Fri May 26 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.5.26-1.fmi
- Require omniORB >= 4.3.0

* Mon Apr 17 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.4.17-1.fmi
- Added PostgreSQL support

* Mon Mar 13 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.3.13-1.fmi
- Enabling own memory mapper configuration (was hardcoded)

* Mon Feb 20 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.2.20-2.fmi
- Bug fix. Empty lines in the producers file caused segmentation fault

* Mon Feb 20 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.2.20-1.fmi
- Minor updates

* Thu Jan 19 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.1.19-1.fmi
- Added possibility to accept/ignore parameters when fetching content information from Radon

* Mon Dec 12 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.12.12-1.fmi
- Added S3 scanning capability

* Thu Nov 10 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.11.10-1.fmi
- Added file cache for scanned grid meta information

* Tue Nov  8 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.11.8-1.fmi
- Added file access information for the new memory mapper

* Thu Oct 20 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.10.20-1.fmi
- Simple http server for receiving and sending http messages

* Mon Oct 10 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.10.10-1.fmi
- Minor updates

* Thu Sep 29 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.9.29-1.fmi
- Minor configuration updates

* Thu Aug 25 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.8.25-1.fmi
- Fixed some problems with producer removal and generation status updates

* Wed Aug 24 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.8.24-1.fmi
- Repackaged due to an ABI change in ServiceImplementation

* Tue Aug 23 2022 Pertti Kinnia <pertti.kinnia@fmi.fi> - 22.8.23-1.fmi
- Repackaged due to API changes in base libraries

* Wed Aug 17 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.8.17-1.fmi
- Added new commands for testing purposes

* Thu Aug  4 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.8.4-1.fmi
- Fixed error in removing producers from Redis when they have been removed from producers.cfg

* Wed Jul 13 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.7.13-1.fmi
- Fix microhttpd ABI incompatibility workaround

* Thu Jun 30 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.6.30-2.fmi
- Fix build for RHEL 9 (+fix typo)

* Thu Jun 30 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.6.30-1.fmi
- Fix build for RHEL 9

* Mon Jun 20 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.6.20-1.fmi
- Add support for RHEL9, upgrade libpqxx to 7.7.0 (rhel8+) and fmt to 8.1.1

* Wed Jun  8 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.8-1.fmi
- Detection on generation updates after they have been marked to be ready

* Thu Jun  2 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.2-1.fmi
- Minor updates
* Mon Mar 28 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.28-1.fmi
- Repackaged due to grid-content ABI changes
* Thu Mar 10 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.10-1.fmi
- Repackaged due to base library ABI changes
* Mon Mar  7 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.7-1.fmi
- Repackaged due to API changes in base libraries
* Mon Feb 28 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.2.28-1.fmi
- Added a parameter for geometry status checking
* Wed Feb  9 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.2.9-1.fmi
- Generation geometry information handling
* Tue Jan 25 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.1.25-1.fmi
- Repackaged due to ABI changes in libraries
* Fri Jan 21 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.1.21-1.fmi
- Repackage due to upgrade of packages from PGDG repo: gdal-3.4, geos-3.10, proj-8.2
* Thu Jan 13 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.1.13-1.fmi
- Added handling of static data such as land/sea masks
* Wed Jan  5 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.1.5-1.fmi
- Repackaged all GRID-packages
* Tue Dec  7 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.12.7-1.fmi
- Upgrade to PostgreSQL 13 and GDAL-3.3
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
