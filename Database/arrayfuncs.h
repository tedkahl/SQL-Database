#pragma once
#include <vector>
using namespace std;

static vector<string> fields = { "firstname","lastname","age","major","id" };

template <typename T>
ostream& operator <<(ostream& outs, const vector<T>& list) //print vector list
{
	for (auto it : list)outs << it << ' ';
	return outs;
}

template <typename T>
vector<T>& operator +=(vector<T>& list, const T& addme)  //list.push_back addme
{
	list.push_back(addme);
	return list;
}

template <typename T>
vector<T>& operator +=(vector<T>& list, const vector<T>& addme)  
{
	list.insert(list.end(), addme.begin(), addme.end());
	return list;
}

template <class T>
T maximal(const T& a, const T& b)                      //return the larger of the two items
{
	if (b > a) return b;
	else return a;
}

template <class T>
void swap(T& a, T& b)
{
	std::swap(a, b);
}

template <class T>
int index_of_maximal(T data[], int n)  //return index of the largest item in data
{
	int max=0;
	for (int i = 1; i < n; i++) if (data[i] > data[max]) max = i;
	return max;
}

template <class T>
void ordered_insert(T data[], int& n, T entry)        //insert entry into the sorted array
{														//data with length n
	int i = n - 1;
	while(i>=0 && data[i]>entry)
	{
		data[i + 1] = data[i];
		i--;
	}
	data[i + 1] = entry;
	n++;
}

template <class T>
int first_ge(const T data[], int n, const T& entry)   //return the first element in data that is
{														//not less than entry
	for (int i = 0; i < n; i++)
	{
		if (data[i] > entry || data[i]==entry) {
			return i;
		}
	}
	return n;
}

template <class T>
void attach_item(T data[], int& n, const T& entry)    //append entry to the right of data
{
	data[n] = entry;
	n++;
}

template <class T>
void insert_item(T data[], int i, int& n, T entry)    //insert entry at index i in data
{
	for (int j = n; j > i; j--) data[j] = data[j - 1];
	data[i] = entry;
	n++;
}

template <class T>
void detach_item(T data[], int& n, T& entry)          //remove the last element in data and place
{														//it in entry
	entry = data[n-1];
	n--;
}

template <class T>
void delete_item(T data[], int i, int& n, T& entry)   //delete item at index i and place it in entry
{
	entry = data[i];
	for (int j = i; j < n - 1; j++)
	{
		data[j] = data[j + 1];
	}
	n--;
}

template <class T>
void merge(T dest[], int& n1, T src[], int& n2)   //append data2 to the right of data1
{
	for (int i = 0; i < n2; i++)
	{
		dest[i + n1] = src[i];
	}
	n1 += n2;
	n2 = 0;
}

template <class T>
void split(T data1[], int& n1, T data2[], int& n2)   //move n/2 elements from the right of data1
{														//and move to data2
	int firstindex = n1 - (n1 / 2); //eg, 1 if n1=2, 2 if n1=3, 2 if n1=4
	for (int i = firstindex; i < n1; i++)
		data2[i-firstindex] = data1[i];

	n2 += (n1 / 2);
	n1 -= (n1 / 2);
}

template <class T>
void copy_array(const T src[], int src_size, T dest[], int& dest_size )//copy src[] into dest[]
{
	for (int i = 0; i < src_size; i++)
	{
		dest[i] = src[i];
	}
	dest_size = src_size;
}

template <class T>
void print_array(const T data[], int n, int pos = -1)  //print array data
{
	if (pos >= 0) cout << data[pos] << endl;
	else
		for (int i= 0; i < n; i++) cout << data[i] << ' ';
	cout << endl;
}

//should this assume sorted? If so change 
template <class T>
bool is_gt(const T data[], int n, const T& item)       //item > all data[i]
{
	for (int i = 0; i < n; i++) if (item <= data[i]) return false;
	return true;
}

template <class T>
bool is_le(const T data[], int n, const T& item)       //item <= all data[i]
{
	for (int i = 0; i < n; i++) if (item > data[i]) return false;
	return true;
}

