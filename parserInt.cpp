//PA3
//Lazaro Ramos
//05-02-2021

#include "parserInt.h"
#include <sstream>

map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constance 
queue <Value> * ValQue; //declare a pointer variable to a queue of Value objects
queue <std::string> VariableQue;

LexItem assigned_variable;
Value assigned_value;
int temp_int;
float temp_float;

bool debug = false;
bool last_comma = false;
bool print_stmt = false;
bool if_stmt = true;
bool in_map = false;
bool is_integer = false;
bool printed = false;
bool negative_sign = false;

//Prog = PROGRAM IDENT {Decl} {Stmt} END PROGRAM IDENT
bool Prog(istream& in, int& line)
{
  if(debug){
    std::cout << "Enter Program" << std::endl;
  }

	bool dl = false, sl = false;
  std::string program_name;
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok.GetToken() == PROGRAM) {
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == IDENT) {
      program_name = tok.GetLexeme();
			dl = Decl(in, line);
			if( !dl  )
			{
				ParseError(line, "Incorrect Declaration in Program");
				return false;
			}
			sl = Stmt(in, line);
			if( !sl  )
			{
				ParseError(line, "Incorrect Statement in program");
				return false;
			}
			tok = Parser::GetNextToken(in, line);
			
			if (tok.GetToken() == END) {
				tok = Parser::GetNextToken(in, line);
				
				if (tok.GetToken() == PROGRAM) {
					tok = Parser::GetNextToken(in, line);
					
					if (tok.GetToken() == IDENT) {
            if(tok.GetLexeme() != program_name)
            {
              ParseError(line, "Incorrect Program Name");
              return false;
            }
						return true;
					}
					else
					{
						ParseError(line, "Missing Program Name");
						return false;
					}	
				}
				else
				{
					ParseError(line, "Missing PROGRAM at the End");
					return false;
				}	
			}
			else
			{
				ParseError(line, "Missing END of Program");
				return false;
			}	
		}
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	
	return false;
}

//Decl = (INTEGER | REAL | CHAR) : IdList
bool Decl(istream& in, int& line) {
  
  if(debug){
    std::cout << "Enter Decl" << std::endl;
  }
  
	bool status = false;
	LexItem tok;
	
	LexItem t = Parser::GetNextToken(in, line);
	
	if(t == INTEGER || t == REAL || t == CHAR) {
    if(t.GetToken() == INTEGER)
    {
      is_integer = true;
    }
		tok = t;
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == COLON) {
			status = IdList(in, line, t);
			//cout<< tok.GetLexeme() << " " << (status? 1: 0) << endl;
			if (status)
			{
				status = Decl(in, line);
				return status;
			}
		}
		else{
			ParseError(line, "Missing Colon");
			return false;
		}
	}

	Parser::PushBackToken(t);
	return true;
}

//Stmt = AssigStmt | IfStmt | PrintStmt
bool Stmt(istream& in, int& line) {
  
  if(debug){
    std::cout << "Enter Stmt" << std::endl;
  }
  
	bool status;
	
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {

	case PRINT:
		status = PrintStmt(in, line);
		
		if(status)
			status = Stmt(in, line);
		break;

	case IF:
		status = IfStmt(in, line);
		if(status)
			status = Stmt(in, line);
		break;

	case IDENT:
    assigned_variable = t;
		Parser::PushBackToken(t);
    status = AssignStmt(in, line);
		if(status)
      status = Stmt(in, line);
		break;

	case READ:
		status = ReadStmt(in, line);
		
		if(status)
			status = Stmt(in, line);
		break;

	default:
		Parser::PushBackToken(t);
		return true;
	}

	return status;
}

//PrintStmt = PRINT, ExprList 
bool PrintStmt(istream& in, int& line) 
{ 
  if(debug){
    std::cout << "Enter PrintStmt" << std::endl;
  }
  
  LexItem t;
  /*create an empty queue of Value objects.*/
  ValQue = new queue<Value>;
  if( (t=Parser::GetNextToken(in,line)) != COMA){
    ParseError(line,"Missing a Comma");
    return false;
  }
  print_stmt = true;
  bool ex=ExprList(in,line);
  print_stmt = false;
  
  if(!ex){
    ParseError(line,"Missing expression after print");
    while( !(*ValQue).empty() )
    {
      ValQue->pop();
    }
    delete ValQue;
    return false;
  }
   
  //Evaluate:print out the list of expressions' values
  while( !(*ValQue).empty() )
  {
    Value nextVal=(*ValQue).front();
    if(!last_comma & !(VariableQue.empty()))
    {
      cout << VariableQue.front() << nextVal << " ";
    }
    if(last_comma & !(VariableQue.empty()))
    {
      cout << VariableQue.front() << nextVal;
    }
    ValQue->pop();
    
    VariableQue.pop();
    if(ValQue->size() == 1)
    {
      last_comma = true;
    }
  }
  cout << endl;
  return ex;
}//PrintStmt

//IfStmt = IF (LogicExpr) THEN {Stmt} END IF
bool IfStmt(istream& in, int& line) {
  
  if(debug){
    std::cout << "Enter IfStmt" << std::endl;
  }
  
  Value val1;
	bool ex=false ; 
	LexItem t;
	
	if( (t=Parser::GetNextToken(in, line)) != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
	
	ex = LogicExpr(in, line, val1);
	if( !ex ) {
		ParseError(line, "Missing if statement Logic Expression");
		return false;
	}

	if((t=Parser::GetNextToken(in, line)) != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}
	
	if((t=Parser::GetNextToken(in, line)) != THEN ) {
		
		ParseError(line, "Missing THEN");
		return false;
	}
  bool st;
  if(if_stmt)
  {
    st = Stmt(in, line);
  }
  
  LexItem temp_token = Parser::GetNextToken(in,line);
  while(temp_token != END)
  {
    temp_token = Parser::GetNextToken(in,line);
  }
  
  if(temp_token == END)
  {
    Parser::PushBackToken(temp_token);
    st = true;
  }

	if( !st ) {
		ParseError(line, "Missing statement for IF");
		return false;
	}
	
	
	
	if((t=Parser::GetNextToken(in, line)) != END ) {
		
		ParseError(line, "Missing END of IF");
		return false;
	}
	if((t=Parser::GetNextToken(in, line)) != IF ) {
		
		ParseError(line, "Missing IF at End of IF statement");
		return false;
	}
	
	return true;
}

//UNUSED
bool ReadStmt(istream& in, int& line)
{
	
	LexItem t;
	
	if( (t=Parser::GetNextToken(in, line)) != COMA ) {
		
		ParseError(line, "Missing a Comma");
		return false;
	}
	
	bool ex = VarList(in, line);
	
	if( !ex ) {
		ParseError(line, "Missing Variable after Read Statement");
		return false;
	}
	return ex;
}

//IdList = IDENT {,IDENT}
bool IdList(istream& in, int& line, LexItem & tok) {
  
  if(debug){
    std::cout << "Enter IdList" << std::endl;
  }
  
	bool status = false;
	string identstr;
	
	tok = Parser::GetNextToken(in, line);
	if(tok == IDENT)
	{
		//set IDENT lexeme to the type tok value
		identstr = tok.GetLexeme();
		if (!(defVar.find(identstr)->second))
		{
			defVar[identstr] = true;
			SymTable[identstr] = tok.GetToken(); //changed from type to tok
		}	
		else
		{
			ParseError(line, "Variable Redefinition");
			return false;
		}
	}
	else
	{
		ParseError(line, "Missing Variable");
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	
	if (tok == COMA) {
		status = IdList(in, line, tok);
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

//UNUSED
bool VarList(istream& in, int& line)
{
	bool status = false;
	string identstr;
	LexItem tok = Parser::GetNextToken(in, line);
  
	status = Var(in, line, tok );
	
	if(!status)
	{
		ParseError(line, "Missing Variable");
		return false;
	}
	
	//LexItem tok was here, had to be moved to fix function
	
	if (tok == COMA) {
		status = VarList(in, line);
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

//Var = IDENT
bool Var(istream& in, int& line, LexItem & tok)
{
  if(debug){
    std::cout << "Enter Var" << std::endl;
  }
  
	//called only from the AssignStmt function
	string identstr;
	
	tok = Parser::GetNextToken(in, line);
	
	if (tok == IDENT){
		identstr = tok.GetLexeme();
		if (!(defVar.find(identstr)->second))
		{
			ParseError(line, "Undeclared Variable");
			return false;
		}
		return true;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return false;
}

//AssignStmt = Var = Expr
bool AssignStmt(istream& in, int& line) 
{
  if(debug){
    std::cout << "Enter AssignStmt" << std::endl;
  }
  
	bool varstatus = false, status = false;
	LexItem t;
	
	varstatus = Var( in, line, t); //added t
	Value val1, val2;
  
	
	if (varstatus){
		t = Parser::GetNextToken(in, line);
		
		if (t == ASSOP)
    {
			status = Expr(in, line, val1);
      val2 = val1;
			if(!status) 
      {
				ParseError(line, "Missing Expression in Assignment Statment");
				return status;
			}
		}
		else if(t.GetToken() == ERR)
    {
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		}
		else 
    {
			ParseError(line, "Missing Assignment Operator =");
			return false;
		}
	}
	else 
  {
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
		return false;
	}
  
  if(debug)
  {
    std::cout << "MAP: return val2: " << val2 << " variable: " 
    << assigned_variable.GetLexeme() << std::endl;
  }
  
  TempsResults[assigned_variable.GetLexeme()] = val2;
  
	return status;
}

//ExprList = Expr {,Expr}
bool ExprList(istream& in, int& line) {
  
  if(debug){
    std::cout << "Enter ExprList" << std::endl;
  }
  
	bool status = false;
  Value val1;

	status = Expr(in, line, val1);
	if(!status){
		ParseError(line, "Missing Expression");
		return false;
	}
  
  if(print_stmt && !in_map)
  {
    if(!negative_sign)
    {
      std::cout << val1;
    }
    if(negative_sign)
    {
      std::cout << "-" << val1;
    }
  }
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMA) {
		status = ExprList(in, line);
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

//Expr = Term {(+|-) Term}
bool Expr(istream& in, int& line, Value & retVal) {
  
  if(debug){
    std::cout << "Enter Expr" << std::endl;
  }
  
  Value val1, val2;
  val1 = retVal;
  if(debug){
    std::cout << "From Expr->Term" << std::endl;
  }
  bool t1 = Term(in, line, val1);
  retVal = val1;
  LexItem tok;
  if(debug){
    std::cout << "In Term retVal: " << retVal << std::endl;
  }
  if( !t1 )
  {
    return false;
  }
  
  tok = Parser::GetNextToken(in, line);
  if(tok.GetToken() == ERR){
    ParseError(line, "Unrecognized Input Pattern");
    cout << "(" << tok.GetLexeme() << ")" << endl;
    return false;
  }
  while ( tok == PLUS || tok == MINUS ) 
  {
    t1 = Term(in, line, val2);
    if( !t1 ) 
    {
      ParseError(line, "Missing operand after operator");
      return false;
    }
    //Evaluate the expression for addition or subtraction
    //and update the retVal object.
    //Check if the operation of PLUS/MINUS is legal for the
    //type of operands.
    if(retVal.GetType()==VCHAR || val2.GetType()==VCHAR)
    {
      ParseError(line,"Run-Time Error-Illegal Mixed Type Operands");
      return false;
    }
    else
    {
      if(tok == PLUS){
          retVal = retVal + val2;
      }
      else if(tok == MINUS){
          retVal = retVal - val2;
      }
    }

    tok=Parser::GetNextToken(in,line);
    if(tok.GetToken() == ERR)
    {
      ParseError(line,"Unrecognized Input Pattern");
      cout<< "(" << tok.GetLexeme() << ")" << endl;
      return false;
    }
  }
  Parser::PushBackToken(tok);
  return true;
}

//Term = SFactor {(*|/) SFactor}
bool Term(istream& in, int& line, Value & retVal) {
  
  if(debug){
    std::cout << "Enter Term" << std::endl;
  }
  
	Value val1, val2;
  val1 = retVal;
	bool t1 = SFactor(in, line, val1);
  retVal = val1;
	LexItem tok;
	
	if( !t1 ) {
		return false;
	}
	tok	= Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
	}
	while ( tok == MULT || tok == DIV  )
	{
		t1 = SFactor(in, line, val2); //need val2 to be r
		
		if( !t1 ) {
			ParseError(line, "Missing operand after operator");
			return false;
		}
		//added from SLIDES
    if(debug)
    {
      std::cout << "return retVal,val1,val2(term): " << retVal << " : " 
      << val1 << " : " << val2 << std::endl;
    }
    if(retVal.GetType()==VCHAR || val2.GetType()==VCHAR)
    {
      ParseError(line,"Run-Time Error-Illegal Mixed Type Operands");
      return false;
    }
    else
    {
      if(tok == MULT){
        retVal = retVal * val2;
      }
      else if(tok == DIV){
        if(val2.GetInt() == 0)
        {
          ParseError(line, "Run-Time Error-Illegal Division by Zero");
          return false;
        }
        retVal = retVal / val2;
      }
    }
		tok	= Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
	}
	Parser::PushBackToken(tok);
  
  if(debug)
  {
    std::cout << "return retVal(Term): " << retVal << " : " 
    << tokenPrint[tok.GetToken()] << " : " << tok.GetLexeme() << std::endl;
  }
	return true;
}

//SFactor = Sign Factor | Factor
bool SFactor(istream& in, int& line, Value & retVal)
{
  if(debug){
    std::cout << "Enter SFactor" << std::endl;
  }
  
  Value val1 = retVal;
	LexItem t = Parser::GetNextToken(in, line);
	bool status;
	int sign = 0;
	if(t == MINUS )
	{
		sign = -1;
	}
	else if(t == PLUS)
	{
		sign = 1;
	}
	else
		Parser::PushBackToken(t);
	
  if(debug){
    std::cout << "From SFactor->Factor value:" << val1 <<  std::endl;
  }
  
	status = Factor(in, line, sign, val1);
  retVal = val1;
  
  if(debug)
  {
    std::cout << "return retVal(SFactor): " << retVal << " : " 
    << tokenPrint[t.GetToken()] << " : " << t.GetLexeme() << std::endl;
  }
  
	return status;
}
//LogicExpr = Expr (== | <) Expr
bool LogicExpr(istream& in, int& line, Value & retVal)
{
  if(debug){
    std::cout << "Enter LogicExpr" << std::endl;
  }
  
	Value val1, val2;
  val1 = retVal;
	bool t1 = Expr(in, line, val1);
  retVal = val1;
	LexItem tok;
	
	if( !t1 ) {
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	if (tok == EQUAL) 
	{
		t1 = Expr(in, line, val2);
    
		if( !t1 ) 
		{
			ParseError(line, "Missing expression after relational operator");
			return false;
		}
    if(val2.GetInt() == retVal.GetInt())
    {
      return true;
    }
    else
      return false;
	}
  if (tok == LTHAN) 
	{
		t1 = Expr(in, line, val2);
		if( !t1 ) 
		{
			ParseError(line, "Missing expression after relational operator");
			return false;
		}
    if(retVal.GetType()==VCHAR || val2.GetType()==VCHAR)
    {
      ParseError(line,"Run-Time Error-Illegal Mixed Type Operands");
      return false;
    }
    if(val2.GetInt() > retVal.GetInt())
    {
      return true;
    }
    else if(val2.GetInt() == retVal.GetInt())
    {
      if_stmt = false;
      return true;
    }
    else
      return false;
	}
  
	Parser::PushBackToken(tok);
	return true;
}

//Factor = IDENT | ICONST | RCONST | SCONST | (Expr)
bool Factor(istream& in, int& line, int sign, Value & retVal) {
  if(debug){
    std::cout << "Enter Factor" << std::endl;
  }

	Value val1;
  val1 = retVal;
	LexItem tok;
  tok = Parser::GetNextToken(in, line);

  if(debug)
  {
    std::cout << "Factor Token: " << tokenPrint[tok.GetToken()] << " : "
    << tok.GetLexeme() << std::endl;
  }
  
	if( tok == IDENT ) {
		//check if the var is defined 
		std::string ident_lexeme = tok.GetLexeme();
    bool add_error = false;
    if(TempsResults.find(ident_lexeme) == TempsResults.end())
    {
      add_error = true;
    }
    
		if (!(defVar.find(ident_lexeme)->second) || add_error)
		{
			ParseError(line, "Undefined Variable");
			return false;	
		}
    
    retVal = TempsResults.find(ident_lexeme)->second;
    if(sign == -1)
    {
      negative_sign = true;
    }
    
		return true;
	}
	else if( tok == ICONST ) {
		temp_int = stoi(tok.GetLexeme());
    retVal = temp_int;
    assigned_value = temp_int;
    if(sign == -1)
    {
      negative_sign = true;
    }
    
    if(debug)
    {
      std::cout << "return retVal(Factor): " << retVal << " : " 
      << tokenPrint[tok.GetToken()] << " : " << tok.GetLexeme() << std::endl;
    }
    
		return true;
	}
	else if( tok == SCONST ) {
		std::string temp_lexeme = tok.GetLexeme();
    std::istringstream input_string(temp_lexeme);
    char ch;
    
    std::string loop_string;
    std::string equal_sign;
    while(input_string.get(ch) && (temp_lexeme.length() < 5))
    {
      if(isalpha(ch))
      {
        equal_sign = ch;
        equal_sign = equal_sign + "= ";
        ch = toupper(ch);
        loop_string = ch;
        
        
        input_string.get(ch);
        if(ch == '=')
        {
          for(auto const& it : TempsResults)
          {
            if(it.first == loop_string)
            {
              in_map = true;
              Value location = TempsResults.find(loop_string)->second;
              ValQue->push(location);
            }
          }
        }
      }
    }
    if(in_map)
    {
      VariableQue.push(equal_sign);
    }
    if(!in_map)
    {
      retVal = tok.GetLexeme();
    }
    if(!in_map && print_stmt)
    {
      retVal = tok.GetLexeme();
      //std::cout << tok.GetLexeme();
    }
		return true;
	}
	else if( tok == RCONST ) {
		temp_float = stof(tok.GetLexeme());
    if(is_integer)
    {
      int integer = temp_float;
      retVal = integer;
      retVal.Value::SetType(VINT);
      return true;
    }
    retVal = temp_float;
    assigned_value = temp_float;
    
    if(debug)
    {
      std::cout << "return retVal(Factor): " << retVal << " : " 
      << tokenPrint[tok.GetToken()] << " : " << tok.GetLexeme() << std::endl;
    }
    
		return true;
	}
	else if( tok == LPAREN ) {
		bool ex = Expr(in, line, val1);
    retVal = val1;
		if( !ex ) {
			ParseError(line, "Missing expression after (");
			return false;
		}
		if( Parser::GetNextToken(in, line) == RPAREN )
			return ex;

		ParseError(line, "Missing ) after expression");
		return false;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	ParseError(line, "Unrecognized input");
	return 0;
}
