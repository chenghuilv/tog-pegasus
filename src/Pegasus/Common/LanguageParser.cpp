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
// Author: Humberto Rivero (hurivero@us.ibm.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Tracer.h>
#include <cstring>

//PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

const LanguageParser LanguageParser::EMPTY = LanguageParser();

void LanguageParser::parseHdr(Array<String> &values, String & hdr){
	// look for ',' which designates distict (Accept/Content)-Language fields
	// the form: [languagetag, languagetag, languagetag] so whitespace removal
	// may be necessary.
	// then store them in the array
	PEG_METHOD_ENTER(TRC_L10N, "LanguageParser::parseHdr");
	Uint32 i = 0;
	while( i != PEG_NOT_FOUND ){
		i = hdr.find(",");
		if( i != PEG_NOT_FOUND ){
			values.append(hdr.subString(0,i));
			while(hdr[i+1] == ' ') i++;  // get rid of whitespace after ","
			hdr = hdr.subString(i+1);
		}
		else{  // only one field, build an object with it
			values.append(hdr);
		}
	}
	PEG_METHOD_EXIT();
}

Real32 LanguageParser::parseAcceptLanguageValue(String &language_tag, String & hdr){
	// look for ';' in hdr, that means we have a quality value to capture
    // if not, we only have a language
    
    // if hdr begins with "x-" then we have a non-IANA (private) language tag
	 PEG_METHOD_ENTER(TRC_L10N, "LanguageParser::parseAcceptLanguageValue");
    Uint32 i;
    Boolean validate_length = true;
	if((( i = hdr.find("x-")) != PEG_NOT_FOUND ) && (i == 0)){
		hdr = convertPrivateLanguageTag(hdr);
		validate_length = false;	
	}
	
	// get rid of any beginning or trailing whitespaces
	Uint32 j;
	while( (j = hdr.find(" ")) != PEG_NOT_FOUND ){
		hdr.remove(j,1);
	}
	
    Real32 quality = 1;
	i = hdr.find(";");
	if(i != PEG_NOT_FOUND){ // extract and store language and quality
		if(isValid(hdr.subString(0,i), validate_length)){
			language_tag = hdr.subString(0,i);
			hdr.remove(0,i+3);  // remove everything but the quality value
		}
		else{
			throw InvalidAcceptLanguageHeader(
				"AcceptLanguage contains too many characters or non-alpha characters");
		}
		//validate quality 	
		quality = atof(hdr.getCString());
		if(quality > 1.0 || quality < 0.0){
			throw InvalidAcceptLanguageHeader(
				"AcceptLanguage contains an invalid quality value");
		}
	}
	else{	// extract and store language, quality defaults to 1.0
		if(isValid(hdr, validate_length)) language_tag = hdr;
		else throw InvalidAcceptLanguageHeader(
				"AcceptLanguage contains too many characters or non-alpha characters");
	}
	
	PEG_METHOD_EXIT();
	return quality;
}

String LanguageParser::parseContentLanguageValue(String & hdr){
	// we are looking for the language part of the hdr only,
	// according to the RFC, there may be parenthesized strings
	// that describe the purpose of the language, we need to ignore those
	PEG_METHOD_ENTER(TRC_L10N, "LanguageParser::parseContentLanguageValue");
	String value = hdr;
	Uint32 i,j;
	while((i = value.find("(")) != PEG_NOT_FOUND){ // get rid of anything in parenthesis in hdr if found
		if((j = value.find(")")) != PEG_NOT_FOUND)
			value.remove(i, (j-i)+1);
		else  throw InvalidContentLanguageHeader(
							"ContentLanguage does not contain terminating ) character");
	}
	// get rid of any beginning or trailing whitespaces
	while( (i = value.find(" ")) != PEG_NOT_FOUND ){
		value.remove(i,1);
	}
	if(!isValid(value)) throw InvalidContentLanguageHeader(
							"ContentLanguage contains too many characters or non-alpha characters");
	PEG_METHOD_EXIT();
	return value;
}

String LanguageParser::getLanguage(String & language_tag){
	// given a language_tag: en-US-mn we want to return "en"
	Uint32 i;
	if((i = language_tag.find(findSeparator(language_tag.getCString()))) != PEG_NOT_FOUND)
		return language_tag.subString(0,i);
	return String(language_tag);
} 

String LanguageParser::getCountry(String & language_tag){
	// given a language_tag: en-US-mn we want to return "US"
	Uint32 i,j;
	if( (i = language_tag.find(findSeparator(language_tag.getCString()))) != PEG_NOT_FOUND )
		if( (j = language_tag.find(i+1, findSeparator(language_tag.getCString()))) != PEG_NOT_FOUND )
			return language_tag.subString(i+1, j-(i+1));
		else 
			return language_tag.subString(i+1);
	return String::EMPTY;
}

String LanguageParser::getVariant(String & language_tag){
	// given a language_tag: en-US-mn we want to return "mn"
	Uint32 i;
	if( (i = language_tag.find(findSeparator(language_tag.getCString()))) != PEG_NOT_FOUND )
		if( (i = language_tag.find(i+1, findSeparator(language_tag.getCString()))) != PEG_NOT_FOUND )
			return language_tag.subString(i+1);
	return String::EMPTY;
}

void LanguageParser::parseLanguageSubtags(Array<String> &subtags, String language_tag){
    PEG_METHOD_ENTER(TRC_L10N, "LanguageParser::parseLanguageSubtags");
	Uint32 i;
	char separator = findSeparator(language_tag.getCString());
	while( (i = language_tag.find(Char16(separator))) != PEG_NOT_FOUND ){
			subtags.append(language_tag.subString(0,i));
			language_tag.remove(0,i + 1);		
	}
	if(language_tag.size() > 0)
		subtags.append(language_tag);
	PEG_METHOD_EXIT();
}

Boolean LanguageParser::isValid(String language_tag, Boolean validate_length){
	//break the String down into parts(subtags), then validate each part
	
	if(language_tag == "*") return true;
	
	Array<String> subtags;
	parseLanguageSubtags(subtags, language_tag);
	if(subtags.size() > 0){ 
		for(int i = 0; i < subtags.size(); i++){
			//length should be 8 or less AND all characters should be A-Z or a-z
			if((validate_length && subtags[i].size() > 8) || !checkAlpha(subtags[i].getCString()))
				return false;
		}
	}
	else{ //nothing back from parseLanguageSubtags
		return false;
	}
	return true;
}

String LanguageParser::convertPrivateLanguageTag(String language_tag){
	// figure out if its a unix style locale or windows locale
	int i;
	if(( i = language_tag.find("pegasus-")) != PEG_NOT_FOUND ){
		language_tag = language_tag.subString(i+5);  // capture the remainder of the string
		return String(replaceSeparator(language_tag.getCString(), '-'));
	}
	//else if( (i = language_tag.find("win-")) != PEG_NOT_FOUND ){
	  // return language_tag.subString(i+4);  // capture the remainder of the string
		// call windows ID to ICU convert routine or locmap.c function here
	//}
	else{
		return language_tag;
	}		
}

Boolean LanguageParser::checkAlpha(CString _str){
	for(int i = 0; i < strlen(_str); i++)
		if( !isalpha(_str[i]) )
			return false;
	return true;
}

char LanguageParser::findSeparator(CString _str){
	for(int i = 0; i < strlen(_str); i++)
		if(!isalnum(_str[i]))
			return _str[i];
	return '\0';
}

CString LanguageParser::replaceSeparator(CString _s, char new_sep){
	char * _str = const_cast<char *>((const char*)_s);
	for(int i = 0; i < strlen(_str); i++)
		_str[i] = (!isalnum(_str[i])) ? new_sep : _str[i];
	return (String(_str)).getCString();
}


PEGASUS_NAMESPACE_END
