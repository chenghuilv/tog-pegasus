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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////


#include <cassert>
#include <strstream>
#include <cstring>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/CommonUTF.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static char * verbose;

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE");

    String s1 = "Hello World";
    String s2 = s1;
    String s3(s2);

    assert(String::equal(s1, s3));

    // Test append characters to String
    String s4 = "Hello";
    s4.append(Char16(0x0000));
    s4.append(Char16(0x1234));
    s4.append(Char16(0x5678));
    s4.append(Char16(0x9cde));
    s4.append(Char16(0xffff));

    {
	ostrstream os;
	os << s4;
	os.put('\0');
#ifndef PEGASUS_HAS_ICU
	const char EXPECTED[] = "Hello\\x0000\\x1234\\x5678\\x9CDE\\xFFFF";
#else
	CString cstr = s4.getCString();
	const char * EXPECTED = (const char *)cstr;
#endif
	char* tmp = os.str();
	assert(strcmp(EXPECTED, tmp) == 0);
#ifdef PEGASUS_PLATFORM_AIX_RS_IBMCXX
        os.freeze(false);
#else
	delete tmp;
#endif
    }

    {
	// Test getCString
	const char STR0[] = "one two three four";
	String s = STR0;
	assert(strcmp(s.getCString(), STR0) == 0);
    }

    {
	// Test remove
	String s = "abcdefg";
	s.remove(3, 3);
	assert(String::equal(s, "abcg"));
	assert(s.size() == 4);

	s = "abcdefg";
	s.remove(3, 4);
	assert(String::equal(s, "abc"));
	assert(s.size() == 3);

	s = "abcdefg";
	s.remove(3);
	assert(String::equal(s, "abc"));
	assert(s.size() == 3);

	s = "abc";
	s.remove(3);
	assert(String::equal(s, "abc"));
	assert(s.size() == 3);

	s = "abc";
	s.remove(0);
	assert(String::equal(s, ""));
	assert(s.size() == 0);

	s = "abc";
	s.remove(0, 1);
	assert(String::equal(s, "bc"));
	assert(s.size() == 2);

	String t1 = "HELLO";
	String t2 = t1;
	t2.toLower();
	assert(String::equal(t1, "HELLO"));
	assert(String::equal(t2, "hello"));
    }

    {
	// another test of the append method
	String t1 = "one";
	t1.append(" two");
    	assert(String::equal(t1, "one two"));
	t1.append(' ');
	t1.append('t');
	t1.append('h');
	t1.append('r');
	t1.append("ee");
	assert(String::equal(t1,"one two three"));
	
	// used as example in Doc.
	String test = "abc";
	test.append("def");
	assert(test == "abcdef");
    }

    // Test of the different overload operators
    {
	// Test the == overload operator
	String t1 = "one";
	String t2 = "one";
	assert(t1 == "one");
	assert("one" == t1);
	assert(t1 == t2);
	assert(t2 == t1);
	assert(String("one") == "one");

	const char STR0[] = "one two three four";
	String s = STR0;
	CString tmp = s.getCString();
	assert(tmp == s);
	assert(s == tmp);
    }

    {
	// Tests of the + Overload operator
	String t1 = "abc";
	String t2 = t1 + t1;
	assert(t2 == "abcabc");
	t1 = "abc";
	t2 = t1 + "def";
	assert(t2 == "abcdef");

	t1 = "ghi";
	assert(t1 == "ghi");

	// ATTN: the following fails because there
	// is no single character overload operator
	// KS: Apr 2001
	// t2 = t1 + 'k' + 'l' + 'm' + "nop";
	t2 = t1 + "k" + "l" + "m" + "nop";
	assert(t2 == "ghiklmnop");
	assert(String::equal(t2,"ghiklmnop"));

	// add tests for != operator.

	t1 = "abc";
	assert(t1 != "ghi");
	assert(t1 != t2);

	// add tests for other compare operators

	// Operater <
	t1 = "ab";
	t2 = "cde";
	assert(t1 < t2);
	assert(t1 <= t2);
	assert(t2 > t1);
	assert(t2 >=t1);
	t2 = t1;
	assert(t1 <= t2);
	assert(t1 >= t2);

	// comparison and equals
	// the compare is for null term strings.
	// Therefore following does not work
	// the compare operators cover the problem
	// for String objects.
	// assert(String::compare(t1,t2) == -1);


	// Tests for compare with same length
	t1 = "abc";
	t2 = "def";
	assert(t1 < t2);

	// comparison and equals
	// compare is for null term strings
	// therefore following does not work.
	//assert(String::compare(t1,t2) == -1); 
    }

    {
	// Test of the [] operator
	String t1 = "abc";
	Char16 c = t1[1];
	// note c is Char16
	assert(c == 'b');

	//ATTN: test for outofbounds exception
	try
	{
	    c = t1[200];
	}
	catch (IndexOutOfBoundsException&)
	{
	assert(true);
	}
    }

    {
	// Test the find function
	String t1 = "abcdef";
	String t2 = "cde";
	String t3 = "xyz";
	String t4 = "abc";
	String t5 = "abd";
	String t6 = "defg";
	assert(t1.find('c') == 2);
	assert(t1.find(t2)==2);
	assert(t1.find(t3)==(Uint32)-1);
	assert(t1.find(t4)==0);
	assert(t1.find(t5)==(Uint32)-1);
	assert(t1.find(t6)==(Uint32)-1);
	assert(t1.find("cde")==2);
	assert(t1.find("def")==3);
	assert(t1.find("xyz")==(Uint32)-1);
	assert(t1.find("a") ==0);

	// test for the case where string
	// partly occurs and then later
	// completely occurs
        String s = "this is an apple";
	assert(s.find("apple")==11);
	assert(s.find("appld")==(Uint32)-1);
	assert(s.find("this")==0);
	assert(s.find("t")==0);
	assert(s.find("e")==15);
	s = "a";
	assert(s.find("b")==(Uint32)-1);
	assert(s.find("a")==0);
	assert(s.find(s)==0);
	s = "aaaapple";
	assert(s.find("apple")==3);

        // 20020715-RK This method was removed from the String class
	//{
	//    String nameSpace = "a#b#c";
	//    nameSpace.translate('#', '/');
	//    assert(nameSpace == "a/b/c");
	//}
    }

    {
	// Test String unicode enablement
	char utf8chr[]    = {  
                              0xCE,0x99,0xCE,0xBF,0xCF,0x8D,0xCE,0xBD,0xCE,
                              0xB9,0xCE,0xBA,0xCE,0xBF,0xCE,0xBD,0xCF,0x84,
                              0x00
                            }; // utf8 string with mutliple byte characters  
	char utf8bad[]    = {  
                              0xFF,0xFF,0xFF
                            }; // utf8 string with mutliple byte characters 
        Char16 utf16chr[] =	{
					  0x0399,0x03BF,0x03CD,0x03BD,0x03B9,
                              0x03BA,0x03BF,0x03BD,0x03C4,0x00
                            };  // utf16 representation of the utf8 string

	String utf16string(utf16chr);
	String utf8string(utf8chr,STRING_FLAG_UTF8);
	String utf16merge(utf8string.getChar16Data());

	CString temp = utf8string.getCString();
	CString temp2 = utf16string.getCString();

	const char*  tmp = (const char *)temp;
	const char*  tmp2 = (const char *)temp2;
	
	assert(utf16string == utf8string);
	assert(utf16string == utf16merge);
        assert(utf16string == utf16chr); 
	assert(utf8string  == utf16chr); 
        
	assert(memcmp(utf8string.getChar16Data(),utf16string.getChar16Data(),sizeof(utf16chr)) == 0);
	assert(strcmp(utf8string.getCString(),utf8chr) == 0);
        assert(strcmp(utf16string.getCString(),utf8chr) == 0);
	assert(strcmp(tmp,utf8chr) == 0);
	assert(strcmp(tmp2,utf8chr) == 0);

        Uint32 count = 0;
	Uint32 size = sizeof(utf8chr);
	while(count<size)
	{
	  	assert(isUTF8(&utf8chr[count]) == true);
	   	UTF8_NEXT(utf8chr,count);
       	}

	count = 0;
	size = sizeof(utf8bad);
	while(count<size)
	{
	  	assert(isUTF8(&utf8bad[count]) == false);
	   	UTF8_NEXT(utf8bad,count);
       	}    
        String little("the quick brown fox jumped over the lazy dog"); 
        String    big("THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG");

	String tmpBig = big;
	String tmpLittle = little;

	tmpBig.toLower(ENGLISH_US);
	assert(tmpBig == little);

	tmpBig.toUpper();	
	assert(tmpBig == big);

	Char16 utf16Chars[] =
        {
        0x6A19,	0x6E96,	0x842C, 0x570B,	0x78BC,
        0x042E, 0x043D, 0x0438, 0x043A, 0x043E, 0x0434,
        0x110B, 0x1172, 0x1102, 0x1165, 0x110F, 0x1169, 0x11AE, 
        0x10E3, 0x10DC, 0x10D8, 0x10D9, 0x10DD, 0x10D3, 0x10D8,
	0xdbc0,	0xdc01, 
        0x05D9, 0x05D5, 0x05E0, 0x05D9, 0x05E7, 0x05D0, 0x05B8, 0x05D3,
        0x064A, 0x0648, 0x0646, 0x0650, 0x0643, 0x0648, 0x062F,
        0x092F, 0x0942, 0x0928, 0x093F, 0x0915, 0x094B, 0x0921,
        0x016A, 0x006E, 0x012D, 0x0063, 0x014D, 0x0064, 0x0065, 0x033D,
        0x00E0, 0x248B, 0x0061, 0x2173, 0x0062, 0x1EA6, 0xFF21, 0x00AA, 0x0325, 0x2173, 0x249C, 0x0063,
        0x02C8, 0x006A, 0x0075, 0x006E, 0x026A, 0x02CC, 0x006B, 0x006F, 0x02D0, 0x0064,
        0x30E6, 0x30CB, 0x30B3, 0x30FC, 0x30C9, 
        0xFF95, 0xFF86, 0xFF7A, 0xFF70, 0xFF84, 0xFF9E, 
        0xC720, 0xB2C8, 0xCF5B, 0x7D71, 0x4E00, 0x78BC,
	0xdbc0,	0xdc01,
        0x00};

	String ugly(utf16Chars);
	assert(ugly == utf16Chars);

#ifdef PEGASUS_HAS_ICU
	ugly.toLower("zh_CN");
	assert(ugly != utf16Chars);

	ugly.toUpper("zh_CN");
	assert(ugly != utf16Chars);
#endif
 
    }
                             
#if 0
    // The match code has been removed from the String class
    // Test the string match functions
    {
        String abc = "abc";
        String ABC = "ABC";
        assert(String::match(abc, "abc"));
        assert(String::match(ABC, "ABC"));
        assert(!String::match(abc, "ABC"));
        assert(!String::match(ABC, "abc"));

        assert(String::matchNoCase(abc, "abc"));
        assert(String::matchNoCase(ABC, "abc"));
        assert(String::matchNoCase(abc, "ABC"));
        assert(String::matchNoCase(ABC, "ABc"));

        assert(String::match(abc, "???"));
        assert(String::match(ABC, "???"));
        assert(String::match(abc, "*"));
        assert(String::match(ABC, "*"));

        assert(String::match(abc, "?bc"));
        assert(String::match(abc, "?b?"));
        assert(String::match(abc, "??c"));
        assert(String::matchNoCase(ABC, "?bc"));
        assert(String::matchNoCase(ABC, "?b?"));
        assert(String::matchNoCase(ABC, "??c"));


        assert(String::match(abc, "*bc"));
        assert(String::match(abc, "a*c"));
        assert(String::match(abc, "ab*"));
        assert(String::match(abc, "a*"));
        // ATTN-RK-P3-20020603: This match code is broken
        //assert(String::match(abc, "[axy]bc"));
        assert(!String::match(abc, "[xyz]bc"));

        assert(!String::match(abc, "def"));
        assert(!String::match(abc, "[de]bc"));
        // ATTN-RK-P3-20020603: This match code is broken
        //assert(String::match(abc, "a[a-c]c"));
        assert(!String::match(abc, "a[d-x]c"));
        // ATTN-RK-P3-20020603: This match code does not yet handle escape chars
        //assert(String::match("*test", "\\*test"));

        assert(String::match("abcdef123", "*[0-9]"));

        assert(String::match("This is a test", "*is*"));
        assert(String::matchNoCase("This is a test", "*IS*"));

        assert(String::match("Hello", "Hello"));
        assert(String::matchNoCase("HELLO", "hello"));
        assert(String::match("This is a test", "This is *"));
        assert(String::match("This is a test", "* is a test"));
        assert(!String::match("Hello", "Goodbye"));

        String tPattern = "When in the * of human*e??nts it be?ome[sS] [0-9] nec*";

        // ATTN-RK-P3-20020603: This match code is broken
        //assert(String::match(
        //    "When in the course of human events it becomes 0 necessary",
        //    tPattern));
        //assert(String::match(
        //    "When in the xyz of human events it becomes 9 necessary",
        //    tPattern));
        //assert(String::match(
        //    "When in the  of human events it becomes 3 necessary",
        //    tPattern));
    }
#endif

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
