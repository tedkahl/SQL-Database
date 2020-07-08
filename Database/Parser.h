#pragma once
#include <string>
#include "MMap.h"
#include "STokenizer.h"
#include "Map.h"
#include "arrayfuncs.h"
#include <vector>
using namespace std;
class Parser
{
public:
	Parser();
	~Parser();
	const enum keys{ZERO,SELECT,STAR,FIELD,SELECTFIELD,
		COMMA,SELECTFIELDCOMMA,FROM,SYMBOL,SELECTNAME,
		SELECTNAMECOMMA,WHERE,DROP,DROPTABLE, DROPNAME,
		CONDITIONFIELD,OPERATOR,CONDITIONVALUE,LOGICALOPERATOR,MAKE,
		INSERT,INTO, INSERTNAME, OPENPAREN, INSERTDATA, 
		INSERTDATACOMMA,CLOSEPAREN,FIELDS, MAKEFIELD, MAKEFIELDCOMMA,
		TABLE,MAKENAME,MAKENAMECOMMA,VALUES,QUOTE }; //SELECT=1 STAR=2 FIELD=3...
	void make_table_1();
	MMap<string,string> get_parse_tree();
	void tokenize(char input[],vector<Token> &v);
	void set_token_codes();
	void set_string(string command);
private:
	string _command;
	STokenizer s;
	int _table[MAX_ROWS][MAX_COLUMNS];
	Map<string,int> tokencodes;
};

Parser::Parser() :_command()
{
	set_token_codes();
	make_table_1();
}

void Parser::set_string(string command)
{
	_command = command;
}
//
//HANDLE QUOTES
//
void Parser::make_table_1() //see state diagram
{
	init_table(_table); 
	mark_cell(_table, ZERO, SELECT, SELECT); //mark row(state) 0, col 1 with 1
	mark_cell(_table, ZERO, MAKE, MAKE);
	mark_cell(_table, ZERO, INSERT, INSERT);
	mark_cell(_table, ZERO, DROP, DROP);

	//DROP
	mark_cell(_table, DROP, TABLE, DROPTABLE);
	mark_cell(_table, DROPTABLE, SYMBOL, DROPNAME);

	//SELECT
	mark_cell(_table, SELECT, STAR, STAR);
	mark_cell(_table, SELECT, SYMBOL, SELECTFIELD);

	mark_cell(_table, SELECTFIELD, COMMA, SELECTFIELDCOMMA);
	mark_cell(_table, SELECTFIELDCOMMA, SYMBOL, SELECTFIELD);

	mark_cell(_table, STAR, FROM, FROM);
	mark_cell(_table, SELECTFIELD, FROM, FROM);

	mark_cell(_table, FROM, SYMBOL, SELECTNAME);
	mark_cell(_table, SELECTNAME, COMMA, SELECTNAMECOMMA);
	mark_cell(_table, SELECTNAMECOMMA, SYMBOL, SELECTNAME);

	mark_cell(_table, SELECTNAME, WHERE, WHERE);
	mark_cell(_table, WHERE, SYMBOL, CONDITIONFIELD); //a field is any symbol
	mark_cell(_table, CONDITIONFIELD, OPERATOR, OPERATOR);
	mark_cell(_table, OPERATOR, SYMBOL, CONDITIONVALUE); 
	mark_cell(_table, CONDITIONVALUE, LOGICALOPERATOR, LOGICALOPERATOR); //paretheses banned for now
	mark_cell(_table, LOGICALOPERATOR, SYMBOL, CONDITIONFIELD);

	//INSERT
	mark_cell(_table, INSERT, INTO, INTO);
	mark_cell(_table, INTO, SYMBOL, INSERTNAME);
	mark_cell(_table, INSERTNAME, VALUES, VALUES);
	mark_cell(_table, VALUES, SYMBOL, INSERTDATA);
	mark_cell(_table, VALUES, QUOTE, QUOTE);
	mark_cell(_table, INSERTDATA, COMMA, INSERTDATACOMMA);
	mark_cell(_table, INSERTDATACOMMA, SYMBOL, INSERTDATA);

	//MAKE
	mark_cell(_table, MAKE, TABLE, TABLE);
	mark_cell(_table, TABLE, SYMBOL, MAKENAME);
	mark_cell(_table, MAKENAME, FIELDS, FIELDS);
	mark_cell(_table, FIELDS, SYMBOL, MAKEFIELD);
	mark_cell(_table, MAKEFIELD, COMMA, MAKEFIELDCOMMA);
	mark_cell(_table, MAKEFIELDCOMMA, SYMBOL, MAKEFIELD);

	mark_success(_table, SELECTNAME);
	mark_success(_table, MAKEFIELD);
	mark_success(_table, CONDITIONVALUE);
	mark_success(_table, INSERTDATA);
	mark_success(_table, DROPNAME);
}

MMap<string, string> Parser::get_parse_tree()
{
	MMap<string, string> parsetree;
	vector<Token> tokens;
	tokenize((char*)_command.c_str(), tokens);

	int state = 0;
	string t;
	for (size_t i = 0; i < tokens.size(); i++)
	{
		t = tokens[i].token_str();
		if (tokencodes.contains(t)) 
		{
			state = _table[state][tokencodes[t]];
			if (state == -1) break;
		}
		else if(tokens[i].type_string()=="ALPHANUM") state = _table[state][SYMBOL];
		//maybe there is a better way to do this with switch, but this seems more convenient for now
		if (state == SELECT || state == MAKE || state == INSERT||state==DROP) 
			parsetree["command"] += t;
		else if (state == SELECTFIELD || state == STAR||state==MAKEFIELD) 
			parsetree["fields"] += t;
		else if (state == SELECTNAME|| state == INSERTNAME|| state == MAKENAME||state==DROPNAME) 
			parsetree["tablename"] += t;
		else if (state == INSERTDATA) 
			parsetree["data"] += t;
		else if (state == CONDITIONFIELD || state == OPERATOR || 
			state == CONDITIONVALUE || state == LOGICALOPERATOR) 
			parsetree["condition"] += t;

	}
	if (!is_success(_table, state))
	{
		parsetree["command"] += (string)"0";
		parsetree["command"][0] = (string)"invalid"; 
	}
//	parsetree.print();
	return parsetree;
}

void Parser::set_token_codes()//force lower case for now
{
	tokencodes["drop"] = DROP;
	tokencodes["select"] = SELECT;
	tokencodes["*"] = STAR;
	tokencodes["\""] = QUOTE;
	tokencodes["from"] = FROM;
	tokencodes[","] = COMMA;
	tokencodes["("] = OPENPAREN;
	tokencodes[")"] = CLOSEPAREN;
	tokencodes["fields"] = FIELDS;
	tokencodes["values"] = VALUES;
	tokencodes["make"] = MAKE;
	tokencodes["create"] = MAKE;
	tokencodes["insert"] = INSERT;
	tokencodes["table"] = TABLE;
	tokencodes["into"] = INTO;
	//for (size_t i = 0; i < fields.size(); i++)tokencodes[fields[i]] = FIELD;
	tokencodes["where"] = WHERE;
	tokencodes["="] = OPERATOR;
	tokencodes["=="] = OPERATOR;
	tokencodes[">="] = OPERATOR;
	tokencodes["<="] = OPERATOR;
	tokencodes[">"] = OPERATOR;
	tokencodes["<"] = OPERATOR;
	tokencodes["and"] = LOGICALOPERATOR;
	tokencodes["or"] = LOGICALOPERATOR;
}

void Parser::tokenize(char input[], vector<Token> &tokens)
{
	Token t;
	s.set_string(input);
	while (s.more()) {
		s >> t;
		if (t.token_str()[0] == 34) //if parser returns quoted phrase, trim quotes here.
		{
			string trimquotes = t.token_str().substr(1, t.token_str().length() - 2);
			t = Token(trimquotes, t.type());
		}
		if (t.type_string() != "SPACE") tokens.push_back(t);
	}
}

Parser::~Parser()
{
}