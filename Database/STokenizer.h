#pragma once
#include "pch.h"
#include <string>
#include <iostream>
#include <assert.h>
#include "TableFuncs.h"
using namespace std;
const int MAX_BUFFER = 500;
//const int MAX_BUFFER = 20; //-for testing

class Token
{
public:
	Token();
	Token(string str, int type);
	Token(const Token& other);
	friend ostream& operator <<(ostream& outs, const Token& t) {
		cout << t._token;
		return outs;
	}

	Token& operator=(const Token& rhs);


	int type();
	string type_string();
	string token_str();
private:
	string _token;
	int _type;
};

class STokenizer
{
public:
	STokenizer();
	STokenizer(char str[]);
	STokenizer(const STokenizer& other);

	bool done();            //true: there are no more tokens
	bool more();            //true: there are more tokens
	//-- big three --

	//---------------
	//extract one token (very similar to the way cin >> works)
	friend STokenizer& operator >> (STokenizer& s, Token& t);

	//set a new string as the input string
	void set_string(const char str[]);
	bool eOf() { return endflag; }
	int get_pos() { return _pos; }
private:
	//create table for all the tokens we will recognize
	//                      (e.g. doubles, words, etc.)
	void make_table(int _table[][MAX_COLUMNS]);
	int make_alpha_table(int table[][MAX_COLUMNS], int starting_row);
	int make_punct_table(int table[][MAX_COLUMNS], int starting_row);
	int make_space_table(int table[][MAX_COLUMNS], int starting_row);

	//extract the longest string that match
	//     one of the acceptable token types
	bool get_token(int start_state, string& token);

	//---------------------------------
	char _buffer[MAX_BUFFER];       //input string
	unsigned int _pos;                       //current position in the string
	int _table[MAX_ROWS][MAX_COLUMNS];
	int alpha_row;
	int punct_row;
	int space_row;
	bool endflag = false;
};
Token::Token() {}

Token::Token(string str, int type) {
	_token = str;
	_type = type;
}
Token::Token(const Token& other) {
	_token = other._token;
	_type = other._type;
}

Token& Token::operator=(const Token& rhs) {
	_token = rhs._token;
	_type = rhs._type;
	return *this;
}

int Token::type() {
	return _type;
}

string Token::type_string() {
	switch (_type)
	{
	case 1: return "ALPHANUM";
	case 2: return "PUNCTUATION";
	case 3: return "SPACE";
	case 4: return "UNKNOWN";
	}
	return NULL;
}

string Token::token_str() {
	return _token;
}


STokenizer::STokenizer() {
	make_table(_table);
	_pos = 0;
}


STokenizer::STokenizer(char str[]) {
	make_table(_table);
	_pos = 0;
	set_string(str);
}

STokenizer::STokenizer(const STokenizer& other) {
	copy_table(other._table, _table);
	int alpha_row = other.alpha_row;
	int punct_row = other.punct_row;
	int space_row = other.space_row;

	set_string(other._buffer);
	_pos = other._pos;
}
//true: there are no more tokens
bool STokenizer::done() {
	return (_pos >= strlen(_buffer));
}

//true: there are more tokens
bool STokenizer::more() {
	return (_pos < strlen(_buffer));
}

void STokenizer::make_table(int _table[][MAX_COLUMNS]) {
	init_table(_table);
	int row = 0;
	row = make_alpha_table(_table, row);
	row = make_punct_table(_table, row);
	make_space_table(_table, row);
#ifdef DEBUG
	print_table(_table, 'a', 'z');
#endif
}

int STokenizer::make_alpha_table(int table[][MAX_COLUMNS], int starting_row) {
	alpha_row = starting_row;
	mark_fail(table, starting_row);
	mark_success(table, starting_row + 1);
	mark_success(table, starting_row + 4);
	
	mark_cells(starting_row, table, 'a', 'z', starting_row + 1);
	mark_cells(starting_row, table, 'A', 'Z', starting_row + 1);
	mark_cells(starting_row, table, '0', '9', starting_row + 1);    //state [0] --- DIGITS ---> [1]
	mark_cells(starting_row + 1, table, 'a', 'z', starting_row + 1);
	mark_cells(starting_row + 1, table, 'A', 'Z', starting_row + 1);
	mark_cells(starting_row + 1, table, '0', '9', starting_row + 1);    //state [0] --- DIGITS ---> [1]

	mark_cell(table, starting_row, 34, starting_row + 2); 
	mark_cells(starting_row + 2, table, 'a', 'z', starting_row + 3);
	mark_cells(starting_row + 2, table, 'A', 'Z', starting_row + 3);
	mark_cells(starting_row + 2, table, '0', '9', starting_row + 3);    //state [0] --- DIGITS ---> [1]
	mark_cell(table, starting_row+2, ' ', starting_row + 3); 
	mark_cells(starting_row + 3, table, 'a', 'z', starting_row + 3);
	mark_cells(starting_row + 3, table, 'A', 'Z', starting_row + 3);
	mark_cells(starting_row + 3, table, '0', '9', starting_row + 3);    //state [0] --- DIGITS ---> [1]
	mark_cell(table, starting_row + 3, ' ', starting_row + 3);

	mark_cell(table, starting_row + 3, 34, starting_row + 4);
	
	return starting_row + 5;
}

int STokenizer::make_punct_table(int table[][MAX_COLUMNS], int starting_row) {
	punct_row = starting_row;
	mark_fail(table, starting_row);
	mark_success(table, starting_row + 1);

	mark_cells(starting_row, table, '!', '/', starting_row + 1);
	mark_cells(starting_row, table, ':', '@', starting_row + 1);
	mark_cells(starting_row, table, '[', '`', starting_row + 1);
	mark_cells(starting_row, table, '{', '~', starting_row + 1);
	mark_cell(table, starting_row, 34, -1); //don't parse double quotes as punctuation
	mark_cells(starting_row + 1, table, '!', '/', starting_row + 1);
	mark_cells(starting_row + 1, table, ':', '@', starting_row + 1);
	mark_cells(starting_row + 1, table, '[', '`', starting_row + 1);
	mark_cells(starting_row + 1, table, '{', '~', starting_row + 1);
	mark_cell(table, starting_row + 1, 34, -1); //don't parse double quotes as punctuation

	return starting_row + 2;
}

int STokenizer::make_space_table(int table[][MAX_COLUMNS], int starting_row) {
	space_row = starting_row;
	mark_fail(table, starting_row);
	mark_success(table, starting_row + 1);

	mark_cell(table, starting_row, ' ',  starting_row + 1);
	mark_cell(table, starting_row, '	', starting_row + 1);
	mark_cell(table, starting_row, '\n', starting_row + 1);

	mark_cell(table, starting_row + 1, ' ', starting_row + 1);
	mark_cell(table, starting_row + 1, '	', starting_row + 1);
	mark_cell(table, starting_row + 1, '\n', starting_row + 1);
	return starting_row + 2;
}

//extract the longest string that match
//     one of the acceptable token types
bool STokenizer::get_token(int start_state, string& token) {
	int state = start_state;
	unsigned int new_position = _pos;
	endflag = true;
	bool found = false;

	for (int i = _pos; _buffer[i] != '\0'; i++) {
		if (_buffer[i] < 0 || _buffer[i]>MAX_COLUMNS) {
			endflag = false;
			break;
		} //unknown character

		state = _table[state][_buffer[i]];
		if (state == -1) {
			endflag = false;
			break;
		}
		if (is_success(_table, state)) {
			found = true;
			new_position = i + 1;
		}
	}

	if (found) {
		token = ((string)_buffer).substr(_pos, new_position - _pos);

		_pos = new_position;
		return true;
	}

	return false;
}

STokenizer& operator >> (STokenizer& s, Token& t) {
	string tokenstr = "";
	int type = -1;
	if (s.get_token(s.alpha_row, tokenstr)) type = 1;
	else if (s.get_token(s.punct_row, tokenstr)) type = 2;
	else if (s.get_token(s.space_row, tokenstr)) type = 3;
	else {
		tokenstr += s._buffer[s._pos];
		s._pos++;
		type = 4;
	}
	Token new_t(tokenstr, type);
	t = new_t;
	return s;
}

//set a new string as the input string
void STokenizer::set_string(const char str[]) {
	assert(strlen(str) <= MAX_BUFFER);
	_pos = 0;
	endflag = false;
	int i = 0;
	for (i; str[i] != '\0'; i++) _buffer[i] = str[i];
	_buffer[i] = '\0';
#ifdef DEBUG
	cout << "input string: " << _buffer << endl;
	cout << "current position: " << _pos << endl;
#endif
}