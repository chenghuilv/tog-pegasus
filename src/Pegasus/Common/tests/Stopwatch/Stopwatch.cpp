//BEGIN_LICENSE
//
// Copyright (c) 2000 The Open Group, BMC Software, Tivoli Systems, IBM
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//END_LICENSE
//BEGIN_HISTORY
//
// Author:
//
// $Log: Stopwatch.cpp,v $
// Revision 1.3  2001/04/11 00:23:44  mike
// new files
//
// Revision 1.2  2001/04/10 23:01:53  mike
// Added new TimeValue class and regression tests for it.
// Modified Stopwatch class to use TimeValue class.
//
// Revision 1.1  2001/02/17 20:08:06  mike
// new
//
//
//
//END_HISTORY

#include <iostream>
#include <cassert>
#include <Pegasus/Common/Stopwatch.h>

using namespace Pegasus;
using namespace std;

int main()
{
    Stopwatch sw;
    System::sleep(5);
    // sw.printElapsed();
    double elapsed = sw.getElapsed();
    assert(elapsed >= 4.5 && elapsed <= 5.5);

    cout << "+++++ passed all tests" << endl;

    return 0;
}
