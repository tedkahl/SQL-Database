#pragma once
#include "pch.h"
#include <iostream>
#include "arrayfuncs.h"
#include <string>
#include <assert.h>
#include "random.h"
using namespace std;
template <class T>
class BPTree
{
public:
	class Iterator {
	public:
		friend class BPTree;
		Iterator(BPTree<T>* _it = NULL, int _key_ptr = 0) :it(_it), key_ptr(_key_ptr) {}
		
		Iterator operator=(const Iterator& rhs)
		{
			it = rhs.it;
			key_ptr = rhs.key_ptr;
			return *this;
		}
		T operator *() {
			assert(key_ptr < it->data_count);
			return it->data[key_ptr];
		}

		T& operator ->() {
			assert(key_ptr < it->data_count);
			return it->data[key_ptr];
		}

		Iterator operator++(int un_used) {		//it++
			Iterator* hold = this;
			if (key_ptr == it->data_count - 1) {
				it = it->next;
				key_ptr = 0;
			}
			else key_ptr++;
			return *hold;
		}

		Iterator& operator++() //++i returns reference
		{
			if (key_ptr == it->data_count - 1) {
				it = it->next;
				key_ptr = 0;
			}
			else key_ptr++;
			return *this;
		}

		friend bool operator ==(const Iterator& lhs, const Iterator& rhs) {
			return (lhs.it == rhs.it && lhs.key_ptr == rhs.key_ptr);
		}

		friend bool operator !=(const Iterator& lhs, const Iterator& rhs) {
			return !(lhs == rhs);
		}

		bool is_null() { return !it; }
	private:
		BPTree<T>* it;
		int key_ptr;
	};

	BPTree(bool dups = false);
	//big three:
	BPTree(const BPTree<T>& other);
	~BPTree();
	BPTree<T>& operator =(const BPTree<T>& RHS);

	int insert(const T& entry);                //insert entry into the tree
	int remove(const T& entry);                //remove entry from the tree
	bool is_valid() const;
	bool is_valid(T& last, bool& check = false) const;
	void clear_tree();                          //clear this object (delete all nodes etc.)
	void copy_tree(const BPTree<T>& other);
	void copy_tree(const BPTree<T>& other, BPTree<T>*& last);      //copy other into this object
	bool is_valid_leaves();
	Iterator begin();
	Iterator end();
	Iterator upper_bound(const T& entry);
	Iterator lower_bound(const T& entry);

	bool contains(const T& entry) const;              //true if entry can be found in the array
	const T& get(const T& entry) const;               //return a reference to entry in the tree. Item must be found
	T& get_existing(const T& entry);
	T& get(const T& item);
	Iterator find(const T& entry);                    //return a pointer to this key. NULL if not there.

	int size() const;                           //count the number of elements in the tree
	bool empty() const;                         //true if the tree is empty

	void print_tree(int level = 0, ostream &outs = cout) const; //print a readable version of the tree
	void print_debug(int level = 0, ostream &outs = cout) const;
	friend ostream& operator<<(ostream& outs, const BPTree<T>& print_me) {
		print_me.print_tree(0, outs);
		return outs;
	}
private:
	static const int MINIMUM = 1;
	static const int MAXIMUM = 2 * MINIMUM;

	bool dups_ok;                                   //true if duplicate keys may be inserted
	int data_count;                                 //number of data elements
	int _size;
	T data[MAXIMUM + 1];                            //holds the keys
	int child_count;                                //number of children
	BPTree* subset[MAXIMUM + 2];                     //suBPTrees
	BPTree* next;
	T* find_ptr(const T& entry);

	bool is_leaf() const { return child_count == 0; }   //true if this is a leaf node

	//insert element functions
	int loose_insert(const T& entry);              //allows MAXIMUM+1 data elements in the root
	void fix_excess(int i);                         //fix excess of data elements in child i

	//remove element functions:
	int loose_remove(const T& entry);              //allows MINIMUM-1 data elements in the root
	void fix_shortage(int i);                       //fix shortage of data elements in child i

	BPTree<T>* get_smallest_node();
	void get_smallest(T& entry);        //entry := leftmost leaf
	void get_smallest_nonequal(T& entry);
	void get_biggest(T& entry);         //entry := rightmost leaf

	void transfer_left(int i);                        //transfer one element LEFT from child i
	void transfer_right(int i);                       //transfer one element RIGHT from child i
	void merge_with_next_subset(int i);             //merge subset i with subset i+1

};

template <typename T>
void BPTree<T>::print_tree(int level, ostream &outs) const
{
	string indent(level, '\t');

	if (is_leaf()) {
		for (int i = data_count - 1; i >= 0; i--)
			outs << indent << data[i] << endl;
		return;
	}

	subset[data_count]->print_tree(level + 1);

	for (int i = data_count - 1; i >= 0; i--)
	{
		outs << indent << data[i] << endl;
		subset[i]->print_tree(level + 1);
	}
	cout << endl;
}


template <typename T>
void BPTree<T>::get_biggest(T& entry)
{
	if (is_leaf()) entry = data[data_count - 1];
	else subset[child_count - 1]->get_biggest(entry);
}

template <typename T>
void BPTree<T>::get_smallest(T& entry)
{
	if (is_leaf()) entry = data[0];
	else subset[0]->get_smallest(entry);
}

template<typename T>
void BPTree<T>::get_smallest_nonequal(T& entry)
{
	BPTree<T>* s = get_smallest_node();
	if (!(s->data[0] == entry)) entry = s->data[0]; //if smallest value is nonequal, behave like get_smallest
	else {	//else use the next value
		BPTree<T>::Iterator temp(s, 0); 							
		temp++;
		if (!temp.is_null())entry = *temp; //if temp is null then inner node will be handled by merge anyway
	}
}

template <typename T>
BPTree<T>* BPTree<T>::get_smallest_node()
{
	if (is_leaf()) return this;
	else return subset[0]->get_smallest_node();
}


template <typename T>
bool BPTree<T>::empty() const
{
	return(data_count == 0 && child_count == 0);
}

template<typename T>
void BPTree<T>::fix_shortage(int i)
{
	if (i > 0 && subset[i - 1]->data_count > MINIMUM)  //case 1: subset[i-1] has a spare element
		transfer_right(i);

	else if (i<child_count - 1 && subset[i + 1]->data_count > MINIMUM)  //case 2: subset[i+1] has a spare element
		transfer_left(i);

	else if (i > 0)  //case 3: subset[i-1] has MINIMUM elements, merge subset[i-1] with [i]
		merge_with_next_subset(i - 1);

	else if (i < child_count - 1) //case 4: merge i with i+1
		merge_with_next_subset(i);
}

template <typename T>
void BPTree<T>::merge_with_next_subset(int i) {
	T temp;
	BPTree* temp2;
	delete_item(data, i, data_count, temp);
	if (!subset[0]->is_leaf()) attach_item(subset[i]->data, subset[i]->data_count, temp);
	//merge two subsets
	merge(subset[i]->data, subset[i]->data_count, subset[i + 1]->data, subset[i + 1]->data_count);
	merge(subset[i]->subset, subset[i]->child_count, subset[i + 1]->subset, subset[i + 1]->child_count);
	subset[i]->next = subset[i + 1]->next;
	delete subset[i + 1];
	delete_item(subset, i + 1, child_count, temp2); //moves later child ptrs over and decrements child count
}

template <typename T>
void BPTree<T>::transfer_right(int i)
{
	T temp;
	BPTree<T>* temp2;

	detach_item(subset[i - 1]->data, subset[i - 1]->data_count, temp); //removes last item of subset[i-1] data
																		//and decreases data_count
	if (subset[i]->is_leaf()) insert_item(subset[i]->data, 0, subset[i]->data_count, temp);

	else
	{
		insert_item(subset[i]->data, 0, subset[i]->data_count, data[i - 1]); //complete normal rotation

		detach_item(subset[i - 1]->subset, subset[i - 1]->child_count, temp2);
		insert_item(subset[i]->subset, 0, subset[i]->child_count, temp2);
	}
	data[i - 1] = temp;

}

template <typename T>
void BPTree<T>::transfer_left(int i)
{
	T temp;
	BPTree<T>* temp2;

	delete_item(subset[i + 1]->data, 0, subset[i + 1]->data_count, temp); //removes first item of subset[i+1] data
																		//and decreases data_count
	if (subset[i]->is_leaf()) {
		data[i] = subset[i + 1]->data[0];
		attach_item(subset[i]->data, subset[i]->data_count, temp);
	}
	else
	{
		attach_item(subset[i]->data, subset[i]->data_count, data[i]);
		data[i] = temp;
		delete_item(subset[i + 1]->subset, 0, subset[i + 1]->child_count, temp2);
		attach_item(subset[i]->subset, subset[i]->child_count, temp2);
	}
}

//0 = fail 1= success
template <typename T>
int BPTree<T>::remove(const T& entry)
{
	int success = loose_remove(entry);
	if (success) _size--;
	if (data_count == 0) {
		if (child_count == 1) {
			BPTree<T>* temp = subset[0];
			copy_array(subset[0]->data, subset[0]->data_count, data, data_count);
			copy_array(subset[0]->subset, subset[0]->child_count, subset, child_count);
			temp->child_count = 0;
			delete temp;
		}
	}
	return success;
}

template <typename T>
int BPTree<T>::loose_remove(const T& entry)
{
	int success = 0;
	T deleted;
	int index = first_ge(data, data_count, entry);
	bool found = (data[index] == entry && index < data_count);

	if (is_leaf()) {
		if (found) {
			delete_item(data, index, data_count, deleted); //case 2 leaf + found
			success = 1;
		}
		return success;	//case 1 leaf + not found
	}

	else if (!found) {
		success=subset[index]->loose_remove(entry); //case 3 not leaf + not found
		if (subset[index]->data_count < MINIMUM) fix_shortage(index);
	}

	else {										//case 4 not leaf + found
		success = 1;
		subset[index + 1]->get_smallest_nonequal(data[index]);
		subset[index + 1]->loose_remove(entry);
		if (subset[index + 1]->data_count < MINIMUM) fix_shortage(index + 1);
	}
	return success;
}

template <typename T>
void BPTree<T>::print_debug(int level, ostream &outs) const
{
	string indent(level, '\t');

	if (is_leaf()) {
		for (int i = data_count - 1; i >= 0; i--)
			outs << indent << data[i] << " i:" << i << " d:" << data_count << "c:" << child_count << endl;
		return;
	}

	subset[data_count]->print_debug(level + 1);

	for (int i = data_count - 1; i >= 0; i--)
	{
		outs << indent << data[i] << " i:" << i << " d:" << data_count << "c:" << child_count << endl;
		subset[i]->print_debug(level + 1);
	}
	cout << endl;
}


template <typename T>
T& BPTree<T>::get(const T& item) {
	if (!contains(item)) insert(item);

	return get_existing(item);
}

template <typename T>
T& BPTree<T>::get_existing(const T& entry)
{
	int index = first_ge(data, data_count, entry);
	bool found = (data[index] == entry && index < data_count);
	if (found) {
		if (is_leaf()) return data[index];
		else subset[index + 1]->get_existing(entry);
	}

	else {
		if (is_leaf())	assert(found);
		subset[index]->get_existing(entry);
	}
}

template <typename T>
typename BPTree<T>::Iterator BPTree<T>::upper_bound(const T& entry)
{
	Iterator bound;
	bound = lower_bound(entry);
	Iterator test = end();
	if (bound != end() && *bound == entry) bound++;
	return bound;
}

template <typename T>
typename BPTree<T>::Iterator BPTree<T>::lower_bound(const T& entry)
{
	int index = first_ge(data, data_count, entry);
	bool found = (data[index] == entry && index < data_count);
	if (found) {
		if (is_leaf()) return Iterator(this, index);
		else subset[index + 1]->lower_bound(entry);
	}
	else {
		if (is_leaf())
		{
			if (index < data_count) return Iterator(this, index); //index is already the first greater key
			else {
				Iterator it(this, index - 1);
				++it;
				return it;
			}
		}
		subset[index]->lower_bound(entry);
	}
}

template <typename T>
typename BPTree<T>::Iterator BPTree<T>::find(const T& entry)
{
	int index = first_ge(data, data_count, entry);
	bool found = (data[index] == entry && index < data_count);
	if (found) {
		if (is_leaf()) return Iterator(this, index);
		else subset[index + 1]->find(entry);
	}
	else {
		if (is_leaf())	return Iterator(); //null iterator
		subset[index]->find(entry);
	}
}


template <typename T>
T* BPTree<T>::find_ptr(const T& entry)
{
	int index = first_ge(data, data_count, entry);
	bool found = (data[index] == entry && index < data_count);
	if (found) {
		if (is_leaf()) return (T*)data[index];
		else subset[index + 1]->find(entry);
	}
	else {
		if (is_leaf())	return NULL;
		subset[index]->find(entry);
	}
}

template <typename T>
bool BPTree<T>::is_valid() const
{
	T temp;
	bool temp2 = false;
	return (is_valid(temp, temp2) && is_valid_leaves());
}

template <typename T>
bool BPTree<T>::is_valid(T& last, bool& check) const
{
	if (is_leaf()) {
		int start = data_count - 1;
		if (check == false) //Navigate down the rightmost path before setting last
		{
			check = true;
			last = data[start];
			start--;
		}

		for (int i = start; i >= 0; i--) //check that data in the leaf is in order
		{
			if (data[i] <= last)	last = data[i];
			else return false;
		}
		return true;
	}

	if (!subset[data_count]->is_valid(last, check)) return false;

	T temp;
	for (int i = data_count - 1; i >= 0; i--)
	{
		subset[i + 1]->get_smallest(temp);
		if (temp != data[i]) return false; //check inner node condition
		
		if ((data[i] <= last)) last = data[i];
		else return false;
		if (!subset[i]->is_valid(last, check)) return false;
	}
	return true;
}

template <typename T>
int BPTree<T>::size() const
{
	return _size;
}

template <typename T>
const T& BPTree<T>::get(const T& entry) const                  //return a reference to entry in the tree
{
	int index = first_ge(data, data_count, entry);
	bool found = (data[index] == entry && index < data_count);
	if (found) {
		if (is_leaf())return data[index];
		subset[index + 1]->get_existing(entry);
	}

	else {
		if (is_leaf()) assert(found);
		subset[index]->get_existing(entry); //recursive call switches to identical function get_existing
											//just calling get could call the other get 
	}
}

template <typename T>
typename BPTree<T>::Iterator BPTree<T>::begin()
{
	if (is_leaf()) return Iterator(this, 0);
	else return subset[0]->begin();
}

template <typename T>
typename BPTree<T>::Iterator BPTree<T>::end()
{
	BPTree<T>* b=this;
	if (_size == 0) return Iterator(b, 0);
	while (!b->is_leaf()) b = b->subset[b->data_count];
	b = b->next;
	return Iterator(b, 0);
}

template <typename T>
bool BPTree<T>::is_valid_leaves()
{
	if (_size == 0) return true;
	Iterator i = begin();
	T last = *i;
	++i;
	int count = 1;
	while (!i.is_null())
	{
		if (*i > last) last = *i;
		else return false;
		++i;
		++count;
	}

	if(count==_size)return true;
}

template <typename T>
bool BPTree<T>::contains(const T& entry) const                  //return a reference to entry in the tree
{
	int index = first_ge(data, data_count, entry);
	bool found = (data[index] == entry && index < data_count);
	if (found) return true;
	else
	{
		if (is_leaf()) return false;
		subset[index]->contains(entry);
	}
}


template <typename T>
void BPTree<T>::fix_excess(int i)                         //fix excess of data elements in child i
{
	BPTree* newsubset = new BPTree();
	insert_item(subset, i + 1, child_count, newsubset);
	newsubset->next = subset[i]->next;
	subset[i]->next = newsubset;

	split(subset[i]->data, subset[i]->data_count,
		newsubset->data, newsubset->data_count); //split child data to new subset

	split(subset[i]->subset, subset[i]->child_count,
		newsubset->subset, newsubset->child_count); //split child children to new subset

	T lastitem;
	detach_item(subset[i]->data, subset[i]->data_count, lastitem); //detach last element of excess child
	ordered_insert(data, data_count, lastitem); //move to current level

	if (subset[i]->is_leaf())
		insert_item(newsubset->data, 0, newsubset->data_count,
			data[i]);
	//duplicate item in  new leaf
}

//1=inserted as leaf 2=added to existing node. Should never fail.
template <typename T>
int BPTree<T>::insert(const T& entry)
{
	int success=loose_insert(entry);
	if (success)_size++;
	if (data_count > MAXIMUM)
	{
		BPTree* temp = new BPTree();

		copy_array(data, data_count, temp->data, temp->data_count);
		copy_array(subset, child_count, temp->subset, temp->child_count);
		data_count = 0;
		child_count = 1;
		subset[0] = temp;
		fix_excess(0);
	}
	return success;
}

template <typename T>
int BPTree<T>::loose_insert(const T& entry)              //allows MAXIMUM+1 data elements in the root
{
	int success = 0;
	int index = first_ge(data, data_count, entry);
	bool found = (data[index] == entry && index < data_count);

	if (found) {
		if (is_leaf()) {
			if (!(data[index] == entry) || dups_ok) {
				data[index] += entry;
				success = 2; //added to existing node
			}
		}
		else {
			success = subset[index + 1]->loose_insert(entry); //item found not node, recursive call i+1
			if (subset[index + 1]->data_count > MAXIMUM) fix_excess(index + 1);
		}
	}

	else {
		if (is_leaf()) {
			ordered_insert(data, data_count, entry);
			success = 1;
		}
		else {
			success=subset[index]->loose_insert(entry);
			if (subset[index]->data_count > MAXIMUM) fix_excess(index);
		}
	}
	return success;
}

template <typename T>
void BPTree<T>::clear_tree()
{
	for (int i = 0; i < child_count; i++)
	{
		if (subset[i]->is_leaf()) delete subset[i];
		else subset[i]->clear_tree();
	}
	data_count = 0;
	child_count = 0;
}

template <typename T>
void BPTree<T>::copy_tree(const BPTree& other)
{
	BPTree<T>* last = NULL;
	copy_tree(other, last);
}

template <typename T>
void BPTree<T>::copy_tree(const BPTree& other, BPTree*& last) //remember the last leaf added. When a new leaf
{																//is added, connect it to last. 
	copy_array(other.data, other.data_count, data, data_count);
	child_count = other.child_count;
	if (!is_leaf()) {
		for (int i = child_count - 1; i >= 0; i--) {
			subset[i] = new BPTree<T>();
			subset[i]->copy_tree(*other.subset[i], last);
			if (subset[i]->is_leaf()) {
				subset[i]->next = last;
				last = subset[i];
			}
		}
	}
}

template <typename T>
BPTree<T>::BPTree(bool dups) :dups_ok(dups), next(NULL), data_count(0), child_count(0),_size(0) {}

template <typename T>
BPTree<T>::BPTree(const BPTree<T>& other)
{
	copy_tree(other);
	_size = other.size();
	dups_ok = other.dups_ok;
	next = other.next;
}

template <typename T>
BPTree<T>& BPTree<T>::operator=(const BPTree<T>& rhs)
{
	clear_tree();
	copy_tree(rhs);
	_size = rhs.size();
	dups_ok = rhs.dups_ok;
	return *this;
}

template <typename T>
BPTree<T>::~BPTree()
{
	clear_tree();
}
