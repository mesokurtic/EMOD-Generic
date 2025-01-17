/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <map>
#include <functional>

#include "IdmApi.h"
#include "Configuration.h"
#include "FactorySupport.h"
#include "Configure.h"
#include "SimulationEnums.h"

// This macro, GET_CONFIGURABLE(Category), is the convenient and fast way to access the SimulationConfig parameters
// within monolithic executable Eradication.exe.  This can be used in DLL's as well since we include the pointer
// to the SimulationConfig object in Environment.
#define GET_CONFIGURABLE(Category)  ((Kernel::Category*)(Environment::get##Category()))

using namespace std;

namespace Kernel
{
    struct VectorParameters;
    struct MalariaParameters;

    class SimulationConfig;

    class ISimulationConfigFactory
    {
    public:
        virtual void Register(string classname, instantiator_function_t _if) = 0;
    };

    class IDMAPI SimulationConfigFactory : public ISimulationConfigFactory
    {
    public:
        static ISimulationConfigFactory * getInstance();

        static SimulationConfig* CreateInstance(Configuration * config);
        void Register(string classname, instantiator_function_t _if);

    protected:
        static support_spec_map_t& getRegisteredClasses();

    private:
        static ISimulationConfigFactory * _instance;
    };


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SimulationConfig Class Layout Special Notes
// Note 1:
// All non-primitive objects on the stack (like STL or user-defined type) have to be declared at the end
// for the primitive objects in the SimulationConfig object crossing the DLL/EModule boundary
// without affecting the memory image and therefore its member values
// Note 2:
// All the non-primitive _static_ objects will not be able to cross DLL/EModule boundary
// so their values will be different on two sides of memory space even for the same SimulationConfig object
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! DMB 9/7/2016 - This would be a bigger issue if we were supporting the ability of users to create their DLL to
// !!! work with our EXE.  Since we only support the situation where the user builds both the EXE and DLL,
// !!! the memory layout and boundary issues are not really a concern.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details

    class IDMAPI SimulationConfig : public JsonConfigurable
    {

        DECLARE_FACTORY_REGISTERED(SimulationConfigFactory, SimulationConfig, IConfigurable)

    public:
        DECLARE_CONFIGURED(SimulationConfig)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

        VectorParameters*  vector_params;
        MalariaParameters* malaria_params;

        ///////////////////////////////////////////
        SimulationConfig();
        virtual ~SimulationConfig();

        virtual QuickBuilder SimulationConfig::GetSchema() override;

        const Configuration* GetJsonConfigObj() const { return m_jsonConfig; }

    private: // for serialization to work
        const Configuration* m_jsonConfig;

        void VectorInitConfig( const Configuration* inputJson );
        void VectorCheckConfig( const Configuration* inputJson );
        void VectorAddSchema( json::QuickBuilder& retJson );

        void MalariaInitConfig( const Configuration* inputJson );
        void MalariaCheckConfig( const Configuration* inputJson );
        void MalariaAddSchema( json::QuickBuilder& retJson );
    };
#pragma warning( pop )
}
