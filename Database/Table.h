#pragma once
#include "pch.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <string>
#include <vector>
#include <stack>
#include "MMap.h"
#include "Map.h"
#include "Stokenizer.h"
using namespace std;
const int max_fields=20;
const int max_field_size = 1000;
const int max_name_length = 100;
const enum keys { OPENPAREN, SYMBOL, OR, AND, RELATIONAL };
struct Record
{
public:
	Record& operator=(const Record& rhs)
	{
		numfields = rhs.numfields;
		_recno = rhs._recno;
		for (int i = 0; i < numfields; i++)
		{
			strcpy_s(_data[i], rhs._data[i]);
		}
		return *this;
	}
	Record() :_data() {}

	Record(vector<string> data, long recno):_recno(recno),numfields(data.size())
	{
		for (size_t i = 0; i < data.size(); i++)
		{
			strcpy_s(_data[i], data[i].c_str());
		}
	}
	char _data[max_fields][max_field_size];
	long _recno;
	int numfields;
};

class Table
{
public:
	Table() {};
	Table(string tname);
	Table(string tname, vector<string> fields);
	Table& operator=(const Table& rhs);
	Table(const Table& other);
	Record record_at(size_t index);
	long write(Record &r);
	long size() const { return fsize; }
	void init_file(const char* fname);
	vector<long> _select(string field, string value, string op);
	vector<long> _select_all();
	Table select(vector<string> &fields, vector<string> &condition);
	void insert_into(vector<string> data);
	void set_field_codes();
	bool has_field(string field);
	stack<string> shunting_yard(vector<string> condition);
	vector<long> process_condition(stack<string> &s);
	friend ostream& operator<<(ostream& outs, Table& print_me)
	{
		for (size_t i = 0; i < print_me._fields.size(); i++)
		{
			outs << print_me._fields[i] << '\t';
		}
		outs << endl;
		for (size_t i= 0; i < print_me.fsize; i++)
		{
			outs << i << '.';
			print_me.print_record(outs, print_me.record_at(i));
		}
		return outs;
	}
	string to_string();
	vector<string> fields();
	void print_record(ostream& outs,Record r);
	bool is_valid() { return _is_valid; }
private:
	bool _is_valid;
	void make_fields_file(const char* _fname);
	int token_code(string s);
	string fname;
	string _tname;
	vector<long> intersection(vector<long> v1, vector<long> v2);
	vector<long> _union(vector<long> v1, vector<long> v2);
	size_t fsize;
	void init_indices();
	long next_rec_no();
	void load_data();
	fstream f;
	Map<string, MMap<string, long> > indices;
	vector<string> _fields;
	Map<string, int> field_codes;
};

string Table::to_string()
{
	string ret = "Table name: " + _tname + ", records: "+std::to_string((int)fsize)+"\n\t";
	for (size_t i = 0; i < _fields.size(); i++)
	{
		ret+= (_fields[i] + '\t');
	}
	ret += '\n';
	Record r;
	for (size_t i = 0; i < fsize; i++)
	{
		r = record_at(i);
		ret += std::to_string((int)i);
		ret += '.';
		for (size_t j = 0; j < r.numfields; j++)
		{
			ret += (string(r._data[j]) + '\t');
		}
		ret += '\n';
	}
	return ret;
}

long Table::next_rec_no()//could be needed with a delete command
{
	return fsize;
}

vector<string> Table::fields()
{
	return _fields;
}
void Table::print_record(ostream& outs, const Record r)
{
	for (size_t i = 0; i < _fields.size(); i++)
		outs << r._data[i] << '/t';
}

Table& Table::operator=(const Table& rhs)
{
	if (rhs.fname=="") return *this; //if rhs is just a dummy table created by default constructor, do nothing.
	fname = rhs.fname;
	fsize = rhs.fsize;
	_fields = rhs._fields;
	indices = rhs.indices;
	set_field_codes();
	f.open(fname.c_str(), ios::in | ios::out | ios::binary);
	if (!f) init_file(fname.c_str());
	for (size_t i = 0; i < rhs._fields.size(); i++) _fields[i] = rhs._fields[i];
	return *this;
}

Table::Table(const Table& other)
{
	if (other.fname=="") return; //if other is a dummy, do nothing
	fname = other.fname;
	fsize = other.fsize;
	_fields = other._fields;
	indices = other.indices;
	set_field_codes();
	f.open(fname.c_str(), ios::in | ios::out | ios::binary);
	if (!f) init_file(fname.c_str());
	for (size_t i = 0; i < other._fields.size(); i++) _fields[i] = other._fields[i];

}

bool Table::has_field(string field)
{
	return indices.contains(field);
}

//given name of text file: Delete any file with this name, then create one and fill it with fields.
void Table::make_fields_file(const char* _fname)
{
	remove(_fname);
	f.close();
	f.open(_fname, ios::out);
	for (auto it : _fields)
	{
		f.write((it+'\n').c_str(),it.length()+1);
	}
	f.close();
}

void Table::init_indices()
{
	MMap<string, long>* m;
	for (size_t i = 0; i < _fields.size(); i++) { //build indices
		m = new MMap<string, long>;
		indices.insert(_fields[i], *m); //inializes indices[first], indices[last] etc
	}
}

//load table from table name
Table::Table(string tname) :f(), _is_valid(true), _tname(tname)
{
	fname = (tname + ".tbl");
	f.open((tname + ".txt").c_str(), ios::in);
	if (!f)
	{
		cout << "fields file not found.";
		_is_valid = false;
		return;
	}
	string field;
	while (getline(f,field))
	{
		_fields += field;
	}
	init_indices();
	set_field_codes();
	f.close();
	f.open(fname, ios::in | ios::out | ios::binary);
	if (!f) {
		cout << "table file not found.";
		_is_valid = false;
		return;
	}
	load_data();
}


//create new table from table name and fields
Table::Table(string tname, vector<string> fields) :f(), fsize(0), _fields(fields), _is_valid(true), _tname(tname)
{
	set_field_codes();
	fname = (tname + ".tbl");
	make_fields_file( (tname + ".txt").c_str() );
	f.open(fname.c_str(), ios::in | ios::out | ios::binary);
	if (!f) init_file(fname.c_str());
	init_indices();
}

//called by load constructor. Assumes fields are set and table file is open correctly.
void Table::load_data()
{
	Record r;
	assert(f.is_open());
	f.clear();
	fsize = 0;
	f.read((char*)&r, sizeof(Record));
	while (f.good())
	{
		fsize++;
		for (size_t i = 0; i < _fields.size(); i++) //for each field
			indices[_fields[i]][r._data[i]] += r._recno; //add corresponding data to appropriate map in indices
		f.read((char*)&r, sizeof(Record));
	}
	f.seekp(fsize * sizeof(Record));
}

void Table::init_file(const char* fname)
{
	f.clear();
	f.open(fname, ios::out | ios::binary);
	f.close();
	f.open(fname, ios::in | ios::out | ios::binary);
}

//Used by shunting_yard. Gives a number indicating operator precedence for tokens in condition.  
//this is a map, so it could use our data structure,
//but I think the scale is small enough to do it this way 
int Table::token_code(string s)
{
	if (s == "(") return OPENPAREN;
	if (s == ">" || s == "<" || s == "=" || 
		s == "==" || s == ">=" || s == "<=") return RELATIONAL;
	if (s == "and") return AND;
	if (s == "or") return OR;
	else return SYMBOL;
}

//condition is already parsed thus assumed to be valid
stack<string> Table::shunting_yard(vector<string> condition)
{
	stack<string> out;
	stack<string> s;
	for (auto it : condition)
	{
		if (it == "(") s.push(it);
		else if (it == ")")
		{
			while (s.top() != "(")
			{
				out.push(s.top());
				s.pop();
			}
			s.pop();
		}

		else if (token_code(it)==SYMBOL) out.push(it); //if token is not an operator add to out
		else if (s.empty() || token_code(s.top())<=token_code(it)) s.push(it); //if stack is empty or lower precedence, push new operator
		else //if operator on stack has >= precedence, process it
		{
			while (!s.empty()&&token_code(s.top()) > token_code(it))
			{
				out.push(s.top());
				s.pop();
			}
			s.push(it);
		}
	}
	while (!s.empty()) { //pop rest of stack
		out.push(s.top());
		s.pop();
	}
	return out;
}

vector<long> Table::_select(string field, string value, string op) //get the record nos corresponding to a single
													  //conditional statement
{
	vector<long> recnos;
	if (op == "=" || op == "==")
	{
		if(indices[field].contains(value))
			recnos = indices[field][value]; //just copy value_list for the specified key
	}
	if (op == "<")
		for (MMap<string, long>::Iterator it = indices[field].begin();
			it != indices[field].lower_bound(value); it++)
			recnos += (*it).value_list;
	if (op == "<=")
		for (MMap<string, long>::Iterator it = indices[field].begin();
			it != indices[field].upper_bound(value); it++)
			recnos += (*it).value_list;
	if (op == ">")
		for (MMap<string, long>::Iterator it = indices[field].upper_bound(value);
			it != indices[field].end(); it++)
			recnos += (*it).value_list;
	if (op == ">=")
		for (MMap<string, long>::Iterator it = indices[field].lower_bound(value);
			it != indices[field].end(); it++)
			recnos += (*it).value_list;
	return recnos;
}
//A recursive function to convert an arbitrary-length conditional statement
//into a list of record numbers. Calls _select() to handle an individual clause, as well as
// intersection() and union() to join them
vector<long> Table::process_condition(stack<string> &s)
{
	vector<long> list1;
	vector<long> list2;
	string top = s.top();
	s.pop();
	switch (token_code(top))
	{
	case OR:
		list1 = process_condition(s);
		list2 = process_condition(s);
		return _union(list1, list2);
		break;
	case AND:
		list1 = process_condition(s);
		list2 = process_condition(s);
		return intersection(list1, list2);
		break;
	case RELATIONAL:
		string value, field, op;
		op = top;
		value = s.top(); s.pop();
		field = s.top(); s.pop();
		return _select(field, value, op);
		break;
	}
}


vector<long> Table::_select_all()
{
	vector<long> recnos;
	for (MMap<string, long>::Iterator it = indices[_fields[0]].begin();
		it != indices[_fields[0]].end(); it++)
		recnos += (*it).value_list;
	return recnos;
}

//the function called by SQL. Gets the list of recnos based on condition, then returns a new table.
Table Table::select(vector<string> &fields, vector<string> &condition)
{
	vector<long> recnos;
	if (condition.size() == 0) recnos = _select_all();
	else{
		stack<string> c = shunting_yard(condition);
		recnos = process_condition(c);
	}
	 
	remove("result.tbl"); //I think this is necessary? Creating the table to return creates two new files,
	remove("result.txt"); //which would otherwise stick around to mess up future selects.
	Table t("result",fields);
	Record r;
	vector<string> data;
	for (size_t i = 0; i < recnos.size(); i++)
	{
		data.clear();
		r = record_at(recnos[i]);
		for (size_t j=0; j < fields.size(); j++) data.push_back(r._data[field_codes[fields[j]]]);
		t.insert_into(data);
	}
	return t;
}

/*intersection of two vectors. Unwieldy but should be O(nlogn)
*/
vector<long> Table::intersection(vector<long> v1, vector<long> v2)
{
	Map<long, long> m;
	vector<long> out;
	for (size_t i = 0; i < v1.size(); i++)
	{
		m.insert(v1[i],v1[i]); 
	}
	for (size_t i = 0; i < v2.size(); i++)
	{
		if (m.contains(v2[i])) out.push_back(v2[i]);
	}
	return out;
}

//add everything in v to the same map with no duplicates allowed. Then vectorize.
vector<long> Table::_union(vector<long> v1, vector<long> v2)
{
	Map<long, long> m;
	for (size_t i = 0; i < v1.size(); i++)
	{
		m.insert(v1[i], v1[i]);
	}
	for (size_t i = 0; i < v2.size(); i++)
	{
		m.insert(v2[i], v2[i]);
	}
	return m.vectorize();
}

void Table::set_field_codes()
{
	for (size_t i = 0; i < _fields.size(); i++)
	{
		field_codes.insert(_fields[i], i);
	}
}

void Table::insert_into(vector<string> data) //data in order, so if codes are first-0, last-1, age-2 then data
											//is Joe, Smith, 19 etc.
{
	long recno = next_rec_no();
	Record r(data,recno);
	write(r);
	for (size_t i = 0; i < _fields.size(); i++)
	{
		indices[_fields[i]] [data[i]] += recno;
		//cout << _fields[i] << endl;
		//indices[_fields[i]].print();
	}
	fsize++;
}

Record Table::record_at(size_t index)
{
	assert((0 <= index) && (index < fsize));
	f.seekg(index * sizeof(Record));
	Record r;
	f.read((char*)&r, sizeof(Record));
	return r;
}

//write a new record to end of current file
long Table::write(Record &r)
{
	f.seekp(fsize * sizeof(Record));
	f.write((char*)&r, sizeof(Record));
	return fsize + 1;
}
