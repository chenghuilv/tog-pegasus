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

#ifndef Pegasus_DebianPackageManagerData_h
#define Pegasus_DebianPackageManagerData_h


#include <stdio.h>

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMDateTime.h>
#include "PackageManagerData.h"
#include "DebianPackageInformation.h"

#define DEFAULT_DEBIAN_DIRECTORY "/var/lib/dpkg"
#define DEBIAN_STATUS_FILENAME "status"

PEGASUS_NAMESPACE_BEGIN

//
// DebianPackageManagerData
//
class DebianPackageManagerData : public PackageManagerData
{

public:
   DebianPackageManagerData(void);
   DebianPackageManagerData(const String &databaseDirectory);
   ~DebianPackageManagerData(void);

   int initialize(void);
   void terminate(void);

   Array<PackageInformation *> GetAllPackages(void);
   PackageInformation *GetFirstPackage(void);
   PackageInformation *GetNextPackage(void);
   void EndGetPackage(void);
   PackageInformation *GetPackage(const String &name,const String &version);

private:

   FILE * OpenStatusFile(const String &filename);
   int CloseStatusFile(FILE * handle);
   DebianPackageInformation * ReadStatusPackage(FILE * handle);
   String ReadStatusLine(FILE * handle);

   char buffer[BUFSIZ];
   int offset;

   String databaseDirectory;
   FILE * statusFile;

};

PEGASUS_NAMESPACE_END

#endif
