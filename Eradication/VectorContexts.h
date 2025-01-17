/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "VectorEnums.h"
#include "IVectorHabitat.h"
#include "IVectorPopulation.h"

namespace Kernel
{
    struct IStrainIdentity;
    struct IVectorCohort;
    class  VectorHabitat;
    class  VectorMatingStructure;
    class  VectorProbabilities;
    class  LarvalHabitatMultiplier;

    typedef std::list<IVectorPopulation*> VectorPopulationReportingList_t;

    struct IVectorSimulationContext : public ISupports
    {
        virtual void  PostMigratingVector( const suids::suid& nodeSuid, IVectorCohort* ind ) = 0;
        virtual float GetNodePopulation( const suids::suid& nodeSuid ) = 0;
        virtual float GetAvailableLarvalHabitat( const suids::suid& nodeSuid, const std::string& rSpeciesID ) = 0 ;
    };

    class INodeVector : public ISupports
    {
    public:
        virtual const VectorPopulationReportingList_t& GetVectorPopulationReporting() const = 0;
        virtual void AddVectors( const std::string& releasedSpecies, const VectorMatingStructure& _vector_genetics, uint32_t releasedNumber ) = 0;
        virtual void processImmigratingVector( IVectorCohort* immigrant ) = 0;
        virtual std::string GetHabitatName(int) = 0;

        virtual VectorProbabilities* GetVectorLifecycleProbabilities() = 0;
        virtual IVectorHabitat*      GetVectorHabitatBySpeciesAndType( std::string& species, VectorHabitatType::Enum type, const Configuration* inputJson ) = 0;
        virtual VectorHabitatList_t* GetVectorHabitatsBySpecies( std::string& species ) = 0;
        virtual float                GetLarvalHabitatMultiplier( VectorHabitatType::Enum type, const std::string& species ) const = 0;
    };

    struct IIndividualHumanVectorContext : public ISupports
    {
        virtual float GetRelativeBitingRate(void) const = 0;
    };

    struct ISusceptibilityVector : public ISupports
    {
        virtual void  SetRelativeBitingRate( float rate ) = 0;
        virtual float GetRelativeBitingRate(void) const = 0;
    };

    struct IVectorInterventionsEffects : ISupports
    {
        virtual float GetDieBeforeFeeding() = 0;
        virtual float GetHostNotAvailable() = 0;
        virtual float GetDieDuringFeeding() = 0;
        virtual float GetDiePostFeeding() = 0;
        virtual float GetSuccessfulFeedHuman() = 0;
        virtual float GetSuccessfulFeedAD() = 0;
        virtual float GetOutdoorDieBeforeFeeding() = 0;
        virtual float GetOutdoorHostNotAvailable() = 0;
        virtual float GetOutdoorDieDuringFeeding() = 0;
        virtual float GetOutdoorDiePostFeeding() = 0;
        virtual float GetOutdoorSuccessfulFeedHuman() = 0;
        virtual float GetblockIndoorVectorAcquire() = 0;
        virtual float GetblockIndoorVectorTransmit() = 0;
        virtual float GetblockOutdoorVectorAcquire() = 0;
        virtual float GetblockOutdoorVectorTransmit() = 0;

        virtual void UpdateProbabilityOfBlocking( float prob ) = 0;
        virtual void UpdateProbabilityOfKilling( float prob ) = 0;

        virtual void UpdateRelativeBitingRate( float rate ) = 0;

        virtual void UpdatePhotonicFenceKillingRate( float rate ) = 0;
        virtual void UpdateArtificialDietAttractionRate( float rate ) = 0;
        virtual void UpdateArtificialDietKillingRate( float rate ) = 0;
        virtual void UpdateInsecticidalDrugKillingProbability( float prob ) = 0;

        virtual void UpdateProbabilityOfIndRepBlocking( float prob ) = 0;
        virtual void UpdateProbabilityOfIndRepKilling( float prob ) = 0;

        virtual void ApplyHouseBlockingProbability( float prob ) = 0;
        virtual void UpdateProbabilityOfScreenKilling( float prob ) = 0;

        virtual ~IVectorInterventionsEffects() { }
    };

    struct INodeVectorInterventionEffects : ISupports
    {
        virtual float GetLarvalKilling(VectorHabitatType::Enum) = 0;
        virtual float GetLarvalHabitatReduction(VectorHabitatType::Enum, const std::string& species) = 0;
        virtual float GetVillageSpatialRepellent() = 0;
        virtual float GetADIVAttraction() = 0;
        virtual float GetADOVAttraction() = 0;
        virtual float GetPFVKill() = 0;
        virtual float GetOutdoorKilling() = 0;
        virtual float GetOutdoorKillingMale() = 0;
        virtual float GetSugarFeedKilling() = 0;
        virtual float GetOviTrapKilling(VectorHabitatType::Enum) = 0;
        virtual float GetAnimalFeedKilling() = 0;
        virtual float GetOutdoorRestKilling() = 0;
        virtual float GetIndoorKilling() = 0;
    };

    class INodeVectorInterventionEffectsApply : public ISupports
    {
    public:
        virtual void UpdateLarvalKilling( VectorHabitatType::Enum habitat, float killing ) = 0;
        virtual void UpdateLarvalHabitatReduction( VectorHabitatType::Enum habitat, float reduction ) = 0;
        virtual void UpdateLarvalHabitatReduction( const LarvalHabitatMultiplier& lhm ) = 0;
        virtual void UpdateOutdoorKilling( float killing ) = 0;
        virtual void UpdateOviTrapKilling(VectorHabitatType::Enum habitat, float killing) = 0;
        virtual void UpdateVillageSpatialRepellent(float) = 0;
        virtual void UpdateADIVAttraction(float) = 0;
        virtual void UpdateADOVAttraction(float) = 0;
        virtual void UpdatePFVKill(float) = 0;
        virtual void UpdateOutdoorKillingMale(float) = 0;
        virtual void UpdateSugarFeedKilling(float) = 0;
        virtual void UpdateAnimalFeedKilling(float) = 0;
        virtual void UpdateOutdoorRestKilling(float) = 0;
        virtual void UpdateIndoorKilling( float ) = 0;

        virtual void ReleaseMosquitoes( NonNegativeFloat cost, const std::string& species, const VectorMatingStructure& genetics, uint32_t number ) = 0;
    };
}
