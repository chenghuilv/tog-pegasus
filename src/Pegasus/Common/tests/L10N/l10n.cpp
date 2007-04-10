//%2006////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2006 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; Symantec Corporation; The Open Group.
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
//=============================================================================
//
//%////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>

#include <Pegasus/Common/PegasusAssert.h>

#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Common/MessageLoader.h>

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/InternalException.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void testLanguageParser()
{
    // Test the LanguageTag parser
    {
        String tag1("en-US-mn-blah-blah");
        String language;
        String country;
        String variant;

        LanguageParser::parseLanguageTag(tag1, language, country, variant);

        PEGASUS_TEST_ASSERT(language == "en");
        PEGASUS_TEST_ASSERT(country == "US");
        PEGASUS_TEST_ASSERT(variant == "mn-blah-blah");
    }

    // Test handling of Accept-Languages whitespace and comments
    {
        AcceptLanguageList al = LanguageParser::parseAcceptLanguageHeader(
            "    en-US-mn (should not appear)  ,"
            "(and)en-US-ca   (!!!)  ;(less) q(uality) = (just) 0.5 (half)  ");
        PEGASUS_TEST_ASSERT(al.size() == 2);
        PEGASUS_TEST_ASSERT(al.getLanguageTag(0).toString() == "en-US-mn");
        PEGASUS_TEST_ASSERT(al.getQualityValue(0) == 1.0);
        PEGASUS_TEST_ASSERT(al.getLanguageTag(1).toString() == "en-US-ca");
        PEGASUS_TEST_ASSERT(al.getQualityValue(1) == 0.5);
        PEGASUS_TEST_ASSERT(LanguageParser::buildAcceptLanguageHeader(al) ==
            "en-US-mn,en-US-ca;q=0.500");
    }

    // Test handling of Content-Languages whitespace and comments
    {
        ContentLanguageList cl = LanguageParser::parseContentLanguageHeader(
            "    en-US-mn (should not appear)  ,"
            "(and)en-US-ca   (if you can imagine) (!!!)  ");
        PEGASUS_TEST_ASSERT(cl.size() == 2);
        PEGASUS_TEST_ASSERT(cl.getLanguageTag(0).toString() == "en-US-mn");
        PEGASUS_TEST_ASSERT(cl.getLanguageTag(1).toString() == "en-US-ca");
        PEGASUS_TEST_ASSERT(LanguageParser::buildContentLanguageHeader(cl) ==
            "en-US-mn,en-US-ca");
    }

    // Test handling of non-ASCII characters in Content-Languages comment
    {
        String headerValue = "en-US (will add non-ASCII character in comment)";
        headerValue[14] = 132;
        ContentLanguageList cl =
            LanguageParser::parseContentLanguageHeader(headerValue);
        PEGASUS_TEST_ASSERT(cl.size() == 1);
        PEGASUS_TEST_ASSERT(cl.getLanguageTag(0).toString() == "en-US");
        PEGASUS_TEST_ASSERT(
            LanguageParser::buildContentLanguageHeader(cl) == "en-US");
    }

    // Test handling of non-ASCII characters in Content-Languages value
    {
        String headerValue = "en-US-ca (will add non-ASCII character in tag)";
        headerValue[4] = 132;
        Boolean gotException = false;
        try
        {
            ContentLanguageList cl =
                LanguageParser::parseContentLanguageHeader(headerValue);
        }
        catch (const InvalidContentLanguageHeader&)
        {
            gotException = true;
        }
        PEGASUS_TEST_ASSERT(gotException);
    }

    // Test handling of trailing escape character in Content-Languages value
    {
        String headerValue = "en-US-ca (trailing escape character) \\";
        Boolean gotException = false;
        try
        {
            ContentLanguageList cl =
                LanguageParser::parseContentLanguageHeader(headerValue);
        }
        catch (const InvalidContentLanguageHeader&)
        {
            gotException = true;
        }
        PEGASUS_TEST_ASSERT(gotException);
    }
}


void testLanguageTag()
{
    try
    {
        // Test string value constructor and accessor methods

        {
            String tag1("en-US-mn");

            PEGASUS_TEST_ASSERT(LanguageTag(tag1).toString() == "en-US-mn");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "en");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "US");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getVariant() == "mn");
        }

        {
            String tag1("en-US-123");

            PEGASUS_TEST_ASSERT(LanguageTag(tag1).toString() == "en-US-123");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "en");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "US");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getVariant() == "123");
        }

        {
            String tag1("eng-1a-C3P0");

            PEGASUS_TEST_ASSERT(LanguageTag(tag1).toString() == "eng-1a-C3P0");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "eng");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "1a");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getVariant() == "C3P0");
        }

        {
            String tag1("en-my-weird-dialect");

            PEGASUS_TEST_ASSERT(
                LanguageTag(tag1).toString() == "en-my-weird-dialect");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "en");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "my");
            PEGASUS_TEST_ASSERT(
                LanguageTag(tag1).getVariant() == "weird-dialect");
        }

        {
            String tag1("en-quite-a-weird-dialect");

            PEGASUS_TEST_ASSERT(
                LanguageTag(tag1).toString() == "en-quite-a-weird-dialect");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "en");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "");
            PEGASUS_TEST_ASSERT(
                LanguageTag(tag1).getVariant() == "quite-a-weird-dialect");
        }

        {
            String tag1("x-pig-latin");

            PEGASUS_TEST_ASSERT(LanguageTag(tag1).toString() == "x-pig-latin");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getVariant() == "");
        }

        {
            String tag1("i-latin-for-pigs");

            PEGASUS_TEST_ASSERT(
                LanguageTag(tag1).toString() == "i-latin-for-pigs");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getLanguage() == "");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getCountry() == "");
            PEGASUS_TEST_ASSERT(LanguageTag(tag1).getVariant() == "");
        }

        // Test copy constructor, assignment operator, and equality operator

        LanguageTag lt1("en-US-ca");
        LanguageTag lt2(lt1);
        LanguageTag lt3 = lt2;
        LanguageTag lt4("EN-us-Ca");
        LanguageTag lt5("en-US-mn");
        LanguageTag lt6;
        LanguageTag lt7 = lt6;
        LanguageTag lt8 = lt1;
        lt7 = lt1;
        lt7 = lt7;
        lt8 = lt6;
        lt8 = lt1;

        PEGASUS_TEST_ASSERT(lt1 == lt2);
        PEGASUS_TEST_ASSERT(lt1 == lt3);
        PEGASUS_TEST_ASSERT(lt2 == lt3);
        PEGASUS_TEST_ASSERT(lt1 == lt4);
        PEGASUS_TEST_ASSERT(lt1 != lt5);
        PEGASUS_TEST_ASSERT(lt3 != lt5);
        PEGASUS_TEST_ASSERT(lt1 == lt8);
        PEGASUS_TEST_ASSERT(lt7 == lt8);

        // Test invalid language tag:  Empty string
        {
            Boolean gotException = false;

            try
            {
                LanguageTag lt("");
            }
            catch (Exception&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid language tag:  Digit in primary subtag
        {
            Boolean gotException = false;

            try
            {
                LanguageTag lt("e4-US-ca");
            }
            catch (Exception&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid language tag:  Primary subtag too short
        {
            Boolean gotException = false;

            try
            {
                LanguageTag lt("e-US-ca");
            }
            catch (Exception&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid language tag:  Primary subtag too long
        {
            Boolean gotException = false;

            try
            {
                LanguageTag lt("engl-US-ca");
            }
            catch (Exception&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid language tag:  Primary subtag too long
        {
            Boolean gotException = false;

            try
            {
                LanguageTag lt("englishman-US-ca");
            }
            catch (Exception&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid language tag:  Single character second subtag
        {
            Boolean gotException = false;

            try
            {
                LanguageTag lt("en-U-ca");
            }
            catch (Exception&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid language tag:  Second subtag too long
        {
            Boolean gotException = false;

            try
            {
                LanguageTag lt("en-UnitedStates-ca");
            }
            catch (Exception&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid language tag:  Third subtag too long
        {
            Boolean gotException = false;

            try
            {
                LanguageTag lt("en-US-california");
            }
            catch (Exception&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid language tag:  Empty subtag
        {
            Boolean gotException = false;

            try
            {
                LanguageTag lt("en--ca");
            }
            catch (Exception&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid language tag:  Non-ASCII primary tag
        {
            Boolean gotException = false;

            try
            {
                String tag = "en-US-ca";
                tag[1] = 132;
                LanguageTag lt(tag);
            }
            catch (Exception&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid language tag:  Non-ASCII subtag
        {
            Boolean gotException = false;

            try
            {
                String tag = "en-US-ca";
                tag[4] = 132;
                LanguageTag lt(tag);
            }
            catch (Exception&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test uninitialized object:  getLanguage() method
        {
            Boolean gotException = false;
            LanguageTag lt;

            try
            {
                String language = lt.getLanguage();
            }
            catch (UninitializedObjectException&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test uninitialized object:  getCountry() method
        {
            Boolean gotException = false;
            LanguageTag lt;

            try
            {
                String country = lt.getCountry();
            }
            catch (UninitializedObjectException&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test uninitialized object:  getVariant() method
        {
            Boolean gotException = false;
            LanguageTag lt;

            try
            {
                String variant = lt.getVariant();
            }
            catch (UninitializedObjectException&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test uninitialized object:  toString() method
        {
            Boolean gotException = false;
            LanguageTag lt;

            try
            {
                String languageString = lt.toString();
            }
            catch (UninitializedObjectException&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test uninitialized object:  equality operator
        {
            Boolean gotException = false;
            LanguageTag lt1;
            LanguageTag lt2("en-US-ca");

            try
            {
                Boolean test = (lt1 == lt2);
            }
            catch (UninitializedObjectException&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test uninitialized object:  assignment
        {
            LanguageTag lt1;
            LanguageTag lt2("en-US-ca");

            lt1 = lt2;

            PEGASUS_TEST_ASSERT(lt1.toString() == "en-US-ca");
        }

        // Test uninitialized object:  unassignment
        {
            Boolean gotException = false;
            LanguageTag lt1("en-US-ca");
            LanguageTag lt2;

            lt1 = lt2;

            try
            {
                String languageTag = lt1.toString();
            }
            catch (UninitializedObjectException&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }
    }
    catch (Exception& e)
    {
        cout << "Unexpected exception: " << e.getMessage() << endl;
        exit(1);
    }
}


void testAcceptLanguageList()
{
    try
    {
        AcceptLanguageList al = LanguageParser::parseAcceptLanguageHeader(
            "en-US-mn;q=.9,fr-FR;q=.1,en, fr;q=.2,la-SP-bal;q=.7,*;q=.01");

        PEGASUS_TEST_ASSERT(al.size() == 6);

        PEGASUS_TEST_ASSERT(al.getLanguageTag(0).toString() == "en");
        PEGASUS_TEST_ASSERT(al.getLanguageTag(1).toString() == "en-US-mn");
        PEGASUS_TEST_ASSERT(al.getLanguageTag(2).toString() == "la-SP-bal");
        PEGASUS_TEST_ASSERT(al.getLanguageTag(3).toString() == "fr");
        PEGASUS_TEST_ASSERT(al.getLanguageTag(4).toString() == "fr-FR");
        PEGASUS_TEST_ASSERT(al.getLanguageTag(5).toString() == "*");

        PEGASUS_TEST_ASSERT(LanguageParser::buildAcceptLanguageHeader(al) ==
            "en,en-US-mn;q=0.900,la-SP-bal;q=0.700,fr;q=0.200,fr-FR;q=0.100,"
                "*;q=0.010");

        // Test insert() method

        al.insert(LanguageTag("en-XX-xx"), 1.0);
        PEGASUS_TEST_ASSERT(al.size() == 7);
        PEGASUS_TEST_ASSERT(al.find(LanguageTag("en-XX-xx")) != PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(
            al.getQualityValue(al.find(LanguageTag("en-XX-xx"))) == 1.0);

        // Test remove() method

        Uint32 index = al.find(LanguageTag("en-XX-xx"));
        al.remove(index);
        PEGASUS_TEST_ASSERT(al.find(LanguageTag("en-XX-xx")) == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(al.size() == 6);

        // Test assignment operator and equality operator

        AcceptLanguageList al1;

        al1 = al;
        PEGASUS_TEST_ASSERT(al1 == al);

        al1 = al1;
        PEGASUS_TEST_ASSERT(al1 == al);

        al1.remove(0);
        PEGASUS_TEST_ASSERT(al1 != al);

        // Test inequality operator
        {
            AcceptLanguageList list1;
            AcceptLanguageList list2;

            list1.insert(LanguageTag("en-US"), 1);
            list1.insert(LanguageTag("fr"), Real32(0.8));
            list2 = list1;
            PEGASUS_TEST_ASSERT(list1 == list2);

            list2.remove(1);
            PEGASUS_TEST_ASSERT(list1 != list2);

            list2.insert(LanguageTag("fr"), Real32(0.7));
            PEGASUS_TEST_ASSERT(list1 != list2);

            list2.remove(1);
            list2.insert(LanguageTag("de"), Real32(0.8));
            PEGASUS_TEST_ASSERT(list1 != list2);
        }

        // Test clear() method

        al1.clear();
        PEGASUS_TEST_ASSERT(al1.size() == 0);

        // Test sorting of quality values
        {
            AcceptLanguageList al = LanguageParser::parseAcceptLanguageHeader(
                "de;q=0.000,it;q=0.50,*;q=0.25,en-US-ca;q=1.00");

            PEGASUS_TEST_ASSERT(al.getLanguageTag(0).toString() == "en-US-ca");
            PEGASUS_TEST_ASSERT(al.getQualityValue(0) == 1.0);
            PEGASUS_TEST_ASSERT(al.getLanguageTag(1).toString() == "it");
            PEGASUS_TEST_ASSERT(al.getQualityValue(1) == 0.5);
            PEGASUS_TEST_ASSERT(al.getLanguageTag(2).toString() == "*");
            PEGASUS_TEST_ASSERT(al.getQualityValue(2) == 0.25);
            PEGASUS_TEST_ASSERT(al.getLanguageTag(3).toString() == "de");
            PEGASUS_TEST_ASSERT(al.getQualityValue(3) == 0.0);

            PEGASUS_TEST_ASSERT(LanguageParser::buildAcceptLanguageHeader(al) ==
                "en-US-ca,it;q=0.500,*;q=0.250,de;q=0.000");
        }

        // Test invalid quality value syntax:  Missing "q"
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader("en-US-ca;");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid quality value syntax:  Missing "="
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader("en-US-ca;q");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid quality value syntax:  Unexpected character at "q"
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader("en-US-ca;r=.9");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid quality value syntax:  Unexpected character at "="
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader(
                        "en-US-ca;q+0.1");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid quality value syntax:  Extra semicolon
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader(
                        "en-US-ca;;q=0.1");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid quality value syntax:  Negative quality value
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader(
                        "en-US-ca;q=-0.1");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid quality value syntax:  Quality value too large
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader(
                        "en-US-ca;q=1.1");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid quality value syntax:  Invalid trailing characters
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader(
                        "en-US-ca;q=0.1a");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid quality value syntax:  Quality value too long
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader(
                        "en-US-ca;q=0.1110");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid comment syntax:  Missing closing parenthesis
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader(
                        "en-US-ca(;q=0.1111");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test valid comment syntax
        {
            AcceptLanguageList al1 = LanguageParser::parseAcceptLanguageHeader(
                "en(english)-(\\(USA\\))US-\\c\\a;q(quality)=0.1(not much)");
            AcceptLanguageList al2 = LanguageParser::parseAcceptLanguageHeader(
                "en-US-ca;q=0.1");
            PEGASUS_TEST_ASSERT(al1 == al2);
        }

        // Test valid comment and whitespace syntax
        {
            AcceptLanguageList al1 = LanguageParser::parseAcceptLanguageHeader(
                "en (english)-(\\( USA \\))US-\\c \\a   ;q(quality) =0.1  "
                    "(not much) ");
            AcceptLanguageList al2 = LanguageParser::parseAcceptLanguageHeader(
                "en-US-ca;q=0.1");
            PEGASUS_TEST_ASSERT(al1 == al2);
        }

        // Test invalid whitespace syntax
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader(
                        "en-US-ca\\ ;q=0.1");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid language tag:  Trailing '-'
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader(
                        "en-US-ca-;q=0.1");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid Accept-Language value:  Empty string
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader("");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid Accept-Language value:  Comment and whitespace only
        {
            Boolean gotException = false;

            try
            {
                AcceptLanguageList al =
                    LanguageParser::parseAcceptLanguageHeader(
                        " (comment only)");
            }
            catch (InvalidAcceptLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }
    }
    catch (Exception& e)
    {
        cout << "Unexpected exception: " << e.getMessage() << endl;
        exit(1);
    }
}


void testContentLanguageList()
{
    try
    {
        ContentLanguageList cl = LanguageParser::parseContentLanguageHeader(
            "en-US-mn,fr-FR,en, fr(oh you french), la-SP-bal");

        for (Uint32 index = 0; index < cl.size(); index++)
        {
            LanguageTag lt = cl.getLanguageTag(index);

            if (index == 3)
            {
                PEGASUS_TEST_ASSERT(String::equal(lt.toString(), "fr"));
            }
        }

        PEGASUS_TEST_ASSERT(cl.size() == 5);

        // Add LanguageTag

        cl.append(LanguageTag("en-XX-xx"));
        PEGASUS_TEST_ASSERT(cl.size() == 6);
        PEGASUS_TEST_ASSERT(cl.find(LanguageTag("en-XX-xx")) != PEG_NOT_FOUND);

        // Remove LanguageTag

        Uint32 index = cl.find(LanguageTag("en-XX-xx"));
        cl.remove(index);
        PEGASUS_TEST_ASSERT(cl.find(LanguageTag("en-XX-xx")) == PEG_NOT_FOUND);
        PEGASUS_TEST_ASSERT(cl.size() == 5);

        // Test assignment operator and equality operator

        ContentLanguageList cl1;
        cl1 = cl;
        PEGASUS_TEST_ASSERT(cl1 == cl);

        cl1 = cl1;
        PEGASUS_TEST_ASSERT(cl1 == cl);

        cl1.remove(0);
        PEGASUS_TEST_ASSERT(cl1 != cl);

        // Test clear() method

        cl1.clear();
        PEGASUS_TEST_ASSERT(cl1.size() == 0);

        // Test invalid Content-Language value:  Invalid character
        {
            Boolean gotException = false;

            try
            {
                ContentLanguageList cl =
                    LanguageParser::parseContentLanguageHeader("en-4%5US-mn");
            }
            catch(InvalidContentLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid Content-Language value:  Empty string
        {
            Boolean gotException = false;

            try
            {
                ContentLanguageList cl =
                    LanguageParser::parseContentLanguageHeader("");
            }
            catch (InvalidContentLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid Content-Language value:  Comment and whitespace only
        {
            Boolean gotException = false;

            try
            {
                ContentLanguageList cl =
                    LanguageParser::parseContentLanguageHeader(
                        " (comment only)");
            }
            catch (InvalidContentLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }

        // Test invalid Content-Language value:  "*" language tag
        {
            Boolean gotException = false;

            try
            {
                ContentLanguageList cl =
                    LanguageParser::parseContentLanguageHeader("en, *, es");
            }
            catch (InvalidContentLanguageHeader&)
            {
                gotException = true;
            }

            PEGASUS_TEST_ASSERT(gotException);
        }
    }
    catch (Exception& e)
    {
        cout << "Unexpected exception: " << e.getMessage() << endl;
        exit(1);
    }
}

void testMessageLoader()
{
    MessageLoaderParms mlp(
        "CIMStatusCode.CIM_ERR_SUCCESS",
        "Default CIMStatusCode, $0 $1",
        "rab oof is foo bar backwards",
        64000);

    mlp.msg_src_path = "test/pegasusTest";
    mlp.acceptlanguages = LanguageParser::parseAcceptLanguageHeader("en-US");

#ifdef PEGASUS_HAS_ICU

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = 64,000");

    // test for return content languages

    PEGASUS_TEST_ASSERT(LanguageParser::buildContentLanguageHeader(
        mlp.contentlanguages) == "en-US");

#else

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) ==
        "Default CIMStatusCode, rab oof is foo bar backwards 64000");

#endif

    //
    // should load en-US resource because of single element fallback logic
    //

    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-US-mn"), 1.0);

#ifdef PEGASUS_HAS_ICU

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = 64,000");

#else

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) ==
        "Default CIMStatusCode, rab oof is foo bar backwards 64000");

#endif

    //
    // testing first element fallback after acceptlanguages has been exhausted
    //

    MessageLoaderParms mlp1("CIMStatusCode.CIM_ERR_SUCCESS",
                            "Default CIMStatusCode, $0 $1",
                            "rab oof is foo bar backwards","fr");

    mlp1.msg_src_path = "test/pegasusTest";
    mlp1.acceptlanguages.clear();
    mlp1.acceptlanguages.insert(LanguageTag("fr-FR"), 1.0);
    mlp1.acceptlanguages.insert(LanguageTag("bl-ow"), 1.0);

#ifdef PEGASUS_HAS_ICU

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL fr rab oof is foo bar backwards, "
            "number = fr");

#else

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) ==
        "Default CIMStatusCode, rab oof is foo bar backwards 64000");

#endif

    //
    // use gobal default message switch for messageloading
    //

    MessageLoader::_useDefaultMsg = true;

    mlp.acceptlanguages.clear();
    mlp.acceptlanguages.insert(LanguageTag("en-US"), 1.0);

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp) ==
        "Default CIMStatusCode, rab oof is foo bar backwards 64000");

    //
    // set static AcceptLanguageList in message loader
    //

    MessageLoader::_useDefaultMsg = false;
    MessageLoader::_acceptlanguages.insert(LanguageTag("st-at-ic"), 1.0);

    MessageLoaderParms mlp_static(
        "CIMStatusCode.CIM_ERR_SUCCESS","Default CIMStatusCode, $0",
        "rab oof is foo bar backwards static");
    mlp_static.msg_src_path = "test/pegasusTest";

#ifdef PEGASUS_HAS_ICU

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp_static) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL st_at_ic rab oof is foo bar backwards "
            "static");

#else

    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp_static) ==
        "Default CIMStatusCode, rab oof is foo bar backwards static");

#endif
}

//
// Tests the substitutions into the message
//
void testMessageLoaderSubs()
{
    MessageLoader::_acceptlanguages.clear();

    //
    // Test Uint64 support.  ICU does not support Uint64, so there
    // is special handling for it in MessageLoader.
    //

    //
    // Uint64 Substitution is the biggest positive to fit in int64_t.
    // This does not test the special code in MessageLoader.
    //
    MessageLoaderParms mlp1(
        "CIMStatusCode.CIM_ERR_SUCCESS",
        "Default CIMStatusCode, $0 $1",
        String("rab oof is foo bar backwards"),
        PEGASUS_UINT64_LITERAL(0x7fffffffffffffff));
    mlp1.msg_src_path = "test/pegasusTest";
    mlp1.acceptlanguages = LanguageParser::parseAcceptLanguageHeader("en-US");

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = 9,223,372,036,854,775,807");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards "
            "9223372036854775807");
#endif

    //
    // Uint64 substitution is too big to fit int64_t.
    // Tests the special MessageLoader
    // code for this.  Expect the number to be unformatted.
    //
    mlp1.arg1 = PEGASUS_UINT64_LITERAL(0x8000000000000000);

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = 9223372036854775808");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards "
            "9223372036854775808");
#endif

    //
    // Sint64 substitution - biggest negative.
    //
    mlp1.arg1 = PEGASUS_SINT64_LITERAL(0x8000000000000000);

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = -9,223,372,036,854,775,808");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards "
            "-9223372036854775808" );
#endif

    //
    // Uint32 substitution - biggest possible
    //
    mlp1.arg1 = (Uint32)(0xffffffff);

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = 4,294,967,295");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards 4294967295");
#endif

    //
    // Sint32 substitution - biggest negative
    //
    mlp1.arg1 = (Sint32)(0x80000000);

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = -2,147,483,648");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards -2147483648");
#endif

    //
    // Real64 substitution
    //
    mlp1.arg1 = (Real64)-64000.125;

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = -64,000.125");
#else
    // Commenting out due to platform differences
    // The main purpose of this tests is ICU substitution.
    // cout << MessageLoader::getMessage(mlp1) << endl;
    // PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
    //     "Default CIMStatusCode, rab oof is foo bar backwards -64000.125");
#endif

    //
    // Boolean substitution = true
    //
    mlp1.arg1 = true;

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = true");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards true");
#endif

    //
    // Boolean substitution = false
    //
    mlp1.arg1 = false;

#ifdef PEGASUS_HAS_ICU
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "CIM_ERR_SUCCESS: SUCCESSFUL en-us rab oof is foo bar backwards, "
            "number = false");
#else
    PEGASUS_TEST_ASSERT(MessageLoader::getMessage(mlp1) ==
        "Default CIMStatusCode, rab oof is foo bar backwards false");
#endif
}

int main(int argc, char *argv[])
{
#ifdef PEGASUS_HAS_ICU

    // If PEGASUS_MSG_HOME is set then use that as the message
    // home for this test.
    // This will ignore any msg home defined for the platform (Constants.h)
    // since this is a test environment, not a production environment.
    const char* env = getenv("PEGASUS_MSG_HOME");
    if (env != NULL)
    {
        MessageLoader::setPegasusMsgHome(env);
    }
    else
    {
        // PEGASUS_MSG_HOME is not set.  Since we need the test messages,
        // use PEGASUS_HOME as the message home.
        env = getenv("PEGASUS_HOME");
        if (env != NULL)
        {
            String msghome(env);
            msghome.append("/msg");
            MessageLoader::setPegasusMsgHome(msghome);
        }
        else
        {
            PEGASUS_STD(cout) << "Either PEGASUS_MSG_HOME or PEGASUS_HOME "
                "needs to be set for this test!" << PEGASUS_STD(endl);
            exit(-1);
        }
    }

    // If PEGASUS_USE_DEFAULT_MESSAGES env var is set then we need
    // to make sure that it doesn't break this test.
    // Reset _useDefaultMsg to make sure PEGASUS_USE_DEFAULT_MESSAGES
    // is ignored.
    MessageLoader::_useDefaultMsg = false;

#endif

    testLanguageParser();
    testLanguageTag();
    testAcceptLanguageList();
    testContentLanguageList();
    testMessageLoader();
    testMessageLoaderSubs();

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
