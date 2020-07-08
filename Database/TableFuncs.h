#pragma once
#include <iostream>
using namespace std;
const int MAX_ROWS = 50;
const int MAX_COLUMNS = 255;

void init_table(int _table[][MAX_COLUMNS]) {
	for (int i = 0; i < MAX_ROWS; i++) {
		for (int j = 0; j < MAX_COLUMNS; j++) _table[i][j] = -1;
		_table[i][0] = 0;
	}
}

//Mark this state (row) with a 1 (success)
void mark_success(int _table[][MAX_COLUMNS], int state) {
	_table[state][0] = 1;
}

//Mark this state (row) with a 0 (fail)
void mark_fail(int _table[][MAX_COLUMNS], int state) {
	_table[state][0] = 0;
}

//true if state is a success state
bool is_success(int _table[][MAX_COLUMNS], int state) {
	if (state == -1) return false;
	else return _table[state][0];
}

//Mark a range of cells in the array. 
void mark_cells(int row, int _table[][MAX_COLUMNS], int from, int to, int state) {
	for (int i = from; i <= to; i++) _table[row][i] = state;
}

void copy_table(const int from[][MAX_COLUMNS], int to[][MAX_COLUMNS]) {
	for (int i = 0; i < MAX_ROWS; i++)
		for (int j = 0; j < MAX_COLUMNS; j++) {
			to[i][j] = from[i][j];
		}
}

//Mark columns represented by the string columns[] for this row
void mark_cells(int row, int _table[][MAX_COLUMNS], const char columns[], int state) {
	for (int i = 0; columns[i] != '\0'; i++) {
		_table[row][columns[i]] = state;
	}

}

//Mark this row and column
void mark_cell(int _table[][MAX_COLUMNS], int row, int column, int state) { _table[row][column] = state; }

//This can realistically be used on a small table
void print_table(int _table[][MAX_COLUMNS], int from, int to) {
	for (int i = 0; i < MAX_ROWS; i++) {
		for (int j = from; j <= to; j++)
		{
			cout << _table[i][j] << ' ';
		}
		cout << endl;
	}
}