#pragma once
#include "pch.h"
#include "BPTree.h"

using namespace std;

template<typename K, typename V>
class Pair
{
public:
	K _key;
	V _value;
	
	Pair(const K& k = K(), const V& v = V()) {
		_key = k;
		_value = v;
	}

	friend ostream& operator<<(ostream& outs, const Pair& print_me)
	{
		outs << print_me._key;// << " " << print_me._value;
		return outs;
	}

	Pair& operator+=(const Pair<K, V> rhs)
	{
		_key = rhs._key; //maybe not needed?
		_value = rhs._value;
		return *this;
	}

	bool operator==(const Pair<K, V> rhs) const	{ return _key == rhs._key; }
	bool operator<=(const Pair<K, V> rhs) const	{ return _key <= rhs._key; }
	bool operator>(const Pair<K, V> rhs) const { return _key > rhs._key; }
	bool operator<(const Pair<K, V> rhs) const { return _key < rhs._key; }
};

template <typename K, typename V>
class Map
{
public:
	typedef BPTree<Pair<K, V> > map_base;
	class Iterator {
	public:
		friend class Map;
		Iterator(typename map_base::Iterator it) :_it(it) {}
		Iterator operator ++(int unused) { return ++_it; }
		Iterator& operator ++() { return _it++; }
		Pair<K, V> operator *() { return *_it; }
		friend bool operator ==(const Iterator& lhs, const Iterator& rhs) { return lhs._it == rhs._it; }
		friend bool operator !=(const Iterator& lhs, const Iterator& rhs) { return lhs._it != rhs._it; }
	private:
		typename map_base::Iterator _it;
	};

	Map();
	Map(const Map& other);
	Map& operator= (const Map& other);
	void print_debug();
	V& operator[](const K& key);
	const V& operator[](const K& key) const;
	const bool contains(const Pair<K,V>& entry) const; 
	bool is_valid() { return _map.is_valid(); }
	
	vector<V> vectorize() //be careful with this
	{
		vector<V> v;
		for (Iterator i = begin(); i != end(); i++) v.push_back((*i)._value);
		return v;
	}

	void insert(const K& k, const V& v);
	void erase(const K& key);
	void clear();
	V get(const K& key);
	int count(const K& key);

	Iterator lower_bound(const K& key);
	Iterator upper_bound(const K& key);
	Iterator begin();
	Iterator end();

	int size() const { return _map.size(); }
	bool empty() const;

	friend ostream& operator<<(ostream& outs, const Map<K, V>& print_me) {
		outs << print_me._map << endl;
		return outs;
	}

private:
	map_base _map;
	int key_count;
};

template <typename K, typename V>
int Map<K,V>::count(const K& entry)
{
	return (int)(_map.contains(entry));
}

template <typename K, typename V>
typename Map<K,V>::Iterator Map<K, V>::begin() 
{
	return Map<K, V>::Iterator(_map.begin());
}

template <typename K, typename V>
typename Map<K, V>::Iterator Map<K, V>::end()
{
	return Map<K, V>::Iterator(_map.end());
}

template <typename K, typename V>
typename Map<K, V>::Iterator Map<K, V>::upper_bound(const K& key)
{
	return _map.upper_bound(Pair<K, V>(key));
}

template <typename K, typename V>
typename Map<K, V>::Iterator Map<K, V>::lower_bound(const K& key)
{
	return _map.lower_bound(Pair<K,V>(key));
}

template <typename K, typename V>
bool Map<K, V>::empty() const
{
	return _map.size() == 0;
}

template <typename K, typename V>
void Map<K, V>::insert(const K& k, const V& v) 
{
	if (_map.insert(Pair<K, V>(k, v)) == 1) key_count++;
}

template <typename K, typename V>
void Map<K, V>::erase(const K& key) 
{
	if (_map.remove(Pair<K,V>(key)) == 1) key_count--;
}

template <typename K, typename V>
void Map<K, V>::clear() 
{
	_map.clear_tree();
}

template <typename K, typename V>
const bool Map<K, V>::contains(const Pair<K,V>& entry) const
{
	return _map.contains(entry);
}

template <typename K, typename V>
void Map<K, V>::print_debug()
{
	_map.print_debug();
}

template <typename K, typename V>
V Map<K, V>::get(const K& key)
{
	assert(_map.contains(key));
	return _map.get(Pair<K,V>(key))._value;
}

template <typename K, typename V>
V& Map<K, V>::operator[] (const K& key)
{
	return _map.get(Pair<K, V>(key))._value;
	key_count = _map.size();
}

template <typename K, typename V>
const V& Map<K, V>::operator[] (const K& key) const
{
	return _map.get(Pair<K, V>(key))._value;
}

template<typename K, typename V>
Map<K, V>::Map() : _map(false) { }

template<typename K, typename V>
Map<K, V>::Map(const Map& other) : _map(other._map) { }

template<typename K, typename V>
Map<K, V>& Map<K, V>::operator=(const Map<K, V>& rhs)
{
	_map = rhs._map;
	return *this;
}