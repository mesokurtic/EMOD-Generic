/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>

#include "ISupports.h"
#include "NodeEventContext.h"
#include "NodeEventContextHost.h"
#include "VectorEnums.h"
#include "VectorContexts.h"
#include "VectorMatingStructure.h"
#include "Types.h"
#include "LarvalHabitatMultiplier.h"

namespace Kernel
{
    class Simulation;

    class NodeVectorEventContextHost :
        public NodeEventContextHost,
        public INodeVectorInterventionEffects,
        public INodeVectorInterventionEffectsApply
    {
        IMPLEMENT_NO_REFERENCE_COUNTING()

    public:
        NodeVectorEventContextHost(Node* _node);
        virtual ~NodeVectorEventContextHost();

        virtual INodeVectorInterventionEffects*      GetNodeVectorInterventionEffects()       override;
        virtual INodeVectorInterventionEffectsApply* GetNodeVectorInterventionEffectsApply()  override;

        // INodeVectorInterventionEffectsApply
        virtual void UpdateLarvalKilling( VectorHabitatType::Enum habitat, float killing ) override;
        virtual void UpdateLarvalHabitatReduction( VectorHabitatType::Enum habitat, float reduction ) override;
        virtual void UpdateLarvalHabitatReduction( const LarvalHabitatMultiplier& lhm ) override;
        virtual void UpdateOutdoorKilling( float killing ) override;
        virtual void UpdateVillageSpatialRepellent(float reduction) override;
        virtual void UpdateADIVAttraction(float reduction) override;
        virtual void UpdateADOVAttraction(float reduction) override;
        virtual void UpdatePFVKill(float killing) override;
        virtual void UpdateOutdoorKillingMale(float killing) override;
        virtual void UpdateSugarFeedKilling(float killing) override;
        virtual void UpdateOviTrapKilling(VectorHabitatType::Enum  habitat, float killing) override;
        virtual void UpdateAnimalFeedKilling(float killing) override;
        virtual void UpdateOutdoorRestKilling(float killing) override;
        virtual void UpdateIndoorKilling(float killing) override;

        virtual void ReleaseMosquitoes( NonNegativeFloat cost, const std::string& species, const VectorMatingStructure& genetics, uint32_t number ) override;

        // INodeVectorInterventionEffects;
        virtual float GetLarvalKilling(VectorHabitatType::Enum) override;
        virtual float GetLarvalHabitatReduction(VectorHabitatType::Enum, const std::string& species) override;
        virtual float GetVillageSpatialRepellent() override;
        virtual float GetADIVAttraction() override;
        virtual float GetADOVAttraction() override;
        virtual float GetPFVKill() override;
        virtual float GetOutdoorKilling() override;
        virtual float GetOutdoorKillingMale() override;
        virtual float GetSugarFeedKilling() override;
        virtual float GetOviTrapKilling(VectorHabitatType::Enum) override;
        virtual float GetAnimalFeedKilling() override;
        virtual float GetOutdoorRestKilling() override;
        virtual float GetIndoorKilling() override;

        VectorHabitatType::Enum larval_killing_target;
        VectorHabitatType::Enum larval_reduction_target;
        VectorHabitatType::Enum ovitrap_killing_target;

        LarvalHabitatMultiplier larval_reduction;

    protected: 
        float pLarvalKilling;
        float pLarvalHabitatReduction;
        float pVillageSpatialRepellent;
        float pADIVAttraction;
        float pADOVAttraction;
        float pPFVKill;
        float pOutdoorKilling;
        float pOutdoorKillingMale;
        float pSugarFeedKilling;
        float pOviTrapKilling;
        float pAnimalFeedKilling;
        float pOutdoorRestKilling;
        float pIndoorKilling;

    private:
        NodeVectorEventContextHost() : NodeEventContextHost(nullptr) { }
    };
}
