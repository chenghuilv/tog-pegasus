//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001 BMC Software, Hewlett-Packard Company, IBM,
// The Open Group, Tivoli Systems
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies of substantial portions of this software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//==============================================================================
//
// Author: Bapu Patil, Hewlett-Packard Company (bapu_patil@hp.com)
//
// Modified By:
//              Nag Boranna, Hewlett-Packard Company (nagaraja_boranna@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include "LocalAuthFile.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


LocalAuthFile::LocalAuthFile(const String& userName)
    : _userName(userName),
      _filePathName(String::EMPTY),
      _challenge(String::EMPTY)
{

}

LocalAuthFile::~LocalAuthFile() 
{ 

}

//
// create a file and make a random token data entry to it.
// send the file name back to caller
//
String LocalAuthFile::create()
{
    return (String::EMPTY);
}

//
// remove the file that was created
//
Boolean LocalAuthFile::remove()
{
    return (true);
}

//
// Get the string that was created as a challenge string
//
String LocalAuthFile::getChallengeString()
{
    return (_challenge);
}

//
// changes the file owner to one specified
//
Boolean LocalAuthFile::_changeFileOwner(const String& fileName)
{
    return (true);
}

//
// Generate  random token string
//
String LocalAuthFile::_generateRandomTokenString()
{
    return (String::EMPTY);
}

PEGASUS_NAMESPACE_END
