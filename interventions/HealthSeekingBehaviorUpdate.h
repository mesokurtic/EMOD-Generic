/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>

#include "Interventions.h"
#include "Configuration.h"
#include "InterventionFactory.h"
#include "InterventionEnums.h"
#include "FactorySupport.h"
#include "Configure.h"


namespace Kernel
{
    struct ITBInterventionsContainer;

    class HealthSeekingBehaviorUpdate : public BaseIntervention
    {
        DECLARE_FACTORY_REGISTERED(IndividualIVFactory, HealthSeekingBehaviorUpdate, IDistributableIntervention)

    public: 
        HealthSeekingBehaviorUpdate();
        virtual ~HealthSeekingBehaviorUpdate() { };

        virtual bool Configure( const Configuration * config ) override;

        // We inherit AddRef/Release abstractly through IHealthSeekBehavior,
        // even though BaseIntervention has a non-abstract version.
        virtual int32_t AddRef() override { return BaseIntervention::AddRef(); }
        virtual int32_t Release() override { return BaseIntervention::Release(); }

        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pCCO ) override;
        virtual void SetContextTo(IIndividualHumanContext *context) override;
        virtual void Update(float dt) override;

    protected:
        float new_probability_of_seeking;
        ITBInterventionsContainer* ihsbuea;
    };
}

