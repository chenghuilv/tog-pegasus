#ifndef Pegasus_CQLFunction_h
#define Pegasus_CQLFunction_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/CQL/CQLValue.h>
#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/CQL/CQLScope.h>
//#include <Pegasus/CQL/CQLExpression.h>
#include <Pegasus/CQL/CQLPredicate.h>


PEGASUS_NAMESPACE_BEGIN

class PEGASUS_CQL_LINKAGE CQLFactory;
class PEGASUS_CQL_LINKAGE CQLFunctionRep;
//class PEGASUS_CQL_LINKAGE CQLPredicate;

 /** The Enum is private, the definition is public.
      */
    enum  FunctionOpType { CLASSNAMEEXP, 
			   CLASSNAME, 
			   CLASSPATH, 
			   COUNT, 
			   COUNTDISTINCT, 
			   COUNTDISTINCTEXPR, 
			   CREATEARRAY, 
			   DATETIME,
			   HOSTNAME,
			   MAX,
			   MEAN,
			   MEDIAN,
			   MIN,
			   MODELPATH,
			   NAMESPACENAME,
			   NAMESPACEPATH,
			   OBJECTPATH,
			   SCHEME,
			   SUM,
			   USERINFO,
			   UPPERCASE };
/**
   CQLFunction objects are populated by the
   Bison code.

   Supported functions are in accordance with the
   DMTF CQL Specification.
   TODO:  THIS LIST IS SUBJECT TO CHANGE
 
    classname( <expr> )
    classname( )
    count(*)
    count( distinct * )
    count( distinct <expr> )
    something for createarray
    something for datetime
    something for hostname
    max( <expr> )
    mean( <expr> )
    median( <expr> )
    min( <expr> )
    something for modelpath
    something for namespacename
    something for namespacepath
    something for objectpath
    something for scheme
    sum( <expr> )
    something for userinfo
    uppercase( <expr> )

  */


class PEGASUS_CQL_LINKAGE CQLFunction
{
  public:
   
    CQLFunction();
    
    CQLFunction(const CQLFunction& inFunc);

//    CQLFunction(FunctionOpType inFunctionOpType, Array<CQLExpression> inParms);
    
    CQLFunction(CQLIdentifier inOpType, Array<CQLPredicate> inParms);

    ~CQLFunction();
    /** 
       The getValue method validates the parms versus FunctionOpType.
               (A) resolves prarameter  types
               (B) number of parms
        and then actually executes the function.
        Returns a CQLValue object that has already been resolved.
      */
    CQLValue resolveValue(CIMInstance CI, QueryContext& queryCtx);
    
   Array<CQLPredicate> getParms();
   
   FunctionOpType getFunctionType();

   String toString();

   void applyContext(QueryContext& inContext);

   CQLFunction& operator=(const CQLFunction& rhs);
   
   Boolean operator==(const CQLFunction& func);
   
   Boolean operator!=(const CQLFunction& func);
   
   friend class CQLFactory;

  private:

  CQLFunctionRep *_rep;

};

PEGASUS_NAMESPACE_END

#endif 
