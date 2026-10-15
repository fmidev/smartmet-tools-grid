/*! \file
 *  \brief CLI tool that creates a new grid file by projecting data from a source grid file into a template-defined target grid.
 */

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

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/time.h>
#include <vector>


using namespace SmartMet;



/*! \brief Init. */

namespace
{
void init()
{
  try
  {
    char *configFile = getenv(SMARTMET_GRID_CONFIG_FILE);
    if (configFile == nullptr)
    {
      printf("%s not defined!\n",SMARTMET_GRID_CONFIG_FILE);
      // NOLINTNEXTLINE(concurrency-mt-unsafe): single-threaded command line tool
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





/*! \brief Print usage. */

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



/*! \brief Set message lat lon v1. */




/*! \brief Set message rotated lat lon v1. */




/*! \brief Set message polar stereographic v1. */




/*! \brief Set message lambert conformal v1. */




/*! \brief Set message mercator v1. */




/*! \brief Set message lat lon v2. */





/*! \brief Set message rotated lat lon v2. */





/*! \brief Set message polar stereographic v2. */





/*! \brief Set message lambert conformal v2. */





/*! \brief Set message mercator v2. */

}  // namespace




/*! \brief Program entry point. */

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

    if (version != 1 && version != 2)
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

      int year = 0;
      int month = 0;
      int day = 0;
      int hour = 0;
      int minute = 0;
      int second = 0;
      splitTimeString(forecastTime,year,month,day,hour,minute,second);


      GRID::Message *newMessage = nullptr;

      if (version == 1)
      {
        using namespace SmartMet::GRIB1;

        newMessage = targetGridFile.newMessage(T::FileTypeValue::Grib1);

        printf("** VERSION %u\n",dataMessage->getGribVersion());

        if (C_UINT(version) == dataMessage->getGribVersion())
        {
          Int64 v = 0;

          auto properties = gribProperty.getPropertyVector();

          for (auto &propertie : properties)
          {
            if (configurationFile.getAttributeValue(propertie.first.c_str(),v))
            {
              newMessage->setProperty(propertie.second,v);
              std::cout << "SET " << propertie.first << " : " << v << "\n";
            }
            else
            if (dataMessage->getProperty(propertie.second,v))
            {
              std::cout << "*SET " << propertie.first << " : " << v << "\n";
              newMessage->setProperty(propertie.second,v);
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

        Int64 v = 0;

        auto properties = gribProperty.getPropertyVector();

        for (auto &propertie : properties)
        {
          if (configurationFile.getAttributeValue(propertie.first.c_str(),v))
          {
            newMessage->setProperty(propertie.second,v);
            std::cout << "SET " << propertie.first << " : " << v << "\n";
          }
          else
          if (dataMessage->getProperty(propertie.second,v))
          {
            std::cout << "*SET " << propertie.first << " : " << v << "\n";
            newMessage->setProperty(propertie.second,v);
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



