#include "pch.h"
#include <iostream>
#include "Parser.h"
#include "random.h"
#include "BPTree.h"
#include <iostream>
#include <string>
#include "Table.h"
#include <assert.h>
#include "arrayfuncs.h"
//#define DEBUG
using namespace std;

class SQL
{
public:
	SQL();
	SQL(const char* filename);
	void run();
	void run_file(const char* fname);
	bool drop_table(string tablename);
private:
	bool fields_match(string tname, vector<string> fields);
	void run_command(MMap<string,string> &parsetree);
	bool add_table(string name, vector<string> fields);
	bool insert(string name, vector<string> data);
	bool select(string name, vector<string> fields, vector<string> condition);
	void init_file(const char* fname);
	void load_tables();
	Map<string,Table> _tables;
	const char* tfname = "tables.txt";
	Parser p;
	STokenizer s;
	fstream f;
	fstream output;
};

SQL::SQL()
{
	init_file(tfname);
	load_tables();
}

SQL::SQL(const char* filename)
{
	init_file(tfname);
	load_tables();
	run_file(filename);
}

//one table name per line
void SQL::load_tables()
{
	string name;

	cout << "Loading tables:" << endl;
	output<< "Loading tables:" << endl;
	while (getline(f, name))
	{
		cout << name << endl;
		output << name << endl;
		Table tbl(name.c_str());
		if(tbl.is_valid())
		_tables[name] = tbl;
	}
	f.close();
}

void SQL::init_file(const char* fname)
{
	f.open(tfname, ios::in);
	if (f.fail())
	{
		f.clear();
		f.open(fname, ios::out);
		f.close();
		f.open(fname, ios::in);
		return;
	}
}

//adds table to _tables and updates tables.txt
bool SQL::add_table(string name, vector<string> fields)
{
	cout << "SQL::create table " << name << " fields " << fields<<endl;
	output << "SQL::create table " << name << " fields " << fields << endl;
	fstream f;
	f.open(tfname, ios::app);
	if (_tables.contains(name))
		name += '1';
	int count = 2;
	while (_tables.contains(name))
	{
		name = name.substr(0, name.length() - 1);
		name += count;
		count++;
	}
	Table t (name, fields);
	_tables.insert(name, t);
	name += '\n';
	f << name;
	if (f) {
		cout << "Created " << name << endl;
		output << "Created " << name << endl;
		return true;
	}
	else
	{
		cout << "Error creating table" << endl;
		output << "Error creating table" << endl;
	}
	return false;
}

void SQL::run_file(const char* fname)
{
	string s(fname);
	s = s.substr(0, s.length() - 4);
	s += "_output.txt";
	remove(s.c_str());
	output.open(s.c_str(), ios::app);

	f.open(fname,ios::in);
	assert(f.is_open());

	string line;
	string print;
	int count = 1;
	while (getline(f,line))
	{
		if (line.substr(0,2) != "//")
		{
			cout << '[' << count << ']' << line << endl;
			output << '[' << count << ']' << line << endl;
			p.set_string(line);
			MMap<string, string> parsetree = p.get_parse_tree();
			run_command(parsetree);
			count++;
		}
		else
		{
			cout << line << endl;
			output << line << endl;
		}
	}
}

bool SQL::drop_table(string tablename)
{
	if (!_tables.contains(tablename))
	{
		cout << "SQL: Drop failed." << endl;
		output << "SQL: Drop failed." << endl;
		return false;
	}
	fstream f;
	_tables.erase(tablename);
	remove((tablename + ".tbl").c_str());
	remove((tablename + ".txt").c_str());
	remove(tfname);
	f.open(tfname, ios::app);
	for (Map<string, Table>::Iterator it = _tables.begin(); it != _tables.end(); it++)
	{
		f << (*it)._key<<"\n";
	}
	cout << "SQL: " << tablename << " dropped." << endl;
	output << "SQL: " << tablename << " dropped." << endl;
	return true;
}

bool SQL::insert(string tablename, vector<string> data)
{	
	if (data.size() == _tables[tablename].fields().size()) //is the data a valid match for the fields
	{
		_tables[tablename].insert_into(data);
		cout << "SQL: Inserted" << endl;
		output << "SQL: Inserted" << endl;
		return true;
	}
	cout << "SQL: Insert failed" << endl;
	output<< "SQL: Insert failed" << endl;
	return false;
}

bool SQL::select(string tablename, vector<string> fields, vector<string> condition)
{
	if (fields_match(tablename, fields) && _tables.contains(tablename))
	{
		if (fields[0] == "*") fields = _tables[tablename].fields();

		Table t = _tables[tablename].select(fields, condition);
		cout << t.to_string();
		output << t.to_string();
		return true;
	}
	cout << "SQL: Select failed. " << endl;
	output << "SQL: Select failed. " << endl;
	return false;
}

void SQL::run_command(MMap<string,string> &parsetree)
{
	string command = parsetree["command"][0];
	string tablename;
	vector<string> condition,fields,data;
	if (command != "invalid")
	{
		tablename = parsetree["tablename"][0];
		condition = parsetree["condition"];
		fields = parsetree["fields"];
		data = parsetree["data"];
		//cout << "SQL:: " << command << endl;
		//output << "SQL:: " << command << endl;
	}
	else
	{
		cout << "SQL: invalid input." << endl;
		output << "SQL: invalid input." << endl;
	}
	if (command == "create"	|| command == "make")
		add_table(tablename, fields);

	else if (command == "insert")
		insert(tablename, data);

	else if (command == "select") 
		select(tablename, fields, condition);

	else if (command == "drop")
		drop_table(tablename.c_str());

	cout << "SQL: DONE." << endl;
	output << "SQL: DONE." << endl;
}


//each field mentioned in a select must belong to the specified table
bool SQL::fields_match(string tname, vector<string> fields)
{
	if (fields[0] == "*") return true;
	for (auto it : fields)
	{
		if (!_tables[tname].has_field(it)) return false;
	}
	return true;
}

void SQL::run()
{
	string input="";
	remove("output.txt");
	output.open("output.txt", ios::app);
	while (true)
	{
		cout << "Enter command" << endl;
		getline(cin, input);
		if (input == "exit") return;
		output << input;
		p.set_string(input);
		MMap<string, string> parsetree = p.get_parse_tree();
		run_command(parsetree);
	}
}