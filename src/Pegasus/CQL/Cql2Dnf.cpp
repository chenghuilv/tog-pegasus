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
// Author:      Markus Mueller (sedgewick_de@yahoo.de)
//
// Modified By: Adrian Schuur, schuur@de.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////


//#include "CMPI_Version.h"
#include "Cql2Dnf.h"
#include <Pegasus/Common/Stack.h>
//#include <Pegasus/WQL/WQLParser.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T term_el
# include <Pegasus/Common/ArrayImpl.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T eval_el
# include <Pegasus/Common/ArrayImpl.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T stack_el
# include <Pegasus/Common/ArrayImpl.h>
#undef PEGASUS_ARRAY_T

//
// Terminal element methods 
//
void term_el::negate()
{
    switch (_simplePredicate.getOperation())
    {
        case EQ: _simplePredicate.setOperation(NE); break;
        case NE: _simplePredicate.setOperation(EQ); break;
        case LT: _simplePredicate.setOperation(GE); break;
        case LE: _simplePredicate.setOperation(GT); break;
        case GT: _simplePredicate.setOperation(LE); break;
        case GE: _simplePredicate.setOperation(LT); break;
        case IS_NULL: 
        case IS_NOT_NULL: 
        case ISA:
        case LIKE:
        default: break;
    }
};
/*
String opnd2string(const WQLOperand &o) {
    switch (o.getType()) {
    case WQLOperand::PROPERTY_NAME:
       return o.getPropertyName();
    case WQLOperand::STRING_VALUE:
       return o.getStringValue();
    case WQLOperand::INTEGER_VALUE:
       return Formatter::format("$0",o.getIntegerValue());
    case WQLOperand::DOUBLE_VALUE:
       return Formatter::format("$0",o.getDoubleValue());
    case WQLOperand::BOOLEAN_VALUE:
       return Formatter::format("$0",o.getBooleanValue());
    default: ;
   }
   return "NULL_VALUE";
}

*/
/*
CMPIPredOp mapOperation(WQLOperation op) {
   static CMPIPredOp ops[]={(CMPIPredOp)0,(CMPIPredOp)0,(CMPIPredOp)0,
      CMPI_PredOp_Equals,
      CMPI_PredOp_NotEquals,
      CMPI_PredOp_LessThan,
      CMPI_PredOp_LessThanOrEquals,
      CMPI_PredOp_GreaterThan,
      CMPI_PredOp_GreaterThanOrEquals,
      (CMPIPredOp)0,(CMPIPredOp)0,(CMPIPredOp)0,(CMPIPredOp)0,(CMPIPredOp)0,(CMPIPredOp)0};
   return ops[(int)op];
}

CMPIType mapType(WQLOperand::Type typ) {
   switch (typ) {
    case WQLOperand::PROPERTY_NAME:
       return CMPI_nameString;
    case WQLOperand::STRING_VALUE:
       return CMPI_charString;
    case WQLOperand::INTEGER_VALUE:
       return CMPI_integerString;
    case WQLOperand::DOUBLE_VALUE:
       return CMPI_realString;
    case WQLOperand::BOOLEAN_VALUE:
       return CMPI_booleanString;
    case WQLOperand::NULL_VALUE:
       return CMPI_null;
  }
  return CMPI_null;
}

int term_el::toStrings(CMPIType &typ, CMPIPredOp &opr, String &o1, String &o2) const {
   opr=mapOperation(op);
   o1=opnd2string(opn1);
   o2=opnd2string(opn2);
   if (opn1.getType()==WQLOperand::PROPERTY_NAME) typ=mapType(opn2.getType());
   else typ=mapType(opn1.getType());
   return 0;
}
*/
//
// Evaluation heap element methods
//
stack_el eval_el::getFirst() 
{ 
   return stack_el(opn1, is_terminal1);
}

stack_el eval_el::getSecond()
{
   return stack_el(opn2, is_terminal2);
}

void eval_el::setFirst(const stack_el s)
{
     opn1 = s.opn;
     is_terminal1 = s.is_terminal;
}

void eval_el::setSecond(const stack_el s)
{
    opn2 = s.opn;
    is_terminal2 = s.is_terminal;
}

void eval_el::assign_unary_to_first(const eval_el & assignee)
{
    opn1 = assignee.opn1;
    is_terminal1 = assignee.is_terminal1;
}

void eval_el::assign_unary_to_second(const eval_el & assignee)
{
    opn2 = assignee.opn1;
    is_terminal2 = assignee.is_terminal1;
}

// Ordering operators, so that op1 > op2 for all non-terminals
// and terminals appear in the second operand first
void eval_el::order(void)
{
    int k;
    if ((!is_terminal1) && (!is_terminal2))
        if ((k = opn2) > opn1)
        {
            opn2 = opn1;
            opn1 =  k;
        }
    else if ((is_terminal1) && (!is_terminal2))
        if ((k = opn2) > opn1)
        {
            opn2 = opn1;
            opn1 =  k;
            is_terminal1 = false;
            is_terminal2 = true;
        }
}

//
// Helper function copied from WQLSelectStatement
// 
/*
template<class T>
inline static Boolean _Compare(const T& x, const T& y, WQLOperation op)
{
    switch (op)
    {
        case WQL_EQ: 
            return x == y;

        case WQL_NE: 
            return x != y;

        case WQL_LT: 
            return x < y;
        case WQL_LE: 
            return x <= y;

        case WQL_GT: 
            return x > y;

        case WQL_GE: 
            return x >= y;

        default:
            PEGASUS_ASSERT(0);
    }

    return false;
}

static bool operator==(const WQLOperand& x, const WQLOperand& y)
{
   if (x.getType()==y.getType()) switch (x.getType()) {
   case WQLOperand::PROPERTY_NAME:
      return x.getPropertyName()==y.getPropertyName();
   case WQLOperand::INTEGER_VALUE:
      return x.getIntegerValue()==y.getIntegerValue();
   case WQLOperand::DOUBLE_VALUE:
      return x.getDoubleValue()==y.getDoubleValue();
   case WQLOperand::BOOLEAN_VALUE:
      return x.getBooleanValue()==y.getBooleanValue();
   case WQLOperand::STRING_VALUE:
      return x.getStringValue()==y.getStringValue();
   case WQLOperand::NULL_VALUE: 
      return true;
   }
   return false;
}
*/
static bool operator==(const term_el& x, const term_el& y)
{
	return x._simplePredicate == y._simplePredicate; 
}
/*
static void addIfNotExists(TableauRow &tr, const term_el& el)
{
   for (int i=0,m=tr.size(); i<m; i++) {
      if (tr[i]==el) return;
   }
   tr.append(el);
}
*/
/*
static Boolean _Evaluate(
    const WQLOperand& lhs, 
    const WQLOperand& rhs, 
    WQLOperation op)
{
    switch (lhs.getType())
    {
        case WQLOperand::NULL_VALUE:
        {
            // This cannot happen since expressions of the form
            // OPERAND OPERATOR NULL are converted to unary form.
            // For example: "count IS NULL" is treated as a unary
            // operation in which IS_NULL is the unary operation
            // and count is the the unary operand.

            PEGASUS_ASSERT(0);
            break;
        }

        case WQLOperand::INTEGER_VALUE:
        {
            return _Compare(
                lhs.getIntegerValue(),
                rhs.getIntegerValue(),
                op);
        }

        case WQLOperand::DOUBLE_VALUE:
        {
            return _Compare(
                lhs.getDoubleValue(),
                rhs.getDoubleValue(),
                op);
        }

        case WQLOperand::BOOLEAN_VALUE:
        {
            return _Compare(
                lhs.getBooleanValue(),
                rhs.getBooleanValue(),
                op);
        }

        case WQLOperand::STRING_VALUE:
        {
            return _Compare(
                lhs.getStringValue(),
                rhs.getStringValue(),
                op);
        }

        default:
            PEGASUS_ASSERT(0);
    }

    return false;
}
*/

//
// CQL Compiler methods
//
    
/*
Cql2Dnf::Cql2Dnf(const String condition, const String pref) 
{
    WQLSelectStatement wqs;
    WQLParser::parse(pref+condition,wqs);
    eval_heap.reserveCapacity(16);
    terminal_heap.reserveCapacity(16);
    _tableau.clear();
    compile(&wqs);
}
*/
Cql2Dnf::Cql2Dnf() 
{
    eval_heap.reserveCapacity(16);
    terminal_heap.reserveCapacity(16);
    //_tableau.clear();
}

Cql2Dnf::Cql2Dnf(CQLSelectStatement & cqs)
{
    eval_heap.reserveCapacity(16);
    terminal_heap.reserveCapacity(16);
    compile(&cqs);
}

Cql2Dnf::Cql2Dnf(CQLSelectStatement * cqs)
{
    eval_heap.reserveCapacity(16);
    terminal_heap.reserveCapacity(16);
    compile(cqs);
}

Cql2Dnf::~Cql2Dnf() {}

void Cql2Dnf::compile(CQLSelectStatement * cqs)
{
    if (!cqs->hasWhereClause()) return;

    _strip_ops_operands(cqs);
    _buildEvalHeap();
    _pushNOTDown();
    _factoring();
    _construct(); // rebuild the statement
/*
    Array<stack_el> disj;
    _gatherDisj(disj);
    if (disj.size() == 0)
        if (terminal_heap.size() > 0)
           // point to the remaining terminal element
            disj.append(stack_el(0,true));

    for (Uint32 i=0, n =disj.size(); i< n; i++)
    {
        TableauRow tr;
        Array<stack_el> conj;

        if (!disj[i].is_terminal)
        {
           _gatherConj(conj, disj[i]);
            for( Uint32 j=0, m = conj.size(); j < m; j++)
	        addIfNotExists(tr,terminal_heap[conj[j].opn]);
//                tr.append(terminal_heap[conj[j].opn]);
        }
        else
	   addIfNotExists(tr,terminal_heap[disj[i].opn]);
//	   tr.append(terminal_heap[disj[i].opn]);
        _tableau.append(tr);
    }
*/
    eval_heap.clear();
       
    //print();
    //printTableau();
    //_sortTableau();
}
/*
Boolean CMPI_Wql2Dnf::evaluate(WQLPropertySource * source) const
{
   Boolean b = false;
   WQLOperand lhs, rhs;

   for(Uint32 i=0,n = _tableau.size(); i < n; i++)
   {
       TableauRow tr = _tableau[i];
       for(Uint32 j=0,m = tr.size(); j < m; j++)
       {
           lhs = tr[j].opn1;
           CMPI_Wql2Dnf::_ResolveProperty(lhs,source);
           rhs = tr[j].opn2;
           CMPI_Wql2Dnf::_ResolveProperty(rhs,source);

           if (rhs.getType() != lhs.getType())
               throw TypeMismatchException();

           if (!_Evaluate(lhs, rhs, tr[j].op))
           {
               b = false;
               break;
           }
           else
               b = true;
       }
       if (b) return true;
   }
   return false;
}
*/
/*
void Cql2Dnf::print(void)
{
for (Uint32 i=0, n=eval_heap.size();i < n;i++) {
    WQLOperation wop = eval_heap[i].op;
    if (wop == WQL_IS_TRUE) continue;
    cout << "Eval element " << i << ": ";
    if (eval_heap[i].is_terminal1) cout << "T(";
    else cout << "E(";
    cout << eval_heap[i].opn1 << ") ";
    cout << WQLOperationToString(eval_heap[i].op);
    if (eval_heap[i].is_terminal2) cout << " T(";
    else cout << " E(";
    cout << eval_heap[i].opn2 << ")" << endl;
}
for (Uint32 i=0, n=terminal_heap.size();i < n;i++) {
    cout << "Terminal expression " << i << ": ";
    cout << terminal_heap[i].opn1.toString() << " ";
    cout << WQLOperationToString(terminal_heap[i].op) << " "
         << terminal_heap[i].opn2.toString() << endl;
}
}
*/
/*
void CMPI_Wql2Dnf::printTableau(void)
{
   for(Uint32 i=0,n = _tableau.size(); i < n; i++)
   {
       cout << "Tableau " << i << endl;
       TableauRow tr = _tableau[i];
       for(Uint32 j=0,m = tr.size(); j < m; j++)
       {
           cout << tr[j].opn1.toString() << " ";
           cout << WQLOperationToString(tr[j].op) << " "
                << tr[j].opn2.toString() << endl;
       }

   }

}
*/
void Cql2Dnf::_buildEvalHeap()
{

    //WQLSelectStatement* that = (WQLSelectStatement*)wqs;

    Stack<stack_el> stack;

    // Operation conversion variable from OperationType enum to ExpressionOpType enum
    ExpressionOpType expOp;

    // Counter for Operands

    Uint32 j = 0;

    //cerr << "Build eval heap\n";

    for (Uint32 i = 0, n = _operations.size(); i < n; i++)
    {
        OperationType op = _operations[i];

        switch (op)
        {
            case CQL_OR:
            case CQL_AND:
            {
                PEGASUS_ASSERT(stack.size() >= 2);

                stack_el op1 = stack.top();
                stack.pop();

                stack_el op2 = stack.top();

                // generate Eval expression
                eval_heap.append(eval_el(0, op , op1.opn, op1.is_terminal,
                                 op2.opn , op2.is_terminal));

                stack.top() = stack_el(eval_heap.size()-1, false);

                break;
            }

            case CQL_NOT:
            {
                PEGASUS_ASSERT(stack.size() >= 1);

                stack_el op1 = stack.top();

                // generate Eval expression
                eval_heap.append(eval_el(0, op , op1.opn, op1.is_terminal,
                                 -1, true));

                stack.top() = stack_el(eval_heap.size()-1, false);

                break;
            }

            case CQL_EQ: expOp = EQ;
            case CQL_NE: expOp = NE;
            case CQL_LT: expOp = LT;
            case CQL_LE: expOp = LE;
            case CQL_GT: expOp = GT;
            case CQL_GE: expOp = GE;
            {
                PEGASUS_ASSERT(_operands.size() >= 2);

                CQLExpression lhs = _operands[j++];

                CQLExpression rhs = _operands[j++];

		CQLSimplePredicate sp(lhs,rhs,expOp);

                terminal_heap.push(term_el(false, sp));

                stack.push(stack_el(terminal_heap.size()-1, true));

                break;
            }
/*
            case WQL_IS_TRUE:
            case WQL_IS_NOT_FALSE:
            {
                PEGASUS_ASSERT(stack.size() >= 1);
                break;
            }
*/
            case CQL_IS_NULL:
            {
                PEGASUS_ASSERT(_operands.size() >= 1);
                CQLExpression expression = _operands[j++];
		CQLSimplePredicate dummy(expression,EQ);
                terminal_heap.push(term_el(false, dummy));

                stack.push(stack_el(terminal_heap.size()-1, true));

                break;
            }

            case CQL_IS_NOT_NULL:
            {
                PEGASUS_ASSERT(_operands.size() >= 1);
                CQLExpression expression = _operands[j++];
                CQLSimplePredicate dummy(expression,NE);
                terminal_heap.push(term_el(false, dummy));

                stack.push(stack_el(terminal_heap.size()-1, true));

                break;
            }
	    case CQL_NOOP:
	    default: break;
        }
    }

    PEGASUS_ASSERT(stack.size() == 1);
}

void Cql2Dnf::_pushNOTDown()
{
    for (int i=eval_heap.size()-1; i >= 0; i--)
    {
        Boolean _found = false;
        int k;

        // Order all operators, so that op1 > op2 for non-terminals
        // and terminals appear as second operand

        eval_heap[i].order();

        // First solve the unary NOT operator

        if (eval_heap[i].op == CQL_NOT)
       {
            // This serves as the equivalent of an empty operator
            eval_heap[i].op = CQL_NOOP;

            // Substitute this expression in all higher order eval statements
            // so that this node becomes disconnected from the tree

            for (int j=eval_heap.size()-1; j > i;j--)
            {
               // Test first operand
               if ((!eval_heap[j].is_terminal1) && (eval_heap[j].opn1 == i))

                   eval_heap[j].assign_unary_to_first(eval_heap[i]);

               // Test second operand
               if ((!eval_heap[j].is_terminal2) && (eval_heap[j].opn2 == i))

                   eval_heap[j].assign_unary_to_second(eval_heap[i]);
            }

            // Test: Double NOT created by moving down

            if (eval_heap[i].mark)
               eval_heap[i].mark = false;
            else
               _found = true;
            // else indicate a pending NOT to be pushed down further
        }

        // Simple NOT created by moving down

        if (eval_heap[i].mark)
        {
            // Remove the mark, indicate a pending NOT to be pushed down
            // further and switch operators (AND / OR)

            eval_heap[i].mark=false;
            if (eval_heap[i].op == CQL_OR) eval_heap[i].op = CQL_AND;
            else if (eval_heap[i].op == CQL_AND) eval_heap[i].op = CQL_OR;

            // NOT operator is already ruled out
            _found = true;
        }

        // Push a pending NOT further down
        if (_found)
        {
             // First operand

             int j = eval_heap[i].opn1;
             if (eval_heap[i].is_terminal1)
                 // Flip NOT mark
                 terminal_heap[j].negate();
             else
                 eval_heap[j].mark = !(eval_heap[j].mark);

             //Second operand (if it exists)

             if ((j = eval_heap[i].opn2) >= 0)
             {
                 if (eval_heap[i].is_terminal2)
                     // Flip NOT mark
                     terminal_heap[j].negate();
                 else
                     eval_heap[j].mark = !(eval_heap[j].mark);
             }
        }
    }
}

void Cql2Dnf::_factoring(void)
{
    int i = 0,n = eval_heap.size();
    //for (int i=eval_heap.size()-1; i >= 0; i--)
    while (i < n)
    {
        int _found = 0;
        int index = 0;

        // look for expressions (A | B) & C  ---> A & C | A & B
        if (eval_heap[i].op == CQL_AND)
        {
            if (!eval_heap[i].is_terminal1)
            {
                index = eval_heap[i].opn1; // remember the index
                if (eval_heap[index].op == CQL_OR) _found = 1;
            }

            if ((_found == 0) && (!eval_heap[i].is_terminal2))
            {
                index = eval_heap[i].opn2; // remember the index
                if (eval_heap[index].op == CQL_OR) _found = 2;
            }

            if (_found != 0)
            {
                 //int u1,u1_t,u2,u2_t,u3,u3_t;
                 stack_el s;

                 if (_found == 1)
                     s = eval_heap[i].getSecond();
                 else
                     s = eval_heap[i].getFirst();

                 // insert two new expression before entry i
                 eval_el evl;

                 evl = eval_el(false, CQL_OR, i+1, false, i, false);
                 if ((Uint32 )i < eval_heap.size()-1)
                     eval_heap.insert(i+1, evl);
                 else
                     eval_heap.append(evl);
                 eval_heap.insert(i+1, evl);

                 for (int j=eval_heap.size()-1; j > i + 2; j--)
                 {
                     //eval_heap[j] = eval_heap[j-2];

                     // adjust pointers

                     if ((!eval_heap[j].is_terminal1)&&
                         (eval_heap[j].opn1 >= i))
                         eval_heap[j].opn1 += 2;
                     if ((!eval_heap[j].is_terminal2)&&
                         (eval_heap[j].opn2 >= i))
                         eval_heap[j].opn2 += 2;
                 }

                 n+=2; // increase size of array

                 // generate the new expressions : new OR expression


                 // first new AND expression
                 eval_heap[i+1].mark = false;
                 eval_heap[i+1].op = CQL_AND;
                 eval_heap[i+1].setFirst(s);
                 eval_heap[i+1].setSecond( eval_heap[index].getFirst());
                 eval_heap[i+1].order();


                 // second new AND expression
                 eval_heap[i].mark = false;
                 eval_heap[i].op = CQL_AND;
                 eval_heap[i].setFirst(s);
                 eval_heap[i].setSecond( eval_heap[index].getSecond());
                 eval_heap[i].order();

                 // mark the indexed expression as inactive
                 //eval_heap[index].op = WQL_IS_TRUE; possible disconnects
                 i--;

            } /* endif _found > 0 */

        } /* endif found AND operator */

        i++; // increase pointer
    }
}
/*
void Cql2Dnf::_gatherDisj(Array<stack_el>& stk)
{
    _gather(stk, stack_el(0,true), true);
}

void Cql2Dnf::_gatherConj(Array<stack_el>& stk, stack_el sel)
{
    _gather(stk, sel, false);
}

void Cql2Dnf::_gather(Array<stack_el>& stk, stack_el sel, Boolean or_flag)
{
    Uint32 i = 0;

    stk.clear();
    stk.reserveCapacity(16);

    if ((i = eval_heap.size()) == 0) return;

    while (eval_heap[i-1].op == WQL_IS_TRUE)
    {
        eval_heap.remove(i-1);
        i--;
        if (i == 0) return;
    }
    //if (i == 0) return;

    if (or_flag)
        stk.append(stack_el(i-1,false));
    else
    {
       if (sel.is_terminal) return;
       stk.append(sel);
    }

    i = 0;

    while (i<stk.size())
    {
        int k = stk[i].opn;

        if ((k < 0) || (stk[i].is_terminal))
           i++;
        else
        {
            if ( ((eval_heap[k].op != WQL_OR) && (or_flag)) ||
                 ((eval_heap[k].op != WQL_AND) && (!or_flag))  )
                i++;
            else
            {
                // replace the element with disjunction
                stk[i] = eval_heap[k].getSecond();
                stk.insert(i, eval_heap[k].getFirst());
                if (or_flag)
                    eval_heap[k].op = WQL_IS_TRUE;
            }
        }
    }
}
*/
void Cql2Dnf::_strip_ops_operands(CQLSelectStatement *cqs)
{
	//
	// depth first search for all operations and operands
	// extract operations and operands and store in respective arrays for later processing
	//
	CQLPredicate topLevel = cqs->getPredicate();
	_destruct(topLevel);
}

OperationType Cql2Dnf::_convertOpType(ExpressionOpType op){
	switch(op){
		case EQ: return CQL_EQ;
		case NE: return CQL_NE;
		case GT: return CQL_GT;
		case LT: return CQL_LT;
		case GE: return CQL_GE;
		case LE: return CQL_LE;
		case IS_NULL: return CQL_IS_NULL;
		case IS_NOT_NULL: return CQL_IS_NOT_NULL;
		case ISA:
		case LIKE: 
		default: return CQL_NOOP;
	}
}

void Cql2Dnf::_destruct(CQLPredicate& _p){
	if(_p.isSimple()){
		CQLSimplePredicate _sp = _p.getSimplePredicate();
		_operations.append(_convertOpType(_sp.getOperation()));
		_operands.append(_sp.getLeftExpression());
		_operands.append(_sp.getRightExpression());
	}
	else{
		Array<CQLPredicate> _preds = _p.getPredicates();
		Array<BooleanOpType> _boolops = _p.getOperators();
		for(Uint32 i=0;i<_preds.size();i++){
			_destruct(_preds[i]);
			if(_preds[i].getInverted()){
				_operations.append(CQL_NOT);
			}
			if(i > 0){
				if(_boolops[i-1] == AND){
					_operations.append(CQL_AND);
				}
				if(_boolops[i-1] == OR){
                                        _operations.append(CQL_OR);
                                }
			}
		}
	}
}

void Cql2Dnf::_construct(){
	//
	// Each eval_el on the eval heap contains all the information needed to make a CQLPredicate.  
	// We will build a CQLPredicate for every element in the eval heap. So there is a 1 to 1 correspondence
	// between elements in the eval heap and elements in the CQLPredicate array used below.  
	// The first eval_el on the eval heap will always contain at least one terminal if the operation is a NOT
	// or two terminals if the operation is AND or OR.  We are guaranteed to build a CQLPredicate from the first
	// position in the eval_heap array. 
	//
	// The key to the algorithm is the isterminalX flag.  When set to true, we go to the 
	// term_heap and get the CQLSimplePredicate.  When set to false, we go to the _preds array below
	// and get the CQLPredicate.  Since there is a 1 - 1 correspondence, as explained above, the index
	// referred to by eval.opn1 or eval.opn2 is valid into the _preds array.
	//
	// For ANDs and ORs, we need two operands, as explained above, we get those operands
	// from either the term_heap or the _preds array.  For NOTs, we need only 1 operand, and that
	// comes from either the term_heap or the _preds array.
	//
	// When finished, the last element in the _preds array contains the top level CQLPredicate (the rebuilt tree)
	//
	// Example:  a=b^(!c=d v e=f)
	// If the current eval_heap looks like: 
	//	0,NOT,1,True,-1,True [index = 0]
	// 	0,OR,2,True,0,False  [index = 1]
	// 	0,AND,1,False,0,True [index = 2]
	//
	// And the current term_heap looks like:
	//	CQLSimplePredicate(a=b) [index = 0]
	// 	CQLSimplePredicate(c=d) [index = 1]
	//      CQLSimplePredicate(e=f) [index = 0]
	//
	// The _preds array at the end would look like:
	//	CQLPredicate(!c==d)        [index = 0]
	//	CQLPredicate(e==f v !c==d) [index = 1]
	// 	CQLPredicate((e==f v !c==d) ^ a==b) [index = 2]  (the rebuilt tree)
	//

	Array<CQLPredicate> _preds;
	for(Uint32 i=0;i<eval_heap.size();i++){
		eval_el eval = eval_heap[i];
		if(eval.is_terminal1 && eval.is_terminal2){
			switch(eval.op){
				case CQL_NOT:
                        	{
                                	_preds.append(CQLPredicate(terminal_heap[eval.opn1]._simplePredicate,true));
					break;
                                }
				case CQL_NOOP:
				{
					_preds.append(CQLPredicate(terminal_heap[eval.opn1]._simplePredicate,false));
					break;
				}
                        	case CQL_AND:
                        	{
					CQLPredicate p(terminal_heap[eval.opn1]._simplePredicate,false);
					p.appendPredicate(CQLPredicate(terminal_heap[eval.opn2]._simplePredicate,AND));
					_preds.append(p);
					break;
                        	}
                        	case CQL_OR:
                        	{
					CQLPredicate p(terminal_heap[eval.opn1]._simplePredicate,false);
                                        p.appendPredicate(CQLPredicate(terminal_heap[eval.opn2]._simplePredicate,OR));
                                        _preds.append(p);
					break;
                        	}
				case CQL_EQ:
                		case CQL_NE:
                		case CQL_GT:
                		case CQL_LT:
                		case CQL_GE: 
                		case CQL_LE:
                		case CQL_IS_NULL:
                		case CQL_IS_NOT_NULL: break;
				
			}
		}else if(eval.is_terminal1 && !eval.is_terminal2){
			switch(eval.op){
				case CQL_NOT:
                                {
                                        _preds.append(CQLPredicate(terminal_heap[eval.opn1]._simplePredicate,true));
					break;
                                }
				case CQL_NOOP:
                                {
                                        _preds.append(CQLPredicate(terminal_heap[eval.opn1]._simplePredicate,false));
					break;
                                }
                                case CQL_AND:
                                {
                                        CQLPredicate p(terminal_heap[eval.opn1]._simplePredicate,false);
                                        p.appendPredicate(_preds[eval.opn2],AND);
                                        _preds.append(p);
					break;
                                }
                                case CQL_OR:
                                {
                                        CQLPredicate p(terminal_heap[eval.opn1]._simplePredicate,false);
                                        p.appendPredicate(_preds[eval.opn2],OR);
                                        _preds.append(p);
					break;
                                }
				case CQL_EQ:
                                case CQL_NE:
                                case CQL_GT:
                                case CQL_LT:
                                case CQL_GE:
                                case CQL_LE:
                                case CQL_IS_NULL:
                                case CQL_IS_NOT_NULL: break;
                               
			}
		}else if(!eval.is_terminal1 && eval.is_terminal2){
                        switch(eval.op){
                                case CQL_NOT:
                                {
					CQLPredicate p = _preds[eval.opn1];
                                        p.setInverted();
                                        _preds.append(p);
					break;
                                }
				case CQL_NOOP:
                                {
                                        _preds.append(_preds[eval.opn1]);
					break;
                                }
                                case CQL_AND:
                                {
                                        CQLPredicate p(_preds[eval.opn1]);
                                        p.appendPredicate(CQLPredicate(terminal_heap[eval.opn2]._simplePredicate,AND));
                                        _preds.append(p);
					break;
                                }
                                case CQL_OR:
                                {
					CQLPredicate p(_preds[eval.opn1]);
                                        p.appendPredicate(CQLPredicate(terminal_heap[eval.opn2]._simplePredicate,OR));
                                        _preds.append(p);	
					break;
                                }
				case CQL_EQ:
                                case CQL_NE:
                                case CQL_GT:
                                case CQL_LT:
                                case CQL_GE:
                                case CQL_LE:
                                case CQL_IS_NULL:
                                case CQL_IS_NOT_NULL: break;
                               
                        }                                                                                                                                    
                }else{ // !eval.is_terminal1 && !eval.is_terminal2
			switch(eval.op){
                                case CQL_NOT:
                                {
					CQLPredicate p = _preds[eval.opn1];
					p.setInverted();
                                        _preds.append(p);
					break;
                                }
				case CQL_NOOP:
                                {
                                        _preds.append(_preds[eval.opn1]);
					break;
                                }
                                case CQL_AND:
                                {
                                        CQLPredicate p(_preds[eval.opn1]);
                                        p.appendPredicate(_preds[eval.opn2],AND);
                                        _preds.append(p);
					break;
                                }
                                case CQL_OR:
                                {
                                        CQLPredicate p(_preds[eval.opn1],false);
                                        p.appendPredicate(_preds[eval.opn2],OR);
                                        _preds.append(p);
					break;
                                }
				case CQL_EQ:
                                case CQL_NE:
                                case CQL_GT:
                                case CQL_LT:
                                case CQL_GE:
                                case CQL_LE:
                                case CQL_IS_NULL:
                                case CQL_IS_NOT_NULL: break;
                                
                        }

		}
	} // end for(...)

	_dnfPredicate = _preds[_preds.size()-1];
}

CQLPredicate Cql2Dnf::getDnfPredicate(){
	return _dnfPredicate;
}

PEGASUS_NAMESPACE_END
