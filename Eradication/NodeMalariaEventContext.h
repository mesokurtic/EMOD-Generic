/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "NodeVectorEventContext.h"
#include "VectorDefs.h"

namespace Kernel
{
    class INodeMalariaInterventionEffects : public ISupports
    {
        public:
            virtual void ChallengeWithSporozoites(int n_sporozoites, float coverage=1.0f, tAgeBitingFunction=nullptr ) = 0;
            virtual void ChallengeWithInfectiousBites(int n_bites, float coverage=1.0f, tAgeBitingFunction=nullptr) = 0;
    };

    class NodeMalariaEventContextHost : public NodeVectorEventContextHost,
                                        public INodeMalariaInterventionEffects
    {
        IMPLEMENT_NO_REFERENCE_COUNTING()

    public:
        NodeMalariaEventContextHost(Node* _node);
        virtual ~NodeMalariaEventContextHost();

        virtual INodeMalariaInterventionEffects* GetNodeMalariaInterventionEffects() override;

        virtual void ChallengeWithSporozoites( int n_sporozoites, float coverage, tAgeBitingFunction );
        virtual void ChallengeWithInfectiousBites( int n_bites, float coverage, tAgeBitingFunction );
    };
}