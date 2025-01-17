/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "Simulation.h"
#include "IIdGeneratorSTI.h"
#include "ISTISimulationContext.h"
#include "Sugar.h" // for DECLARE_VIRTUAL_BASE

namespace Kernel
{
    class IndividualHumanSTI;
    struct IRelationship;

    class SimulationSTI : public Simulation, public IIdGeneratorSTI, public ISTISimulationContext
    {
        GET_SCHEMA_STATIC_WRAPPER(SimulationSTI)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        virtual ~SimulationSTI(void);
        static SimulationSTI *CreateSimulation();
        static SimulationSTI *CreateSimulation(const ::Configuration *config);

        // methods of IIdGeneratorSTI
        virtual suids::suid GetNextRelationshipSuid() override;

        // methods of ISTISimulationContext
        virtual void AddTerminatedRelationship( const suids::suid& nodeSuid, const suids::suid& relId ) override;
        virtual bool WasRelationshipTerminatedLastTimestep( const suids::suid& relId ) const override;

        // INodeInfoFactory
        virtual INodeInfo* CreateNodeInfo() override;
        virtual INodeInfo* CreateNodeInfo( int rank, INodeContext* pNC ) override;

    protected:
        SimulationSTI();

        virtual void Initialize() override;
        virtual void Initialize(const ::Configuration *config) override;

        virtual bool ValidateConfiguration(const ::Configuration *config) override;

        virtual bool Configure( const ::Configuration *json );
        virtual void Reports_CreateBuiltIn();

        // Allows correct type of Node to be added by classes derived from Simulation
        virtual void addNewNodeFromDemographics( ExternalNodeId_t externalNodeId,
                                                 suids::suid node_suid,
                                                 NodeDemographicsFactory *nodedemographics_factory,
                                                 ClimateFactory *climate_factory ) override;

        suids::distributed_generator relationshipSuidGenerator;

        bool report_relationship_start;
        bool report_relationship_end;
        bool report_relationship_consummated;
        bool report_transmission;
    };
}
