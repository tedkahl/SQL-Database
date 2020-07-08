// BTree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "BPTree.h"
#include "arrayfuncs.h"
#include <string>
#include <assert.h>
#include "Map.h"
#include "MMap.h"
#include "random.h"
using namespace std;

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
void BPTree<T>::remove(const T& entry)
{
	loose_remove(entry);
	if (data_count == 0) {
		if (child_count == 1) {
			BPTree<T>* temp = subset[0];
			copy_array(subset[0]->data, subset[0]->data_count, data, data_count);
			copy_array(subset[0]->subset, subset[0]->child_count, subset, child_count);
			temp->child_count = 0;
			delete temp;
		}
	}
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

//precondition: entry is the value to be replaced by smallest (can't be temp variable)
template<typename T>
void BPTree<T>::get_smallest_nonequal(T& entry)
{
	BPTree<T>* s = get_smallest_node();
	if (s->data[0] != entry) entry = s->data[0]; //if smallest value is nonequal, behave like get_smallest
	else {
		BPTree<T>::Iterator temp(s, 0); //else use the next value, which is the correct smallest value as 
									  //the equal value will be removed							
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

template <typename T>
void BPTree<T>::loose_remove(const T& entry)
{
	T deleted;
	int index = first_ge(data, data_count, entry);
	bool found = (data[index] == entry && index < data_count);

	if (is_leaf()) {
		if (found) delete_item(data, index, data_count, deleted); //case 2 leaf + found
		return;	//case 1 leaf + not found
	}

	else if (!found) {
		subset[index]->loose_remove(entry); //case 3 not leaf + not found
		if (subset[index]->data_count < MINIMUM) fix_shortage(index);
	}

	else {										//case 4 not leaf + found
		subset[index + 1]->get_smallest_nonequal(data[index]);
		subset[index + 1]->loose_remove(entry);
		if (subset[index + 1]->data_count < MINIMUM) fix_shortage(index + 1);
	}
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
	return is_valid(temp, temp2);
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
		if (temp != data[i]) return false;
		if ((data[i] <= last)) last = data[i];
		else return false;
		if (!subset[i]->is_valid(last, check)) return false;
	}
	return true;
}

template <typename T>
int BPTree<T>::size() const
{
	Iterator i = begin();
	++i;
	int count = 1;
	while (!i.is_null())
	{
		++i;
		++count;
	}

	return count;
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
	if (is_leaf()) return Iterator(this, data_count - 1);
	else return subset[data_count]->end();
}

template <typename T>
bool BPTree<T>::is_valid_leaves()
{
	Iterator i = begin();
	T last = *i;
	++i;
	while (!i.is_null())
	{
		if (*i > last) last = *i;
		else return false;
		++i;
	}

	return true;
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
#ifdef DEBUG
	cout << "at node " << data[0] << " fixing subset" << i << endl;
#endif
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

template <typename T>
void BPTree<T>::insert(const T& entry)
{
	loose_insert(entry);
	if (data_count > MAXIMUM)
	{
#ifdef DEBUG
		cout << "Before adding new root" << endl;
		print_debug();
		cout << "Adding new root" << endl;
#endif
		BPTree* temp = new BPTree();

		copy_array(data, data_count, temp->data, temp->data_count);
		copy_array(subset, child_count, temp->subset, temp->child_count);
		data_count = 0;
		child_count = 1;
		subset[0] = temp;
		fix_excess(0);
	}
}

template <typename T>
void BPTree<T>::loose_insert(const T& entry)              //allows MAXIMUM+1 data elements in the root
{
	int index = first_ge(data, data_count, entry);
	bool found = (data[index] == entry && index < data_count);

	if (found) {
		if (is_leaf()) {
			if (!(data[index] == entry) || dups_ok) data[index] += entry;
		}
		else {
			subset[index + 1]->loose_insert(entry);//item found not node, recursive call i+1
			if (subset[index + 1]->data_count > MAXIMUM) fix_excess(index + 1);
		}
	}

	else {
		if (is_leaf()) {
			ordered_insert(data, data_count, entry);
		}
		else {
			subset[index]->loose_insert(entry);
			if (subset[index]->data_count > MAXIMUM) fix_excess(index);
		}
	}
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
BPTree<T>::BPTree(bool dups) :dups_ok(dups), next(NULL), data_count(0), child_count(0) {}

template <typename T>
BPTree<T>::BPTree(const BPTree<T>& other)
{
	copy_tree(other);
	dups_ok = other.dups_ok;
	next = other.next;
}

template <typename T>
BPTree<T>& BPTree<T>::operator=(const BPTree<T>& rhs)
{
	clear_tree();
	copy_tree(rhs);
	dups_ok = rhs.dups_ok;
	return *this;
}

template <typename T>
BPTree<T>::~BPTree()
{
	clear_tree();
}


