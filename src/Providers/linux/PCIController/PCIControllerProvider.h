//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software, Hewlett-Packard Company, IBM,
// The Open Group, Tivoli Systems
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE INCLUDED IN
// ALL COPIES OR SUBSTANTIAL PORTIONS OF THE SOFTWARE. THE SOFTWARE IS PROVIDED
// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//==============================================================================
//
// Author: Christopher Neufeld <neufeld@linuxcare.com>
//         David Kennedy       <dkennedy@linuxcare.com>
//
// Modified By: David Kennedy       <dkennedy@linuxcare.com>
//              Christopher Neufeld <neufeld@linuxcare.com>
//              Al Stone            <ahs3@fc.hp.com>
//
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_PCIControllerProvider_h
#define Pegasus_PCIControllerProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMInstance.h>

#include "PCIControllerData.h"


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

#define DEBUG(X) Logger::put(Logger::DEBUG_LOG, "Linux_PCIControllerProvider", Logger::INFORMATION, "$0", X)

/** The actual work of answering provider queries for the
 *  Linux_PCIController class is done by this class.  The method
 *  functions are just like all other Linuxcare-written providers. */
class LinuxPCIControllerProvider : public CIMInstanceProvider
{
   private:
      /** The name of the class used.  If this provider ever extends to
       *  covering the CIM_PCIBridge and CIM_PCIDevice classes, it will have to
       *  be able to distinguish under which mode it is operating.  This helps
       *  keep things straight. */
      String classname;

   public:

      LinuxPCIControllerProvider(String const &cname) { classname = cname; }
      ~LinuxPCIControllerProvider() {}

      void getInstance(const OperationContext& context,
		       const CIMObjectPath& ref,
		       const Uint32 flags,
		       const CIMPropertyList& propertyList,
		       ResponseHandler<CIMInstance>& handler );

      void enumerateInstances(const OperationContext& context,
			      const CIMObjectPath& ref,
			      const Uint32 flags,
			      const CIMPropertyList& propertyList,
			      ResponseHandler<CIMInstance>& handler );

      void enumerateInstanceNames(const OperationContext& context,
			          const CIMObjectPath &ref,
			          ResponseHandler<CIMObjectPath>& handler );

      void modifyInstance(const OperationContext& context,
		          const CIMObjectPath& ref,
		          const CIMInstance& instanceObject,
		          const Uint32 flags,
		          const CIMPropertyList& propertyList,
		          ResponseHandler<CIMInstance>& handler );

      void createInstance(const OperationContext& context,
		          const CIMObjectPath& ref,
		          const CIMInstance& instanceObject,
		          ResponseHandler<CIMObjectPath>& handler );

      void deleteInstance(const OperationContext& context,
		          const CIMObjectPath& ref,
		          ResponseHandler<CIMInstance>& handler );

      void initialize(CIMOMHandle& handle);
      void terminate(void);

   protected:
      CIMObjectPath fill_reference(String const& nameSpace,
	    			  String const& className,
			          PCIControllerData const* ptr);

      CIMInstance build_instance(String const& className,
			         PCIControllerData const* ptr);
};


PEGASUS_NAMESPACE_END

#endif 
