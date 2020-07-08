#pragma once
#include <vector>
#include <iostream>
#include "BPTree.h"
using namespace std;

template <typename K, typename V>
struct MPair {
	K key;
	vector<V> value_list;

	MPair(const K& k = K()) :value_list(), key(k) {}
	MPair(const K& k, const V& v) {
		value_list(1, v);
		key = k;
	}
	MPair(const K& k, const vector<V>& vlist) {
		value_list = vlist;
		key = k;
	}

	friend ostream& operator <<(ostream& outs, const MPair<K, V>& print_me) {
		outs << print_me.key << ' ';
		for (auto it : print_me.value_list) {
			outs << it << ' ';
		}
		outs << endl;
		return outs;
	}

	friend bool operator ==(const MPair<K, V>& lhs, const MPair<K, V>& rhs){ return lhs.key == rhs.key;	}
	friend bool operator < (const MPair<K, V>& lhs, const MPair<K, V>& rhs)	{ return lhs.key < rhs.key;	}
	friend bool operator <= (const MPair<K, V>& lhs, const MPair<K, V>& rhs){ return lhs.key <= rhs.key; }
	friend bool operator > (const MPair<K, V>& lhs, const MPair<K, V>& rhs){return lhs.key > rhs.key;}
	friend MPair<K, V> operator += (MPair<K, V>& lhs, const MPair<K, V>& rhs)
	{
		lhs.value_list.insert(lhs.value_list.end(), rhs.value_list.begin(), rhs.value_list.end());
		return lhs;
	}

	friend MPair<K, V> operator += (MPair<K, V>& lhs, V& rhs)
	{
		lhs.push_back(rhs);
		return lhs;
	}

};

template <typename K, typename V>
class MMap
{
public:
	typedef BPTree<MPair<K, V> > map_base;
	class Iterator {
	public:
		friend class MMap;
		Iterator(typename map_base::Iterator it) :_it(it) {}
		Iterator operator ++(int unused) { 
			++_it; 
			return *this; 
		}
		Iterator& operator ++() { 
			Iterator* hold = this; 
			++_it;
			return *hold;
		}
		MPair<K, V> operator *() { return *_it; }

		friend bool operator ==(const Iterator& lhs, const Iterator& rhs) { return lhs._it == rhs._it; }
		friend bool operator !=(const Iterator& lhs, const Iterator& rhs) { return lhs._it != rhs._it; }
	private:
		typename map_base::Iterator _it;
	};
	MMap();
	MMap(const MMap& other);
	MMap& operator=(const MMap& rhs);

	Iterator lower_bound(const K& key);
	Iterator upper_bound(const K& key);
	Iterator begin();
	Iterator end();
	int count(const K& key);
	const bool contains(const MPair<K, V>& entry) const;
	bool is_valid() { return _map.is_valid(); }

	const vector<V>& operator[](const K& key) const;
	vector<V>& operator[](const K& key);

	void print();

	friend ostream& operator<<(ostream& outs, const MMap<K, V>& print_me) {
		outs << print_me._map << endl;
		return outs;
	}
private:
	map_base _map;
};

template <typename K, typename V>
int MMap<K,V>::count(const K& key)
{
	assert(_map.contains(MPair<K,V>(key)));
	return _map.get(MPair<K, V>(key)).size();
}

template <typename K, typename V>
const bool MMap<K,V>::contains(const MPair<K, V>& entry) const
{
	return _map.contains(entry);
}

template <typename K, typename V>
vector<V>& MMap<K, V>::operator[](const K& key) {
	return _map.get(MPair<K, V>(key)).value_list;
}

template <typename K, typename V>
const vector<V>& MMap<K, V>::operator[](const K& key) const {
	return _map.get(MPair<K, V>(key)).value_list;
}

template <typename K, typename V>
typename MMap<K, V>::Iterator MMap<K, V>::begin()
{
	return MMap<K, V>::Iterator(_map.begin());
}

template <typename K, typename V>
typename MMap<K, V>::Iterator MMap<K, V>::end()
{
	return MMap<K, V>::Iterator(_map.end());
}

template <typename K, typename V>
typename MMap<K, V>::Iterator MMap<K, V>::upper_bound(const K& key)
{
	return _map.upper_bound(MPair<K, V>(key));
}

template <typename K, typename V>
typename MMap<K, V>::Iterator MMap<K, V>::lower_bound(const K& key)
{
	return _map.lower_bound(MPair<K, V>(key));
}

template <typename K, typename V>
MMap<K, V>::MMap() : _map(true) { }

template <typename K, typename V>
MMap<K, V>::MMap(const MMap& other) : _map(other._map) { }

template <typename K, typename V>
MMap<K, V>& MMap<K, V>::operator=(const MMap<K, V>& rhs)
{
	_map = rhs._map;
	return *this;
}

template <typename K, typename V>
void MMap<K, V>::print()
{
	_map.print_tree();
}
