#include "pch.h"
#include "Parser.h"
#include "Map.h"
#include "StrTokenizer.h"
#include "MMap.h"
#include "arrayfuncs.h"
#include <vector>
#include <string>
using namespace std;

Parser::Parser()
{
}

void Parser::make_table_1()
{
	init_table(_table);
	mark_cell(_table,ZERO, SELECT, SELECT); //mark row(state) 0, col 1 with 1
	mark_cell(_table, SELECT, STAR, STAR);
	mark_cell(_table, STAR, FROM, FROM);
	mark_cell(_table, FROM, SYMBOL, SYMBOL + 1);
	mark_success(_table, SYMBOL + 1);
}

MMap<string,string> Parser::get_command()
{
	MMap<string, string> parsetree;
	vector<string> tokens;
	string input;
	cin >> input;
	tokenize(input, tokens);
	
	int state = 0;
	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (tokencodes.contains(tokens[i])) {
			state = _table[state][tokencodes[tokens[i]]];
			if (state == -1) break;
			switch (tokencodes[tokens[i]]) //possible bugs? Are all commands code 0, for example?
			{
			case 0: parsetree["command"] += tokens[i]; break;
			case 1: parsetree["fields"] += tokens[i]; break;
			}
		}

		else {
			parsetree["symbol"] += tokens[i];
			state = _table[state][SYMBOL];
		}
	}
	if(!is_success(_table, state)) parsetree["command"][0] = (string)"invalid"; //is this right?
	return parsetree;
}

void Parser::make_map()//force lower case for now
{
	tokencodes["select"] = SELECT;
	tokencodes["*"] = STAR;
	tokencodes["from"] = FROM;
}

void Parser::tokenize(string input, vector<string> &tokens)
{
	Token t;
	s >> t;
	tokens.push_back(t.token_str());
	while (s.more()) {
		s >> t;
		tokens.push_back(t.token_str());
	}
}

Parser::~Parser()
{
}
