/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "Assortivity.h"
#include "Exceptions.h"
#include "RANDOM.h"
#include "IndividualEventContext.h"
#include "Node.h"
#include "Simulation.h"

SETUP_LOGGING( "Assortivity" )

namespace Kernel 
{
    std::string Assortivity::ValuesToString( const std::vector<std::string>& rList )
    {
        std::stringstream ss ;
        for( auto val : rList )
        {
            ss << "'" << val << "' " ;
        }
        return ss.str() ;
    }

    Assortivity::Assortivity( RelationshipType::Enum relType, RANDOMBASE* prng )
        : IAssortivity()
        , m_RelType( relType )
        , m_pRNG( prng )
        , m_Group( AssortivityGroup::NO_GROUP )
        , m_PropertyKey()
        , m_Axes()
        , m_WeightingMatrix()
        , m_StartYear( MIN_YEAR )
        , m_StartUsing(false)
    {
        //release_assert( m_pRNG != nullptr );
    }

    Assortivity::~Assortivity()
    {
    }

    void Assortivity::SetParameters( RANDOMBASE* prng )
    {
        m_pRNG = prng;
        release_assert( m_pRNG != nullptr );
    }

    bool Assortivity::Configure(const Configuration *config)
    {
        bool ret = false ;
        bool prev_use_defaults = JsonConfigurable::_useDefaults ;
        bool resetTrackMissing = JsonConfigurable::_track_missing;
        JsonConfigurable::_track_missing = false;
        JsonConfigurable::_useDefaults = false ;
        try
        {

            initConfig( "Group", m_Group, config, MetadataDescriptor::Enum("m_Group", "TBD", MDD_ENUM_ARGS(AssortivityGroup)) ); 

            if( JsonConfigurable::_dryrun || (m_Group != AssortivityGroup::NO_GROUP) )
            {
                initConfigTypeMap( "Axes", &m_Axes, "TBD - The axes (row/columns) of the weighting matrix." );

                initConfigTypeMap( "Weighting_Matrix_RowMale_ColumnFemale", &m_WeightingMatrix, "TBD - Values to assign a possible pairing.  Rows are indexed by the male attribute and columns by the female attribute.", 0.0f, 1.0f );

                if( JsonConfigurable::_dryrun || (m_Group == AssortivityGroup::INDIVIDUAL_PROPERTY) )
                {
                    std::string rel_type_str = RelationshipType::pairs::lookup_key( m_RelType ) ;
                    std::string param_name = rel_type_str + std::string(":Property_Name") ;
                    m_PropertyKey.SetParameterName( param_name );
                    initConfigTypeMap( "Property_Name", &m_PropertyKey, "TBD - The name of the property to base the assortivity on." );
                }
            }

            initConfigTypeMap( "Start_Year", &m_StartYear, "TBD - The year to start using the assortivity preference.", MIN_YEAR, MAX_YEAR, MIN_YEAR, "Group", "STI_COINFECTION_STATUS,HIV_INFECTION_STATUS,HIV_TESTED_POSITIVE_STATUS,HIV_RECEIVED_RESULTS_STATUS");
            AddConfigurationParameters( m_Group, config );

            ret = JsonConfigurable::Configure( config );

            JsonConfigurable::_useDefaults = prev_use_defaults ;
            JsonConfigurable::_track_missing = resetTrackMissing;
        }
        catch( DetailedException& e )
        {
            JsonConfigurable::_useDefaults = prev_use_defaults ;
            JsonConfigurable::_track_missing = resetTrackMissing;

            std::stringstream ss ;
            ss << e.GetMsg() << "\n" << "Was reading values for " << RelationshipType::pairs::lookup_key( m_RelType ) << "." ;
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }
        catch( const json::Exception &e )
        {
            JsonConfigurable::_useDefaults = prev_use_defaults ;
            JsonConfigurable::_track_missing = resetTrackMissing;

            std::stringstream ss ;
            ss << e.what() << "\n" << "Was reading values for " << RelationshipType::pairs::lookup_key( m_RelType ) << "." ;
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }

        if( ret )
        {
            if( m_Group == AssortivityGroup::STI_INFECTION_STATUS )
            {
                if(!MatchesDependency(config, "Simulation_Type", "STI_SIM"))
                {
                    std::stringstream ss ;
                    ss << RelationshipType::pairs::lookup_key( GetRelationshipType() ) << ":Group"; 
                    throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, 
                                                            ss.str().c_str(), AssortivityGroup::pairs::lookup_key(GetGroup()).c_str(), 
                                                            "Simulation_Type", "other than STI_SIM",
                                                            "STI_INFECTION_STATUS is only valid with STI_SIM." );
                }
                CheckAxesForTrueFalse();
                SortMatrixFalseTrue();
            }
            else if( m_Group == AssortivityGroup::INDIVIDUAL_PROPERTY )
            {
                if( !m_PropertyKey.IsValid() )
                {
                    std::stringstream ss ;
                    ss << RelationshipType::pairs::lookup_key( m_RelType ) << ":Property_Name must be defined and cannot be empty string." ;
                    throw GeneralConfigurationException(__FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
                }
                CheckAxesForProperty();
            }
            else if( m_Group != AssortivityGroup::NO_GROUP )
            {
                if(!MatchesDependency(config, "Simulation_Type", "HIV_SIM") &&
                    MatchesDependency(config, "Group", "STI_COINFECTION_STATUS,HIV_INFECTION_STATUS,HIV_TESTED_POSITIVE_STATUS,HIV_RECEIVED_RESULTS_STATUS"))
                {
                    std::stringstream ss, detail;
                    ss     << RelationshipType::pairs::lookup_key( GetRelationshipType() ) << ":Group";
                    detail << AssortivityGroup::pairs::lookup_key( GetGroup() ) << " is only valid with HIV_SIM.";

                    throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, 
                                                            ss.str().c_str(), AssortivityGroup::pairs::lookup_key(GetGroup()).c_str(),
                                                            "Simulation_Type", "other than HIV_SIM",
                                                            detail.str().c_str() );
                }
                CheckDerivedValues();
            }

            if( m_Group != AssortivityGroup::NO_GROUP )
            {
                CheckMatrix();
            }
        }
        return ret ;
    }

    void Assortivity::CheckAxesForTrueFalse()
    {
        for( int i = 0 ; i < m_Axes.size() ; i++ )
        {
            std::transform( m_Axes[i].begin(), m_Axes[i].end(), m_Axes[i].begin(), ::toupper );
        }
        if( (m_Axes.size() != 2) ||
            ((m_Axes[0] != "TRUE" ) && (m_Axes[1] != "TRUE" )) ||
            ((m_Axes[0] != "FALSE") && (m_Axes[1] != "FALSE")) )
        {
            std::stringstream ss ;
            ss << "The " << RelationshipType::pairs::lookup_key( m_RelType ) << ":Group (" 
               << AssortivityGroup::pairs::lookup_key( m_Group ) <<") requires that the Axes names(="
               << ValuesToString( m_Axes )
               <<") are 'TRUE' and 'FALSE'.  Order is up to the user." ;
            throw GeneralConfigurationException(__FILE__, __LINE__, __FUNCTION__,  ss.str().c_str() );
        }
    }

    void Assortivity::CheckAxesForProperty()
    {
        IPKeyValueContainer property_values ;
        try
        {
            property_values = IPFactory::GetInstance()->GetIP( m_PropertyKey.ToString() )->GetValues<IPKeyValueContainer>();
        }
        catch( DetailedException& )
        {
            std::stringstream ss ;
            ss <<  RelationshipType::pairs::lookup_key( m_RelType ) 
               << ":Property_Name(=" << m_PropertyKey.ToString() << ") is not defined in the demographics." ;
            throw GeneralConfigurationException(__FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }

        std::set<std::string> values_as_set = property_values.GetValuesToStringSet();
        bool invalid_axes = property_values.Size() != m_Axes.size();
        for( int i = 0 ; !invalid_axes && (i < m_Axes.size()) ; i++ )
        {
            invalid_axes = (values_as_set.count( m_Axes[i] ) != 1 );
        }
        if( invalid_axes )
        {
            std::stringstream ss ;
            ss << "The " << RelationshipType::pairs::lookup_key( m_RelType ) << ":Group (" 
               << AssortivityGroup::pairs::lookup_key( m_Group ) <<") requires that the Axes names"
               << "(=" << ValuesToString( m_Axes ) << ") "
               << "match the property values"
               << "(=" << property_values.GetValuesToString() << ") "
               << "defined in the demographics for Property=" << m_PropertyKey.ToString() << "." ;
            throw GeneralConfigurationException(__FILE__, __LINE__, __FUNCTION__, ss.str().c_str() ) ;
        }
    }

    void Assortivity::CheckMatrix()
    {
        bool invalid_matrix = m_WeightingMatrix.size() != m_Axes.size() ;
        for( int i = 0 ; !invalid_matrix && (i < m_WeightingMatrix.size()) ; i++ )
        {
            invalid_matrix = m_WeightingMatrix[i].size() != m_Axes.size() ;
        }
        if( invalid_matrix )
        {
            std::stringstream ss ;
            ss <<  "The " << RelationshipType::pairs::lookup_key( m_RelType ) 
               << ":Weighting Matrix must be a square matrix whose dimensions are equal to the number of Axes." ;
            throw GeneralConfigurationException(__FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }

        // -------------------------------------------------------------------
        // --- Also check that a single row or single column is not all zeros.
        // -------------------------------------------------------------------
        for( int i = 0 ; i < m_WeightingMatrix.size() ; i++ )
        {
            bool row_ok = false ;
            bool col_ok = false ;
            for( int j = 0 ; j < m_WeightingMatrix.size() ; j++ )
            {
                row_ok |= m_WeightingMatrix[ i ][ j ] > 0.0 ;
                col_ok |= m_WeightingMatrix[ j ][ i ] > 0.0 ;
            }
            if( !row_ok || !col_ok )
            {
                std::stringstream ss ;
                ss <<  "The " << RelationshipType::pairs::lookup_key( m_RelType ) 
                   << ":Weighting Matrix cannot have a row or column where all the values are zero.  " ;
                if( !row_ok )
                   ss << "Row " << (i+1) << " is all zeros." ;
                else
                   ss << "Column " << (i+1) << " is all zeros." ;
                throw GeneralConfigurationException(__FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            }
        }
    }

    void Assortivity::SortMatrixFalseTrue()
    {
        if (m_Axes[0] == "TRUE")
        {
            m_Axes[0] = "FALSE";
            m_Axes[1] = "TRUE";

            float tmp = m_WeightingMatrix[0][0];
            m_WeightingMatrix[0][0] = m_WeightingMatrix[1][1];
            m_WeightingMatrix[1][1] = tmp;

            tmp = m_WeightingMatrix[0][1];
            m_WeightingMatrix[0][1] = m_WeightingMatrix[1][0];
            m_WeightingMatrix[1][0] = tmp;
        }
    }

    bool Assortivity::UsesStartYear() const
    {
        return (m_Group == AssortivityGroup::STI_COINFECTION_STATUS)
            || (m_Group == AssortivityGroup::HIV_INFECTION_STATUS)
            || (m_Group == AssortivityGroup::HIV_TESTED_POSITIVE_STATUS)
            || (m_Group == AssortivityGroup::HIV_RECEIVED_RESULTS_STATUS);
    }

    int GetIndexSTI( const Assortivity* pAssortivity, IIndividualHumanSTI* pIndividual )
    {
        return (pIndividual->IsInfected() ? 1 : 0) ; 
    }

    int GetIndexStiCoInfection( const Assortivity* pAssortivity, IIndividualHumanSTI* pIndividual )
    {
        return (pIndividual->HasSTICoInfection() ? 1 : 0) ; 
    }

    std::string GetStringValueIndividualProperty( const Assortivity* pAssortivity, IIndividualHumanSTI* pIndividual )
    {
        IPKey key = pAssortivity->GetPropertyKey();
        return pIndividual->GetPropertiesConst().Get( key ).GetValueAsString() ;
    }

    IIndividualHumanSTI* Assortivity::SelectPartner( IIndividualHumanSTI* pPartnerA,
                                                     const list<IIndividualHumanSTI*>& potentialPartnerList )
    {
        release_assert( pPartnerA != nullptr );

        if( potentialPartnerList.size() <= 0 )
        {
            return nullptr ;
        }

        AssortivityGroup::Enum group = GetGroupToUse() ;

        IIndividualHumanSTI* p_partner_B = nullptr ;
        switch( group )
        {
            case AssortivityGroup::NO_GROUP:
                p_partner_B = potentialPartnerList.front();
                break;

            case AssortivityGroup::STI_INFECTION_STATUS:
                p_partner_B = FindPartner( pPartnerA, potentialPartnerList, GetIndexSTI );
                break;

            case AssortivityGroup::INDIVIDUAL_PROPERTY:
                p_partner_B = FindPartnerIP( pPartnerA, potentialPartnerList, GetStringValueIndividualProperty );
                break;

            case AssortivityGroup::STI_COINFECTION_STATUS:
                p_partner_B = FindPartner( pPartnerA, potentialPartnerList, GetIndexStiCoInfection );
                break;

            default:
                p_partner_B = SelectPartnerForExtendedGroups( group,  pPartnerA, potentialPartnerList );
        }

        return p_partner_B ;
    }

    IIndividualHumanSTI* Assortivity::SelectPartnerForExtendedGroups( AssortivityGroup::Enum group,
                                                                      IIndividualHumanSTI* pPartnerA,
                                                                      const list<IIndividualHumanSTI*>& potentialPartnerList )
    {
        throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "group", group, AssortivityGroup::pairs::lookup_key( group ) );
    }

    void Assortivity::Update( const IdmDateTime& rCurrentTime, float dt )
    {
        float current_year = rCurrentTime.Year() ;
        m_StartUsing = m_StartYear < current_year ;
    }

    AssortivityGroup::Enum Assortivity::GetGroupToUse() const
    {
        AssortivityGroup::Enum group = GetGroup() ;
        if( !m_StartUsing )
        {
            group = AssortivityGroup::NO_GROUP ;
        }
        return group ;
    }

    struct PartnerScore
    {
        IIndividualHumanSTI* pPartner ;
        float score ;

        PartnerScore() : pPartner(nullptr), score(0.0) {};
        PartnerScore( IIndividualHumanSTI* p, float s ) : pPartner(p), score(s) {};
    };

#define PS_MAX_LIST (50000)
    static PartnerScore* PS_list = nullptr;
    static int PS_size = -1;

    IIndividualHumanSTI* Assortivity::FindPartner( IIndividualHumanSTI* pPartnerA,
                                                   const list<IIndividualHumanSTI*>& potentialPartnerList,
                                                   tGetIndexFunc func)
    {
        if (PS_size == -1)
        {
            PS_list = (PartnerScore*)malloc( PS_MAX_LIST * sizeof( PartnerScore ) );
            memset( PS_list, 0, PS_MAX_LIST * sizeof( PartnerScore ) );
            PS_size = 0;
        }

        // --------------------------------------------------------------
        // --- Get the index into the matrix for the male/partnerA based 
        // --- on his attribute and the axes that the attribute is in
        // --------------------------------------------------------------
        int a_index = func(this, pPartnerA);

        // -----------------------------------------------------------------------
        // --- Find the score for each female/partnerB given this particular male
        // -----------------------------------------------------------------------
        PS_size = 0;
        float total_score = 0.0f;
        for (auto p_partner_B : potentialPartnerList)
        {
            int b_index = func(this, p_partner_B);
            float score = m_WeightingMatrix[a_index][b_index];
            if( (score > 0.0) && (PS_size < PS_MAX_LIST) )
            {
                PS_list[PS_size].pPartner = p_partner_B;
                PS_list[PS_size].score = score;
                ++PS_size;
                total_score += score;
            }
        }

        // -------------------------------------------------------------------------
        // --- Select the partner based on their score/probability of being selected
        // -------------------------------------------------------------------------
        release_assert(m_pRNG != nullptr);
        float ran_score = m_pRNG->e() * total_score;
        float cum_score = 0.0;
        for (int i = 0; i < PS_size; ++i)
        {
            cum_score += PS_list[i].score;
            if (cum_score > ran_score)
            {
                return PS_list[i].pPartner;
            }
        }

        return nullptr;
    }

    //static std::vector<PartnerScore> partner_score_list;
    IIndividualHumanSTI* Assortivity::FindPartnerIP( IIndividualHumanSTI* pPartnerA,
                                                     const list<IIndividualHumanSTI*>& potentialPartnerList,
                                                     tGetStringValueFunc func)
    {
        if( PS_size == -1 )
        {
            PS_list = (PartnerScore*)malloc( PS_MAX_LIST * sizeof( PartnerScore ) );
            memset( PS_list, 0, PS_MAX_LIST * sizeof( PartnerScore ) );
            PS_size = 0;
        }

        // --------------------------------------------------------------
        // --- Get the index into the matrix for the male/partnerA based 
        // --- on his attribute and the axes that the attribute is in
        // --------------------------------------------------------------
        int a_index = pPartnerA->GetAssortivityIndex(m_RelType);
        if (a_index == -1)
        {
            a_index = GetIndex(func(this, pPartnerA));
            pPartnerA->SetAssortivityIndex(m_RelType, a_index);
        }

        // -----------------------------------------------------------------------
        // --- Find the score for each female/partnerB given this particular male
        // -----------------------------------------------------------------------
        PS_size = 0;
        float total_score = 0.0f;
        for (auto p_partner_B : potentialPartnerList)
        {
            int b_index = p_partner_B->GetAssortivityIndex(m_RelType);
            if (b_index == -1)
            {
                b_index = GetIndex(func(this, p_partner_B));
                p_partner_B->SetAssortivityIndex(m_RelType, b_index);
            }
            float score = m_WeightingMatrix[a_index][b_index];
            if( (score > 0.0) && (PS_size < PS_MAX_LIST) )
            {
                PS_list[ PS_size ].pPartner = p_partner_B;
                PS_list[ PS_size ].score    = score;
                ++PS_size;

                total_score += score;
            }
        }

        // -------------------------------------------------------------------------
        // --- Select the partner based on their score/probability of being selected
        // -------------------------------------------------------------------------
        release_assert(m_pRNG != nullptr);
        float ran_score = m_pRNG->e() * total_score;
        float cum_score = 0.0;
        for( int i = 0 ; i < PS_size ; ++i )
        {
            cum_score += PS_list[ i ].score;
            if (cum_score > ran_score)
            {
                return PS_list[i].pPartner;
            }
        }

        return nullptr;
    }

    int Assortivity::GetIndex( const std::string& rStringValue )
    {
        int index = std::find( m_Axes.begin(), m_Axes.end(), rStringValue ) - m_Axes.begin() ;
        if( (0 > index) || (index >= m_Axes.size()) )
        {
            std::stringstream ss ;
            ss << "The value (" << rStringValue << ") was not one of the Axes names ("
                << ValuesToString( m_Axes ) << ")." ;
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }
        return index ;
    }

    REGISTER_SERIALIZABLE(Assortivity);

    void Assortivity::serialize(IArchive& ar, Assortivity* obj)
    {
        Assortivity& sort = *obj;

        std::string key_name;
        if( ar.IsWriter() )
        {
            key_name = sort.m_PropertyKey.ToString();
        }

        ar.labelElement("m_RelType"        ) & (uint32_t&)sort.m_RelType;
        ar.labelElement("m_Group"          ) & (uint32_t&)sort.m_Group;
        ar.labelElement("m_PropertyName"   ) & key_name;
        ar.labelElement("m_Axes"           ) & sort.m_Axes;
        ar.labelElement("m_WeightingMatrix") & sort.m_WeightingMatrix;
        ar.labelElement("m_StartYear"      ) & sort.m_StartYear;
        ar.labelElement("m_StartUsing"     ) & sort.m_StartUsing;

        if( ar.IsReader() )
        {
            sort.m_PropertyKey = IPKey( key_name );
        }

        //RANDOMBASE*                     m_pRNG ;
    }
}
