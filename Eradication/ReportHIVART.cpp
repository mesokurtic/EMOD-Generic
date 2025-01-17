/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include <string>
#include "ReportHIVART.h"
#include "Log.h"
#include "Exceptions.h"
#include "NodeEventContext.h"
#include "IndividualEventContext.h"
#include "FileSystem.h"
#include "IIndividualHumanHIV.h"
#include "SusceptibilityHIV.h"
#include "IHIVInterventionsContainer.h"
#include "SimulationEnums.h"
#include "INodeContext.h"

SETUP_LOGGING( "ReportHIVART" )

namespace Kernel
{
    IReport* ReportHIVART::Create( ISimulation* sim )
    {
        return new ReportHIVART( sim );
    }

    ReportHIVART::ReportHIVART( ISimulation* sim )
        : BaseTextReportEvents("ReportHIVART.csv")
        , simulation( sim )
    {
        // default constructor listens to nothing
        eventTriggerList.push_back( EventTrigger::StartedART );
        eventTriggerList.push_back( EventTrigger::StoppedART );
    }

    ReportHIVART::~ReportHIVART()
    {
    }

    std::string ReportHIVART::GetHeader() const 
    {
        std::stringstream header ;
        header << "Year"        << ","
               << "Node_ID"     << ","
               << "ID"          << ","
               << "Age"         << ","
               << "Gender"      << ","
               << "CD4"         << ","
               << "StartingART";

        return header.str();
    }

    bool ReportHIVART::notifyOnEvent( IIndividualHumanEventContext* context, const EventTrigger::Enum& trigger )
    {
        IHIVInterventionsContainer* p_hiv_container = context->GetInterventionsContext()->GetContainerHIV();
        release_assert(p_hiv_container);
        IHIVMedicalHistory* med_parent = p_hiv_container->GetHIVMedicalHistory();

        float            sim_year = simulation->GetSimulationTime().Year();
        ExternalNodeId_t node_id  = context->GetNodeEventContext()->GetNodeContext()->GetExternalID();
        int              id       = context->GetSuid().data;
        float            age      = context->GetAge();
        bool             gender   = (context->GetGender() == Gender::MALE) ? 0 : 1 ;
        float            cd4count = med_parent->LastRecordedCD4();

        bool startingART = 1;
        if( trigger == EventTrigger::StoppedART )
        {
            startingART = 0;
        }

        GetOutputStream() << sim_year    << "," 
                          << node_id     << ","
                          << id          << ","
                          << age         << ","
                          << gender      << ","
                          << cd4count    << ","
                          << startingART << std::endl ;

        return true ;
    }
}
