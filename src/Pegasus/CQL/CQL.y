%{
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CommonUTF.h>
#include <Pegasus/CQL/CQLFactory.h>
#include "memobjs.h"
#include <stdio.h>

#define yyparse CQL_parse
#define CQLPREDICATE 0
#define CQLVALUE 1
#define CQLIDENTIFIER 2
#define CQLFUNCTION 3
#define CQLCHAINEDIDENTIFIER 4

int yylex();
int yyerror(char * err){printf("%s\n", err); return 1;}
extern char * yytext;
int chain_state;
CQLFactory _factory;

PEGASUS_NAMESPACE_BEGIN
                                                                                
extern CQLParserState* globalParserState;
                                                                                
PEGASUS_NAMESPACE_END


%}
%union {
   char * strValue;
   String * _string;
   CQLValue * _value;
   CQLSelectStatement * _ss;
   CQLIdentifier * _identifier;
   CQLChainedIdentifier * _chainedIdentifier;
   CQLTerm * _term;
   CQLFactor * _factor;
   CQLPredicate * _predicate;
   CQLSimplePredicate * _simplePredicate;
   ExpressionOpType _opType;
   CQLExpression * _expression;
   void * _node;
}

/* terminals */
%token <strValue> IDENTIFIER 
%token <strValue> STRING_LITERAL
%token <strValue> BINARY
%token <strValue> NEGATIVE_BINARY
%token <strValue> HEXADECIMAL
%token <strValue> NEGATIVE_HEXADECIMAL
%token <strValue> INTEGER 
%token <strValue> NEGATIVE_INTEGER
%token <strValue> REAL 
%token <strValue> NEGATIVE_REAL
%token <strValue> TRUE 
%token <strValue> FALSE 
%token <strValue> SCOPED_PROPERTY
%token <strValue> LPAR 
%token <strValue> RPAR 
%token <strValue> HASH
%token <strValue> DOT 
%token <strValue> LBRKT 
%token <strValue> RBRKT 
%token <strValue> UNDERSCORE
%token <strValue> COMMA 
%token <strValue> CONCAT 
%token <strValue> DBL_PIPE
%token <strValue> PLUS
%token <strValue> MINUS 
%token <strValue> TIMES 
%token <strValue> DIV 
%token <strValue> IS 
%token <strValue> _NULL 
%token <_opType> EQ
%token <_opType> NE 
%token <_opType> GT 
%token <_opType> LT 
%token <_opType> GE 
%token <_opType> LE 
%token <_opType> _ISA 
%token <_opType> _LIKE 
%token <strValue> NOT AND OR 
%token <strValue> SCOPE 
%token <strValue> ANY EVERY IN SATISFIES 
%token <strValue> STAR 
%token <strValue> DOTDOT 
%token <strValue> SHARP DISTINCT 
%token <strValue> SELECT 
%token <strValue> FIRST 
%token <strValue> FROM 
%token <strValue> WHERE 
%token <strValue> ORDER 
%token <strValue> BY 
%token <strValue> ASC 
%token <strValue> DESC 
%token <strValue> AS 
%token <strValue> UNEXPECTED_CHAR


/* grammar - non terminals */
%type <_identifier> identifier
%type <strValue> class_name
%type <strValue> class_path
%type <_identifier> property_scope
%type <_identifier> scoped_property
%type <_string> literal_string
%type <_value> binary_value
%type <_value> hex_value
%type <_value> decimal_value
%type <_value> real_value
%type <_value> literal
%type <_string> array_index
%type <_string> array_index_list
%type <_node> chain
%type <_value> concat
%type <_factor> factor
%type <_term> term
%type <_expression> arith
%type <_value> value_symbol
%type <_expression> arith_or_value_symbol
%type <_opType> comp_op
%type <_predicate> comp
%type <strValue> optional_not
%type <strValue> expr_factor
%type <strValue> expr_term
%type <strValue> expr
%type <strValue> arg_list
%type <strValue> arg_list_sub
%type <strValue> from_specifier
%type <strValue> from_criteria
%type <strValue> star_expr
%type <strValue> selected_entry
%type <strValue> select_list
%type <strValue> select_list_tail
%type <strValue> search_condition
%type <strValue> optional_where
%type <strValue> select_statement

%start select_statement

%%
identifier : IDENTIFIER 
             { 
                 printf("BISON::identifier\n");
                 $$ = new CQLIdentifier(String(CQL_lval.strValue));
             }
;

class_name : identifier  
             {
                 String tmp = $1->getName().getString(); 
                 printf("BISON::class_name = %s\n", (const char *)tmp.getCString()); 
             }
;

class_path : class_name 
             { 
                 printf("BISON::class_path\n"); 
             }
;

property_scope : class_path SCOPE
             { 
                 printf("BISON::property_scope = %s\n",$1); 
                 /*$$ = new CQLIdentifier(String(CQL_lval.strValue));*/
             }
;

scoped_property : SCOPED_PROPERTY
                  {
			printf("BISON::scoped_property = %s\n",CQL_lval.strValue);
		        String tmp(CQL_lval.strValue);
		        $$ = new CQLIdentifier(tmp);
                  }
;   

literal_string : STRING_LITERAL 
             { 
                 printf("BISON::literal_string-> %s\n",CQL_lval.strValue); 
		 if(isUTF8Str(CQL_lval.strValue)){
		     $$ = new String(CQL_lval.strValue);
		 }else{
		     printf("BISON::literal_string-> BAD UTF\n");
		 }
             }
;

binary_value : BINARY 
	       { 
                   printf("BISON::binary_value-> %s\n",CQL_lval.strValue); 
                   $$ = new CQLValue(CQL_lval.strValue, Binary); 
               }
             | NEGATIVE_BINARY 
               { 
                   printf("BISON::binary_value-> %s\n",CQL_lval.strValue); 
                   $$ = new CQLValue(CQL_lval.strValue, Binary, false); 
               }
;

hex_value : HEXADECIMAL 
            { 
                printf("BISON::hex_value-> %s\n",CQL_lval.strValue); 
                $$ = new CQLValue(CQL_lval.strValue, Hex);
            }
          | NEGATIVE_HEXADECIMAL 
            { 
                printf("BISON::hex_value-> %s\n",CQL_lval.strValue); 
                $$ = new CQLValue(CQL_lval.strValue, Hex, false);
            }
;

decimal_value : INTEGER 
                { 
                    printf("BISON::decimal_value-> %s\n",CQL_lval.strValue); 
                    $$ = new CQLValue(CQL_lval.strValue, Decimal); 
                }
              | NEGATIVE_INTEGER 
                { 
                    printf("BISON::decimal_value-> %s\n",CQL_lval.strValue); 
                    $$ = new CQLValue(CQL_lval.strValue, Decimal, false);
                }
;

real_value : REAL 
             { 
                 printf("BISON::real_value-> %s\n",CQL_lval.strValue); 
                 $$ = new CQLValue(CQL_lval.strValue, Real);
             }
           | NEGATIVE_REAL 
             { 
                 printf("BISON::real_value-> %s\n",CQL_lval.strValue); 
                 $$ = new CQLValue(CQL_lval.strValue, Real, false);
             }
;

literal : literal_string 
          {
              printf("BISON::literal->literal_string\n");
              $$ = new CQLValue(*$1);
          }
        | decimal_value
          {
              printf("BISON::literal->decimal_value\n");
          }
        | binary_value
          {
              printf("BISON::literal->binary_value\n");
          }
        | hex_value
          {
              printf("BISON::literal->hex_value\n");
          }
        | real_value
          {
              printf("BISON::literal->real_value\n");
          }
        | TRUE
          {
              printf("BISON::literal->TRUE\n");
              $$ = new CQLValue(Boolean(true));
          }
        | FALSE
          {
              printf("BISON::literal->FALSE\n");
              $$ = new CQLValue(Boolean(false));
          }
;

array_index : expr
              {
                  printf("BISON::array_index->expr\n");
                  /* drill down expr and check its _value to make sure it is an unsigned int */
                  /* if yes, then grab the value and toString() and assign $$ to it */
                  /* else error */
              }
;

array_index_list : array_index
                   {
                       printf("BISON::array_index_list->array_index\n");
                   }
;

chain : literal
        {
            printf("BISON::chain->literal\n");
            chain_state = CQLVALUE;
        }
      | LPAR expr RPAR
        {
            printf("BISON::chain-> ( expr )\n");
            chain_state = CQLPREDICATE;
	    $$ = $2;
        }
      | identifier
        {
            printf("BISON::chain->identifier\n");
           chain_state = CQLIDENTIFIER;
        }
      | identifier HASH literal_string
        {
            printf("BISON::chain->identifier#literal_string\n");
            String tmp = $1->getName().getString();
            tmp.append("#").append(*$3);
            $$ = new CQLIdentifier(tmp);
	    chain_state = CQLIDENTIFIER;
        }
      | scoped_property
        {
	    printf("BISON::chain-> scoped_property\n");
            chain_state = CQLIDENTIFIER;
	    $$ = $1;
        }
      | identifier LPAR arg_list RPAR
        {
            printf("BISON::chain-> identifier( arg_list )\n");
            chain_state = CQLFUNCTION;
        }
      | chain DOT scoped_property
        {
	    printf("BISON::chain-> chain DOT scoped_property\n");
	    if(chain_state == CQLIDENTIFIER){
                $$ = new CQLChainedIdentifier(*(CQLIdentifier*)$1);
                ((CQLChainedIdentifier*)$$)->append(*$3);
            }else if(chain_state == CQLCHAINEDIDENTIFIER){
		    ((CQLChainedIdentifier*)$$)->append(*$3);
	    }else{
		/* error */
            }

            chain_state = CQLCHAINEDIDENTIFIER;
        }
      | chain DOT identifier
        {
            printf("BISON::chain->chain.identifier\n");
            if(chain_state == CQLIDENTIFIER){
		$$ = new CQLChainedIdentifier(*(CQLIdentifier*)$1);
		((CQLChainedIdentifier*)$$)->append(*$3);
            }else if(chain_state == CQLCHAINEDIDENTIFIER){
                ((CQLChainedIdentifier*)$$)->append(*(CQLIdentifier*)$3);
            }else{
                /* error */
            }
                                                                                                        
            chain_state = CQLCHAINEDIDENTIFIER;

        }
      | chain DOT identifier HASH literal_string
        {
            printf("BISON::chain->chain.identifier#literal_string\n");
            if(chain_state == CQLIDENTIFIER){
                $$ = new CQLChainedIdentifier(*(CQLIdentifier*)$1);
                String tmp($3->getName().getString());
		tmp.append("#").append(*$5);
                CQLIdentifier tmpid(tmp);
                ((CQLChainedIdentifier*)$$)->append(tmpid);
            }else if(chain_state == CQLCHAINEDIDENTIFIER){
		String tmp($3->getName().getString());
                tmp.append("#").append(*$5);
                CQLIdentifier tmpid(tmp);
                ((CQLChainedIdentifier*)$1)->append(tmpid);
		$$ = $1;
            }else{
                /* error */
            }
                                                                                                        
            chain_state = CQLCHAINEDIDENTIFIER;

        }
      | chain LBRKT array_index_list RBRKT
        {
            printf("BISON::chain->chain[ array_index_list ]\n");
            if(chain_state == CQLIDENTIFIER){
                CQLIdentifier tmpid = *(CQLIdentifier*)$1;
		String tmp = tmpid.getName().getString();
		tmp.append("[").append(*$3).append("]");
		$$  = new CQLIdentifier(tmp);
	    }else if(chain_state == CQLCHAINEDIDENTIFIER){
	        CQLChainedIdentifier tmpcid = *(CQLChainedIdentifier*)$1;
		CQLIdentifier tmpid = tmpcid.getLastIdentifier();
		String tmp = tmpid.getName().getString();
                tmp.append("[").append(*$3).append("]");
		tmpcid.append(*(new CQLIdentifier(tmp)));
	    }else{
		/* error */
	    }
        }
;

concat : chain
         {
             printf("BISON::concat->chain\n");
             $$ = new CQLValue(*(CQLChainedIdentifier*)$1);
         }
       | concat DBL_PIPE chain
         {
             printf("BISON::concat||chain\n");
	     if(chain_state == CQLVALUE){
		if(((CQLValue*)$3)->getValueType() == String_type){
	                if(((CQLValue*)$1)->getValueType() == String_type){
                        	$$ = new CQLValue((*(CQLValue*)$3) + (*(CQLValue*)$1));
				                   
        	        }

		}
             }
         }
;

factor : concat
         {
             printf("BISON::factor->concat\n");
             $$ = new CQLFactor(*(CQLValue*)$1);
         }         
       | PLUS concat
         {
             printf("BISON::factor->PLUS concat\n");
             $$ = new CQLFactor(*(CQLValue*)$1);
         }
       | MINUS concat
         {
             printf("BISON::factor->MINUS concat\n");
             CQLValue *tmp = (CQLValue*)$1;
             tmp->invert();
	     $$ = new CQLFactor(*tmp);
         }
;

term : factor
       {
           printf("BISON::term->factor\n");
           $$ = new CQLTerm(*$1);
       }
     | term STAR factor
       {
           printf("BISON::term->term STAR factor\n");
           $1->appendOperation(mult, *$3);
           $$ = $1;
       }
     | term DIV factor
       {
           printf("BISON::term->term DIV factor\n");
           $1->appendOperation(divide, *$3);
           $$ = $1;
       }
;

arith : term
        {
            printf("BISON::arith->term\n");
            $$ = new CQLExpression(*$1);
        }
      | arith PLUS term
        {
            printf("BISON::arith->arith PLUS term\n");
            $1->appendOperation(plus, *$3);
            $$ = $1;
        }
      | arith MINUS term
        {
            printf("BISON::arith->arith MINUS term\n");
	    $1->appendOperation(minus, *$3);
            $$ = $1;
        }
;

value_symbol : HASH literal_string
               {
                   printf("BISON::value_symbol->#literal_string\n");
		   String tmp("#");
		   tmp.append(*$2);
		   CQLIdentifier tmpid(tmp);
		   $$ = new CQLValue(tmpid);
               }
;

arith_or_value_symbol : arith
                        {
                            printf("BISON::arith_or_value_symbol->arith\n");
                        }
                      | value_symbol
                        {
                            printf("BISON::arith_or_value_symbol->value_symbol\n");
			    $$ = (CQLExpression*)(_factory.makeObject(new CQLFactor(*$1), Expression));
                        }
;

comp_op : EQ 
          {
              printf("BISON::comp_op->EQ\n");
          }
        | NE
          {
              printf("BISON::comp_op->NE\n");
          }
        | GT 
          {
              printf("BISON::comp_op->GT\n");
          }
        | LT
          {
              printf("BISON::comp_op->LT\n");
          }
        | GE
          {
              printf("BISON::comp_op->GE\n");
          }
        | LE
          {
              printf("BISON::comp_op->LE\n");
          }
;

comp : arith
       {
           printf("BISON::comp->arith\n");
	   CQLSimplePredicate tmp(*$1);
	   $$ = new CQLPredicate(tmp);
       }
     | arith IS NOT _NULL
       {
           printf("BISON::comp->arith IS NOT _NULL\n");
	   CQLSimplePredicate tmp(*$1, IS_NOT_NULL);
           $$ = new CQLPredicate(tmp);
       }
     | arith IS _NULL
       {
           printf("BISON::comp->arith IS _NULL\n");
	   CQLSimplePredicate tmp(*$1, IS_NULL);
           $$ = new CQLPredicate(tmp);
       }
     | arith comp_op arith_or_value_symbol
       {
           printf("BISON::comp->arith comp_op arith_or_value_symbol\n");
	   CQLSimplePredicate tmp(*$1, *$3, $2);
           $$ = new CQLPredicate(tmp);
       }
     | value_symbol comp_op arith
       {
           printf("BISON::comp->value_symbol comp_op arith\n");
           CQLExpression *tmpexpr = (CQLExpression*)(_factory.makeObject(new CQLFactor(*$1), Expression));
	   CQLSimplePredicate tmp(*tmpexpr, *$3, $2);
           $$ = new CQLPredicate(tmp);
       }
     | arith _ISA identifier
       {
           printf("BISON::comp->arith _ISA identifier\n");
	   CQLExpression *tmpexpr = (CQLExpression*)(_factory.makeObject(new CQLChainedIdentifier(*$3), Expression));
           CQLSimplePredicate tmp(*$1, *tmpexpr, ISA);
           $$ = new CQLPredicate(tmp);
       }
     | arith _LIKE literal_string
       {
           printf("BISON::comp->arith _LIKE literal_string\n");
           CQLExpression *tmpexpr = (CQLExpression*)(_factory.makeObject(new CQLIdentifier(*$3), Expression));
           CQLSimplePredicate tmp(*$1, *tmpexpr, LIKE);
           $$ = new CQLPredicate(tmp);
       }
;
/*
optional_not : {;} 
             | NOT
               {
                   printf("BISON::optional_not->NOT\n");
               }
;
*/
expr_factor : comp
              {
                  printf("BISON::expr_factor->comp\n");
              }
            | NOT comp
              {
                  printf("BISON::expr_factor->NOT comp\n");
              }
;

expr_term : expr_factor
            {
                printf("BISON::expr_term->expr_factor\n");
            }
          | expr_term AND expr_factor
            {
                printf("BISON::expr_term->expr_term AND expr_factor\n");
            }
;

expr : expr_term 
       {
           printf("BISON::expr->expr_term\n");
       }
     | expr OR expr_term
       {
           printf("BISON::expr->expr OR expr_term\n");
       }
;

arg_list : {;}
         | STAR
           {
               printf("BISON::arg_list->STAR\n");
           }
         | DISTINCT STAR
           {
               printf("BISON::arg_list->DISTINCT STAR\n");
           }
         | arg_list_sub arg_list_tail
           {
               printf("BISON::arg_list->arg_list_sub arg_list_tail\n");
           }
;

arg_list_sub : expr
               {
                   printf("BISON::arg_list_sub->expr\n");
               }   
             | DISTINCT expr
               {
                   printf("BISON::arg_list_sub->DISTINCT expr\n");
               }
;

arg_list_tail : {;}
              | COMMA arg_list_sub arg_list_tail
              {
                  printf("BISON::arg_list_tail->COMMA arg_list_sub arg_list_tail\n");
              }
;

from_specifier : class_path
                 {
                     printf("BISON::from_specifier->class_path\n");
                 } 
;

from_criteria : from_specifier
                {
                    printf("BISON::from_criteria->from_specifier\n");
                }
;

star_expr : STAR
            {
                printf("BISON::star_expr->STAR\n");
            }
;

selected_entry : expr 
                 {
                     printf("BISON::selected_entry->expr\n");
                 }
               | star_expr
                 {
                     printf("BISON::selected_entry->star_expr\n");
                 }
;

select_list : selected_entry select_list_tail
            {
                printf("BISON::select_list->selected_entry select_list_tail\n");
            }
;

select_list_tail : {;} /* empty */
                 | COMMA selected_entry select_list_tail
                   {
                       printf("BISON::select_list_tail->COMMA selected_entry select_list_tail\n");
                   }
;

search_condition : expr
                   {
                       printf("BISON::search_condition->expr\n");
                   }
;

optional_where : {;}
               | WHERE search_condition
                 {
                     printf("BISON::optional_where->WHERE search_condition\n");
                 }
;

select_statement : SELECT select_list FROM from_criteria optional_where 
                   {
                       printf("select_statement\n\n");
                   }
;

%%
