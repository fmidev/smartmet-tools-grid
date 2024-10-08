#include "grid-files/grid/GridFile.h"
#include "grid-files/grid/PrintOptions.h"
#include "grid-files/identification/GridDef.h"
#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/GeneralDefinitions.h"
//#include "grid-files/common/ImageFunctions.h"
#include "grid-files/common/FileWriter.h"
#include "grid-files/grid/Typedefs.h"
#include "grid-files/grid/ValueCache.h"

#include <iostream>
#include <stdexcept>
#include <string.h>
#include <vector>
#include <sys/time.h>


using namespace SmartMet;



void init()
{
  try
  {
    char *configFile = getenv(SMARTMET_GRID_CONFIG_FILE);
    if (configFile == nullptr)
    {
      printf("%s not defined!\n",SMARTMET_GRID_CONFIG_FILE);
      exit(-1);
    }

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    Identification::gridDef.init(configFile);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}





void print_usage()
{
  printf("-------------------------------------------------------------------------------- \n");
  printf("PROGRAM :\n");
  printf("  The program creates a target grid file from an existing grid file.\n");
  printf("\n");
  printf("USAGE :\n");
  printf("  grid_create <gridTemplateFile> <gridSourceFile> <startGridIndex> <endGridIndex> <gridTargetFile>\n");
  printf("\n");
  printf("PARAMETERS :\n");
  printf("  <gridTemplateFile>  -- Grid template file\n");
  printf("  <gridSourceFile>    -- Grid source file (can contain multiple grids)\n");
  printf("  <startGridIndex>    -- First grid/message to convert (firstIndex = 0)\n");
  printf("  <endGridIndex>      -- Last grid/message to convert (firstIndex = 0)\n");
  printf("  <gridTargetFile>    -- Name of the target grid file\n");
  printf("\n");
  printf("EXAMPLES :\n");
  printf("  grid_create /usr/share/smartmet/templates/grib2/lambertConformal.cfg sourceGribFile.grib2 0 10000 targetGribFile.grib2\n");
  printf("-------------------------------------------------------------------------------- \n");
  printf("\n");
}



void setMessage_LatLon_v1(GRID::Message *message,GRID::Message *dataMessage)
{
  try
  {
    using namespace SmartMet::GRIB1;

    int width = 300;
    int height = 380;
    int sz = width * height;
    int levelType = 1;
    int level = 0;

    message->setProperty(Property::IndicatorSection::EditionNumber,1);

    message->setProperty(Property::ProductSection::TableVersion,128);
    message->setProperty(Property::ProductSection::Centre,98);
    message->setProperty(Property::ProductSection::GeneratingProcessIdentifier,149);
    message->setProperty(Property::ProductSection::GridDefinitionNumber,255);
    message->setProperty(Property::ProductSection::SectionFlags,128);
    message->setProperty(Property::ProductSection::IndicatorOfParameter,49);
    message->setProperty(Property::ProductSection::IndicatorOfTypeOfLevel,1);
    message->setProperty(Property::ProductSection::Level,0);
    message->setProperty(Property::ProductSection::YearOfCentury,18);
    message->setProperty(Property::ProductSection::Month,10);
    message->setProperty(Property::ProductSection::Day,17);
    message->setProperty(Property::ProductSection::Hour,0);
    message->setProperty(Property::ProductSection::Minute,0);
    message->setProperty(Property::ProductSection::UnitOfTimeRange,1);
    message->setProperty(Property::ProductSection::P1,0);
    message->setProperty(Property::ProductSection::P2,1);
    message->setProperty(Property::ProductSection::TimeRangeIndicator,2);
    message->setProperty(Property::ProductSection::NumberIncludedInAverage,0);
    message->setProperty(Property::ProductSection::NumberMissingFromAveragesOrAccumulations,0);
    message->setProperty(Property::ProductSection::CenturyOfReferenceTimeOfData,21);
    message->setProperty(Property::ProductSection::SubCentre,0);
    message->setProperty(Property::ProductSection::DecimalScaleFactor,0);
    message->setProperty(Property::ProductSection::ForecastType,1);
    message->setProperty(Property::ProductSection::ForecastNumber,-1);


    message->setProperty(Property::GridSection::NumberOfVerticalCoordinateValues,0);
    message->setProperty(Property::GridSection::PvlLocation,255);
    message->setProperty(Property::GridSection::DataRepresentationType,0);

    message->setProperty(Property::GridSection::LatLon::Ni,width);
    message->setProperty(Property::GridSection::LatLon::Nj,height);
    message->setProperty(Property::GridSection::LatLon::IDirectionIncrement,30000/width);
    message->setProperty(Property::GridSection::LatLon::JDirectionIncrement,38000/height);

    message->setProperty(Property::GridSection::GridArea::LatitudeOfFirstGridPoint,40000);
    message->setProperty(Property::GridSection::GridArea::LongitudeOfFirstGridPoint,5000);
    message->setProperty(Property::GridSection::GridArea::LatitudeOfLastGridPoint,78000);
    message->setProperty(Property::GridSection::GridArea::LongitudeOfLastGridPoint,35000);

    message->setProperty(Property::GridSection::ResolutionFlags::ResolutionAndComponentFlags,48);

    message->setProperty(Property::GridSection::ScanningMode::ScanMode,64);

    message->setProperty(Property::DataSection::Flags,8);
    message->setProperty(Property::DataSection::BinaryScaleFactor,0);
    message->setProperty(Property::DataSection::ReferenceValue,0.0);
    message->setProperty(Property::DataSection::BitsPerValue,32);
    message->setProperty(Property::DataSection::PackingMethod,0);



    // ### DATA SECTION ###

    message->initSpatialReference();
    T::Coordinate_svec coordinates = message->getGridLatLonCoordinates();

    T::ParamValue_vec values;
    dataMessage->getGridValueVectorByCoordinateList(T::CoordinateTypeValue::LATLON_COORDINATES,*coordinates,1,values);

    message->setGridValues(values);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
  }
}




void setMessage_RotatedLatLon_v1(GRID::Message *message,GRID::Message *dataMessage)
{
  try
  {
    using namespace SmartMet::GRIB1;

    int width = 1030;
    int height = 816;
    int sz = width * height;
    int levelType = 1;
    int level = 0;

    message->setProperty(Property::IndicatorSection::EditionNumber,1);

    message->setProperty(Property::ProductSection::TableVersion,128);
    message->setProperty(Property::ProductSection::Centre,98);
    message->setProperty(Property::ProductSection::GeneratingProcessIdentifier,149);
    message->setProperty(Property::ProductSection::GridDefinitionNumber,255);
    message->setProperty(Property::ProductSection::SectionFlags,128);
    message->setProperty(Property::ProductSection::IndicatorOfParameter,49);
    message->setProperty(Property::ProductSection::IndicatorOfTypeOfLevel,1);
    message->setProperty(Property::ProductSection::Level,0);
    message->setProperty(Property::ProductSection::YearOfCentury,18);
    message->setProperty(Property::ProductSection::Month,10);
    message->setProperty(Property::ProductSection::Day,17);
    message->setProperty(Property::ProductSection::Hour,0);
    message->setProperty(Property::ProductSection::Minute,0);
    message->setProperty(Property::ProductSection::UnitOfTimeRange,1);
    message->setProperty(Property::ProductSection::P1,0);
    message->setProperty(Property::ProductSection::P2,1);
    message->setProperty(Property::ProductSection::TimeRangeIndicator,2);
    message->setProperty(Property::ProductSection::NumberIncludedInAverage,0);
    message->setProperty(Property::ProductSection::NumberMissingFromAveragesOrAccumulations,0);
    message->setProperty(Property::ProductSection::CenturyOfReferenceTimeOfData,21);
    message->setProperty(Property::ProductSection::SubCentre,0);
    message->setProperty(Property::ProductSection::DecimalScaleFactor,0);
    message->setProperty(Property::ProductSection::ForecastType,1);
    message->setProperty(Property::ProductSection::ForecastNumber,-1);


    message->setProperty(Property::GridSection::NumberOfVerticalCoordinateValues,0);
    message->setProperty(Property::GridSection::PvlLocation,255);
    message->setProperty(Property::GridSection::DataRepresentationType,10);

    message->setProperty(Property::GridSection::RotatedLatLon::Ni,width);
    message->setProperty(Property::GridSection::RotatedLatLon::Nj,height);
    message->setProperty(Property::GridSection::RotatedLatLon::IDirectionIncrement,68);
    message->setProperty(Property::GridSection::RotatedLatLon::JDirectionIncrement,68);

    message->setProperty(Property::GridSection::GridArea::LatitudeOfFirstGridPoint,-24000);
    message->setProperty(Property::GridSection::GridArea::LongitudeOfFirstGridPoint,-33500);
    message->setProperty(Property::GridSection::GridArea::LatitudeOfLastGridPoint,31420);
    message->setProperty(Property::GridSection::GridArea::LongitudeOfLastGridPoint,36472);

    message->setProperty(Property::GridSection::ResolutionFlags::ResolutionAndComponentFlags,136);

    message->setProperty(Property::GridSection::ScanningMode::ScanMode,64);


    message->setProperty(Property::GridSection::Rotation::LatitudeOfSouthernPole,-30000);
    message->setProperty(Property::GridSection::Rotation::LongitudeOfSouthernPole,0);
    message->setProperty(Property::GridSection::Rotation::AngleOfRotationInDegrees,0);


    message->setProperty(Property::DataSection::Flags,8);
    message->setProperty(Property::DataSection::BinaryScaleFactor,0);
    message->setProperty(Property::DataSection::ReferenceValue,0.0);
    message->setProperty(Property::DataSection::BitsPerValue,32);
    message->setProperty(Property::DataSection::PackingMethod,0);



    // ### DATA SECTION ###

    message->initSpatialReference();
    T::Coordinate_svec coordinates = message->getGridLatLonCoordinates();

    T::ParamValue_vec values;
    dataMessage->getGridValueVectorByCoordinateList(T::CoordinateTypeValue::LATLON_COORDINATES,*coordinates,1,values);

    message->setGridValues(values);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
  }
}




void setMessage_PolarStereographic_v1(GRID::Message *message,GRID::Message *dataMessage)
{
  try
  {
    using namespace SmartMet::GRIB1;

    int width = 824;
    int height = 935;
    int sz = width * height;
    int levelType = 1;
    int level = 0;

    message->setProperty(Property::IndicatorSection::EditionNumber,1);

    message->setProperty(Property::ProductSection::TableVersion,128);
    message->setProperty(Property::ProductSection::Centre,98);
    message->setProperty(Property::ProductSection::GeneratingProcessIdentifier,149);
    message->setProperty(Property::ProductSection::GridDefinitionNumber,255);
    message->setProperty(Property::ProductSection::SectionFlags,128);
    message->setProperty(Property::ProductSection::IndicatorOfParameter,49);
    message->setProperty(Property::ProductSection::IndicatorOfTypeOfLevel,1);
    message->setProperty(Property::ProductSection::Level,0);
    message->setProperty(Property::ProductSection::YearOfCentury,18);
    message->setProperty(Property::ProductSection::Month,10);
    message->setProperty(Property::ProductSection::Day,17);
    message->setProperty(Property::ProductSection::Hour,0);
    message->setProperty(Property::ProductSection::Minute,0);
    message->setProperty(Property::ProductSection::UnitOfTimeRange,1);
    message->setProperty(Property::ProductSection::P1,0);
    message->setProperty(Property::ProductSection::P2,1);
    message->setProperty(Property::ProductSection::TimeRangeIndicator,2);
    message->setProperty(Property::ProductSection::NumberIncludedInAverage,0);
    message->setProperty(Property::ProductSection::NumberMissingFromAveragesOrAccumulations,0);
    message->setProperty(Property::ProductSection::CenturyOfReferenceTimeOfData,21);
    message->setProperty(Property::ProductSection::SubCentre,0);
    message->setProperty(Property::ProductSection::DecimalScaleFactor,0);
    message->setProperty(Property::ProductSection::ForecastType,1);
    message->setProperty(Property::ProductSection::ForecastNumber,-1);


    message->setProperty(Property::GridSection::NumberOfVerticalCoordinateValues,0);
    message->setProperty(Property::GridSection::PvlLocation,255);
    message->setProperty(Property::GridSection::DataRepresentationType,5);


    message->setProperty(Property::GridSection::PolarStereographic::Nx,width);
    message->setProperty(Property::GridSection::PolarStereographic::Ny,height);
    message->setProperty(Property::GridSection::PolarStereographic::LatitudeOfFirstGridPoint,18145);
    message->setProperty(Property::GridSection::PolarStereographic::LongitudeOfFirstGridPoint,217107);
    message->setProperty(Property::GridSection::PolarStereographic::OrientationOfTheGrid,249000);
    message->setProperty(Property::GridSection::PolarStereographic::DxInMetres,10000);
    message->setProperty(Property::GridSection::PolarStereographic::DyInMetres,10000);
    message->setProperty(Property::GridSection::PolarStereographic::ProjectionCentreFlag,0);

    message->setProperty(Property::GridSection::ResolutionFlags::ResolutionAndComponentFlags,8);

    message->setProperty(Property::GridSection::ScanningMode::ScanMode,64);

    message->setProperty(Property::DataSection::Flags,8);
    message->setProperty(Property::DataSection::BinaryScaleFactor,0);
    message->setProperty(Property::DataSection::ReferenceValue,0.0);
    message->setProperty(Property::DataSection::BitsPerValue,32);
    message->setProperty(Property::DataSection::PackingMethod,0);


    // ### DATA SECTION ###

    message->initSpatialReference();
    T::Coordinate_svec coordinates = message->getGridLatLonCoordinates();

    T::ParamValue_vec values;
    dataMessage->getGridValueVectorByCoordinateList(T::CoordinateTypeValue::LATLON_COORDINATES,*coordinates,1,values);

    message->setGridValues(values);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
  }
}




void setMessage_LambertConformal_v1(GRID::Message *message,GRID::Message *dataMessage)
{
  try
  {
    using namespace SmartMet::GRIB1;

    int width = 1796;
    int height = 2321;
    int sz = width * height;
    int levelType = 1;
    int level = 0;

    message->setProperty(Property::IndicatorSection::EditionNumber,1);

    message->setProperty(Property::ProductSection::TableVersion,128);
    message->setProperty(Property::ProductSection::Centre,98);
    message->setProperty(Property::ProductSection::GeneratingProcessIdentifier,149);
    message->setProperty(Property::ProductSection::GridDefinitionNumber,255);
    message->setProperty(Property::ProductSection::SectionFlags,128);
    message->setProperty(Property::ProductSection::IndicatorOfParameter,49);
    message->setProperty(Property::ProductSection::IndicatorOfTypeOfLevel,1);
    message->setProperty(Property::ProductSection::Level,0);
    message->setProperty(Property::ProductSection::YearOfCentury,18);
    message->setProperty(Property::ProductSection::Month,10);
    message->setProperty(Property::ProductSection::Day,17);
    message->setProperty(Property::ProductSection::Hour,0);
    message->setProperty(Property::ProductSection::Minute,0);
    message->setProperty(Property::ProductSection::UnitOfTimeRange,1);
    message->setProperty(Property::ProductSection::P1,0);
    message->setProperty(Property::ProductSection::P2,1);
    message->setProperty(Property::ProductSection::TimeRangeIndicator,2);
    message->setProperty(Property::ProductSection::NumberIncludedInAverage,0);
    message->setProperty(Property::ProductSection::NumberMissingFromAveragesOrAccumulations,0);
    message->setProperty(Property::ProductSection::CenturyOfReferenceTimeOfData,21);
    message->setProperty(Property::ProductSection::SubCentre,0);
    message->setProperty(Property::ProductSection::DecimalScaleFactor,0);
    message->setProperty(Property::ProductSection::ForecastType,1);
    message->setProperty(Property::ProductSection::ForecastNumber,-1);


    message->setProperty(Property::GridSection::NumberOfVerticalCoordinateValues,0);
    message->setProperty(Property::GridSection::PvlLocation,255);
    message->setProperty(Property::GridSection::DataRepresentationType,3);

    message->setProperty(Property::GridSection::LambertConformal::Nx,width);
    message->setProperty(Property::GridSection::LambertConformal::Ny,height);
    message->setProperty(Property::GridSection::LambertConformal::LatitudeOfFirstGridPoint,52302);
    message->setProperty(Property::GridSection::LambertConformal::LongitudeOfFirstGridPoint,1918);
    message->setProperty(Property::GridSection::LambertConformal::LoV,15000);
    message->setProperty(Property::GridSection::LambertConformal::DxInMetres,1000);
    message->setProperty(Property::GridSection::LambertConformal::DyInMetres,1000);
    message->setProperty(Property::GridSection::LambertConformal::ProjectionCentreFlag,0);
    message->setProperty(Property::GridSection::LambertConformal::Latin1,63000);
    message->setProperty(Property::GridSection::LambertConformal::Latin2,63000);
    message->setProperty(Property::GridSection::LambertConformal::LatitudeOfSouthernPole,0);
    message->setProperty(Property::GridSection::LambertConformal::LongitudeOfSouthernPole,0);

    message->setProperty(Property::GridSection::ResolutionFlags::ResolutionAndComponentFlags,48);

    message->setProperty(Property::GridSection::ScanningMode::ScanMode,64);

    message->setProperty(Property::DataSection::Flags,8);
    message->setProperty(Property::DataSection::BinaryScaleFactor,0);
    message->setProperty(Property::DataSection::ReferenceValue,0.0);
    message->setProperty(Property::DataSection::BitsPerValue,32);
    message->setProperty(Property::DataSection::PackingMethod,0);



    // ### DATA SECTION ###

    message->initSpatialReference();
    T::Coordinate_svec coordinates = message->getGridLatLonCoordinates();

    T::ParamValue_vec values;
    dataMessage->getGridValueVectorByCoordinateList(T::CoordinateTypeValue::LATLON_COORDINATES,*coordinates,1,values);

    message->setGridValues(values);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
  }
}




void setMessage_Mercator_v1(GRID::Message *message,GRID::Message *dataMessage)
{
  try
  {
    using namespace SmartMet::GRIB1;

    int width = 1044;
    int height = 864;
    int sz = width * height;
    int levelType = 1;
    int level = 0;

    message->setProperty(Property::IndicatorSection::EditionNumber,1);

    message->setProperty(Property::ProductSection::TableVersion,128);
    message->setProperty(Property::ProductSection::Centre,98);
    message->setProperty(Property::ProductSection::GeneratingProcessIdentifier,149);
    message->setProperty(Property::ProductSection::GridDefinitionNumber,255);
    message->setProperty(Property::ProductSection::SectionFlags,128);
    message->setProperty(Property::ProductSection::IndicatorOfParameter,49);
    message->setProperty(Property::ProductSection::IndicatorOfTypeOfLevel,1);
    message->setProperty(Property::ProductSection::Level,0);
    message->setProperty(Property::ProductSection::YearOfCentury,18);
    message->setProperty(Property::ProductSection::Month,10);
    message->setProperty(Property::ProductSection::Day,17);
    message->setProperty(Property::ProductSection::Hour,0);
    message->setProperty(Property::ProductSection::Minute,0);
    message->setProperty(Property::ProductSection::UnitOfTimeRange,1);
    message->setProperty(Property::ProductSection::P1,0);
    message->setProperty(Property::ProductSection::P2,1);
    message->setProperty(Property::ProductSection::TimeRangeIndicator,2);
    message->setProperty(Property::ProductSection::NumberIncludedInAverage,0);
    message->setProperty(Property::ProductSection::NumberMissingFromAveragesOrAccumulations,0);
    message->setProperty(Property::ProductSection::CenturyOfReferenceTimeOfData,21);
    message->setProperty(Property::ProductSection::SubCentre,0);
    message->setProperty(Property::ProductSection::DecimalScaleFactor,0);
    message->setProperty(Property::ProductSection::ForecastType,1);
    message->setProperty(Property::ProductSection::ForecastNumber,-1);


    message->setProperty(Property::GridSection::NumberOfVerticalCoordinateValues,0);
    message->setProperty(Property::GridSection::PvlLocation,255);
    message->setProperty(Property::GridSection::DataRepresentationType,1);

    message->setProperty(Property::GridSection::Mercator::Ni,width);
    message->setProperty(Property::GridSection::Mercator::Nj,height);
    message->setProperty(Property::GridSection::Mercator::Latin,15850);
    message->setProperty(Property::GridSection::Mercator::DiInMetres,3000);
    message->setProperty(Property::GridSection::Mercator::DjInMetres,3000);

    message->setProperty(Property::GridSection::GridArea::LatitudeOfFirstGridPoint,3949);
    message->setProperty(Property::GridSection::GridArea::LongitudeOfFirstGridPoint,95372);
    message->setProperty(Property::GridSection::GridArea::LatitudeOfLastGridPoint,27090);
    message->setProperty(Property::GridSection::GridArea::LongitudeOfLastGridPoint,124628);

    message->setProperty(Property::GridSection::ResolutionFlags::ResolutionAndComponentFlags,129);

    message->setProperty(Property::GridSection::ScanningMode::ScanMode,64);

    message->setProperty(Property::DataSection::Flags,8);
    message->setProperty(Property::DataSection::BinaryScaleFactor,0);
    message->setProperty(Property::DataSection::ReferenceValue,0.0);
    message->setProperty(Property::DataSection::BitsPerValue,32);
    message->setProperty(Property::DataSection::PackingMethod,0);



    // ### DATA SECTION ###

    message->initSpatialReference();
    T::Coordinate_svec coordinates = message->getGridLatLonCoordinates();

    T::ParamValue_vec values;
    dataMessage->getGridValueVectorByCoordinateList(T::CoordinateTypeValue::LATLON_COORDINATES,*coordinates,1,values);

    message->setGridValues(values);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
  }
}




void setMessage_LatLon_v2(GRID::Message *message,GRID::Message *dataMessage)
{
  try
  {
    using namespace SmartMet::GRIB2;

    int width = 300;
    int height = 380;
    int sz = width * height;
    int levelType = 103;
    int level = 10;

    // ### INDICATOR SECTION ###

    message->setProperty(Property::IndicatorSection::Discipline,0LL);

    // ### IDENTFICATION SECTION ###

    message->setProperty(Property::IdentificationSection::Centre,78);
    message->setProperty(Property::IdentificationSection::SubCentre,255);
    message->setProperty(Property::IdentificationSection::TablesVersion,15);
    message->setProperty(Property::IdentificationSection::LocalTablesVersion,1);
    message->setProperty(Property::IdentificationSection::SignificanceOfReferenceTime,1);
    message->setProperty(Property::IdentificationSection::Year,2018);
    message->setProperty(Property::IdentificationSection::Month,8);
    message->setProperty(Property::IdentificationSection::Day,20);
    message->setProperty(Property::IdentificationSection::Hour,0LL);
    message->setProperty(Property::IdentificationSection::Minute,0LL);
    message->setProperty(Property::IdentificationSection::Second,0LL);
    message->setProperty(Property::IdentificationSection::ProductionStatusOfProcessedData,0LL);
    message->setProperty(Property::IdentificationSection::TypeOfProcessedData,1);

    // ### PRODUCT SECTION ###

    message->setProperty(Property::ProductSection::ProductDefinitionTemplateNumber,ProductSection::Template::NormalProduct);
    message->setProperty(Property::ProductSection::NV,0LL);

    message->setProperty(Property::ProductSection::ParameterSettings::ParameterCategory,1);
    message->setProperty(Property::ProductSection::ParameterSettings::ParameterNumber,1);
    message->setProperty(Property::ProductSection::ParameterSettings::TypeOfGeneratingProcess,2);
    message->setProperty(Property::ProductSection::ParameterSettings::BackgroundProcess,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::GeneratingProcessIdentifier,181);
    message->setProperty(Property::ProductSection::ParameterSettings::HoursAfterDataCutoff,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::MinutesAfterDataCutoff,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::IndicatorOfUnitOfTimeRange,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::ForecastTime,0LL);

    message->setProperty(Property::ProductSection::HorizontalSettings::TypeOfFirstFixedSurface,levelType);
    message->setProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfFirstFixedSurface,0LL);
    message->setProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfFirstFixedSurface,level);
    //message->setProperty(Property::ProductSection::HorizontalSettings::TypeOfSecondFixedSurface,0LL);
    //message->setProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfSecondFixedSurface,0LL);
    //message->setProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfSecondFixedSurface,0LL);

    /*
    message->setProperty(Property::ProductSection::StatisticalSettings::YearOfEndOfOverallTimeInterval,2018);
    message->setProperty(Property::ProductSection::StatisticalSettings::MonthOfEndOfOverallTimeInterval,8);
    message->setProperty(Property::ProductSection::StatisticalSettings::DayOfEndOfOverallTimeInterval,20);
    message->setProperty(Property::ProductSection::StatisticalSettings::HourOfEndOfOverallTimeInterval,1);
    message->setProperty(Property::ProductSection::StatisticalSettings::MinuteOfEndOfOverallTimeInterval,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::SecondOfEndOfOverallTimeInterval,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::NumberOfTimeRange,1LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::NumberOfMissingInStatisticalProcess,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::TypeOfStatisticalProcessing,2);
    message->setProperty(Property::ProductSection::StatisticalSettings::TypeOfTimeIncrement,2);
    message->setProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeRange,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::LengthOfTimeRange,60);
    //message->setProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeIncrement,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::TimeIncrement,0LL);
     */


    // ### GRID SECTION ###

    message->setProperty(Property::GridSection::GridDefinitionTemplateNumber,GridSection::Template::LatLon);

    message->setProperty(Property::GridSection::SourceOfGridDefinition,0LL);
    message->setProperty(Property::GridSection::NumberOfGridPoints,sz);
    message->setProperty(Property::GridSection::NumberOfOctetsForNumberOfPoints,0LL);
    message->setProperty(Property::GridSection::InterpretationOfNumberOfPoints,0LL);

    message->setProperty(Property::GridSection::EarthShape::ShapeOfTheEarth,6);

    message->setProperty(Property::GridSection::LatLon::IDirectionIncrement,30000000/width);
    message->setProperty(Property::GridSection::LatLon::JDirectionIncrement,38000000/height);
    message->setProperty(Property::GridSection::LatLon::ScanningMode,64);

    message->setProperty(Property::GridSection::Grid::Ni,width);
    message->setProperty(Property::GridSection::Grid::Nj,height);
    message->setProperty(Property::GridSection::Grid::BasicAngleOfTheInitialProductionDomain,0LL);
    //message->setProperty(Property::GridSection::Grid::SubdivisionsOfBasicAngle,0LL);
    message->setProperty(Property::GridSection::Grid::LatitudeOfFirstGridPoint,40000000);
    message->setProperty(Property::GridSection::Grid::LongitudeOfFirstGridPoint,5000000);
    message->setProperty(Property::GridSection::Grid::LatitudeOfLastGridPoint,78000000);
    message->setProperty(Property::GridSection::Grid::LongitudeOfLastGridPoint,35000000);
    message->setProperty(Property::GridSection::Grid::ResolutionAndComponentFlags,48);


    // ### REPRESENTATION SECTION ####

    message->setProperty(Property::RepresentationSection::OriginalValues::TypeOfOriginalFieldValues,0LL);

    message->setProperty(Property::RepresentationSection::RepresentationTemplateNumber,RepresentationSection::Template::GridDataRepresentation);
    message->setProperty(Property::RepresentationSection::Packing::BinaryScaleFactor,-11);
    message->setProperty(Property::RepresentationSection::Packing::DecimalScaleFactor,0LL);
    message->setProperty(Property::RepresentationSection::Packing::BitsPerValue,16);


    // ### DATA SECTION ###

    message->initSpatialReference();
    T::Coordinate_svec coordinates = message->getGridLatLonCoordinates();

    T::ParamValue_vec values;
    dataMessage->getGridValueVectorByCoordinateList(T::CoordinateTypeValue::LATLON_COORDINATES,*coordinates,1,values);

    message->setGridValues(values);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
  }
}





void setMessage_RotatedLatLon_v2(GRID::Message *message,GRID::Message *dataMessage)
{
  try
  {
    using namespace SmartMet::GRIB2;

    int width = 1030;
    int height = 816;
    int sz = width * height;
    int levelType = 103;
    int level = 10;

    // ### INDICATOR SECTION ###

    message->setProperty(Property::IndicatorSection::Discipline,0LL);


    // ### IDENTIFICATION SECTION ###

    message->setProperty(Property::IdentificationSection::Centre,86);
    message->setProperty(Property::IdentificationSection::SubCentre,255);
    message->setProperty(Property::IdentificationSection::TablesVersion,15);
    message->setProperty(Property::IdentificationSection::LocalTablesVersion,1);
    message->setProperty(Property::IdentificationSection::SignificanceOfReferenceTime,1);
    message->setProperty(Property::IdentificationSection::Year,2018);
    message->setProperty(Property::IdentificationSection::Month,8);
    message->setProperty(Property::IdentificationSection::Day,20);
    message->setProperty(Property::IdentificationSection::Hour,0LL);
    message->setProperty(Property::IdentificationSection::Minute,0LL);
    message->setProperty(Property::IdentificationSection::Second,0LL);
    message->setProperty(Property::IdentificationSection::ProductionStatusOfProcessedData,0LL);
    message->setProperty(Property::IdentificationSection::TypeOfProcessedData,1);


    // ### PRODUCT SECTION ###

    message->setProperty(Property::ProductSection::ProductDefinitionTemplateNumber,ProductSection::Template::EnsembleDerivedForecast);
    message->setProperty(Property::ProductSection::NV,0LL);

    message->setProperty(Property::ProductSection::ParameterSettings::ParameterCategory,0);
    message->setProperty(Property::ProductSection::ParameterSettings::ParameterNumber,0);
    message->setProperty(Property::ProductSection::ParameterSettings::TypeOfGeneratingProcess,2);
    message->setProperty(Property::ProductSection::ParameterSettings::BackgroundProcess,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::GeneratingProcessIdentifier,181);
    message->setProperty(Property::ProductSection::ParameterSettings::HoursAfterDataCutoff,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::MinutesAfterDataCutoff,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::IndicatorOfUnitOfTimeRange,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::ForecastTime,0LL);

    message->setProperty(Property::ProductSection::HorizontalSettings::TypeOfFirstFixedSurface,levelType);
    message->setProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfFirstFixedSurface,0LL);
    message->setProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfFirstFixedSurface,level);
    //message->setProperty(Property::ProductSection::HorizontalSettings::TypeOfSecondFixedSurface,0LL);
    //message->setProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfSecondFixedSurface,0LL);
    //message->setProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfSecondFixedSurface,0LL);

    /*
    message->setProperty(Property::ProductSection::StatisticalSettings::YearOfEndOfOverallTimeInterval,2018);
    message->setProperty(Property::ProductSection::StatisticalSettings::MonthOfEndOfOverallTimeInterval,8);
    message->setProperty(Property::ProductSection::StatisticalSettings::DayOfEndOfOverallTimeInterval,20);
    message->setProperty(Property::ProductSection::StatisticalSettings::HourOfEndOfOverallTimeInterval,1);
    message->setProperty(Property::ProductSection::StatisticalSettings::MinuteOfEndOfOverallTimeInterval,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::SecondOfEndOfOverallTimeInterval,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::NumberOfTimeRange,1LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::NumberOfMissingInStatisticalProcess,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::TypeOfStatisticalProcessing,2);
    message->setProperty(Property::ProductSection::StatisticalSettings::TypeOfTimeIncrement,2);
    message->setProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeRange,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::LengthOfTimeRange,60);
    //message->setProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeIncrement,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::TimeIncrement,0LL);
     */


    message->setProperty(Property::ProductSection::EpsSettings::TypeOfEnsembleForecast,3);
    message->setProperty(Property::ProductSection::EpsSettings::PerturbationNumber,1);
    message->setProperty(Property::ProductSection::EpsSettings::NumberOfForecastsInEnsemble,0LL);

    message->setProperty(Property::ProductSection::DerivedSettings::DerivedForecast,0LL);
    message->setProperty(Property::ProductSection::DerivedSettings::NumberOfForecastsInEnsemble,0LL);


    // ### GRID SECTION ###

    message->setProperty(Property::GridSection::GridDefinitionTemplateNumber,GridSection::Template::RotatedLatLon);

    message->setProperty(Property::GridSection::SourceOfGridDefinition,0LL);
    message->setProperty(Property::GridSection::NumberOfGridPoints,sz);
    message->setProperty(Property::GridSection::NumberOfOctetsForNumberOfPoints,0LL);
    message->setProperty(Property::GridSection::InterpretationOfNumberOfPoints,0LL);

    message->setProperty(Property::GridSection::EarthShape::ShapeOfTheEarth,6);

    message->setProperty(Property::GridSection::LatLon::IDirectionIncrement,68000);
    message->setProperty(Property::GridSection::LatLon::JDirectionIncrement,68000);
    message->setProperty(Property::GridSection::LatLon::ScanningMode,64L);

    message->setProperty(Property::GridSection::Grid::Ni,width);
    message->setProperty(Property::GridSection::Grid::Nj,height);
    message->setProperty(Property::GridSection::Grid::BasicAngleOfTheInitialProductionDomain,0LL);
    //message->setProperty(Property::GridSection::Grid::SubdivisionsOfBasicAngle,0LL);
    message->setProperty(Property::GridSection::Grid::LatitudeOfFirstGridPoint,-24000000);
    message->setProperty(Property::GridSection::Grid::LongitudeOfFirstGridPoint,335000000);
    message->setProperty(Property::GridSection::Grid::LatitudeOfLastGridPoint,31420000);
    message->setProperty(Property::GridSection::Grid::LongitudeOfLastGridPoint,36472000);
    message->setProperty(Property::GridSection::Grid::ResolutionAndComponentFlags,136);

    message->setProperty(Property::GridSection::Rotation::LatitudeOfSouthernPole,-30000000);
    message->setProperty(Property::GridSection::Rotation::LongitudeOfSouthernPole,0LL);
    message->setProperty(Property::GridSection::Rotation::AngleOfRotation,0LL);


    message->setProperty(Property::RepresentationSection::RepresentationTemplateNumber,RepresentationSection::Template::GridDataRepresentation);
    message->setProperty(Property::RepresentationSection::Packing::BinaryScaleFactor,-11);
    message->setProperty(Property::RepresentationSection::Packing::DecimalScaleFactor,0LL);
    message->setProperty(Property::RepresentationSection::Packing::BitsPerValue,16);

    message->setProperty(Property::RepresentationSection::OriginalValues::TypeOfOriginalFieldValues,0LL);

    message->initSpatialReference();
    T::Coordinate_svec coordinates = message->getGridLatLonCoordinates();

    T::ParamValue_vec values;
    dataMessage->getGridValueVectorByCoordinateList(T::CoordinateTypeValue::LATLON_COORDINATES,*coordinates,1,values);

    message->setGridValues(values);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
  }
}





void setMessage_PolarStereographic_v2(GRID::Message *message,GRID::Message *dataMessage)
{
  try
  {
    using namespace SmartMet::GRIB2;

    int width = 824;
    int height = 935;
    int sz = width * height;
    int levelType = 103;
    int level = 10;

    // ### INDICATOR SECTION ###

    message->setProperty(Property::IndicatorSection::Discipline,0LL);


    // ### IDENTIFICATION SECTION ###

    message->setProperty(Property::IdentificationSection::Centre,78);
    message->setProperty(Property::IdentificationSection::SubCentre,255);
    message->setProperty(Property::IdentificationSection::TablesVersion,15);
    message->setProperty(Property::IdentificationSection::LocalTablesVersion,1);
    message->setProperty(Property::IdentificationSection::SignificanceOfReferenceTime,1);
    message->setProperty(Property::IdentificationSection::Year,2018);
    message->setProperty(Property::IdentificationSection::Month,8);
    message->setProperty(Property::IdentificationSection::Day,20);
    message->setProperty(Property::IdentificationSection::Hour,0LL);
    message->setProperty(Property::IdentificationSection::Minute,0LL);
    message->setProperty(Property::IdentificationSection::Second,0LL);
    message->setProperty(Property::IdentificationSection::ProductionStatusOfProcessedData,0LL);
    message->setProperty(Property::IdentificationSection::TypeOfProcessedData,1);


    // ### PRODUCT SECTION ###

    message->setProperty(Property::ProductSection::ProductDefinitionTemplateNumber,ProductSection::Template::NormalProduct);
    message->setProperty(Property::ProductSection::NV,0LL);

    message->setProperty(Property::ProductSection::ParameterSettings::ParameterCategory,2);
    message->setProperty(Property::ProductSection::ParameterSettings::ParameterNumber,0);
    message->setProperty(Property::ProductSection::ParameterSettings::TypeOfGeneratingProcess,2);
    message->setProperty(Property::ProductSection::ParameterSettings::BackgroundProcess,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::GeneratingProcessIdentifier,181);
    message->setProperty(Property::ProductSection::ParameterSettings::HoursAfterDataCutoff,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::MinutesAfterDataCutoff,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::IndicatorOfUnitOfTimeRange,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::ForecastTime,0LL);

    message->setProperty(Property::ProductSection::HorizontalSettings::TypeOfFirstFixedSurface,levelType);
    message->setProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfFirstFixedSurface,0LL);
    message->setProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfFirstFixedSurface,level);
    //message->setProperty(Property::ProductSection::HorizontalSettings::TypeOfSecondFixedSurface,0LL);
    //message->setProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfSecondFixedSurface,0LL);
    //message->setProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfSecondFixedSurface,0LL);

/*
    message->setProperty(Property::ProductSection::StatisticalSettings::YearOfEndOfOverallTimeInterval,2018);
    message->setProperty(Property::ProductSection::StatisticalSettings::MonthOfEndOfOverallTimeInterval,8);
    message->setProperty(Property::ProductSection::StatisticalSettings::DayOfEndOfOverallTimeInterval,20);
    message->setProperty(Property::ProductSection::StatisticalSettings::HourOfEndOfOverallTimeInterval,1);
    message->setProperty(Property::ProductSection::StatisticalSettings::MinuteOfEndOfOverallTimeInterval,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::SecondOfEndOfOverallTimeInterval,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::NumberOfTimeRange,1LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::NumberOfMissingInStatisticalProcess,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::TypeOfStatisticalProcessing,2);
    message->setProperty(Property::ProductSection::StatisticalSettings::TypeOfTimeIncrement,2);
    message->setProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeRange,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::LengthOfTimeRange,60);
    //message->setProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeIncrement,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::TimeIncrement,0LL);
*/

    // ### GRID SECTION ###

    message->setProperty(Property::GridSection::GridDefinitionTemplateNumber,GridSection::Template::PolarStereographic);

    message->setProperty(Property::GridSection::SourceOfGridDefinition,0LL);
    message->setProperty(Property::GridSection::NumberOfGridPoints,sz);
    message->setProperty(Property::GridSection::NumberOfOctetsForNumberOfPoints,0LL);
    message->setProperty(Property::GridSection::InterpretationOfNumberOfPoints,0LL);

    message->setProperty(Property::GridSection::EarthShape::ShapeOfTheEarth,6);

    message->setProperty(Property::GridSection::PolarStereographic::Nx,width);
    message->setProperty(Property::GridSection::PolarStereographic::Ny,height);
    message->setProperty(Property::GridSection::PolarStereographic::LatitudeOfFirstGridPoint,18145030);
    message->setProperty(Property::GridSection::PolarStereographic::LongitudeOfFirstGridPoint,217107456);
    message->setProperty(Property::GridSection::PolarStereographic::ResolutionAndComponentFlags,8);
    message->setProperty(Property::GridSection::PolarStereographic::LaD,60000000);
    message->setProperty(Property::GridSection::PolarStereographic::OrientationOfTheGrid,249000000);
    message->setProperty(Property::GridSection::PolarStereographic::Dx,10000000);
    message->setProperty(Property::GridSection::PolarStereographic::Dy,10000000);
    message->setProperty(Property::GridSection::PolarStereographic::ProjectionCentreFlag,0LL);
    message->setProperty(Property::GridSection::PolarStereographic::ScanningMode,64);


    // ### REPRESENTATION SECTION ###

    message->setProperty(Property::RepresentationSection::RepresentationTemplateNumber,RepresentationSection::Template::GridDataRepresentation);

    message->setProperty(Property::RepresentationSection::Packing::BinaryScaleFactor,-11);
    message->setProperty(Property::RepresentationSection::Packing::DecimalScaleFactor,0LL);
    message->setProperty(Property::RepresentationSection::Packing::BitsPerValue,16);

    message->setProperty(Property::RepresentationSection::OriginalValues::TypeOfOriginalFieldValues,0LL);


    // ### DATA SECTION ###

    message->initSpatialReference();
    T::Coordinate_svec coordinates = message->getGridLatLonCoordinates();

    T::ParamValue_vec values;
    dataMessage->getGridValueVectorByCoordinateList(T::CoordinateTypeValue::LATLON_COORDINATES,*coordinates,1,values);

    message->setGridValues(values);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
  }
}





void setMessage_LambertConformal_v2(GRID::Message *message,GRID::Message *dataMessage)
{
  try
  {
    using namespace SmartMet::GRIB2;

    int width = 349;
    int height = 277;
    int sz = width * height;
    int levelType = 103;
    int level = 10;

    // ### INDICATOR SECTION ###

    message->setProperty(Property::IndicatorSection::Discipline,0LL);


    // ### IDENTIFICATION SECTION ###

    message->setProperty(Property::IdentificationSection::Centre,78);
    message->setProperty(Property::IdentificationSection::SubCentre,255);
    message->setProperty(Property::IdentificationSection::TablesVersion,15);
    message->setProperty(Property::IdentificationSection::LocalTablesVersion,1);
    message->setProperty(Property::IdentificationSection::SignificanceOfReferenceTime,1);
    message->setProperty(Property::IdentificationSection::Year,2018);
    message->setProperty(Property::IdentificationSection::Month,8);
    message->setProperty(Property::IdentificationSection::Day,20);
    message->setProperty(Property::IdentificationSection::Hour,0LL);
    message->setProperty(Property::IdentificationSection::Minute,0LL);
    message->setProperty(Property::IdentificationSection::Second,0LL);
    message->setProperty(Property::IdentificationSection::ProductionStatusOfProcessedData,0LL);
    message->setProperty(Property::IdentificationSection::TypeOfProcessedData,1);

    // ### PRODUCT SECTION ###

    message->setProperty(Property::ProductSection::ProductDefinitionTemplateNumber,ProductSection::Template::NormalProduct);
    message->setProperty(Property::ProductSection::NV,0LL);

    message->setProperty(Property::ProductSection::ParameterSettings::ParameterCategory,2);
    message->setProperty(Property::ProductSection::ParameterSettings::ParameterNumber,22);
    message->setProperty(Property::ProductSection::ParameterSettings::TypeOfGeneratingProcess,2);
    message->setProperty(Property::ProductSection::ParameterSettings::BackgroundProcess,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::GeneratingProcessIdentifier,2);
    message->setProperty(Property::ProductSection::ParameterSettings::HoursAfterDataCutoff,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::MinutesAfterDataCutoff,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::IndicatorOfUnitOfTimeRange,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::ForecastTime,0LL);

    message->setProperty(Property::ProductSection::HorizontalSettings::TypeOfFirstFixedSurface,levelType);
    message->setProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfFirstFixedSurface,0LL);
    message->setProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfFirstFixedSurface,level);
    //message->setProperty(Property::ProductSection::HorizontalSettings::TypeOfSecondFixedSurface,0LL);
    //message->setProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfSecondFixedSurface,0LL);
    //message->setProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfSecondFixedSurface,0LL);

/*
    message->setProperty(Property::ProductSection::StatisticalSettings::YearOfEndOfOverallTimeInterval,2018);
    message->setProperty(Property::ProductSection::StatisticalSettings::MonthOfEndOfOverallTimeInterval,8);
    message->setProperty(Property::ProductSection::StatisticalSettings::DayOfEndOfOverallTimeInterval,20);
    message->setProperty(Property::ProductSection::StatisticalSettings::HourOfEndOfOverallTimeInterval,1);
    message->setProperty(Property::ProductSection::StatisticalSettings::MinuteOfEndOfOverallTimeInterval,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::SecondOfEndOfOverallTimeInterval,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::NumberOfTimeRange,1LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::NumberOfMissingInStatisticalProcess,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::TypeOfStatisticalProcessing,2);
    message->setProperty(Property::ProductSection::StatisticalSettings::TypeOfTimeIncrement,2);
    message->setProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeRange,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::LengthOfTimeRange,60);
    //message->setProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeIncrement,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::TimeIncrement,0LL);
*/

    //message->setProperty(Property::ProductSection::LevelId,103);
    //message->setProperty(Property::ProductSection::Level,2);

    //message->setGridGeometry(401);  // 1097 x 657
    //message->setGribParameter(std::string("132"));

    message->setProperty(Property::GridSection::SourceOfGridDefinition,0LL);
    message->setProperty(Property::GridSection::NumberOfGridPoints,sz);
    message->setProperty(Property::GridSection::NumberOfOctetsForNumberOfPoints,0LL);
    message->setProperty(Property::GridSection::InterpretationOfNumberOfPoints,0LL);
    message->setProperty(Property::GridSection::GridDefinitionTemplateNumber,GridSection::Template::LambertConformal);
    //message->setProperty(Property::GridSection::GridDefinitionTemplateNumber,GridSection::Template::PolarStereographic);
    //message->setProperty(Property::GridSection::GridDefinitionTemplateNumber,GridSection::Template::RotatedLatLon);

/*
    message->setProperty(Property::GridSection::PolarStereographic::Nx,width);
    message->setProperty(Property::GridSection::PolarStereographic::Ny,height);
    message->setProperty(Property::GridSection::PolarStereographic::LatitudeOfFirstGridPoint,18145030);
    message->setProperty(Property::GridSection::PolarStereographic::LongitudeOfFirstGridPoint,217107456);
    message->setProperty(Property::GridSection::PolarStereographic::ResolutionAndComponentFlag,8);
    message->setProperty(Property::GridSection::PolarStereographic::LaD,60000000);
    message->setProperty(Property::GridSection::PolarStereographic::OrientationOfTheGrid,249000000);
    message->setProperty(Property::GridSection::PolarStereographic::Dx,10000000);
    message->setProperty(Property::GridSection::PolarStereographic::Dy,10000000);
    message->setProperty(Property::GridSection::PolarStereographic::ProjectionCentreFlag,0LL);
    message->setProperty(Property::GridSection::PolarStereographic::ScanningMode,64);
*/

    message->setProperty(Property::GridSection::LambertConformal::Nx,width);
    message->setProperty(Property::GridSection::LambertConformal::Ny,height);
    message->setProperty(Property::GridSection::LambertConformal::LatitudeOfFirstGridPoint,1000000);
    message->setProperty(Property::GridSection::LambertConformal::LongitudeOfFirstGridPoint,214500000);
    message->setProperty(Property::GridSection::LambertConformal::ResolutionAndComponentFlags,56);
    message->setProperty(Property::GridSection::LambertConformal::LaD,50000000);
    message->setProperty(Property::GridSection::LambertConformal::LoV,253000000);
    message->setProperty(Property::GridSection::LambertConformal::Dx,32463000);
    message->setProperty(Property::GridSection::LambertConformal::Dy,32463000);
    message->setProperty(Property::GridSection::LambertConformal::ProjectionCentreFlag,0LL);
    message->setProperty(Property::GridSection::LambertConformal::ScanningMode,64);
    message->setProperty(Property::GridSection::LambertConformal::Latin1,50000000);
    message->setProperty(Property::GridSection::LambertConformal::Latin2,50000000);
    message->setProperty(Property::GridSection::LambertConformal::LatitudeOfSouthernPole,0LL);
    message->setProperty(Property::GridSection::LambertConformal::LongitudeOfSouthernPole,0LL);


    /*
 *
 *
    message->setProperty(Property::GridSection::LatLon::IDirectionIncrement,125000);
    message->setProperty(Property::GridSection::LatLon::JDirectionIncrement,125000);
    message->setProperty(Property::GridSection::LatLon::ScanningMode,0LL);
  */
    message->setProperty(Property::GridSection::EarthShape::ShapeOfTheEarth,6);
/*
    message->setProperty(Property::GridSection::Grid::Ni,width);
    message->setProperty(Property::GridSection::Grid::Nj,height);
    message->setProperty(Property::GridSection::Grid::BasicAngleOfTheInitialProductionDomain,0LL);
    //message->setProperty(Property::GridSection::Grid::SubdivisionsOfBasicAngle,0LL);
    message->setProperty(Property::GridSection::Grid::LatitudeOfFirstGridPoint,-22500000);
    message->setProperty(Property::GridSection::Grid::LongitudeOfFirstGridPoint,-26000000);
    message->setProperty(Property::GridSection::Grid::LatitudeOfLastGridPoint,35000000);
    message->setProperty(Property::GridSection::Grid::LongitudeOfLastGridPoint,40000000);
    message->setProperty(Property::GridSection::Grid::ResolutionAndComponentFlags,48);

    message->setProperty(Property::GridSection::Rotation::LatitudeOfSouthernPole,-30000000);
    message->setProperty(Property::GridSection::Rotation::LongitudeOfSouthernPole,0LL);
    message->setProperty(Property::GridSection::Rotation::AngleOfRotation,0LL);
*/

    message->setProperty(Property::RepresentationSection::RepresentationTemplateNumber,RepresentationSection::Template::GridDataRepresentation);
    message->setProperty(Property::RepresentationSection::Packing::BinaryScaleFactor,-11);
    message->setProperty(Property::RepresentationSection::Packing::DecimalScaleFactor,0LL);
    message->setProperty(Property::RepresentationSection::Packing::BitsPerValue,16);

    message->setProperty(Property::RepresentationSection::OriginalValues::TypeOfOriginalFieldValues,0LL);

    message->initSpatialReference();
    T::Coordinate_svec coordinates = message->getGridLatLonCoordinates();

    T::ParamValue_vec values;
    dataMessage->getGridValueVectorByCoordinateList(T::CoordinateTypeValue::LATLON_COORDINATES,*coordinates,1,values);

    message->setGridValues(values);

  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
  }
}





void setMessage_Mercator_v2(GRID::Message *message,GRID::Message *dataMessage)
{
  try
  {
    using namespace SmartMet::GRIB2;

    int width = 300;
    int height = 380;
    int sz = width * height;
    int levelType = 1;
    int level = 0;

    // ### INDICATOR SECTION ###

    message->setProperty(Property::IndicatorSection::Discipline,0LL);


    // ### IDENTIFICATION SECTION ###

    message->setProperty(Property::IdentificationSection::Centre,78);
    message->setProperty(Property::IdentificationSection::SubCentre,255);
    message->setProperty(Property::IdentificationSection::TablesVersion,15);
    message->setProperty(Property::IdentificationSection::LocalTablesVersion,1);
    message->setProperty(Property::IdentificationSection::SignificanceOfReferenceTime,1);
    message->setProperty(Property::IdentificationSection::Year,2018);
    message->setProperty(Property::IdentificationSection::Month,8);
    message->setProperty(Property::IdentificationSection::Day,20);
    message->setProperty(Property::IdentificationSection::Hour,0LL);
    message->setProperty(Property::IdentificationSection::Minute,0LL);
    message->setProperty(Property::IdentificationSection::Second,0LL);
    message->setProperty(Property::IdentificationSection::ProductionStatusOfProcessedData,0LL);
    message->setProperty(Property::IdentificationSection::TypeOfProcessedData,1);

    // ### PRODUCT SECTION ###

    message->setProperty(Property::ProductSection::ProductDefinitionTemplateNumber,ProductSection::Template::NormalProduct);
    message->setProperty(Property::ProductSection::NV,0LL);

    message->setProperty(Property::ProductSection::ParameterSettings::ParameterCategory,2);
    message->setProperty(Property::ProductSection::ParameterSettings::ParameterNumber,22);
    message->setProperty(Property::ProductSection::ParameterSettings::TypeOfGeneratingProcess,2);
    message->setProperty(Property::ProductSection::ParameterSettings::BackgroundProcess,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::GeneratingProcessIdentifier,2);
    message->setProperty(Property::ProductSection::ParameterSettings::HoursAfterDataCutoff,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::MinutesAfterDataCutoff,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::IndicatorOfUnitOfTimeRange,0LL);
    message->setProperty(Property::ProductSection::ParameterSettings::ForecastTime,0LL);

    message->setProperty(Property::ProductSection::HorizontalSettings::TypeOfFirstFixedSurface,levelType);
    message->setProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfFirstFixedSurface,0LL);
    message->setProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfFirstFixedSurface,level);
    //message->setProperty(Property::ProductSection::HorizontalSettings::TypeOfSecondFixedSurface,0LL);
    //message->setProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfSecondFixedSurface,0LL);
    //message->setProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfSecondFixedSurface,0LL);

/*
    message->setProperty(Property::ProductSection::StatisticalSettings::YearOfEndOfOverallTimeInterval,2018);
    message->setProperty(Property::ProductSection::StatisticalSettings::MonthOfEndOfOverallTimeInterval,8);
    message->setProperty(Property::ProductSection::StatisticalSettings::DayOfEndOfOverallTimeInterval,20);
    message->setProperty(Property::ProductSection::StatisticalSettings::HourOfEndOfOverallTimeInterval,1);
    message->setProperty(Property::ProductSection::StatisticalSettings::MinuteOfEndOfOverallTimeInterval,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::SecondOfEndOfOverallTimeInterval,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::NumberOfTimeRange,1LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::NumberOfMissingInStatisticalProcess,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::TypeOfStatisticalProcessing,2);
    message->setProperty(Property::ProductSection::StatisticalSettings::TypeOfTimeIncrement,2);
    message->setProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeRange,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::LengthOfTimeRange,60);
    //message->setProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeIncrement,0LL);
    message->setProperty(Property::ProductSection::StatisticalSettings::TimeIncrement,0LL);
*/

    //message->setProperty(Property::ProductSection::LevelId,103);
    //message->setProperty(Property::ProductSection::Level,2);

    //message->setGridGeometry(401);  // 1097 x 657
    //message->setGribParameter(std::string("132"));

    message->setProperty(Property::GridSection::SourceOfGridDefinition,0LL);
    message->setProperty(Property::GridSection::NumberOfGridPoints,sz);
    message->setProperty(Property::GridSection::NumberOfOctetsForNumberOfPoints,0LL);
    message->setProperty(Property::GridSection::InterpretationOfNumberOfPoints,0LL);
    message->setProperty(Property::GridSection::GridDefinitionTemplateNumber,GridSection::Template::Mercator);
    //message->setProperty(Property::GridSection::GridDefinitionTemplateNumber,GridSection::Template::PolarStereographic);
    //message->setProperty(Property::GridSection::GridDefinitionTemplateNumber,GridSection::Template::RotatedLatLon);


    message->setProperty(Property::GridSection::Mercator::Ni,width);
    message->setProperty(Property::GridSection::Mercator::Nj,height);
    message->setProperty(Property::GridSection::Mercator::LatitudeOfFirstGridPoint,3949000);
    message->setProperty(Property::GridSection::Mercator::LongitudeOfFirstGridPoint,95372000);
    message->setProperty(Property::GridSection::Mercator::LaD,15850000);
    message->setProperty(Property::GridSection::Mercator::LatitudeOfLastGridPoint,27090000);
    message->setProperty(Property::GridSection::Mercator::LongitudeOfLastGridPoint,124628);
    message->setProperty(Property::GridSection::Mercator::OrientationOfTheGrid,0);
    message->setProperty(Property::GridSection::Mercator::Di,3000000);
    message->setProperty(Property::GridSection::Mercator::Dj,3000000);
    message->setProperty(Property::GridSection::Mercator::ResolutionAndComponentFlags,129);
    message->setProperty(Property::GridSection::Mercator::ScanningMode,64);

    message->setProperty(Property::GridSection::EarthShape::ShapeOfTheEarth,6);

    message->setProperty(Property::RepresentationSection::RepresentationTemplateNumber,RepresentationSection::Template::GridDataRepresentation);
    message->setProperty(Property::RepresentationSection::Packing::BinaryScaleFactor,0);
    message->setProperty(Property::RepresentationSection::Packing::DecimalScaleFactor,0LL);
    message->setProperty(Property::RepresentationSection::Packing::BitsPerValue,32);

    message->setProperty(Property::RepresentationSection::OriginalValues::TypeOfOriginalFieldValues,0LL);

    message->initSpatialReference();
    T::Coordinate_svec coordinates = message->getGridLatLonCoordinates();

    T::ParamValue_vec values;
    dataMessage->getGridValueVectorByCoordinateList(T::CoordinateTypeValue::LATLON_COORDINATES,*coordinates,1,values);

    message->setGridValues(values);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
  }
}




int main(int argc, char *argv[])
{
  try
  {
    if (argc < 6)
    {
      print_usage();
      return -1;
    }

    init();

    ConfigurationFile configurationFile(argv[1]);
    configurationFile.print(std::cout,0,0);

    int version = 0;
    configurationFile.getAttributeValue("Grib2.IndicatorSection.EditionNumber",version);

    if (version == 0)
      configurationFile.getAttributeValue("Grib1.IndicatorSection.EditionNumber",version);

    if (version == 0)
    {
      printf("Unknow grib version (%d)!\n",version);
      return -2;
    }

    uint optionFlags = SmartMet::GRID::PrintFlag::data;
    std::string sourceFilename = argv[2];
    std::size_t startIndex = toUInt32(argv[3]);
    std::size_t endIndex = toUInt32(argv[4]);
    std::string targetFilename = argv[5];

    SmartMet::GRID::GridFile sourceGridFile;
    sourceGridFile.read(sourceFilename);

    GRID::GridFile targetGridFile;

    std::size_t messageCount = sourceGridFile.getNumberOfMessages();
    if (messageCount == 0)
    {
      printf("No messaes found!\n");
      return -3;
    }

    if (messageCount > endIndex)
      endIndex = messageCount-1;

    for (std::size_t m=startIndex; m<=endIndex; m++)
    {
      GRID::Message *dataMessage = sourceGridFile.getMessageByIndex(m);
      if (dataMessage == nullptr)
      {
        printf("*** Cannot get data message!\n");
        return -4;
      }

      T::TimeString forecastTime = dataMessage->getForecastTime();

      int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
      splitTimeString(forecastTime,year,month,day,hour,minute,second);


      GRID::Message *newMessage = nullptr;

      if (version == 1)
      {
        using namespace SmartMet::GRIB1;

        newMessage = targetGridFile.newMessage(T::FileTypeValue::Grib1);

        printf("** VERSION %u\n",dataMessage->getGribVersion());

        if (version == dataMessage->getGribVersion())
        {
          long long v = 0;

          auto properties = gribProperty.getPropertyVector();

          for (auto it = properties.begin(); it != properties.end(); ++it)
          {
            if (configurationFile.getAttributeValue(it->first.c_str(),v))
            {
              newMessage->setProperty(it->second,v);
              std::cout << "SET " << it->first << " : " << v << "\n";
            }
            else
            if (dataMessage->getProperty(it->second,v))
            {
              std::cout << "*SET " << it->first << " : " << v << "\n";
              newMessage->setProperty(it->second,v);
            }
          }

  #if 0

          if (dataMessage->getProperty(Property::IndicatorSection::EditionNumber,v))
            newMessage->setProperty(Property::IndicatorSection::EditionNumber,v);

          if (dataMessage->getProperty(Property::ProductSection::TableVersion,v))
            newMessage->setProperty(Property::ProductSection::TableVersion,v);

          if (dataMessage->getProperty(Property::ProductSection::Centre,v))
            newMessage->setProperty(Property::ProductSection::Centre,v);

          if (dataMessage->getProperty(Property::ProductSection::GeneratingProcessIdentifier,v))
            newMessage->setProperty(Property::ProductSection::GeneratingProcessIdentifier,v);

          if (dataMessage->getProperty(Property::ProductSection::GridDefinitionNumber,v))
            newMessage->setProperty(Property::ProductSection::GridDefinitionNumber,v);

          if (dataMessage->getProperty(Property::ProductSection::SectionFlags,v))
            newMessage->setProperty(Property::ProductSection::SectionFlags,v);

          if (dataMessage->getProperty(Property::ProductSection::IndicatorOfParameter,v))
            newMessage->setProperty(Property::ProductSection::IndicatorOfParameter,v);

          if (dataMessage->getProperty(Property::ProductSection::IndicatorOfTypeOfLevel,v))
            newMessage->setProperty(Property::ProductSection::IndicatorOfTypeOfLevel,v);

          if (dataMessage->getProperty(Property::ProductSection::Level,v))
            newMessage->setProperty(Property::ProductSection::Level,v);

          if (dataMessage->getProperty(Property::ProductSection::YearOfCentury,v))
            newMessage->setProperty(Property::ProductSection::YearOfCentury,v);

          if (dataMessage->getProperty(Property::ProductSection::Month,v))
            newMessage->setProperty(Property::ProductSection::Month,v);

          if (dataMessage->getProperty(Property::ProductSection::Day,v))
            newMessage->setProperty(Property::ProductSection::Day,v);

          if (dataMessage->getProperty(Property::ProductSection::Hour,v))
            newMessage->setProperty(Property::ProductSection::Hour,v);

          if (dataMessage->getProperty(Property::ProductSection::Minute,v))
            newMessage->setProperty(Property::ProductSection::Minute,v);

          if (dataMessage->getProperty(Property::ProductSection::UnitOfTimeRange,v))
            newMessage->setProperty(Property::ProductSection::UnitOfTimeRange,v);

          if (dataMessage->getProperty(Property::ProductSection::P1,v))
            newMessage->setProperty(Property::ProductSection::P1,v);

          if (dataMessage->getProperty(Property::ProductSection::P2,v))
            newMessage->setProperty(Property::ProductSection::P2,v);

          if (dataMessage->getProperty(Property::ProductSection::TimeRangeIndicator,v))
            newMessage->setProperty(Property::ProductSection::TimeRangeIndicator,v);

          if (dataMessage->getProperty(Property::ProductSection::NumberIncludedInAverage,v))
            newMessage->setProperty(Property::ProductSection::NumberIncludedInAverage,v);

          if (dataMessage->getProperty(Property::ProductSection::NumberMissingFromAveragesOrAccumulations,v))
            newMessage->setProperty(Property::ProductSection::NumberMissingFromAveragesOrAccumulations,v);

          if (dataMessage->getProperty(Property::ProductSection::CenturyOfReferenceTimeOfData,v))
            newMessage->setProperty(Property::ProductSection::CenturyOfReferenceTimeOfData,v);

          if (dataMessage->getProperty(Property::ProductSection::SubCentre,v))
            newMessage->setProperty(Property::ProductSection::SubCentre,v);

          if (dataMessage->getProperty(Property::ProductSection::DecimalScaleFactor,v))
            newMessage->setProperty(Property::ProductSection::DecimalScaleFactor,v);

          if (dataMessage->getProperty(Property::ProductSection::ForecastType,v))
            newMessage->setProperty(Property::ProductSection::ForecastType,v);

          if (dataMessage->getProperty(Property::ProductSection::ForecastNumber,v))
            newMessage->setProperty(Property::ProductSection::ForecastNumber,v);

          if (dataMessage->getProperty(Property::DataSection::Flags,v))
            newMessage->setProperty(Property::DataSection::Flags,v);

          if (dataMessage->getProperty(Property::DataSection::BinaryScaleFactor,v))
            newMessage->setProperty(Property::DataSection::BinaryScaleFactor,v);

          //if (dataMessage->getProperty(Property::DataSection::ReferenceValue,rv))
          //  newMessage->setProperty(Property::DataSection::ReferenceValue,rv);

          if (dataMessage->getProperty(Property::DataSection::BitsPerValue,v))
            newMessage->setProperty(Property::DataSection::BitsPerValue,v);

          if (dataMessage->getProperty(Property::DataSection::PackingMethod,v))
            newMessage->setProperty(Property::DataSection::PackingMethod,v);
  #endif

        }
      }

      if (version == 2)
      {
        using namespace SmartMet::GRIB2;

        newMessage = targetGridFile.newMessage(T::FileTypeValue::Grib2);

        long long v = 0;

        auto properties = gribProperty.getPropertyVector();

        for (auto it = properties.begin(); it != properties.end(); ++it)
        {
          if (configurationFile.getAttributeValue(it->first.c_str(),v))
          {
            newMessage->setProperty(it->second,v);
            std::cout << "SET " << it->first << " : " << v << "\n";
          }
          else
          if (dataMessage->getProperty(it->second,v))
          {
            std::cout << "*SET " << it->first << " : " << v << "\n";
            newMessage->setProperty(it->second,v);
          }
        }

  #if 0
        if (dataMessage->getProperty(Property::IndicatorSection::Discipline,v))
          newMessage->setProperty(Property::IndicatorSection::Discipline,v);

        if (dataMessage->getProperty(Property::IndicatorSection::EditionNumber,v))
          newMessage->setProperty(Property::IndicatorSection::EditionNumber,v);

        if (dataMessage->getProperty(Property::IdentificationSection::Centre,v))
          newMessage->setProperty(Property::IdentificationSection::Centre,v);

        if (dataMessage->getProperty(Property::IdentificationSection::SubCentre,v))
          newMessage->setProperty(Property::IdentificationSection::SubCentre,v);

        if (dataMessage->getProperty(Property::IdentificationSection::TablesVersion,v))
          newMessage->setProperty(Property::IdentificationSection::TablesVersion,v);

        if (dataMessage->getProperty(Property::IdentificationSection::LocalTablesVersion,v))
          newMessage->setProperty(Property::IdentificationSection::LocalTablesVersion,v);

        if (dataMessage->getProperty(Property::IdentificationSection::SignificanceOfReferenceTime,v))
          newMessage->setProperty(Property::IdentificationSection::SignificanceOfReferenceTime,v);

        if (dataMessage->getProperty(Property::IdentificationSection::Year,v))
          newMessage->setProperty(Property::IdentificationSection::Year,v);

        if (dataMessage->getProperty(Property::IdentificationSection::Month,v))
          newMessage->setProperty(Property::IdentificationSection::Month,v);

        if (dataMessage->getProperty(Property::IdentificationSection::Day,v))
          newMessage->setProperty(Property::IdentificationSection::Day,v);

        if (dataMessage->getProperty(Property::IdentificationSection::Hour,v))
          newMessage->setProperty(Property::IdentificationSection::Hour,v);

        if (dataMessage->getProperty(Property::IdentificationSection::Minute,v))
          newMessage->setProperty(Property::IdentificationSection::Minute,v);

        if (dataMessage->getProperty(Property::IdentificationSection::Second,v))
          newMessage->setProperty(Property::IdentificationSection::Second,v);

        if (dataMessage->getProperty(Property::IdentificationSection::ProductionStatusOfProcessedData,v))
          newMessage->setProperty(Property::IdentificationSection::ProductionStatusOfProcessedData,v);

        if (dataMessage->getProperty(Property::IdentificationSection::TypeOfProcessedData,v))
          newMessage->setProperty(Property::IdentificationSection::TypeOfProcessedData,v);

        if (dataMessage->getProperty(Property::GridSection::SourceOfGridDefinition,v))
          newMessage->setProperty(Property::GridSection::SourceOfGridDefinition,v);

        if (dataMessage->getProperty(Property::GridSection::NumberOfGridPoints,v))
          newMessage->setProperty(Property::GridSection::NumberOfGridPoints,v);

        if (dataMessage->getProperty(Property::GridSection::NumberOfOctetsForNumberOfPoints,v))
          newMessage->setProperty(Property::GridSection::NumberOfOctetsForNumberOfPoints,v);

        if (dataMessage->getProperty(Property::GridSection::InterpretationOfNumberOfPoints,v))
          newMessage->setProperty(Property::GridSection::InterpretationOfNumberOfPoints,v);


        if (dataMessage->getProperty(Property::GridSection::EarthShape::ShapeOfTheEarth,v))
          newMessage->setProperty(Property::GridSection::EarthShape::ShapeOfTheEarth,v);

        if (dataMessage->getProperty(Property::GridSection::EarthShape::ScaleFactorOfRadiusOfSphericalEarth,v))
          newMessage->setProperty(Property::GridSection::EarthShape::ScaleFactorOfRadiusOfSphericalEarth,v);

        if (dataMessage->getProperty(Property::GridSection::EarthShape::ScaledValueOfRadiusOfSphericalEarth,v))
          newMessage->setProperty(Property::GridSection::EarthShape::ScaledValueOfRadiusOfSphericalEarth,v);

        if (dataMessage->getProperty(Property::GridSection::EarthShape::ScaleFactorOfEarthMajorAxis,v))
          newMessage->setProperty(Property::GridSection::EarthShape::ScaleFactorOfEarthMajorAxis,v);

        if (dataMessage->getProperty(Property::GridSection::EarthShape::ScaledValueOfEarthMajorAxis,v))
          newMessage->setProperty(Property::GridSection::EarthShape::ScaledValueOfEarthMajorAxis,v);

        if (dataMessage->getProperty(Property::GridSection::EarthShape::ScaleFactorOfEarthMinorAxis,v))
          newMessage->setProperty(Property::GridSection::EarthShape::ScaleFactorOfEarthMinorAxis,v);

        if (dataMessage->getProperty(Property::GridSection::EarthShape::ScaledValueOfEarthMinorAxis,v))
          newMessage->setProperty(Property::GridSection::EarthShape::ScaledValueOfEarthMinorAxis,v);

        if (dataMessage->getProperty(Property::GridSection::Grid::Ni,v))
          newMessage->setProperty(Property::GridSection::Grid::Ni,v);

        if (dataMessage->getProperty(Property::GridSection::Grid::Nj,v))
          newMessage->setProperty(Property::GridSection::Grid::Nj,v);

        if (dataMessage->getProperty(Property::GridSection::Grid::BasicAngleOfTheInitialProductionDomain,v))
          newMessage->setProperty(Property::GridSection::Grid::BasicAngleOfTheInitialProductionDomain,v);

        if (dataMessage->getProperty(Property::GridSection::Grid::SubdivisionsOfBasicAngle,v))
          newMessage->setProperty(Property::GridSection::Grid::SubdivisionsOfBasicAngle,v);

        if (dataMessage->getProperty(Property::GridSection::Grid::LatitudeOfFirstGridPoint,v))
          newMessage->setProperty(Property::GridSection::Grid::LatitudeOfFirstGridPoint,v);

        if (dataMessage->getProperty(Property::GridSection::Grid::LongitudeOfFirstGridPoint,v))
          newMessage->setProperty(Property::GridSection::Grid::LongitudeOfFirstGridPoint,v);

        if (dataMessage->getProperty(Property::GridSection::Grid::LatitudeOfLastGridPoint,v))
          newMessage->setProperty(Property::GridSection::Grid::LatitudeOfLastGridPoint,v);

        if (dataMessage->getProperty(Property::GridSection::Grid::LongitudeOfLastGridPoint,v))
          newMessage->setProperty(Property::GridSection::Grid::LongitudeOfLastGridPoint,v);

        if (dataMessage->getProperty(Property::GridSection::Grid::ResolutionAndComponentFlags,v))
          newMessage->setProperty(Property::GridSection::Grid::ResolutionAndComponentFlags,v);

        if (dataMessage->getProperty(Property::GridSection::Rotation::LatitudeOfSouthernPole,v))
          newMessage->setProperty(Property::GridSection::Rotation::LatitudeOfSouthernPole,v);

        if (dataMessage->getProperty(Property::GridSection::Rotation::LongitudeOfSouthernPole,v))
          newMessage->setProperty(Property::GridSection::Rotation::LongitudeOfSouthernPole,v);

        if (dataMessage->getProperty(Property::GridSection::Rotation::AngleOfRotation,v))
          newMessage->setProperty(Property::GridSection::Rotation::AngleOfRotation,v);


        if (dataMessage->getProperty(Property::GridSection::LatLon::IDirectionIncrement,v))
          newMessage->setProperty(Property::GridSection::LatLon::IDirectionIncrement,v);

        if (dataMessage->getProperty(Property::GridSection::LatLon::JDirectionIncrement,v))
          newMessage->setProperty(Property::GridSection::LatLon::JDirectionIncrement,v);

        if (dataMessage->getProperty(Property::GridSection::LatLon::ScanningMode,v))
          newMessage->setProperty(Property::GridSection::LatLon::ScanningMode,v);

        if (dataMessage->getProperty(Property::GridSection::PolarStereographic::Nx,v))
          newMessage->setProperty(Property::GridSection::PolarStereographic::Nx,v);

        if (dataMessage->getProperty(Property::GridSection::PolarStereographic::Ny,v))
          newMessage->setProperty(Property::GridSection::PolarStereographic::Ny,v);

        if (dataMessage->getProperty(Property::GridSection::PolarStereographic::LatitudeOfFirstGridPoint,v))
          newMessage->setProperty(Property::GridSection::PolarStereographic::LatitudeOfFirstGridPoint,v);

        if (dataMessage->getProperty(Property::GridSection::PolarStereographic::LongitudeOfFirstGridPoint,v))
          newMessage->setProperty(Property::GridSection::PolarStereographic::LongitudeOfFirstGridPoint,v);

        if (dataMessage->getProperty(Property::GridSection::PolarStereographic::ResolutionAndComponentFlags,v))
          newMessage->setProperty(Property::GridSection::PolarStereographic::ResolutionAndComponentFlags,v);

        if (dataMessage->getProperty(Property::GridSection::PolarStereographic::LaD,v))
          newMessage->setProperty(Property::GridSection::PolarStereographic::LaD,v);

        if (dataMessage->getProperty(Property::GridSection::PolarStereographic::OrientationOfTheGrid,v))
          newMessage->setProperty(Property::GridSection::PolarStereographic::OrientationOfTheGrid,v);

        if (dataMessage->getProperty(Property::GridSection::PolarStereographic::Dx,v))
          newMessage->setProperty(Property::GridSection::PolarStereographic::Dx,v);

        if (dataMessage->getProperty(Property::GridSection::PolarStereographic::Dy,v))
          newMessage->setProperty(Property::GridSection::PolarStereographic::Dy,v);

        if (dataMessage->getProperty(Property::GridSection::PolarStereographic::ProjectionCentreFlag,v))
          newMessage->setProperty(Property::GridSection::PolarStereographic::ProjectionCentreFlag,v);

        if (dataMessage->getProperty(Property::GridSection::PolarStereographic::ScanningMode,v))
          newMessage->setProperty(Property::GridSection::PolarStereographic::ScanningMode,v);

        if (dataMessage->getProperty(Property::GridSection::PolarStereographic::LastProperty,v))
          newMessage->setProperty(Property::GridSection::PolarStereographic::LastProperty,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::Nx,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::Nx,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::Ny,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::Ny,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::LatitudeOfFirstGridPoint,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::LatitudeOfFirstGridPoint,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::LongitudeOfFirstGridPoint,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::LongitudeOfFirstGridPoint,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::ResolutionAndComponentFlags,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::ResolutionAndComponentFlags,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::LaD,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::LaD,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::LoV,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::LoV,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::Dx,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::Dx,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::Dy,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::Dy,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::ProjectionCentreFlag,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::ProjectionCentreFlag,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::ScanningMode,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::ScanningMode,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::Latin1,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::Latin1,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::Latin2,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::Latin2,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::LatitudeOfSouthernPole,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::LatitudeOfSouthernPole,v);

        if (dataMessage->getProperty(Property::GridSection::LambertConformal::LongitudeOfSouthernPole,v))
          newMessage->setProperty(Property::GridSection::LambertConformal::LongitudeOfSouthernPole,v);

        if (dataMessage->getProperty(Property::GridSection::Mercator::Ni,v))
          newMessage->setProperty(Property::GridSection::Mercator::Ni,v);

        if (dataMessage->getProperty(Property::GridSection::Mercator::Nj,v))
          newMessage->setProperty(Property::GridSection::Mercator::Nj,v);

        if (dataMessage->getProperty(Property::GridSection::Mercator::LatitudeOfFirstGridPoint,v))
          newMessage->setProperty(Property::GridSection::Mercator::LatitudeOfFirstGridPoint,v);

        if (dataMessage->getProperty(Property::GridSection::Mercator::LongitudeOfFirstGridPoint,v))
          newMessage->setProperty(Property::GridSection::Mercator::LongitudeOfFirstGridPoint,v);

        if (dataMessage->getProperty(Property::GridSection::Mercator::LaD,v))
          newMessage->setProperty(Property::GridSection::Mercator::LaD,v);

        if (dataMessage->getProperty(Property::GridSection::Mercator::LatitudeOfLastGridPoint,v))
          newMessage->setProperty(Property::GridSection::Mercator::LatitudeOfLastGridPoint,v);

        if (dataMessage->getProperty(Property::GridSection::Mercator::LongitudeOfLastGridPoint,v))
          newMessage->setProperty(Property::GridSection::Mercator::LongitudeOfLastGridPoint,v);

        if (dataMessage->getProperty(Property::GridSection::Mercator::OrientationOfTheGrid,v))
          newMessage->setProperty(Property::GridSection::Mercator::OrientationOfTheGrid,v);

        if (dataMessage->getProperty(Property::GridSection::Mercator::Di,v))
          newMessage->setProperty(Property::GridSection::Mercator::Di,v);

        if (dataMessage->getProperty(Property::GridSection::Mercator::Dj,v))
          newMessage->setProperty(Property::GridSection::Mercator::Dj,v);

        if (dataMessage->getProperty(Property::GridSection::Mercator::ResolutionAndComponentFlags,v))
          newMessage->setProperty(Property::GridSection::Mercator::ResolutionAndComponentFlags,v);

        if (dataMessage->getProperty(Property::GridSection::Mercator::ScanningMode,v))
          newMessage->setProperty(Property::GridSection::Mercator::ScanningMode,v);



        if (dataMessage->getProperty(Property::ProductSection::ProductDefinitionTemplateNumber,v))
          newMessage->setProperty(Property::ProductSection::ProductDefinitionTemplateNumber,v);

        if (dataMessage->getProperty(Property::ProductSection::ParameterSettings::ParameterCategory,v))
          newMessage->setProperty(Property::ProductSection::ParameterSettings::ParameterCategory,v);

        if (dataMessage->getProperty(Property::ProductSection::ParameterSettings::ParameterNumber,v))
          newMessage->setProperty(Property::ProductSection::ParameterSettings::ParameterNumber,v);

        if (dataMessage->getProperty(Property::ProductSection::ParameterSettings::TypeOfGeneratingProcess,v))
          newMessage->setProperty(Property::ProductSection::ParameterSettings::TypeOfGeneratingProcess,v);

        if (dataMessage->getProperty(Property::ProductSection::ParameterSettings::BackgroundProcess,v))
          newMessage->setProperty(Property::ProductSection::ParameterSettings::BackgroundProcess,v);

        if (dataMessage->getProperty(Property::ProductSection::ParameterSettings::GeneratingProcessIdentifier,v))
          newMessage->setProperty(Property::ProductSection::ParameterSettings::GeneratingProcessIdentifier,v);

        if (dataMessage->getProperty(Property::ProductSection::ParameterSettings::HoursAfterDataCutoff,v))
          newMessage->setProperty(Property::ProductSection::ParameterSettings::HoursAfterDataCutoff,v);

        if (dataMessage->getProperty(Property::ProductSection::ParameterSettings::MinutesAfterDataCutoff,v))
          newMessage->setProperty(Property::ProductSection::ParameterSettings::MinutesAfterDataCutoff,v);

        if (dataMessage->getProperty(Property::ProductSection::ParameterSettings::IndicatorOfUnitOfTimeRange,v))
          newMessage->setProperty(Property::ProductSection::ParameterSettings::IndicatorOfUnitOfTimeRange,v);

        if (dataMessage->getProperty(Property::ProductSection::ParameterSettings::ForecastTime,v))
          newMessage->setProperty(Property::ProductSection::ParameterSettings::ForecastTime,v);

        if (dataMessage->getProperty(Property::ProductSection::HorizontalSettings::TypeOfFirstFixedSurface,v))
          newMessage->setProperty(Property::ProductSection::HorizontalSettings::TypeOfFirstFixedSurface,v);

        if (dataMessage->getProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfFirstFixedSurface,v))
          newMessage->setProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfFirstFixedSurface,v);

        if (dataMessage->getProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfFirstFixedSurface,v))
          newMessage->setProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfFirstFixedSurface,v);

        if (dataMessage->getProperty(Property::ProductSection::HorizontalSettings::TypeOfSecondFixedSurface,v))
          newMessage->setProperty(Property::ProductSection::HorizontalSettings::TypeOfSecondFixedSurface,v);

        if (dataMessage->getProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfSecondFixedSurface,v))
          newMessage->setProperty(Property::ProductSection::HorizontalSettings::ScaleFactorOfSecondFixedSurface,v);

        if (dataMessage->getProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfSecondFixedSurface,v))
          newMessage->setProperty(Property::ProductSection::HorizontalSettings::ScaledValueOfSecondFixedSurface,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::YearOfEndOfOverallTimeInterval,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::YearOfEndOfOverallTimeInterval,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::MonthOfEndOfOverallTimeInterval,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::MonthOfEndOfOverallTimeInterval,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::DayOfEndOfOverallTimeInterval,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::DayOfEndOfOverallTimeInterval,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::HourOfEndOfOverallTimeInterval,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::HourOfEndOfOverallTimeInterval,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::MinuteOfEndOfOverallTimeInterval,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::MinuteOfEndOfOverallTimeInterval,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::SecondOfEndOfOverallTimeInterval,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::SecondOfEndOfOverallTimeInterval,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::NumberOfTimeRange,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::NumberOfTimeRange,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::NumberOfMissingInStatisticalProcess,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::NumberOfMissingInStatisticalProcess,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::TypeOfStatisticalProcessing,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::TypeOfStatisticalProcessing,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::TypeOfTimeIncrement,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::TypeOfTimeIncrement,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeRange,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeRange,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::LengthOfTimeRange,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::LengthOfTimeRange,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeIncrement,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::IndicatorOfUnitForTimeIncrement,v);

        if (dataMessage->getProperty(Property::ProductSection::StatisticalSettings::TimeIncrement,v))
          newMessage->setProperty(Property::ProductSection::StatisticalSettings::TimeIncrement,v);

        if (dataMessage->getProperty(Property::ProductSection::EpsSettings::TypeOfEnsembleForecast,v))
          newMessage->setProperty(Property::ProductSection::EpsSettings::TypeOfEnsembleForecast,v);

        if (dataMessage->getProperty(Property::ProductSection::EpsSettings::PerturbationNumber,v))
          newMessage->setProperty(Property::ProductSection::EpsSettings::PerturbationNumber,v);

        if (dataMessage->getProperty(Property::ProductSection::EpsSettings::NumberOfForecastsInEnsemble,v))
          newMessage->setProperty(Property::ProductSection::EpsSettings::NumberOfForecastsInEnsemble,v);

        if (dataMessage->getProperty(Property::ProductSection::DerivedSettings::DerivedForecast,v))
          newMessage->setProperty(Property::ProductSection::DerivedSettings::DerivedForecast,v);

        if (dataMessage->getProperty(Property::ProductSection::DerivedSettings::NumberOfForecastsInEnsemble,v))
          newMessage->setProperty(Property::ProductSection::DerivedSettings::NumberOfForecastsInEnsemble,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::ClusterIdentifier,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::ClusterIdentifier,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::NH,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::NH,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::NL,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::NL,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::TotalNumberOfClusters,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::TotalNumberOfClusters,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::ClusteringMethod,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::ClusteringMethod,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::NorthernLatitudeOfClusterDomain,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::NorthernLatitudeOfClusterDomain,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::SouthernLatitudeOfClusterDomain,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::SouthernLatitudeOfClusterDomain,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::EasternLongitudeOfClusterDomain,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::EasternLongitudeOfClusterDomain,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::WesternLongitudeOfClusterDomain,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::WesternLongitudeOfClusterDomain,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::NumberOfForecastsInTheCluster,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::NumberOfForecastsInTheCluster,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::ScaleFactorOfStandardDeviation,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::ScaleFactorOfStandardDeviation,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::ScaledValueOfStandardDeviation,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::ScaledValueOfStandardDeviation,v);

        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::ScaleFactorOfDistanceFromEnsembleMean,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::ScaleFactorOfDistanceFromEnsembleMean,v);


        if (dataMessage->getProperty(Property::ProductSection::RectangularClusterSettings::ScaledValueOfDistanceFromEnsembleMean,v))
          newMessage->setProperty(Property::ProductSection::RectangularClusterSettings::ScaledValueOfDistanceFromEnsembleMean,v);

        if (dataMessage->getProperty(Property::ProductSection::ProbabilitySettings::ForecastProbabilityNumber,v))
          newMessage->setProperty(Property::ProductSection::ProbabilitySettings::ForecastProbabilityNumber,v);

        if (dataMessage->getProperty(Property::ProductSection::ProbabilitySettings::TotalNumberOfForecastProbabilities,v))
          newMessage->setProperty(Property::ProductSection::ProbabilitySettings::TotalNumberOfForecastProbabilities,v);

        if (dataMessage->getProperty(Property::ProductSection::ProbabilitySettings::ProbabilityType,v))
          newMessage->setProperty(Property::ProductSection::ProbabilitySettings::ProbabilityType,v);

        if (dataMessage->getProperty(Property::ProductSection::ProbabilitySettings::ScaleFactorOfLowerLimit,v))
          newMessage->setProperty(Property::ProductSection::ProbabilitySettings::ScaleFactorOfLowerLimit,v);

        if (dataMessage->getProperty(Property::ProductSection::ProbabilitySettings::ScaledValueOfLowerLimit,v))
          newMessage->setProperty(Property::ProductSection::ProbabilitySettings::ScaledValueOfLowerLimit,v);

        if (dataMessage->getProperty(Property::ProductSection::ProbabilitySettings::ScaleFactorOfUpperLimit,v))
          newMessage->setProperty(Property::ProductSection::ProbabilitySettings::ScaleFactorOfUpperLimit,v);

        if (dataMessage->getProperty(Property::ProductSection::ProbabilitySettings::ScaledValueOfUpperLimit,v))
          newMessage->setProperty(Property::ProductSection::ProbabilitySettings::ScaledValueOfUpperLimit,v);

        if (dataMessage->getProperty(Property::ProductSection::PercentileSettings::PercentileValue,v))
          newMessage->setProperty(Property::ProductSection::PercentileSettings::PercentileValue,v);

        if (dataMessage->getProperty(Property::ProductSection::CategoricalSettings::NumberOfCategories,v))
          newMessage->setProperty(Property::ProductSection::CategoricalSettings::NumberOfCategories,v);

        if (dataMessage->getProperty(Property::ProductSection::CategoricalSettings::CategoryType,v))
          newMessage->setProperty(Property::ProductSection::CategoricalSettings::CategoryType,v);

        if (dataMessage->getProperty(Property::ProductSection::CategoricalSettings::CodeFigure,v))
          newMessage->setProperty(Property::ProductSection::CategoricalSettings::CodeFigure,v);

        if (dataMessage->getProperty(Property::ProductSection::CategoricalSettings::ScaleFactorOfLowerLimit,v))
          newMessage->setProperty(Property::ProductSection::CategoricalSettings::ScaleFactorOfLowerLimit,v);

        if (dataMessage->getProperty(Property::ProductSection::CategoricalSettings::ScaledValueOfLowerLimit,v))
          newMessage->setProperty(Property::ProductSection::CategoricalSettings::ScaledValueOfLowerLimit,v);


        if (dataMessage->getProperty(Property::ProductSection::CategoricalSettings::ScaleFactorOfUpperLimit,v))
          newMessage->setProperty(Property::ProductSection::CategoricalSettings::ScaleFactorOfUpperLimit,v);

        if (dataMessage->getProperty(Property::ProductSection::CategoricalSettings::ScaledValueOfUpperLimit,v))
          newMessage->setProperty(Property::ProductSection::CategoricalSettings::ScaledValueOfUpperLimit,v);

        if (dataMessage->getProperty(Property::RepresentationSection::RepresentationTemplateNumber,v))
          newMessage->setProperty(Property::RepresentationSection::RepresentationTemplateNumber,v);

        if (dataMessage->getProperty(Property::RepresentationSection::Packing::ReferenceValue,v))
          newMessage->setProperty(Property::RepresentationSection::Packing::ReferenceValue,v);

        if (dataMessage->getProperty(Property::RepresentationSection::Packing::BinaryScaleFactor,v))
          newMessage->setProperty(Property::RepresentationSection::Packing::BinaryScaleFactor,v);

        if (dataMessage->getProperty(Property::RepresentationSection::Packing::DecimalScaleFactor,v))
          newMessage->setProperty(Property::RepresentationSection::Packing::DecimalScaleFactor,v);

        if (dataMessage->getProperty(Property::RepresentationSection::Packing::BitsPerValue,v))
          newMessage->setProperty(Property::RepresentationSection::Packing::BitsPerValue,v);

        if (dataMessage->getProperty(Property::RepresentationSection::OriginalValues::TypeOfOriginalFieldValues,v))
          newMessage->setProperty(Property::RepresentationSection::OriginalValues::TypeOfOriginalFieldValues,v);

  #endif
      }

  /*
      for (auto attr = configurationFile.mAttributeList.begin(); attr != configurationFile.mAttributeList.end(); ++attr)
      {
        std::cout << "SET " << attr->mName << " : " << attr->mValue << "\n";
        if (!newMessage->setProperty(attr->mName.c_str(),toInt64(attr->mValue)))
          if (!newMessage->setProperty(attr->mName.c_str(),toDouble(attr->mValue)))
            std::cout << " **** FAILED\n";
      }
  */

      newMessage->initSpatialReference();
      T::Coordinate_svec coordinates = newMessage->getGridLatLonCoordinates();

      T::ParamValue_vec values;
      dataMessage->getGridValueVectorByCoordinateList(T::CoordinateTypeValue::LATLON_COORDINATES,*coordinates,1,values);

      newMessage->setGridValues(values);


      //setMessage_LambertConformal(newMessage,dataMessage);
      //setMessage_PolarStereographic(newMessage,dataMessage);
      //setMessage_LatLon(newMessage,dataMessage);
      //setMessage_RotatedLatLon(newMessage,dataMessage);
      //setMessage_Mercator(newMessage,dataMessage);


      //setMessage_LatLon1(newMessage,dataMessage);
      //setMessage_PolarStereographic1(newMessage,dataMessage);
      //setMessage_RotatedLatLon1(newMessage,dataMessage);
      //setMessage_LambertConformal1(newMessage,dataMessage);
      //setMessage_Mercator1(newMessage,dataMessage);


      newMessage->print(std::cout,0,optionFlags);
    }

    targetGridFile.write(targetFilename);

    targetGridFile.print(std::cout,0,optionFlags);

    return 0;

  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
  }
}



