%{
  /* Flex grammar created from CIM Specification Version 2.2 Appendix A */

  /*
     Note the following implementation details:

       1. The MOF specification has a production of type assocDeclaration,
       but an association is just a type of classDeclaration with a few
       special rules.  At least for the first pass, I'm treating an
       associationDeclaration as a classDeclaration and applying its
       syntactical rules outside of the grammar definition.

       2. Same with the indicationDeclaration.  It appears to be a normal
       classDeclaration with the INDICATION qualifier and no special
       syntactical rules.

       3. The Parser uses String objects throughout to represent
       character data.  However, the tokenizer underneath is probably
       working with 8-bit chars.  If we later use an extended character
       compatible tokenizer, I anticipate NO CHANGE to this parser.

       4. Besides the tokenizer, this parser uses 2 sets of outside
       services:
          1)Class valueFactory.  This has a couple of static methods
	  that assist in creating CIMValue objects from Strings.
	  2)Class cimmofParser.  This has a wide variety of methods
	  that fall into these catagories:
            a) Interfaces to the Repository.  You call cimmofParser::
            methods to query and store compiled CIM elements.
	    b) Error handling.
            c) Data format conversions.
            d) Tokenizer manipulation
            e) Pragma handling
            f) Alias Handling
  */

#define YYSTACKSIZE 2000

#include <cstdlib>
#ifndef PEGASUS_PLATFORM_ZOS_ZSERIES_IBM
#include <malloc.h>
#endif
#include <cstdio>
#include <cstring>
#include <Pegasus/Common/String.h>
#include "cimmofParser.h"
#include "valueFactory.h"
#include "memobjs.h"
#include "qualifierList.h"
#include "objname.h"

//extern cimmofParser g_cimmofParser;

extern int cimmof_lex(void);
extern int cimmof_error(...);
extern char *cimmof_text;
extern void cimmof_yy_less(int n);

/* ------------------------------------------------------------------- */
/* These globals provide continuity between various pieces of a        */
/* declaration.  They are usually interpreted as "these modifiers were */
/* encountered and need to be applied to the finished object".  For    */
/* example, qualifiers are accumulated in g_qualifierList[] as they    */
/* encountered, then applied to the production they qualify when it    */
/* is completed.                                                       */
/* ------------------------------------------------------------------- */
  Uint32 g_flavor = CIMFlavor::NONE;
  Uint32 g_scope = 0;
  //ATTN: BB 2001 BB P1 - Fixed size qualifier list max 10. Make larger or var
  qualifierList g_qualifierList(10);
  CIMMethod *g_currentMethod = 0;
  CIMClass *g_currentClass = 0;
  CIMInstance *g_currentInstance = 0;
  String g_currentAlias = String::EMPTY;
  String g_referenceClassName = String::EMPTY;
  KeyBindingArray g_KeyBindingArray; // it gets created empty
  TYPED_INITIALIZER_VALUE g_typedInitializerValue; 

/* ------------------------------------------------------------------- */
/* Pragmas, except for the Include pragma, are not handled yet    */
/* I don't understand them, so it may be a while before they are       */ 
/* ------------------------------------------------------------------- */
  struct pragma {
    String name;
    String value;
  };

/* ---------------------------------------------------------------- */
/* Use our general wrap manager to handle end-of-file               */
/* ---------------------------------------------------------------- */
extern "C" {
int
cimmof_wrap() {
  return cimmofParser::Instance()->wrapCurrentBuffer();
}
}

/* ---------------------------------------------------------------- */
/* Pass errors to our general log manager.                          */
/* ---------------------------------------------------------------- */
void
cimmof_error(const char *msg) {
  cimmofParser::Instance()->log_parse_error(cimmof_text, msg);
  // printf("Error: %s\n", msg);
}

  %}

%union {
  struct pragma *pragma;
  int              ival;
  //  char             *strval;
  String *         strval;
  CIMType::Tag        datatype;
  CIMValue *          value;
  String *         strptr;
  CIMQualifier *      qualifier;
  CIMProperty *       property;
  CIMMethod *         method;
  CIMClass *      mofclass;
  CIMQualifierDecl *   mofqualifier;
  CIMInstance *   instance;
  CIMReference *  reference;
  modelPath *     modelpath;
  KeyBinding *    keybinding;
  TYPED_INITIALIZER_VALUE * typedinitializer;
}

%token TOK_LEFTCURLYBRACE
%token TOK_RIGHTCURLYBRACE
%token TOK_RIGHTSQUAREBRACKET
%token TOK_LEFTSQUAREBRACKET
%token TOK_LEFTPAREN
%token TOK_RIGHTPAREN
%token TOK_COLON
%token TOK_SEMICOLON
%token TOK_POSITIVE_DECIMAL_VALUE
%token TOK_SIGNED_DECIMAL_VALUE
%token TOK_EQUAL
%token TOK_COMMA
%token TOK_CLASS
%token TOK_REAL_VALUE
%token TOK_CHAR_VALUE
%token TOK_STRING_VALUE
%token TOK_NULL_VALUE
%token TOK_OCTAL_VALUE
%token TOK_HEX_VALUE
%token TOK_BINARY_VALUE
%token TOK_TRUE
%token TOK_FALSE
%token TOK_DQUOTE
%token TOK_PERIOD
%token TOK_SIMPLE_IDENTIFIER
%token TOK_ALIAS_IDENTIFIER
%token TOK_PRAGMA
%token TOK_INCLUDE
%token TOK_AS
%token TOK_INSTANCE
%token TOK_OF
%token TOK_QUALIFIER
%token TOK_SCOPE
%token TOK_SCHEMA
%token TOK_ASSOCIATION
%token TOK_INDICATION
%token TOK_PROPERTY
%token TOK_REFERENCE
%token TOK_METHOD
%token TOK_PARAMETER
%token TOK_ANY
%token TOK_REF
%token TOK_FLAVOR
%token TOK_ENABLEOVERRIDE
%token TOK_DISABLEOVERRIDE
%token TOK_RESTRICTED
%token TOK_TOSUBCLASS
%token TOK_TRANSLATABLE
%token TOK_DT_STR
%token TOK_DT_BOOL
%token TOK_DT_DATETIME
%token TOK_DT_UINT8
%token TOK_DT_SINT8
%token TOK_DT_UINT16
%token TOK_DT_SINT16
%token TOK_DT_UINT32
%token TOK_DT_SINT32
%token TOK_DT_UINT64
%token TOK_DT_SINT64
%token TOK_DT_CHAR8
%token TOK_DT_CHAR16
%token TOK_DT_REAL32
%token TOK_DT_REAL64
%token TOK_UNEXPECTED_CHAR
%token TOK_END_OF_FILE

%type <strval> pragmaName pragmaVal qualifierName keyValuePairName
%type <strval> propertyName parameterName methodName className
%type <strval> fileName referencedObject referenceName referencePath 
%type <strval> TOK_POSITIVE_DECIMAL_VALUE TOK_OCTAL_VALUE TOK_HEX_VALUE 
%type <strval> TOK_SIGNED_DECIMAL_VALUE TOK_BINARY_VALUE
%type <strval> TOK_SIMPLE_IDENTIFIER TOK_STRING_VALUE
%type <strval> stringValue stringValues initializer constantValue
%type <strval> nonNullConstantValue
%type <strval> arrayInitializer constantValues 
%type <strval> integerValue TOK_REAL_VALUE TOK_CHAR_VALUE 
%type <strval> superClass TOK_ALIAS_IDENTIFIER  alias aliasIdentifier
%type <strval> namespaceHandle namespaceHandleRef
%type <strval> referenceInitializer aliasInitializer objectHandle
%type <strval> TOK_UNEXPECTED_CHAR

%type <typedinitializer> typedInitializer typedDefaultValue 
%type <typedinitializer> typedQualifierParameter

%type <modelpath> modelPath 
%type <keybinding> keyValuePair
%type <ival> flavor defaultFlavor array metaElements metaElement scope
%type <ival> booleanValue keyValuePairList
%type <pragma> compilerDirectivePragma
%type <datatype> dataType intDataType realDataType parameterType objectRef
%type <value> qualifierValue
%type <qualifier> qualifier
%type <property> propertyBody propertyDeclaration referenceDeclaration
%type <method> methodStart methodDeclaration
%type <mofclass> classHead classDeclaration
%type <mofqualifier> qualifierDeclaration
%type <instance> instanceHead instanceDeclaration 

%%
mofSpec: mofProductions ;

mofProductions: mofProduction mofProductions
              | /* empty */ ;
// ATTN: P1 KS Apr 2002 Limit in (none) Directive handling. See FIXME below.
mofProduction: compilerDirective { /* FIXME: Where do we put directives? */ }
             | qualifierDeclaration 
                 { cimmofParser::Instance()->addQualifier($1); delete $1; }
             | classDeclaration 
	         { cimmofParser::Instance()->addClass($1); }
             | instanceDeclaration 
                 { cimmofParser::Instance()->addInstance($1); } ;

classDeclaration: classHead  classBody
{
  if (g_currentAlias != String::EMPTY)
    cimmofParser::Instance()->addClassAlias(g_currentAlias, $$, false);
} ;

classHead: qualifierList TOK_CLASS className alias superClass
{
  $$ = cimmofParser::Instance()->newClassDecl(*$3, *$5);
  apply(&g_qualifierList, $$);
  g_currentAlias = *$4;
  if (g_currentClass)
    delete g_currentClass;
  g_currentClass = $$;
  delete $3;
  delete $4;
  delete $5;
} ;

className: TOK_SIMPLE_IDENTIFIER {  } ;

superClass: TOK_COLON className { $$ = $2; }
          | /* empty */ { $$ = new String(String::EMPTY); } ;

classBody: TOK_LEFTCURLYBRACE classFeatures TOK_RIGHTCURLYBRACE TOK_SEMICOLON
         | TOK_LEFTCURLYBRACE TOK_RIGHTCURLYBRACE TOK_SEMICOLON ;

classFeatures: classFeature
             | classFeatures classFeature ;

classFeature: propertyDeclaration  {
  cimmofParser::Instance()->applyProperty(*g_currentClass, *$1); delete $1; } 
            | methodDeclaration {
  cimmofParser::Instance()->applyMethod(*g_currentClass, *$1); }
            | referenceDeclaration {
  cimmofParser::Instance()->applyProperty(*g_currentClass, *$1); delete $1; }; 

methodDeclaration: qualifierList methodStart methodBody methodEnd 
{
  $$ = $2;
  apply(&g_qualifierList, $$);
} ;

methodStart: dataType methodName 
{
  if (g_currentMethod)
    delete g_currentMethod;
  g_currentMethod = 
                 cimmofParser::Instance()->newMethod(*$2, $1) ;
  $$ = g_currentMethod;
  delete $2;
} ;

methodBody: TOK_LEFTPAREN parameters TOK_RIGHTPAREN ;

methodEnd: TOK_SEMICOLON ;

propertyDeclaration: qualifierList propertyBody propertyEnd 
{
   $$ = $2;
  apply(&g_qualifierList, $$);
} ;

// KS 8 March 2002 - Extended to pass isArray and arraySize
propertyBody: dataType propertyName array typedDefaultValue
{
  CIMValue *v = valueFactory::createValue($1, $3, 
                      ($4->type == CIMMOF_NULL_VALUE), $4->value);
  if ($3 == -1) {
    $$ = cimmofParser::Instance()->newProperty(*$2, *v, false, 0);
} else {                                           
    $$ = cimmofParser::Instance()->newProperty(*$2, *v, true, $3);
  }

  delete $2;
  delete $4->value;
  delete v;
} ;

propertyEnd: TOK_SEMICOLON ;

referenceDeclaration: qualifierList referencedObject TOK_REF referenceName
                      referencePath TOK_SEMICOLON 
{
  String s(*$2);
  if (!String::equal(*$5, String::EMPTY))
    s += "." + *$5;
  CIMValue *v = valueFactory::createValue(CIMType::REFERENCE, -1, true, &s);
  //KS add the isArray and arraysize parameters. 8 mar 2002
  $$ = cimmofParser::Instance()->newProperty(*$4, *v, false,0, *$2);
  apply(&g_qualifierList, $$);
  delete $2;
  delete $4;
  delete $5; 
  delete v;
} ;

referencedObject: TOK_SIMPLE_IDENTIFIER { $$ = $1; } ;

referenceName: TOK_SIMPLE_IDENTIFIER { $$ = $1; };

referencePath: TOK_EQUAL stringValue { $$ = $2; }
               | /* empty */ { $$ = new String(String::EMPTY); } ;

methodName: TOK_SIMPLE_IDENTIFIER ;

parameters : parameter
           | parameters TOK_COMMA parameter
           | /* empty */ ;

parameter: qualifierList parameterType parameterName array 
{ // ATTN: P2 2002 Question Need to create default value including type?
  CIMParameter *p = 0;
  cimmofParser *cp = cimmofParser::Instance();
  if ($4 == -1) {
    p = cp->newParameter(*$3, $2, false, 0, g_referenceClassName);
  } else {
    p = cp->newParameter(*$3, $2, true, $4, g_referenceClassName);
  }
  g_referenceClassName = String::EMPTY;
  apply(&g_qualifierList, p);
  cp->applyParameter(*g_currentMethod, *p);
  delete p;
  delete $3;
} ;

parameterType: dataType { $$ = $1; }
             | objectRef { $$ = CIMType::REFERENCE; } ;

objectRef: className TOK_REF {  
                          g_referenceClassName = *$1; } ;

parameterName: TOK_SIMPLE_IDENTIFIER ;

propertyName: TOK_SIMPLE_IDENTIFIER ;

array: TOK_LEFTSQUAREBRACKET TOK_POSITIVE_DECIMAL_VALUE  
         TOK_RIGHTSQUAREBRACKET
                 { $$ = valueFactory::Stoi(*$2);
		   delete $2;
                 }
     | TOK_LEFTSQUAREBRACKET TOK_RIGHTSQUAREBRACKET { $$ = 0; } 
     | /* empty */ { $$ = -1; } ;

typedDefaultValue: TOK_EQUAL typedInitializer { $$ = $2; }  
            | {   /* empty */
                  g_typedInitializerValue.type = CIMMOF_NULL_VALUE;
                  g_typedInitializerValue.value = new String(String::EMPTY); 
                  $$ = &g_typedInitializerValue;
              };

initializer: constantValue { $$ = $1; }
           | arrayInitializer { $$ = $1; }
           | referenceInitializer { $$ = $1; } ;

// The typedInitializer element is syntactially identical to 
// the initializer element. However, the typedInitializer element 
// returns, in addition to the value, the type of the value.
typedInitializer: nonNullConstantValue 
           { 
           g_typedInitializerValue.type = CIMMOF_CONSTANT_VALUE;
           g_typedInitializerValue.value =  $1; 
           $$ = &g_typedInitializerValue;
           }
         | TOK_NULL_VALUE
           {
           g_typedInitializerValue.type = CIMMOF_NULL_VALUE;
           g_typedInitializerValue.value = new String(String::EMPTY); 
           $$ = &g_typedInitializerValue;
           }
         | arrayInitializer
           { 
           g_typedInitializerValue.type = CIMMOF_ARRAY_VALUE;
           g_typedInitializerValue.value =  $1; 
           $$ = &g_typedInitializerValue;
           }
         | referenceInitializer
           { 
           g_typedInitializerValue.type = CIMMOF_REFERENCE_VALUE;
           g_typedInitializerValue.value =  $1; 
           $$ = &g_typedInitializerValue;
           };

constantValues: constantValue { $$ = $1; }
              | constantValues TOK_COMMA constantValue 
                              {
                                *$$ += ","; 
                                *$$ += *$3;
				delete $3;
                              } ;

// The nonNullConstantValue has been added to allow NULL 
// to be distinguished from the EMPTY STRING.

constantValue: nonNullConstantValue {$$ = $1;}
             | TOK_NULL_VALUE { $$ = new String(String::EMPTY); } ;

nonNullConstantValue: integerValue { $$ = $1; }
             | TOK_REAL_VALUE { $$ = $1; }
             | TOK_CHAR_VALUE { $$ =  $1; }
             | stringValues { }
             | booleanValue { $$ = new String($1 ? "T" : "F"); };

integerValue: TOK_POSITIVE_DECIMAL_VALUE
            | TOK_SIGNED_DECIMAL_VALUE
            | TOK_OCTAL_VALUE {
                   $$ = new String(cimmofParser::Instance()->oct_to_dec(*$1));
                   delete $1; }
            | TOK_HEX_VALUE {
                   $$ = new String(cimmofParser::Instance()->hex_to_dec(*$1));
	           delete $1; }
            | TOK_BINARY_VALUE {
                 $$ = new String(cimmofParser::Instance()->binary_to_dec(*$1));
	           delete $1; };

booleanValue: TOK_FALSE { $$ = 0; }
            | TOK_TRUE  { $$ = 1; } ;

stringValues: stringValue { $$ = $1; }
            | stringValues stringValue 
              { 
                *$$ += *$2;  delete $2;
              } ;

stringValue: TOK_STRING_VALUE 
{ 
   String oldrep = *$1;
   String s(oldrep), s1(String::EMPTY);
   // Handle quoted quote
   int len = s.size();
   if (s[len] == '\n') {
     // error: new line inside a string constant unless it is quoted
     if (s[len - 2] == '\\') {
       if (len > 3)
	 s1 = s.subString(1, len-3);
     } else {
       cimmof_error("New line in string constant");
     }
     cimmofParser::Instance()->increment_lineno();
   } else { // Can only be a quotation mark
     if (s[len - 2] == '\\') {  // if it is quoted
       if (len > 3)
	 s1 = s.subString(1, len-3);
       s1 += '\"';
       cimmof_yy_less(len-1);
     } else { // This is the normal case:  real quotes on both end
       s1 = s.subString(1, len - 2) ;
     }
   }
   delete $1; $$ = new String(s1);
} ;

arrayInitializer: 
       TOK_LEFTCURLYBRACE constantValues TOK_RIGHTCURLYBRACE 
           { $$ = $2; } 
     | TOK_LEFTCURLYBRACE  TOK_RIGHTCURLYBRACE 
           { $$ = new String(String::EMPTY); };

referenceInitializer: objectHandle {}
                  | aliasInitializer {  } ;

objectHandle: TOK_DQUOTE namespaceHandleRef modelPath TOK_DQUOTE
{ 
  // The objectName string is decomposed for syntactical purposes 
  // and reassembled here for later parsing in creation of an objname instance 
  String *s = new String(*$2);
  if (!String::equal(*s, String::EMPTY) && $3)
    *s += ":";
  if ($3) {
    *s += $3->Stringrep();
  }
  $$ = s;
  delete $2;
  delete $3;
} ;

aliasInitializer : aliasIdentifier {
  // convert somehow from alias to a CIM object name
  delete $1;
} ;

namespaceHandleRef: namespaceHandle TOK_COLON
                    { }
                  | /* empty */ { $$ = new String(String::EMPTY); };

namespaceHandle: stringValue {};

modelPath: className TOK_PERIOD keyValuePairList {
             modelPath *m = new modelPath(*$1, g_KeyBindingArray);
             g_KeyBindingArray.clear(); 
             delete $1;} ;

keyValuePairList: keyValuePair { $$ = 0; }
                | keyValuePairList TOK_COMMA keyValuePair { $$ = 0; } ;

keyValuePair: keyValuePairName TOK_EQUAL initializer 
              {
		KeyBinding *kb = new KeyBinding(*$1, *$3,
                               modelPath::KeyBindingTypeOf(*$3));
		g_KeyBindingArray.append(*kb);
		delete kb;
		delete $1;
	        delete $3; } ;

keyValuePairName: TOK_SIMPLE_IDENTIFIER ;

alias: TOK_AS aliasIdentifier { $$ = $2; } 
     | /* empty */ { $$ = new String(String::EMPTY); } ;

aliasIdentifier: TOK_ALIAS_IDENTIFIER ;

instanceDeclaration: instanceHead instanceBody
{ 
  $$ = g_currentInstance; 
  if (g_currentAlias != String::EMPTY)
    cimmofParser::Instance()->addInstanceAlias(g_currentAlias, $1, true);
};

instanceHead: qualifierList TOK_INSTANCE TOK_OF className alias 
{
  if (g_currentInstance)
    delete g_currentInstance;
  g_currentAlias = *$5;
  g_currentInstance = cimmofParser::Instance()->newInstance(*$4);
  $$ = g_currentInstance;
  apply(&g_qualifierList, $$);
  delete $4;
  delete $5;
} ;

instanceBody: TOK_LEFTCURLYBRACE valueInitializers TOK_RIGHTCURLYBRACE
              TOK_SEMICOLON ;

valueInitializers: valueInitializer
                 | valueInitializers valueInitializer ;

// ATTN-DE-P1-20020427: Processing NULL Initializer values is incomplete.
// Currently only the arrayInitializer element has been modified to 
// return CIMMOF_NULL_VALUE
valueInitializer: qualifierList TOK_SIMPLE_IDENTIFIER TOK_EQUAL
                  typedInitializer TOK_SEMICOLON 
{
  cimmofParser *cp = cimmofParser::Instance();
  // ATTN: P1 InstanceUpdate function 2001 BB  Instance update needs work here and CIMOM 
  // a property.  It must be fixed in the Common code first.
  // What we have to do here is create a CIMProperty  and initialize it with
  // the value provided.  The name of the property is $2 and it belongs
  // to the class whose name is in g_currentInstance->getClassName().
  // The steps are
  //   2. Get  property declaration's value object
  CIMProperty *oldprop = cp->PropertyFromInstance(*g_currentInstance,
							*$2);
  CIMValue *oldv = cp->ValueFromProperty(*oldprop);

  //   3. create the new Value object of the same type

  // We want createValue to interpret a value as an array if is enclosed 
  // in {}s (e.g., { 2 } or {2, 3, 5}) or it is NULL and the property is 
  // defined as an array. createValue is responsible for the actual
  // validation.

  CIMValue *v = valueFactory::createValue(oldv->getType(),
                 (($4->type == CIMMOF_ARRAY_VALUE) |
                  (($4->type == CIMMOF_NULL_VALUE) & oldprop->isArray()))?0:-1,
                 ($4->type == CIMMOF_NULL_VALUE),
                 $4->value);

  //   4. create a clone property with the new value
  CIMProperty *newprop = cp->copyPropertyWithNewValue(*oldprop, *v);
  //   5. apply the qualifiers; 
  apply(&g_qualifierList, newprop);
  //   6. and apply the CIMProperty to g_currentInstance.
  cp->applyProperty(*g_currentInstance, *newprop);
  delete $2;
  delete $4->value;
  delete oldprop;
  delete oldv;
  delete v;
  delete newprop;
} ;

compilerDirective: compilerDirectiveInclude
{
    //printf("compilerDirectiveInclude "); 
}
                 | compilerDirectivePragma
{
    //printf("compilerDirectivePragma ");
} ;

compilerDirectiveInclude: TOK_PRAGMA TOK_INCLUDE TOK_LEFTPAREN fileName
                          TOK_RIGHTPAREN 
{
  cimmofParser::Instance()->enterInlineInclude(*$4); delete $4;
}               
;

fileName: stringValue { $$ = $1; } ;
 
compilerDirectivePragma: TOK_PRAGMA pragmaName
                   TOK_LEFTPAREN pragmaVal TOK_RIGHTPAREN 
                   { cimmofParser::Instance()->processPragma(*$2, *$4); 
		   delete $2;
		   delete $4;
		   };

qualifierDeclaration: TOK_QUALIFIER qualifierName qualifierValue scope
                       defaultFlavor TOK_SEMICOLON 
{
//    CIMQualifierDecl *qd = new CIMQualifierDecl($2, $3, $4, $5);
	$$ = cimmofParser::Instance()->newQualifierDecl(*$2, $3, $4, $5);
        delete $2;
	delete $3;  // CIMValue object created in qualifierValue production
} ;


qualifierValue: TOK_COLON dataType array typedDefaultValue
{
    $$ = valueFactory::createValue($2, $3, 
                $4->type == CIMMOF_NULL_VALUE, $4->value);
    delete $4->value;
} ;

scope: scope_begin metaElements TOK_RIGHTPAREN { $$ = $2; } ;

scope_begin: TOK_COMMA TOK_SCOPE TOK_LEFTPAREN { g_scope = CIMScope::NONE; } ;

metaElements: metaElement { $$ = $1; }
            | metaElements TOK_COMMA metaElement
	                  { $$ |= $3; } ;
// ATTN:  2001 P3 defer There is not CIMScope::SCHEMA. Spec Limit KS

metaElement: TOK_CLASS       { $$ = CIMScope::CLASS;        }
//           | TOK_SCHEMA      { $$ = CIMScope::SCHEMA;       }
           | TOK_SCHEMA        { $$ = CIMScope::CLASS; }
           | TOK_ASSOCIATION { $$ = CIMScope::ASSOCIATION;  }
           | TOK_INDICATION  { $$ = CIMScope::INDICATION;   }
//           | TOK_QUALIFIER   { $$ = CIMScope::QUALIFIER; }
           | TOK_PROPERTY    { $$ = CIMScope::PROPERTY;     }
           | TOK_REFERENCE   { $$ = CIMScope::REFERENCE;    }
           | TOK_METHOD      { $$ = CIMScope::METHOD;       }
           | TOK_PARAMETER   { $$ = CIMScope::PARAMETER;    }
           | TOK_ANY         { $$ = CIMScope::ANY;          } ;

// Correction KS 4 march 2002 - Set the default if empty
defaultFlavor: TOK_COMMA flavorHead explicitFlavors TOK_RIGHTPAREN
  { $$ = g_flavor; }
	   | /* empty */ { $$ = (CIMFlavor::NONE); } ;

// Correction KS 4 March 2002 - set the defaults (was zero)
// Set the flavors for the defaults required: via DEFAULTS

flavorHead: TOK_FLAVOR TOK_LEFTPAREN { g_flavor = (CIMFlavor::NONE); } ;

explicitFlavors: explicitFlavor

               | explicitFlavors TOK_COMMA explicitFlavor ;


// ATTN:KS-26/03/02 P2 This accumulates the flavor defintions.  However, it allows multiple instances
// of any keyword.  Note also that each entity simply sets a bit so that you may
// set disable and enable and we will not know which overrides the other.
// We need to create the function to insure that you cannot enable then disable or
// accept the latter and override the former.

// The compiler simply provides the flavors defined in the MOF and does not make any
// assumptions about defaults, etc.  That is a problem for resolution of the flavors.
explicitFlavor: TOK_ENABLEOVERRIDE  { g_flavor |=   CIMFlavor::ENABLEOVERRIDE; }
              | TOK_DISABLEOVERRIDE { g_flavor |=   CIMFlavor::DISABLEOVERRIDE; } 
              | TOK_RESTRICTED      { g_flavor |=   CIMFlavor::RESTRICTED; }
              | TOK_TOSUBCLASS      { g_flavor |=   CIMFlavor::TOSUBELEMENTS; }
              | TOK_TRANSLATABLE    { g_flavor |=   CIMFlavor::TRANSLATABLE; } ;

flavor: overrideFlavors { $$ = g_flavor; }
      | /* empty */ { $$ = CIMFlavor::NONE; };

overrideFlavors: explicitFlavor
               | overrideFlavors explicitFlavor ;

 
dataType: intDataType     { $$ = $1; }
        | realDataType    { $$ = $1; }
        | TOK_DT_STR      { $$ = CIMType::STRING;   }
        | TOK_DT_BOOL     { $$ = CIMType::BOOLEAN;  }
        | TOK_DT_DATETIME { $$ = CIMType::DATETIME; } ;

intDataType: TOK_DT_UINT8  { $$ = CIMType::UINT8;  }
           | TOK_DT_SINT8  { $$ = CIMType::SINT8;  }
           | TOK_DT_UINT16 { $$ = CIMType::UINT16; }
           | TOK_DT_SINT16 { $$ = CIMType::SINT16; }
           | TOK_DT_UINT32 { $$ = CIMType::UINT32; }
           | TOK_DT_SINT32 { $$ = CIMType::SINT32; }
           | TOK_DT_UINT64 { $$ = CIMType::UINT64; }
           | TOK_DT_SINT64 { $$ = CIMType::SINT64; }
           | TOK_DT_CHAR16 { $$ = CIMType::CHAR16; } ;

realDataType: TOK_DT_REAL32 { $$ =CIMType::REAL32; }
            | TOK_DT_REAL64 { $$ =CIMType::REAL64; };

qualifierList: qualifierListBegin qualifiers TOK_RIGHTSQUAREBRACKET 
             | /* empty */ { };

qualifierListBegin: TOK_LEFTSQUAREBRACKET { g_qualifierList.init(); } ;

qualifiers: qualifier { }
          | qualifiers TOK_COMMA qualifier { } ;

qualifier: qualifierName typedQualifierParameter flavor
{
  cimmofParser *p = cimmofParser::Instance();
  // The qualifier value can't be set until we know the contents of the
  // QualifierDeclaration.  That's what QualifierValue() does.
  CIMValue *v = p->QualifierValue(*$1, 
                  ($2->type == CIMMOF_NULL_VALUE), *$2->value); 
  $$ = p->newQualifier(*$1, *v, g_flavor);
  g_qualifierList.add($$);
  delete $1;
  delete $2->value;
  delete v;
 } ;

// KS 4 march change g_flavor to set defaults
qualifierName: TOK_SIMPLE_IDENTIFIER { g_flavor = 0; }
             | metaElement { 
                        $$ = new String(ScopeToString($1));
                        g_flavor = 0; } ;

typedQualifierParameter: TOK_LEFTPAREN nonNullConstantValue TOK_RIGHTPAREN 
                    {
                    g_typedInitializerValue.type = CIMMOF_CONSTANT_VALUE;
                    g_typedInitializerValue.value =  $2;
                    $$ = &g_typedInitializerValue;
                    }
                  | TOK_LEFTPAREN TOK_NULL_VALUE TOK_RIGHTPAREN
                    {
                    g_typedInitializerValue.type = CIMMOF_NULL_VALUE;
                    g_typedInitializerValue.value = new String(String::EMPTY);
                    $$ = &g_typedInitializerValue;
                    }
                  | arrayInitializer
                    {
                    g_typedInitializerValue.type = CIMMOF_ARRAY_VALUE;
                    g_typedInitializerValue.value =  $1;
                    $$ = &g_typedInitializerValue;
                    }
                  | {   /* empty */
                    g_typedInitializerValue.type = CIMMOF_NULL_VALUE;
                    g_typedInitializerValue.value = new String(String::EMPTY);
                    $$ = &g_typedInitializerValue;
                    };

pragmaName: TOK_SIMPLE_IDENTIFIER { $$ = $1; } ;

pragmaVal:  TOK_STRING_VALUE { $$ = $1; } ;

%%
