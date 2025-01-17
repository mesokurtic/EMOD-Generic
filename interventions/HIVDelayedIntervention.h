/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "SimulationEnums.h"
#include "DelayedIntervention.h"
#include "Configure.h"
#include "InterpolatedValueMap.h"
#include "EventTrigger.h"

namespace Kernel
{
    class HIVDelayedIntervention: public DelayedIntervention
    {
        DECLARE_FACTORY_REGISTERED(IndividualIVFactory, HIVDelayedIntervention, IDistributableIntervention)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        HIVDelayedIntervention();
        HIVDelayedIntervention( const HIVDelayedIntervention& );

        virtual bool Configure( const Configuration* config ) override;
        virtual void Update(float dt) override;

        virtual void Callback( float dt );

    protected:
        virtual void CalculateDelay();

        InterpolatedValueMap year2DelayMap;
        float days_remaining;

        EventTrigger::Enum broadcast_event;
        EventTrigger::Enum broadcast_on_expiration_event;

        DECLARE_SERIALIZABLE(HIVDelayedIntervention);
    };
}
