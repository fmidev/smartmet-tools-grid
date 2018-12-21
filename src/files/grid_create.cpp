#include "grid-files/grid/PhysicalGridFile.h"
#include "grid-files/grid/PrintOptions.h"
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/GeneralDefinitions.h"
#include "grid-files/common/ImageFunctions.h"
#include "grid-files/grid/Typedefs.h"
#include "grid-files/grid/ValueCache.h"

#include <iostream>
#include <stdexcept>
#include <string.h>
#include <vector>
#include <sys/time.h>


using namespace SmartMet;
using namespace SmartMet::GRIB2;



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
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}





void print_usage()
{
  printf("-------------------------------------------------------------------------------- \n");
  printf("PROGRAM :\n");
  printf("  The program creates a grid file.\n");
  printf("\n");
  printf("USAGE :\n");
  printf("  grid_create <file>\n");
  printf("\n");
  printf("PARAMETERS :\n");
//  printf("  -coordinates   -- Print coordinate values\n");
  printf("\n");
  printf("EXAMPLES :\n");
//  printf("  grid_dump mygribfile.grib2\n");
//  printf("  grid_dump -coordinates -data mygribfile.grib2\n");
  printf("-------------------------------------------------------------------------------- \n");
  printf("\n");
}






void setMessage_LatLon(GRID::Message *message)
{
  try
  {
    int width = 2880;
    int height = 1441;
    int sz = width * height;
    int levelType = 103;
    int level = 10;

    T::ParamValue_vec values;

    for (int y=0; y<height; y++)
    {
      for (int x=0; x<width; x++)
      {
        long val = random();
        values.push_back(((val % 5000))/1000);
      }
    }


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


    // ### GRID SECTION ###

    message->setProperty(Property::GridSection::GridDefinitionTemplateNumber,GridSection::Template::LatLon);

    message->setProperty(Property::GridSection::SourceOfGridDefinition,0LL);
    message->setProperty(Property::GridSection::NumberOfGridPoints,sz);
    message->setProperty(Property::GridSection::NumberOfOctetsForNumberOfPoints,0LL);
    message->setProperty(Property::GridSection::InterpretationOfNumberOfPoints,0LL);

    message->setProperty(Property::GridSection::EarthShape::ShapeOfTheEarth,6);

    message->setProperty(Property::GridSection::LatLon::IDirectionIncrement,67500);
    message->setProperty(Property::GridSection::LatLon::JDirectionIncrement,67500);
    message->setProperty(Property::GridSection::LatLon::ScanningMode,64);

    message->setProperty(Property::GridSection::Grid::Ni,width);
    message->setProperty(Property::GridSection::Grid::Nj,height);
    message->setProperty(Property::GridSection::Grid::BasicAngleOfTheInitialProductionDomain,0LL);
    //message->setProperty(Property::GridSection::Grid::SubdivisionsOfBasicAngle,0LL);
    message->setProperty(Property::GridSection::Grid::LatitudeOfFirstGridPoint,-45000000);
    message->setProperty(Property::GridSection::Grid::LongitudeOfFirstGridPoint,-90000000);
    message->setProperty(Property::GridSection::Grid::LatitudeOfLastGridPoint,45000000);
    message->setProperty(Property::GridSection::Grid::LongitudeOfLastGridPoint,90000000);
    message->setProperty(Property::GridSection::Grid::ResolutionAndComponentFlags,48);


    // ### REPRESENTATION SECTION ####

    message->setProperty(Property::RepresentationSection::OriginalValues::TypeOfOriginalFieldValues,0LL);

    message->setProperty(Property::RepresentationSection::RepresentationTemplateNumber,RepresentationSection::Template::GridDataRepresentation);
    message->setProperty(Property::RepresentationSection::Packing::BinaryScaleFactor,-11);
    message->setProperty(Property::RepresentationSection::Packing::DecimalScaleFactor,0LL);
    message->setProperty(Property::RepresentationSection::Packing::BitsPerValue,16);


    // ### DATA SECTION ###

    message->setGridValues(values);
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
    exception.printError();
  }
}





void setMessage_RotatedLatLon(GRID::Message *message)
{
  try
  {
    int width = 331;
    int height = 289;
    int sz = width * height;
    int levelType = 103;
    int level = 10;

    T::ParamValue_vec values;

    for (int y=0; y<height; y++)
    {
      for (int x=0; x<width; x++)
      {
        long val = random();
        values.push_back(((val % 5000))/1000);
      }
    }


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

    message->setProperty(Property::ProductSection::ProductDefinitionTemplateNumber,ProductSection::Template::EnsembleDerivedForecast);
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

    message->setProperty(Property::GridSection::LatLon::IDirectionIncrement,200000);
    message->setProperty(Property::GridSection::LatLon::JDirectionIncrement,200000);
    message->setProperty(Property::GridSection::LatLon::ScanningMode,0LL);

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


    message->setProperty(Property::RepresentationSection::RepresentationTemplateNumber,RepresentationSection::Template::GridDataRepresentation);
    message->setProperty(Property::RepresentationSection::Packing::BinaryScaleFactor,-11);
    message->setProperty(Property::RepresentationSection::Packing::DecimalScaleFactor,0LL);
    message->setProperty(Property::RepresentationSection::Packing::BitsPerValue,16);

    message->setProperty(Property::RepresentationSection::OriginalValues::TypeOfOriginalFieldValues,0LL);

    message->setGridValues(values);

  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
    exception.printError();
  }
}





void setMessage_PolarStereographic(GRID::Message *message)
{
  try
  {
    int width = 824;
    int height = 935;
    int sz = width * height;
    int levelType = 103;
    int level = 10;

    T::ParamValue_vec values;

    for (int y=0; y<height; y++)
    {
      for (int x=0; x<width; x++)
      {
        long val = random();
        values.push_back(((val % 5000))/1000);
      }
    }


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
    message->setProperty(Property::GridSection::PolarStereographic::ResolutionAndComponentFlag,8);
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

    message->setGridValues(values);
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
    exception.printError();
  }
}





void setMessage_LambertConformal(GRID::Message *message)
{
  try
  {
    int width = 349;
    int height = 277;
    int sz = width * height;
    int levelType = 103;
    int level = 10;

    T::ParamValue_vec values;

    for (int y=0; y<height; y++)
    {
      for (int x=0; x<width; x++)
      {
        long val = random();
        values.push_back(((val % 5000))/1000);
      }
    }


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

    message->setGridValues(values);

  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
    exception.printError();
  }
}





int main(int argc, char *argv[])
{
  try
  {
    if (argc < 2)
    {
      print_usage();
      return -1;
    }

    init();


    GRID::GridFile gridFile;
    uint optionFlags = SmartMet::GRID::PrintFlag::data;
    std::string fname = argv[1];

    gridFile.setGridFile(T::FileTypeValue::Grib2);

    GRID::Message *message = gridFile.newMessage();
    setMessage_LambertConformal(message);

    message = gridFile.newMessage();
    setMessage_PolarStereographic(message);

    message = gridFile.newMessage();
    setMessage_RotatedLatLon(message);

    message = gridFile.newMessage();
    setMessage_LatLon(message);

    gridFile.write(fname);

    gridFile.print(std::cout,0,optionFlags);

    return 0;

  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
    exception.printError();
  }
}



