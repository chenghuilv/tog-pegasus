#ifndef Pegasus_CQLChainedIdentifier_h
#define Pegasus_CQLChainedIdentifier_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/CQL/CQLIdentifier.h>
PEGASUS_NAMESPACE_BEGIN 
class PEGASUS_CQL_LINKAGE CQLScope;
class PEGASUS_CQL_LINKAGE CQLFactory;
class PEGASUS_CQL_LINKAGE CQLChainedIdentifierRep;
/**  
CQLChainedIdentifier class is used for parsing (not resolving)
the identifier portion of a CQL select statement.  
A couple examples of a CQLIdentifier:

  Class.EmbeddedObject.Property[3]
  Class.Property#'OK'


The CQLChainedIdentifier class can parse and return each
 section of a CQL identifier.  The possible sections are:

     class (optional)
     zero or more embedded objects
     property

Note:  this must handle wildcard("*"), embedded objects, arrays, symbolic
constants, etc.
   */
class PEGASUS_CQL_LINKAGE CQLChainedIdentifier
{
  public:

    CQLChainedIdentifier():_rep(0){}
    /**  constructor 
          Parses the string into the various components of a CQL identifier.
          Throws parsing errors.
      */
    CQLChainedIdentifier(String inString);

    CQLChainedIdentifier(CQLIdentifier & id);
 
    CQLChainedIdentifier(const CQLChainedIdentifier& cid);

    ~CQLChainedIdentifier();

    /**  The getSubIdentifiers method
    returns the array of subIdentifiers from a CQL SELECT
    statement.  If there are no subidentifiers, an empty array is returned.
    
    Example:
        myClass.embeddedObject.myProperty would return an array of 2 strings:
    'myClass' and 'embeddedObject'.
        myProperty would return an empty array.
    
      */
    const Array<CQLIdentifier>& getSubIdentifiers()const;

    String toString()const;

    void append(CQLIdentifier & id);

    Uint32 size();

    Boolean prepend(CQLIdentifier & id);

    CQLIdentifier& operator[](Uint32 index);

    Boolean isSubChain(CQLChainedIdentifier & chain);

    CQLIdentifier getLastIdentifier();
	
    void applyScopes(Array<CQLScope>& scopes);

    friend class CQLFactory;

  private:

    void parse(String & string);

    CQLChainedIdentifierRep* _rep;

};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CQLChainedIdentifier_h */
