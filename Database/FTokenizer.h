#pragma once
#include "pch.h"
#include <string>
#include <iostream>
#include <fstream>
#include <assert.h>
#include "STokenizer.h"
using namespace std;

class FTokenizer
{
public:
	const int MAX_BLOCK = MAX_BUFFER;
	FTokenizer(const char* fname);
	FTokenizer(const FTokenizer& other);

	Token next_token();
	int get_pos();
	int get_block_pos();

	bool more();            //true: there are more tokens
	//-- big three --

	//---------------
	//extract one token (very similar to the way cin >> works)
	friend FTokenizer& operator >> (FTokenizer& s, Token& t);

	//set a new string as the input string
	void set_string(const char str[]);
	bool end_of_block() { return _stk.eOf(); }
	void put_back(Token t);

private:
	ifstream _f;   //file being tokenized

	bool get_new_block(); //gets the new block from the file
	string filename;
	STokenizer _stk;     //The STokenizer object to tokenize current block
	int _blockPos;      //Current position in the current block
	int _pos;
	bool _more;         //false if last token of the last block
};
FTokenizer::FTokenizer(const char* fname) {
	_f.open(fname);
	filename = fname;
	_f.seekg(ios::beg);
	_pos = _f.tellg();
	_blockPos = 0;
	get_new_block();
}

FTokenizer::FTokenizer(const FTokenizer& other) {
	_f.open(filename);
	_stk = other._stk;
	_pos = other._pos;
	_blockPos = other._blockPos;
	_more = other._more;
}

Token FTokenizer::next_token() {
	Token t;
	_stk >> t;
	_blockPos += t.token_str().length();
	_pos += t.token_str().length();
	return t;
}

int FTokenizer::get_pos() {
	return _pos;
}

int FTokenizer::get_block_pos() {
	return _blockPos;
}

void FTokenizer::put_back(Token t)
{
	int length = t.token_str().length();

	_pos = _f.tellg();
	_pos -= length;
	_f.seekg(_pos);
	//cout << endl << "Put back " << t << endl;
}

FTokenizer& operator >> (FTokenizer& f, Token& t) {
	t = f.next_token();
	if (f.end_of_block() && !f._f.eof())
	{
		f.put_back(t);
		f.get_new_block();
		t = f.next_token();
	}

	return f;
}

bool FTokenizer::more() {
	return (_stk.more() || !_f.eof());
}//true: there are more tokens

bool FTokenizer::get_new_block() {
	char b[MAX_BUFFER];
	_f.read(b, MAX_BUFFER - 1);

	if (_f.gcount() == 0) {
		cout << "done" << endl;
		return false;
	}

	b[_f.gcount()] = '\0';

	set_string(b);
	_blockPos = 0;
	return true;
}

void FTokenizer::set_string(const char str[]) {
	_stk.set_string(str);
}

