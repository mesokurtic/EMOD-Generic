/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_TYPHOID

#include <math.h>
#include <numeric> // for std::accumulate
#include <functional> // why not algorithm?
#include "Sugar.h"
#include "Exceptions.h"
#include "RANDOM.h"
#include "NodeTyphoid.h"
#include "NodeTyphoidEventContext.h"
#include "IndividualTyphoid.h"
#include "TransmissionGroupsFactory.h"
#include "Properties.h"
#include "PythonSupport.h"

using namespace Kernel;

SETUP_LOGGING( "NodeTyphoid" )

namespace Kernel
{
    NodeTyphoid::NodeTyphoid()
        : NodeEnvironmental()
    {
        delete event_context_host;
        NodeTyphoid::setupEventContextHost();
    }

    NodeTyphoid::NodeTyphoid(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
        : NodeEnvironmental(_parent_sim, externalNodeId, node_suid)
    {
        delete event_context_host;
        NodeTyphoid::setupEventContextHost();
    }

    void NodeTyphoid::Initialize()
    { 
        maxInfectionProb[ TransmissionRoute::CONTACT  ] = 1.0f;
        maxInfectionProb[ TransmissionRoute::ENVIRONMENTAL ] = 1.0f;
        NodeEnvironmental::Initialize();
    }

    void NodeTyphoid::setupEventContextHost()
    {
        event_context_host = _new_ NodeTyphoidEventContextHost(this);
    }

    NodeTyphoid *NodeTyphoid::CreateNode(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
    {
        NodeTyphoid *newnode = _new_ NodeTyphoid(_parent_sim, externalNodeId, node_suid);
        newnode->Initialize();

        return newnode;
    }

    NodeTyphoid::~NodeTyphoid(void)
    {
    }

    void NodeTyphoid::resetNodeStateCounters(void)
    {
        // This is a chance to do a single call into Pythoid at start of timestep
#ifdef ENABLE_PYTHOID
        static PyObject* pFunc = static_cast<PyObject*>(PythonSupport::GetPyFunction( PythonSupport::SCRIPT_TYPHOID, "start_timestep" ));
        if( pFunc )
        {
            PyObject_CallObject( pFunc, nullptr );
        }
#endif

        NodeEnvironmental::resetNodeStateCounters();
    }

    IndividualHuman *NodeTyphoid::createHuman(suids::suid suid, float monte_carlo_weight, float initial_age, int gender)
    {
        return IndividualHumanTyphoid::CreateHuman(this, suid, monte_carlo_weight, initial_age, gender);
    }

    int NodeTyphoid::calcGap()
    {
        int gap = 1;
        float cp = maxInfectionProb[ TransmissionRoute::CONTACT ];
        float ep = maxInfectionProb[ TransmissionRoute::ENVIRONMENTAL ];
        float maxProb = 1-((1-cp)*(1-ep));
        if (maxProb>=1.0)
        {
            gap=1;
        }
        else if (maxProb>0.0)
        {
            gap=int(ceil(log(1.0-GetRng()->e())/log(1.0-maxProb))); //geometric based on fMaxInfectionProb
        }
        return gap;
    }

    // This will actually be by route and be based on typhoid math. We won't iterate over all individuals but use 
    /// contagion populations and typhoid exposure formula before individually heterogeeneous immunity or intervention factors
    //ProbabilityNumber NodeTyphoid::computeMaxInfectionProb( float dt ) const
    void NodeTyphoid::computeMaxInfectionProb( float dt )
    {
        float maxProbRet = 0.0f;
        auto contagionByRouteMap = GetContagionByRoute();
        for( auto map_iter: contagionByRouteMap  )
        {
            const auto& routeName = map_iter.first;
            auto contagion        = map_iter.second;

            // The math below is for ENVIRO ONLY (though contact is closely similar)
            if( routeName == TransmissionRoute::ENVIRONMENTAL )
            {
                auto max_poss_exposures = 3.0f; // what to put here???
                NonNegativeFloat infects = 1.0f-pow( 1.0f + contagion * ( pow( 2.0f, (1/IndividualHumanTyphoid::alpha) ) -1.0f )/IndividualHumanTyphoid::N50, -IndividualHumanTyphoid::alpha ); // Dose-response for prob of infection
                //ProbabilityNumber prob = float( 1.0f - pow(1.0f - infects, max_poss_exposures ) );
                float prob = 1.0f - pow(1.0f - infects, max_poss_exposures );
                if( prob > maxProbRet )
                {
                    maxProbRet = prob;
                }
                maxInfectionProb[ TransmissionRoute::ENVIRONMENTAL ] = maxProbRet;
                LOG_INFO_F( "The max probability of infection over the environmental route (w/o immunity or interventions) for this node is: %f.\n", maxProbRet );
            }
            else // "contact"
            {
                auto contact_contagion = contagion;
                //NonNegativeFloat infects = 1.0f-pow( 1.0f + contact_contagion * ( pow( 2.0f, (1/IndividualHumanTyphoid::alpha) )-1.0f )/IndividualHumanTyphoid::N50,-IndividualHumanTyphoid::alpha );
                ProbabilityNumber infects = contact_contagion / IndividualHumanTyphoidConfig::typhoid_acute_infectiousness;
                int number_of_exposures = 3.0f; // ???
                //ProbabilityNumber prob = float( 1.0f - pow(1.0f - infects, number_of_exposures) );
                float prob = 1.0f - pow(1.0f - infects, number_of_exposures);
                if( prob > maxProbRet )
                {
                    maxProbRet = prob;
                }
                maxInfectionProb[ TransmissionRoute::CONTACT ] = maxProbRet;
                LOG_INFO_F( "The max probability of infection over the contact route (w/o immunity or interventions) for this node is: %f.\n", maxProbRet );
            }
        } 
        gap = 1;
        bSkipping = false; // changes from individual to individual. Initialize to false
    }

}
#endif // ENABLE_TYPHOID
