//%2003////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002  BMC Software, Hewlett-Packard Development
// Company, L. P., IBM Corp., The Open Group, Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L. P.;
// IBM Corp.; EMC Corporation, The Open Group.
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
// Authors: David Rosckes (rosckes@us.ibm.com)
//          Bert Rivero (hurivero@us.ibm.com)
//          Chuck Carmack (carmack@us.ibm.com)
//          Brian Lucier (lucier@us.ibm.com) 
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>
#include <cassert>
                                                                                                                                       
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/CQL/CQLChainedIdentifier.h>
#include <Pegasus/CQL/CQLIdentifier.h>   
#include <Pegasus/CQL/SubRange.h>

PEGASUS_USING_PEGASUS;
                                                                                                                                       
PEGASUS_USING_STD;

void print(CQLIdentifier &_id){
	cout << "Name = " << _id.getName().getString() << endl;
	if(_id.isScoped())
		cout << "Scope = " << _id.getScope() << endl;
	if(_id.isSymbolicConstant())
		cout << "Symbolic Constant = " << _id.getSymbolicConstantName() << endl;
	if(_id.isWildcard())
		cout << "CQLIdentifier = *" << endl;
	if(_id.isArray()){
		cout << "SubRanges: ";
		Array<SubRange> _ranges = _id.getSubRanges();
		for(Uint32 i = 0; i < _ranges.size(); i++){
			cout << _ranges[i].toString() << ",";
		}
		cout << endl;
	}
}

void drive_CQLIdentifier(){
	CQLIdentifier _ID1("ID1");
	CIMName name = _ID1.getName();
	cout << "name = " << name.getString() << endl;
	CQLIdentifier _ID2("ID2");
	cout << "name = " << _ID2.getName().getString() << endl;
	assert(_ID1 != _ID2);
	CQLIdentifier _ID3("*");
	assert(_ID3.isWildcard());
	
	CQLIdentifier scopedID("SCOPE::IDENTIFIER");
	cout << "name = " << scopedID.getName().getString() << endl;
	cout << "scope = " << scopedID.getScope() << endl;

	CQLIdentifier _ID4("A::Name");
	CQLIdentifier _ID4a("A::Name");
	assert(_ID4 == _ID4a);
	
	CQLIdentifier symbolicConstantID("Name#OK");
	cout << "name = " << symbolicConstantID.getName().getString() << endl;
	assert(symbolicConstantID.isSymbolicConstant());
	cout << "symbolicConstant = " << symbolicConstantID.getSymbolicConstantName() << endl;

	CQLIdentifier rangeID("SCOPE::Name[5..,6..,..7,4-5,..]");
	cout << "name = " << rangeID.getName().getString() << endl;
	assert(rangeID.isArray());
	Array<SubRange> subRanges = rangeID.getSubRanges();
	for(Uint32 i = 0; i < subRanges.size(); i++){
		cout << "subRange[" << i << "].start = " << subRanges[i].start << endl;
		cout << "subRange[" << i << "].end = " << subRanges[i].end << endl;
	}
	cout << "scope = " << rangeID.getScope() << endl;

	CQLIdentifier rangeID1("Name[*]");
	assert(rangeID1.isArray());
        Array<SubRange> subRanges1 = rangeID1.getSubRanges();
        for(Uint32 i = 0; i < subRanges1.size(); i++){
                cout << "subRange[" << i << "].start = " << subRanges1[i].start << endl;
                cout << "subRange[" << i << "].end = " << subRanges1[i].end << endl;
        }

	CQLIdentifier invalid("Name#OK[4-5]");
	cout << "name = " << invalid.getName().getString() << endl;
	CQLIdentifier invalid1("Name[4-5]#OK");
}

void drive_CQLChainedIdentifier(){
	CQLChainedIdentifier _CI("CLASS.B::EO.A::PROP[*]");
	Array<CQLIdentifier> _arr = _CI.getSubIdentifiers();
	cout << _CI.toString() << endl;
	for(Uint32 i = 0; i < _arr.size(); i++)
		print(_arr[i]);
}

int main( int argc, char *argv[] ){

        //BEGIN TESTS....

	//drive_CQLIdentifier();
	drive_CQLChainedIdentifier();

	//END TESTS....
	                                                                                                                   
        cout << argv[0] << " +++++ passed all tests" << endl;
                                                                                                                                       
        return 0;
}

