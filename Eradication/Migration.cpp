/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ConfigParams.h"
#include "Migration.h"
#include "FileSystem.h"
#include "JsonObjectDemog.h"
#include "NoCrtWarnings.h"
#include "IIndividualHuman.h"
#include "IIndividualHumanContext.h"
#include "IndividualEventContext.h"
#include "IdmString.h"
#include "RANDOM.h"
#include "INodeContext.h"
#include "ISimulationContext.h"



SETUP_LOGGING( "Migration" )

namespace Kernel
{
    // ------------------------------------------------------------------------
    // --- MigrationRateData
    // ------------------------------------------------------------------------

    MigrationRateData::MigrationRateData()
        : m_ToNodeSuid()
        , m_MigType( MigrationType::LOCAL_MIGRATION )
        , m_InterpMap()
    {
    }

    MigrationRateData::MigrationRateData( suids::suid to_node_id, MigrationType::Enum migType, InterpolationType::Enum interpType )
        : m_ToNodeSuid( to_node_id )
        , m_MigType( migType )
        , m_InterpType( interpType )
        , m_InterpMap()
    {
    }

    const suids::suid MigrationRateData::GetToNodeSuid() const
    {
        return m_ToNodeSuid;
    }

    MigrationType::Enum MigrationRateData::GetMigrationType() const
    {
        return m_MigType;
    }

    InterpolationType::Enum MigrationRateData::GetInterpolationType() const
    {
        return m_InterpType;
    }

    int MigrationRateData::GetNumRates() const
    {
        return m_InterpMap.size();
    }

    void MigrationRateData::AddRate( float ageYears, float rate )
    {
        m_InterpMap.add( ageYears, rate );
    }

    float MigrationRateData::GetRate( float ageYears ) const
    {
        // m_InterpMap is a std::map that is sorted on the first key (i.e. ageYears)
        float first_age_years = m_InterpMap.begin()->first;
        float first_rate      = m_InterpMap.begin()->second;
        float last_age_years  = m_InterpMap.rbegin()->first;
        float last_rate       = m_InterpMap.rbegin()->second;

        // ------------------------------------------------------------------------------------
        // --- Ages that are outside the range given in the metadata file are capped
        // --- at either the first or last value regardless if it is interpolation or piecewise
        // ------------------------------------------------------------------------------------
        float rate; // = 0.0;
        if( ageYears <= first_age_years )
        {
            rate = first_rate;
        }
        else if( ageYears >= last_age_years )
        {
            rate = last_rate;
        }
        else if( m_InterpType == InterpolationType::LINEAR_INTERPOLATION )
        {
            rate = m_InterpMap.getValueLinearInterpolation( ageYears, first_rate );
        }
        else
        {
            rate = m_InterpMap.getValuePiecewiseConstant( ageYears, first_rate );
        }
        return rate;
    }

    // ------------------------------------------------------------------------
    // --- MigrationInfoNull
    // ------------------------------------------------------------------------
    MigrationInfoNull::MigrationInfoNull()
    : m_EmptyListCDF()
    , m_EmptyListNodes()
    , m_EmptyListTypes()
    {
    }

    MigrationInfoNull::~MigrationInfoNull()
    {
    }

    const MigrationParams* MigrationInfoNull::GetParams() const
    {
        return MigrationConfig::GetMigrationParams();
    }

    void MigrationInfoNull::PickMigrationStep( RANDOMBASE* pRNG, 
                                               IIndividualHumanContext * traveler, 
                                               suids::suid &destination, 
                                               MigrationType::Enum &migration_type,
                                               float &time )
    {
        destination = suids::nil_suid();
        migration_type = MigrationType::NO_MIGRATION;
        time = -1.0;
    }

    void MigrationInfoNull::SetContextTo(INodeContext* _parent)
    {
    }

    float MigrationInfoNull::GetTotalRate() const
    {
        return 0.0;
    }

    const std::vector<float>& MigrationInfoNull::GetCumulativeDistributionFunction() const
    {
        return m_EmptyListCDF;
    }

    const std::vector<suids::suid>& MigrationInfoNull::GetReachableNodes() const
    {
        return m_EmptyListNodes;
    }

    const std::vector<MigrationType::Enum>& MigrationInfoNull::GetMigrationTypes() const
    {
        return m_EmptyListTypes;
    }

    // ------------------------------------------------------------------------
    // --- MigrationInfoFixedRate
    // ------------------------------------------------------------------------
    MigrationInfoFixedRate::MigrationInfoFixedRate( INodeContext * _parent ) 
        : m_Parent(_parent) 
        , m_ReachableNodes()
        , m_MigrationTypes()
        , m_RateCDF()
        , m_TotalRate(0.0)
    {
    }

    MigrationInfoFixedRate::~MigrationInfoFixedRate() 
    {
    }

    const MigrationParams* MigrationInfoFixedRate::GetParams() const
    {
        return MigrationConfig::GetMigrationParams();
    }

    void MigrationInfoFixedRate::Initialize( const std::vector<std::vector<MigrationRateData>>& rRateData )
    {
        m_RateCDF.clear();
        if( rRateData.size() > 0 )
        {
            for( auto& mrd : rRateData[0] )
            {
                m_MigrationTypes.push_back( mrd.GetMigrationType() );
                m_ReachableNodes.push_back( mrd.GetToNodeSuid()    );

                release_assert( mrd.GetNumRates() == 1 );

                m_RateCDF.push_back( mrd.GetRate( 0.0 ) );
            }
            SaveRawRates( m_RateCDF );
            NormalizeRates( m_RateCDF, m_TotalRate );
        }
    }

    void MigrationInfoFixedRate::SaveRawRates(std::vector<float>& r_rate_cdf)
    { }

    void MigrationInfoFixedRate::SetContextTo(INodeContext* _parent)
    { 
        m_Parent = _parent; 
    }

    const std::vector<suids::suid>& MigrationInfoFixedRate::GetReachableNodes() const
    {
        return GetReachableNodes( Gender::MALE );
    }

    const std::vector<MigrationType::Enum>& MigrationInfoFixedRate::GetMigrationTypes() const
    { 
        return GetMigrationTypes( Gender::MALE ); 
    }

    void MigrationInfoFixedRate::PickMigrationStep( RANDOMBASE* pRNG,
                                                    IIndividualHumanContext *traveler, 
                                                    suids::suid &destination, 
                                                    MigrationType::Enum &migration_type, 
                                                    float &time )
    {
        float age_years = 0.0;
        Gender::Enum gender = Gender::MALE;
        if( traveler != nullptr )
        {
            age_years = traveler->GetEventContext()->GetAge() / DAYSPERYEAR;
            gender = Gender::Enum(traveler->GetEventContext()->GetGender());
        }

        CalculateRates( gender, age_years );

        float total_rate = GetTotalRate();
        const std::vector<float              >& r_cdf             = GetCumulativeDistributionFunction();
        const std::vector<suids::suid        >& r_reachable_nodes = GetReachableNodes( gender );
        const std::vector<MigrationType::Enum>& r_migration_types = GetMigrationTypes( gender );

        if( (r_cdf.size() == 0) || (total_rate == 0.0) )
        {
            destination = suids::nil_suid();
            migration_type = MigrationType::NO_MIGRATION;
            time = 0.0;
            return;
        }

        int index = 0;

        time = float(pRNG->expdist( total_rate ));

        float desttemp = pRNG->e();
        while( desttemp > r_cdf[index] )
        {
            index++;
        }

        destination    = r_reachable_nodes[index];
        migration_type = r_migration_types[index];
    }

    void MigrationInfoFixedRate::NormalizeRates( std::vector<float>& r_rate_cdf, float& r_total_rate )
    {
        //  Calculate total migration rate
        r_total_rate = 0.0;
        for( int i = 0; i < r_rate_cdf.size(); i++)
        {
            r_total_rate += r_rate_cdf[i];
        }

        if( (r_rate_cdf.size() > 0) && (r_total_rate > 0.0) )
        {
            //  Set probability of each location
            r_rate_cdf[0] /= r_total_rate;
            for(int i = 1; i < r_rate_cdf.size(); i++)
            {
                r_rate_cdf[i] = (r_rate_cdf[i] / r_total_rate) + r_rate_cdf[i-1];
            }

            // Values in the migration_rate_cdf[] are compared against values generated by RANDOMBASE::e(), which 
            // is guaranteed to create values between 0.0f and 1.0f.  We need to explicitly set the last value in
            // migration_rate_cdf to 1.0f, otherwise floating-point rounding errors might result in running past the
            // end of valid values in the array when picking a migration destination
            r_rate_cdf[ r_rate_cdf.size() - 1 ] = 1.0f;
        }
    }

    void MigrationInfoFixedRate::CalculateRates( Gender::Enum gender, float ageYears ) 
    {
        // rates are fixed so do nothing
    }

    const std::vector<float>& MigrationInfoFixedRate::GetCumulativeDistributionFunction() const
    {
        return m_RateCDF;
    }

    float MigrationInfoFixedRate::GetTotalRate() const
    {
        return m_TotalRate;
    }

    const std::vector<suids::suid>& MigrationInfoFixedRate::GetReachableNodes( Gender::Enum gender ) const
    {
        return m_ReachableNodes;
    }

    const std::vector<MigrationType::Enum>& MigrationInfoFixedRate::GetMigrationTypes( Gender::Enum gender ) const
    {
        return m_MigrationTypes;
    }

    // ------------------------------------------------------------------------
    // --- MigrationInfoAgeAndGender
    // ------------------------------------------------------------------------
    MigrationInfoAgeAndGender::MigrationInfoAgeAndGender( INodeContext * _parent ) 
        : MigrationInfoFixedRate( _parent ) 
        , m_RateData()
        , m_ReachableNodesFemale()
        , m_MigrationTypesFemale()
    {
    }

    MigrationInfoAgeAndGender::~MigrationInfoAgeAndGender() 
    {
    }

    void MigrationInfoAgeAndGender::Initialize( const std::vector<std::vector<MigrationRateData>>& rRateData )
    {
        release_assert( rRateData.size() == 2 );

        for( auto& mrd : rRateData[0] )
        {
            m_MigrationTypes.push_back( mrd.GetMigrationType() );
            m_ReachableNodes.push_back( mrd.GetToNodeSuid()    );
        }

        for( auto& mrd : rRateData[1] )
        {
            m_MigrationTypesFemale.push_back( mrd.GetMigrationType() );
            m_ReachableNodesFemale.push_back( mrd.GetToNodeSuid()    );
        }

        m_RateData = rRateData;
    }

    void MigrationInfoAgeAndGender::CalculateRates( Gender::Enum gender, float ageYears ) 
    {
        m_RateCDF.clear();
        m_TotalRate = 0.0;

        int gender_index = int(gender);

        for( auto& mrd : m_RateData[ gender_index ] )
        {
            float rate = mrd.GetRate( ageYears );
            m_RateCDF.push_back( rate );
        }

        NormalizeRates( m_RateCDF, m_TotalRate );
    }

    const std::vector<suids::suid>& MigrationInfoAgeAndGender::GetReachableNodes( Gender::Enum gender ) const
    {
        if( gender == Gender::FEMALE )
        {
            return m_ReachableNodesFemale;
        }
        else
        {
            return m_ReachableNodes;
        }
    }

    const std::vector<MigrationType::Enum>& MigrationInfoAgeAndGender::GetMigrationTypes( Gender::Enum gender ) const
    {
        if( gender == Gender::FEMALE )
        {
            return m_MigrationTypesFemale;
        }
        else
        {
            return m_MigrationTypes;
        }
    }

    // ------------------------------------------------------------------------
    // --- MigrationInfoFile
    // ------------------------------------------------------------------------

    MigrationInfoFile::MigrationInfoFile( MigrationType::Enum migType,
                                          int defaultDestinationsPerNode,
                                          bool enable_migration,
                                          std::string mig_filename,
                                          float mig_modifier )
        : m_Filename(mig_filename)
        , m_IsEnabled(enable_migration)
        , m_xModifier(mig_modifier)
        , m_DestinationsPerNode( defaultDestinationsPerNode )
        , m_MigrationType( migType )
        , m_GenderDataType( GenderDataType::SAME_FOR_BOTH_GENDERS )
        , m_InterpolationType( InterpolationType::LINEAR_INTERPOLATION )
        , m_AgesYears()
        , m_GenderDataSize(0)
        , m_AgeDataSize(0)
        , m_FileStream()
        , m_Offsets()
    {
    }

    MigrationInfoFile::~MigrationInfoFile()
    {
        if( m_FileStream.is_open() )
        {
            m_FileStream.close();
        }
    }

    void MigrationInfoFile::Initialize( const std::string& idreference )
    {
        if( m_IsEnabled )
        {
            if( m_Filename.empty() )
            {
                throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, "Migration_Filename is empty. You must specify a file." );
            }

            std::string filepath = Environment::FindFileOnPath( m_Filename );

            uint32_t expected_binary_file_size = ParseMetadataForFile( filepath, idreference );

            OpenMigrationFile( filepath, expected_binary_file_size );
        }
    }

    bool MigrationInfoFile::ReadData( INodeContext* from_node_ptr, std::vector<std::vector<MigrationRateData>>& rRateData )
    {
        ExternalNodeId_t fromNodeID   = from_node_ptr->GetExternalID();

        bool is_fixed_rate = true;
        if( m_IsEnabled && (m_Offsets.count( fromNodeID ) > 0) )
        {
            if( rRateData.size() != 2 )
            {
                release_assert( rRateData.size() == 0 );
                for( uint32_t ig = 0; ig < 2; ig++ )
                {
                    rRateData.push_back( std::vector<MigrationRateData>() );
                }
            }

            uint32_t num_gender_data_chunks = GetNumGenderDataChunks();

            // data is fixed if one gender and one age and age is max
            is_fixed_rate = (num_gender_data_chunks == 1) && 
                            (m_AgesYears.size() == 1) && 
                            (m_AgesYears[0] == MAX_HUMAN_AGE);

            int gender_file_index = 0;
            for( uint32_t ig = 0; ig < 2; ig++ )
            {
                if( (num_gender_data_chunks == 1) && (ig == 1) )
                {
                    // -----------------------------------------------------
                    // --- The data has one set of rates for both genders 
                    // --- so read the same data into the female parameters.
                    // -----------------------------------------------------
                    gender_file_index = 0;
                }

                int initial_node_data_size = rRateData[ ig ].size();
                for( uint32_t ja = 0; ja < m_AgesYears.size(); ja++ )
                {
                    float age_years = m_AgesYears[ja];

                    uint32_t array_id[MAX_DESTINATIONS] = {0};
                    double   array_rt[MAX_DESTINATIONS] = {0};

                    std::streamoff offset = gender_file_index*m_GenderDataSize + ja*m_AgeDataSize + m_Offsets[ fromNodeID ];

                    m_FileStream.seekg( offset, std::ios::beg );

                    m_FileStream.read( reinterpret_cast<char*>(array_id), m_DestinationsPerNode * sizeof(uint32_t) );
                    m_FileStream.read( reinterpret_cast<char*>(array_rt), m_DestinationsPerNode * sizeof(double)   );

                    if( m_FileStream.fail() )
                    {
                        // This should really just be a safety net
                        std::stringstream ss;
                        ss << "Error reading migration data for node " << fromNodeID;
                        throw FileIOException( __FILE__, __LINE__, __FUNCTION__, m_Filename.c_str(), ss.str().c_str() );
                    }

                    // ---------------------------------------------------------------------------------
                    // --- array_id and array_rt are contiguous.  That is, the values we want to
                    // --- extract may not start at the beginning (i.e. i=0) and may have gaps.  Hence,
                    // --- we use node_index to keep track of what node we are on.
                    // ---------------------------------------------------------------------------------
                    int node_index = initial_node_data_size;
                    for( int i = 0; i < m_DestinationsPerNode; i++ )
                    {
                        if( array_id[ i ] > 0 )
                        {
                            ExternalNodeId_t to_node_id_external = array_id[ i ];
                            suids::suid      to_node_id_suid     = from_node_ptr->GetParent()->GetNodeSuid(to_node_id_external);
                            float rate = array_rt[i] * m_xModifier; // migration tuning knob

                            if( rRateData[ ig ].size() <= node_index )
                            {
                                MigrationRateData mrd( to_node_id_suid, m_MigrationType, m_InterpolationType );
                                rRateData[ ig ].push_back( mrd );
                            }
                            else if( (rRateData[ ig ][ node_index ].GetToNodeSuid()      != to_node_id_suid) ||
                                     (rRateData[ ig ][ node_index ].GetMigrationType() != m_MigrationType) )
                            {
                                std::stringstream ss;
                                ss << "In file '" << m_Filename << "', the 'To' Node IDs are not the same for the Age Data sections for fromNodeId = " << fromNodeID;
                                throw GeneralConfigurationException(  __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
                            }
                            rRateData[ ig ][ node_index ].AddRate( age_years, rate );
                            node_index++;
                        }
                    }
                }
                gender_file_index++;
            }
        }
        return is_fixed_rate;
    }

// json keys in the metadata file
static const char* METADATA              = "Metadata";               // required - Element containing information about the file
static const char* MD_ID_REFERENCE       = "IdReference";            // required - Must equal value from demographics file
static const char* MD_NODE_COUNT         = "NodeCount";              // required - Used to verify size NodeOffsets - 16*NodeCount = # chars in NodeOffsets
static const char* MD_DATA_VALUE_COUNT   = "DatavalueCount";         // optional - Default depends on MigrationType (i.e. MaxDestinationsPerNode)
static const char* MD_MIGRATION_TYPE     = "MigrationType";          // optional - Assume as expected
static const char* MD_GENDER_DATA_TYPE   = "GenderDataType";         // optional - Default is BOTH
static const char* MD_AGES_YEARS         = "AgesYears";              // optional - Array of ages in increasing order - Default all ages - 0 - 125
static const char* MD_INTERPOLATION_TYPE = "InterpolationType";      // optional - Default is LINEAR_INTERPOLATION
static const char* NODE_OFFSETS          = "NodeOffsets";            // required - a map of External Node ID to an address location in the binary file

// Macro that creates a message with the bad input string and a list of possible values.
#define THROW_ENUM_EXCEPTION( EnumName, dataFile, key, inputValue )\
        std::stringstream ss;\
        ss << dataFile << "[" << METADATA << "][" << key << "] = '" << inputValue << "' is not a valid " << key << ".  Valid values are: ";\
        for( int i = 0; i < EnumName::pairs::count(); i++ )\
        {\
            ss << "'" << EnumName::pairs::get_keys()[i] << "', ";\
        }\
        std::string msg = ss.str();\
        msg = msg.substr( 0, msg.length()-2 );\
        throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, msg.c_str() );


    uint32_t MigrationInfoFile::ParseMetadataForFile( const std::string& data_filepath, const std::string& idreference )
    {
        string metadata_filepath = data_filepath + ".json";

        JsonObjectDemog json;
        json.ParseFile( metadata_filepath.c_str() );

        try // try-catch to catch JsonObjectDemog errors like wrong type
        {
            // -------------------------------------
            // --- Check idreference - Must be equal
            // -------------------------------------
            string file_id_reference = json[ METADATA ][ MD_ID_REFERENCE ].AsString();
            string file_idreference_lower(file_id_reference);
            string idreference_lower(idreference);  // Make a copy to transform so we do not modify the original.
            std::transform(idreference_lower.begin(), idreference_lower.end(), idreference_lower.begin(), ::tolower);
            std::transform(file_idreference_lower.begin(), file_idreference_lower.end(), file_idreference_lower.begin(), ::tolower);
            if(file_idreference_lower != idreference_lower )
            {
                std::stringstream ss;
                ss << metadata_filepath << "[" << METADATA << "][" << MD_ID_REFERENCE << "]";
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, 
                                                        "idreference", idreference.c_str(),
                                                        ss.str().c_str(),
                                                        file_id_reference.c_str() );
            }

            // -------------------------------------------------
            // --- Read the DestinationsPerNode if it exists
            // --- This tells us how much data there is per node
            // -------------------------------------------------
            if( json[ METADATA ].Contains( MD_DATA_VALUE_COUNT ) )
            {
                m_DestinationsPerNode = json[ METADATA ][ MD_DATA_VALUE_COUNT ].AsInt();
                if( ( 0 >= m_DestinationsPerNode) || (m_DestinationsPerNode > MAX_DESTINATIONS) )
                {
                    int violated_limit = (0 >= m_DestinationsPerNode) ? 0 : MAX_DESTINATIONS;
                    std::stringstream ss;
                    ss << metadata_filepath << "[" << METADATA << "][" << MD_DATA_VALUE_COUNT << "]";
                    throw OutOfRangeException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str(), m_DestinationsPerNode, violated_limit );
                }
            }

            // ----------------------------------------------------------
            // --- Read MigrationType - Verify that it is what we expect
            // ----------------------------------------------------------
            if( json[ METADATA ].Contains( MD_MIGRATION_TYPE ) )
            {
                std::string file_mig_type_str = json[ METADATA ][ MD_MIGRATION_TYPE ].AsString();
                MigrationType::Enum file_mig_type = MigrationType::Enum(MigrationType::pairs::lookup_value( file_mig_type_str.c_str() ));
                if( file_mig_type == -1 )
                {
                    THROW_ENUM_EXCEPTION( MigrationType, metadata_filepath, MD_MIGRATION_TYPE, file_mig_type_str )
                }
                else if( file_mig_type != m_MigrationType )
                {
                    std::string exp_mig_type_str = MigrationType::pairs::lookup_key( m_MigrationType );
                    std::stringstream ss;
                    ss << metadata_filepath << "[" << METADATA << "][" << MD_MIGRATION_TYPE << "]";
                    throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, 
                                                            "m_MigrationType", exp_mig_type_str.c_str(),
                                                            ss.str().c_str(),
                                                            file_mig_type_str.c_str() );
                }
            }

            // ------------------------
            // --- Read GenderDataType
            // ------------------------
            m_GenderDataType = GenderDataType::SAME_FOR_BOTH_GENDERS;
            if( json[ METADATA ].Contains( MD_GENDER_DATA_TYPE ) )
            {
                std::string gd_str = json[ METADATA ][ MD_GENDER_DATA_TYPE ].AsString();
                m_GenderDataType = GenderDataType::Enum(GenderDataType::pairs::lookup_value( gd_str.c_str() ));
                if( m_GenderDataType == -1 )
                {
                    THROW_ENUM_EXCEPTION( GenderDataType, metadata_filepath, MD_GENDER_DATA_TYPE, gd_str )
                }
            }

            // ---------------------------------------------------------------------------------
            // --- Read AgesYears - In an array of ages in years with values in increasing order
            // ---------------------------------------------------------------------------------
            if( json[ METADATA ].Contains( MD_AGES_YEARS ) )
            {
                std::stringstream errmsg;
                errmsg << metadata_filepath << "[" << METADATA << "][" << MD_AGES_YEARS << "] must be an array of ages"
                       << " in years between 0 and " << MAX_HUMAN_AGE << " and must be in increasing order.";

                JsonObjectDemog ages_array = json[ METADATA ][ MD_AGES_YEARS ];
                if( !ages_array.IsArray() )
                {
                    throw GeneralConfigurationException(  __FILE__, __LINE__, __FUNCTION__, errmsg.str().c_str() );
                }
                float prev = 0.0;
                for( int i = 0; i < ages_array.size(); i++ )
                {
                    float age = ages_array[i].AsFloat();
                    if( (age < 0.0) || (MAX_HUMAN_AGE < age) || (age < prev) )
                    {
                        std::stringstream ss;
                        ss << metadata_filepath << "[" << METADATA << "][" << MD_AGES_YEARS << "][" << i << "] = " << age << ".  " << errmsg.str();
                        throw GeneralConfigurationException(  __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
                    }
                    m_AgesYears.push_back( age );
                    prev = age;
                }
            }
            else
            {
                // Look up logic depends on the default value being the max age
                m_AgesYears.push_back( MAX_HUMAN_AGE );
            }

            // ---------------------------
            // --- Read InterpolationType
            // ---------------------------
            m_InterpolationType = InterpolationType::LINEAR_INTERPOLATION;
            if( json[ METADATA ].Contains( MD_INTERPOLATION_TYPE ) )
            {
                std::string interp_str = json[ METADATA ][ MD_INTERPOLATION_TYPE ].AsString();
                m_InterpolationType = InterpolationType::Enum(InterpolationType::pairs::lookup_value( interp_str.c_str() ));
                if( m_InterpolationType == -1 )
                {
                    THROW_ENUM_EXCEPTION( InterpolationType, metadata_filepath, MD_INTERPOLATION_TYPE, interp_str )
                }
            }

            // ------------------------------------------------------------------------
            // --- Read the NodeCount and the NodeOffsets and verify size is as expected.
            // --- There should be 16 characters for each node.  The first 8 characters
            // --- are for the External Node ID (i.e the node id used in the demographics)
            // --- and the second 8 characters are the offset in the file.
            // --- The node ids in the offset_str are the "from" nodes - the nodes that
            // --- individuals are trying migrate from.
            // ------------------------------------------------------------------------
            int num_nodes = json[ METADATA ][ MD_NODE_COUNT ].AsInt();

            std::string offsets_str = json[ NODE_OFFSETS ].AsString();
            if( offsets_str.length() / 16 != num_nodes )
            {
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, 
                    "offsets_str.length() / 16", int( offsets_str.length() / 16 ), 
                    "num_nodes", num_nodes );
            }

            // -----------------------------------------------
            // --- Extract data from string and place into map
            // -----------------------------------------------
            for( int n = 0; n < num_nodes; n++ )
            {
                ExternalNodeId_t nodeid = 0;
                uint32_t offset = 0;

                sscanf_s(offsets_str.substr((n * 16)    , 8).c_str(), "%x", &nodeid);
                sscanf_s(offsets_str.substr((n * 16) + 8, 8).c_str(), "%x", &offset);

                m_Offsets[ nodeid ] = offset;
            }
        }
        catch( SerializationException& re )
        {
            // ------------------------------------------------------------------------------
            // --- This "catch" is meant to report errors detered in JsonObjectDemog.
            // --- These can consist of missing keys, wrong types, etc.
            // ---
            // --- Pull out the message about what is wrong and pre-pend the filename to it
            // ------------------------------------------------------------------------------
            std::stringstream msg;
            IdmString msg_str = re.GetMsg();
            std::vector<IdmString> splits = msg_str.split('\n');
            release_assert( splits.size() == 4 );
            msg << metadata_filepath << ": " << splits[2];
            throw GeneralConfigurationException(  __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        // -------------------------------------------
        // --- Calculate the expected binary file size
        // -------------------------------------------
        uint32_t num_gender_data_chunks = GetNumGenderDataChunks();
        uint32_t num_age_data_chunks    = m_AgesYears.size();
        m_AgeDataSize                   = m_Offsets.size() * m_DestinationsPerNode * (sizeof(uint32_t) + sizeof(double));
        m_GenderDataSize                = num_age_data_chunks * m_AgeDataSize;
        uint32_t exp_binary_file_size   = num_gender_data_chunks * m_GenderDataSize;

        // ------------------------------------------------------
        // --- Check Offset values to ensure that they are valid
        // ------------------------------------------------------
        for( auto entry : m_Offsets )
        {
            if( entry.second >= exp_binary_file_size )
            {
                char offset_buff[20];
                sprintf_s( offset_buff, 19, "0x%x", entry.second);
                char filesize_buff[ 20 ];
                sprintf_s( filesize_buff, 19, "0x%x", exp_binary_file_size );
                std::stringstream ss;
                ss << std::endl;
                ss << "Invalid '" << NODE_OFFSETS << "' in " << metadata_filepath << "." << std::endl;
                ss << "Node ID=" << entry.first << " has an offset of " << offset_buff;
                ss << " but the '.bin' file size is expected to be " << exp_binary_file_size << "(" << filesize_buff << ")." << std::endl;
                throw FileIOException( __FILE__, __LINE__, __FUNCTION__, m_Filename.c_str(), ss.str().c_str() );
            }
        }

        return exp_binary_file_size;
    }

    void MigrationInfoFile::OpenMigrationFile( const std::string& filepath, uint32_t expected_binary_file_size )
    {
        if( !FileSystem::FileExists( filepath ) )
        {
            throw FileNotFoundException( __FILE__, __LINE__, __FUNCTION__, filepath.c_str() );
        }

        FileSystem::OpenFileForReading( m_FileStream, filepath.c_str(), true );

        m_FileStream.seekg(0, ios::end);
        int filelen = m_FileStream.tellg();

        if( filelen != expected_binary_file_size )
        {
            std::stringstream ss;
            ss << "Detected wrong size for migration data file.  Expected " << expected_binary_file_size << " bytes, read " << filelen << " bytes";
            throw FileIOException( __FILE__, __LINE__, __FUNCTION__, filepath.c_str(), ss.str().c_str() );
        }

        m_FileStream.seekg( 0, ios::beg );
    }

    uint32_t MigrationInfoFile::GetNumGenderDataChunks() const
    {
        uint32_t num_gender_data_chunks; // = 1;
        if( m_GenderDataType == GenderDataType::SAME_FOR_BOTH_GENDERS )
        {
            num_gender_data_chunks = 1; // genders use same data
        }
        else
        {
            num_gender_data_chunks = 2; // the first chuck is for males, the second is for females
        }
        return num_gender_data_chunks;
    }

    // ------------------------------------------------------------------------
    // --- MigrationInfoFactoryFile
    // ------------------------------------------------------------------------

    MigrationInfoFactoryFile::MigrationInfoFactoryFile()
    : m_InfoFileList()
    { }

    MigrationInfoFactoryFile::~MigrationInfoFactoryFile()
    {
        for( auto mig_file : m_InfoFileList )
        {
            delete mig_file;
        }
        m_InfoFileList.clear();
    }

    const MigrationParams* MigrationInfoFactoryFile::GetParams() const
    {
        return MigrationConfig::GetMigrationParams();
    }

    bool MigrationInfoFactoryFile::IsAtLeastOneTypeConfiguredForIndividuals() const
    {
        for( auto mif : m_InfoFileList )
        {
            if( mif->FileEnabled() && !mif->FilenameEmpty() )
                return true;
        }
        return false;
    }

    bool MigrationInfoFactoryFile::IsEnabled( MigrationType::Enum mt ) const
    {
        for( auto mif : m_InfoFileList )
        {
            if( (mif->GetMigrationType() == mt) && mif->FileEnabled() )
                return true;
        }
        return false;
    }

    void MigrationInfoFactoryFile::Initialize( const string& idreference )
    {
        const MigrationParams* mp = MigrationConfig::GetMigrationParams();

        m_InfoFileList.push_back( new MigrationInfoFile( MigrationType::LOCAL_MIGRATION,    MAX_LOCAL_MIGRATION_DESTINATIONS,    mp->enable_mig_local,    mp->mig_file_local,    mp->mig_mult_local ) );
        m_InfoFileList.push_back( new MigrationInfoFile( MigrationType::AIR_MIGRATION,      MAX_AIR_MIGRATION_DESTINATIONS,      mp->enable_mig_air,      mp->mig_file_air,      mp->mig_mult_air ) );
        m_InfoFileList.push_back( new MigrationInfoFile( MigrationType::REGIONAL_MIGRATION, MAX_REGIONAL_MIGRATION_DESTINATIONS, mp->enable_mig_regional, mp->mig_file_regional, mp->mig_mult_regional ) );
        m_InfoFileList.push_back( new MigrationInfoFile( MigrationType::SEA_MIGRATION,      MAX_SEA_MIGRATION_DESTINATIONS,      mp->enable_mig_sea,      mp->mig_file_sea,      mp->mig_mult_sea ) );
        m_InfoFileList.push_back( new MigrationInfoFile( MigrationType::FAMILY_MIGRATION,   MAX_FAMILY_MIGRATION_DESTINATIONS,   mp->enable_mig_family,   mp->mig_file_family,   mp->mig_mult_family ) );

        for( int i = 0; i < m_InfoFileList.size(); i++ )
        {
            if( m_InfoFileList[i] && m_InfoFileList[i]->FileEnabled() )
            {
                m_InfoFileList[i]->Initialize( idreference );
            }
        }
    }

    IMigrationInfo* MigrationInfoFactoryFile::CreateMigrationInfo( INodeContext *pParentNode )
    {
        bool is_fixed_rate = true ;

        std::vector<std::vector<MigrationRateData>> rate_data = GetRateData( pParentNode, m_InfoFileList, &is_fixed_rate );

        // -------------------------------------------------------------------------
        // --- it's possible that all 4 migration-types are empty for a given node,
        // --- i.e. this node is a "fortress/island" node; in that case, we return
        // --- null object so this node is not considered for migration
        // ---
        // --- NOTE: I didn't make Initialize() part of IMigrationInfo so that
        // --- MigrationRateData is known everywhere.
        // -------------------------------------------------------------------------
        IMigrationInfo* p_new_migration_info;

        if( rate_data.size() > 0 )
        {
            if( is_fixed_rate )
            {
                MigrationInfoFixedRate* p_mifr = _new_ MigrationInfoFixedRate( pParentNode );
                p_mifr->Initialize( rate_data ); 
                p_new_migration_info = p_mifr;
            }
            else
            {
                MigrationInfoAgeAndGender* p_miag = _new_ MigrationInfoAgeAndGender( pParentNode );
                p_miag->Initialize( rate_data );
                p_new_migration_info = p_miag;
            }
        }
        else
        {
            p_new_migration_info = new MigrationInfoNull();
        }

        return p_new_migration_info;
    }

    std::vector<std::vector<MigrationRateData>> MigrationInfoFactoryFile::GetRateData( INodeContext *pParentNode,
                                                                                       std::vector<MigrationInfoFile*>& infoFileList,
                                                                                       bool* pIsFixedRate )
    {
        *pIsFixedRate = true;
        std::vector<std::vector<MigrationRateData>> rate_data;
        for( int i = 0; i < infoFileList.size(); i++ )
        {
            if(infoFileList[i] != nullptr)
            {
                MigrationInfoFile* p_mif = dynamic_cast<MigrationInfoFile*>(infoFileList[i]);
                release_assert( p_mif );

                *pIsFixedRate &= p_mif->ReadData( pParentNode, rate_data );
            }
        }

        return rate_data;
    }

    // ------------------------------------------------------------------------
    // --- MigrationInfoFactoryDefault
    // ------------------------------------------------------------------------

    MigrationInfoFactoryDefault::MigrationInfoFactoryDefault( int torusSize )
    : m_TorusSize( torusSize )
    { }

    MigrationInfoFactoryDefault::MigrationInfoFactoryDefault()
    : m_TorusSize(0)
    { }

    MigrationInfoFactoryDefault::~MigrationInfoFactoryDefault()
    { }

    const MigrationParams* MigrationInfoFactoryDefault::GetParams() const
    {
        return MigrationConfig::GetMigrationParams();
    }

    bool MigrationInfoFactoryDefault::IsAtLeastOneTypeConfiguredForIndividuals() const
    {
        return false;
    }

    bool MigrationInfoFactoryDefault::IsEnabled( MigrationType::Enum mt ) const
    {
        return (mt == MigrationType::LOCAL_MIGRATION);
    }

    void MigrationInfoFactoryDefault::Initialize( const string& idreference )
    { }

    IMigrationInfo* MigrationInfoFactoryDefault::CreateMigrationInfo( INodeContext *pParentNode )
    {
        std::vector<std::vector<MigrationRateData>> rate_data = GetRateData( pParentNode, GetParams()->mig_mult_local );

        IMigrationInfo* p_new_migration_info;

        if( GetParams()->enable_mig_local)
        {
            MigrationInfoFixedRate* p_mifr = _new_ MigrationInfoFixedRate( pParentNode );
            p_mifr->Initialize( rate_data );
            p_new_migration_info = p_mifr;
        }
        else
        {
            p_new_migration_info = new MigrationInfoNull();
        }

        return p_new_migration_info;
    }

    std::vector<std::vector<MigrationRateData>> MigrationInfoFactoryDefault::GetRateData( INodeContext *pParentNode, float modifier )
    {
        suids::suid      from_node_suid = pParentNode->GetSuid();
        ExternalNodeId_t from_node_id   = pParentNode->GetExternalID();

        int offsets[]    = {  -(m_TorusSize+1), -m_TorusSize, -(m_TorusSize-1),
                                        -1,                           1,
                               (m_TorusSize-1),  m_TorusSize,  (m_TorusSize+1)};

        double basicrate = 1.0f / MAX_LOCAL_MIGRATION_DESTINATIONS / 10; // on average, a person should go to one of the 8 surrounding nodes every 10 days, per Philip

        basicrate *= modifier;

        // correct offsets if on any of the edges (of numbering grid scheme, not the torus)

        if( from_node_id % m_TorusSize == 1 ) // left edge
        {
            offsets[0] += m_TorusSize;
            offsets[3] += m_TorusSize;
            offsets[5] += m_TorusSize;
        }
        else if( from_node_id % m_TorusSize == 0 ) //right edge
        {
            offsets[2] -= m_TorusSize;
            offsets[4] -= m_TorusSize;
            offsets[7] -= m_TorusSize;
        }

        if( from_node_id <= uint32_t(m_TorusSize) ) // top edge
        {
            offsets[0] += m_TorusSize * m_TorusSize;
            offsets[1] += m_TorusSize * m_TorusSize;
            offsets[2] += m_TorusSize * m_TorusSize;
        }
        else if( from_node_id > uint32_t(m_TorusSize * (m_TorusSize - 1)) ) // bottom edge
        {
            offsets[5] -= m_TorusSize * m_TorusSize;
            offsets[6] -= m_TorusSize * m_TorusSize;
            offsets[7] -= m_TorusSize * m_TorusSize;
        }

        LOG_DEBUG_F( "MAX_LOCAL_MIGRATION_DESTINATIONS = %d\n", MAX_LOCAL_MIGRATION_DESTINATIONS );

        std::vector<std::vector<MigrationRateData>> rate_data;
        rate_data.push_back( std::vector<MigrationRateData>() );

        for (int i = 0; i < MAX_LOCAL_MIGRATION_DESTINATIONS; i++)
        {
            release_assert( from_node_id + offsets[i] >= 1 );
            release_assert( from_node_id + offsets[i] <= uint32_t(m_TorusSize * m_TorusSize) );

            ExternalNodeId_t to_node_id   = from_node_id + offsets[i];
            suids::suid      to_node_suid = pParentNode->GetParent()->GetNodeSuid(to_node_id);

            MigrationRateData mrd( to_node_suid, MigrationType::LOCAL_MIGRATION, InterpolationType::LINEAR_INTERPOLATION );
            mrd.AddRate( MAX_HUMAN_AGE, basicrate );
            rate_data[0].push_back( mrd );
        }

        return rate_data;
    }

}
