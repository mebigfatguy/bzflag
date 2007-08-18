%{
#include <memory>
#include <string>
#include "globals.h"
#include "Rule.h"
#include "RuleSet.h"
#include "Product.h"
#include "Operation.h"
#include "Expression.h"

void yyerror(RuleSet *ruleset, char* s);
int yylex();
void yyunput(int, char*);
%}
%parse-param {RuleSet *ruleset}%union _YYSTYPE {
  char *id;
  float fl;
  ProductVector* pv;
  Product* p;
  OperationVector* ov;
  StringVector* ids;
  Operation* o;
  Expression* e;
}
%start ruleset
%token DEFSIGN EXTRUDE EXPAND RANDOM SUBDIVIDEH SUBDIVIDEV MATERIAL
%token <fl> NUMBER
%token <id> NONTERM
%type <pv> products
%type <p> product
%type <ov> ops
%type <o> op
%type <e> expr
%type <ids> faces faceparam
%%
ruleset : /* empty */
  | ruleset NONTERM products ';' { 
    std::string name = std::string($2);
    ruleset->addRule(name,new Rule(name,$3));
  }
;
products : /* empty */ { $$ = new ProductVector(); }
  | products product { $$ = $1; $$->push_back($2); }
;
product : DEFSIGN NUMBER ':' ops { $$ = new Product($4,$2); }
  | DEFSIGN ops { $$ = new Product($2); }
;
faces : /* empty */ { $$ = new StringVector(); }
  | faces NONTERM  { std::string name = std::string($2); $$->push_back(name); }
  | faces '*' { std::string name = std::string(""); $$->push_back(name); }
;
faceparam : /* empty */ { $$ = NULL; }
  | '[' '@' NONTERM ']' { std::string name = '@'+std::string($3); $$ = new StringVector(); $$->push_back(name); }
  | '[' faces ']' { $$ = $2; }
;
ops : /* empty */ { $$ = new OperationVector(); }
  | ops op { $$ = $1; $$->push_back($2); }
;
op : EXTRUDE '(' expr ')' faceparam { $$ = new OperationExtrude($3,$5); }
  | EXPAND '(' expr ')' { $$ = new OperationExpand($3); }
  | SUBDIVIDEH '(' expr ')' faceparam { $$ = new OperationSubdivide($3,true,$5); }
  | SUBDIVIDEV '(' expr ')' faceparam { $$ = new OperationSubdivide($3,false,$5); }
  | MATERIAL '(' expr ')' { $$ = new OperationMaterial($3); }
  | NONTERM { std::string name = std::string($1); $$ = new OperationNonterminal(name,ruleset); }
;
expr : RANDOM '(' expr ',' expr ',' expr ')' { $$ = new ExpressionRandom($3,$5,$7); }
  | NUMBER { $$ = new ExpressionConst($1); }
  ;
%%