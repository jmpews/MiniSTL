﻿#pragma once
#include "stl_list_iterator.h"
#include "allocator.h"
#include "uninitialized.h"

namespace MiniSTL {

template<class T, class Alloc = simpleAlloc<T> >
class list {
public:// alias declarations
	using value_type = T;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using reference = value_type&;
	using const_reference = const value_type&;
	using size_type = size_t;
	using difference_type = ptrdiff_t;

	using iterator = __list_iterator<T, T&, T*>;
	using const_iterator = __list_iterator<T, const T&, const T*>;
	using reverse_iterator = MiniSTL::__reverse_iterator<iterator>;
	using const_reverse_iterator = MiniSTL::__reverse_iterator<const_iterator>;

private:// interface about allocate/deallocate litsNode
	using list_node = __list_node<T>;
	using list_node_allocator = simpleAlloc<list_node>;

	list_node* get_node() { return list_node_allocator::allocate(); }
	void put_node(list_node* p) { list_node_allocator::deallocate(p); }
	list_node* create_node(const value_type&);
	void destroy_node(list_node* p) { destroy(&(p->data));put_node(p);}

private:// only data member(tail)
	list_node* node;

private:// aux_interface
	void empety_initialized();
	// Move [first, last) before pos
	void transfer(iterator position, iterator first, iterator last);

public:// ctor && dtor
	list() { empety_initialized(); }
	explicit list(size_type, const value_type& value = value_type());
	list(std::initializer_list<value_type> il) { empety_initialized(); insert(begin(), il.begin(), il.end()); }
	~list() { clear(); put_node(node); }

public:// swap
	void swap(list& rhs) noexcept { MiniSTL::swap(node,rhs.node); }

public:// copy operation
	list(const list&);
	list& operator=(const list&) noexcept;

private:// aux interface for assign
	void fill_assign(size_type,const value_type&);
	template<class Integer>
	void assign_dispatch(Integer n,Integer val,_true_type){
		fill_assign(static_cast<size_type>(n),static_cast<value_type>(val));
	}
	template<class InputIterator>
	void assign_dispatch(InputIterator,InputIterator,_false_type);

public:// assignment
	void assign(size_type n,const value_type& val) { fill_assign(n,val); }
	template<class InputIterator>
	void assign(InputIterator first,InputIterator last){
		assign_dispatch(first,last,_is_integer_t<InputIterator>());
	}
	list& operator=(std::initializer_list<T> ils) { assign(ils.begin(),ils.end()); return *this; }

public:// move operation
	list(list&&) noexcept;
	list& operator=(list&&) noexcept;

public:// getter
	bool empty() const noexcept { return node->next == node; }
	size_type size() const noexcept { return MiniSTL::distance(cbegin(), cend());}
	const_iterator cbegin() const noexcept {return node->next;}
	const_iterator cend() const noexcept  { return node; }
	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }
	const_reference front() const noexcept { return *begin(); }
	const_reference back() const noexcept { return *(--end()); }

public:// setter
	iterator begin() noexcept { return node->next; }
	iterator end() noexcept { return node; }
	reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
	reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
	reference front() noexcept { return *begin(); }
	reference back() noexcept { return *(--end()); }

public:// resize
	void resize(size_type,const value_type& val = value_type());

private:// aux interface for insert
	void fill_insert(iterator, size_type, const value_type&);
	template <class Integer>
	void insert_dispatch(iterator pos,Integer n,Integer val,_true_type){
		fill_insert(pos,static_cast<size_type>(n),static_cast<value_type>(val));
	}
	template <class InputIterator>
	void insert_dispatch(iterator,InputIterator,InputIterator,_false_type);

public:// insert
	iterator insert(iterator, const value_type&);
	template<class InputIterator>
	void insert(iterator pos,InputIterator first, InputIterator last){
		insert_dispatch(pos,first,last,_is_integer_t<InputIterator>());
	}

public:// erase
	iterator erase(iterator);
	iterator erase(iterator, iterator);

public:// push && pop
	void push_front(const T& value) { insert(begin(), value); }
	void push_back(const T& value) { insert(end(), value); }
	void pop_fornt() { erase(begin()); }
	void pop_back() { iterator temp = end();erase(--temp); }

public:// other interface
	void unique();
	void splice(iterator, list&);
	void splice(iterator, list&, iterator);
	void merge(list&);
	void reverse();
	void sort();
	void clear();
	void remove(const T&);
};

template<class T, class Alloc>
inline typename list<T, Alloc>::list_node* list<T, Alloc>::create_node(const T & value){
	list_node* p = get_node();
	try{
		construct(&p->data, value);
	}
	catch(std::exception){
		put_node(p);
		throw;
	}
	return p;
}

template<class T, class Alloc>
inline void list<T, Alloc>::empety_initialized(){
	node = get_node();
	node->next = node;
	node->prev = node;
}

template<class T, class Alloc>
template<class InputIterator>
void list<T, Alloc>::assign_dispatch(InputIterator first,InputIterator last,_false_type){
	iterator start = begin();
	iterator finish = end();
	for(;start != finish && first != last;++start,++finish)
		*start = *first;
	if(first == last)
		erase(start,finish);
	else
		insert(start,first,last);
}

template<class T, class Alloc>
inline void list<T, Alloc>::resize(size_type new_size,const value_type& val){
	iterator cur = begin();
	size_type len = 0;
	for(;cur != end() && len < new_size;++cur,++len);
	if(len == new_size)
		erase(cur,end());
	else // i == end()
		insert(end(),new_size-len,val);
}

template<class T, class Alloc>
inline typename list<T, Alloc>::iterator list<T, Alloc>::insert(iterator position, const value_type& value){
	list_node* temp = create_node(value);
	temp->next = position.node;
	temp->prev = position.node->prev;
	position.node->prev->next = temp;
	position.node->prev = temp;
	return temp;
}

template<class T, class Alloc>
void list<T, Alloc>::fill_insert(iterator position, size_type n, const value_type& value){
	for (size_type i = n; i != 0; --i)
		position = insert(position, value);
}

template<class T, class Alloc>
template<class InputIterator>
void list<T, Alloc>::insert_dispatch(iterator position, InputIterator first, InputIterator last, _false_type){
	for (; first != last; ++first)
		position = insert(position, *first);
}

template<class T, class Alloc>
inline typename list<T, Alloc>::iterator list<T, Alloc>::erase(iterator position){
	list_node* next_node = position.node->next;
	list_node* prev_node = position.node->prev;
	prev_node->next = next_node;
	next_node->prev = prev_node;
	destroy_node(position.node);
	return iterator(next_node);
}

template<class T, class Alloc>
typename list<T, Alloc>::iterator list<T, Alloc>::erase(iterator first, iterator last){
	iterator res;
	while(first!=last)
		res = erase(first++);
	return res;
}

template<class T, class Alloc>
inline void list<T, Alloc>::transfer(iterator position, iterator first, iterator last){
	if (position != last) {
		(*(static_cast<list_node*>((*last.node).prev))).next = position.node;
		(*(static_cast<list_node*>((*first.node).prev))).next = last.node;
		(*(static_cast<list_node*>((*position.node).prev))).next = first.node;
		list_node* temp = static_cast<list_node*>((*position.node).prev);
		(*position.node).prev = (*last.node).prev;
		last.node->prev = first.node->prev;
		first.node->prev = temp;
	}
}

template<class T, class Alloc>
list<T, Alloc>::list(size_type n, const value_type & value){
	empety_initialized();
	while (n--)
		push_back(value);
}

template<class T, class Alloc>
list<T, Alloc>::list(const list &rhs){
	empety_initialized();
	for (auto it = rhs.cbegin(); it != rhs.cend(); ++it)
		push_back(*it);
}

template<class T, class Alloc>
inline list<T, Alloc>& list<T, Alloc>::operator=(const list &rhs) noexcept{
	list temp(rhs);
	swap(temp);
	return *this;
}

template<class T, class Alloc>
inline list<T, Alloc>::list(list &&rhs) noexcept{
	node = rhs.node;
	rhs.node = nullptr;
}

template<class T, class Alloc>
inline list<T, Alloc>& list<T, Alloc>::operator=(list&& rhs) noexcept{
	MiniSTL::swap(node,rhs.node);
	return *this;
}

template<class T, class Alloc>
void list<T, Alloc>::clear() {
	list_node* cur = node->next;
	while (cur != node) {
		list_node* temp = cur;
		cur = cur->next;
		destroy_node(temp);
	}
	node->next = node;
	node->prev = node;
}

template<class T, class Alloc>
void list<T, Alloc>::remove(const T& value) {
	iterator first = begin();
	iterator last = end();
	while (first != last) {
		iterator next = first;
		++next;
		if (*first == value) erase(first);
		first = next;
	}
}

template<class T, class Alloc>
void list<T, Alloc>::unique() {
	iterator first = begin();
	iterator last = end();
	if (first == last) return;
	iterator next = first;
	while (++next != last) {
		if (*first == *next) {
			erase(next);
			next = first;
		}
		else
			first = next;
	}
}

template<class T, class Alloc>
inline void list<T, Alloc>::splice(iterator position, list& x) {
	if (!x.empty())
		transfer(position, x.begin(), x.end());
}

template<class T, class Alloc>
inline void list<T, Alloc>::splice(iterator position, list &, iterator i) {
	iterator j = i;
	++j;
	// i==pos 自身无法插于自身之前
	// j==pos 已处于pos之前
	if (position == i || position == j) return;
	transfer(position, i, j);
}

// need two lists' elements are ordered
template<class T, class Alloc>
void list<T, Alloc>::merge(list& x) {
	iterator first1 = begin();
	iterator last1 = end();
	iterator first2 = x.begin();
	iterator last2 = x.end();

	while (first1 != last1 && first2 != last2) {
		if (*first2 < *first1) {
			iterator next = first2;
			transfer(first1, first2, ++next);
			first2 = next;
		}
		else
			++first1;
	}
	if (first2 != last2) 
		transfer(last1, first2, last2);
}

template<class T, class Alloc>
void list<T, Alloc>::reverse() {
	// empty || size()==1
	if (node->next == node || node->next->next == node) 
		return;
	iterator first = begin();
	++first;// begin自身并不需要移动，它将作为指示末元素的哨兵（确切地说，最终begin.node->next == end.node)
	while (first != end()) {
		iterator old = first;
		++first;
		transfer(begin(), old, first);
	}
}

// More information can be seen at https://blog.csdn.net/qq276592716/article/details/7932483
template<class T, class Alloc>
void list<T, Alloc>::sort() {
	if (node->next == node || node->next->next == node) return;
	// 数据缓存区 counter[n]中最多存放2^(n+1)个元素，若大于则与counter[n+1]作归并
	list carry;
	list counter[64];
	int fill = 0;
	while (!empty()) {
		carry.splice(carry.begin(), *this, begin());
		int i = 0;
		while (i < fill && !counter[i].empty()) {
			counter[i].merge(carry);
			carry.swap(counter[i++]);
		}
		carry.swap(counter[i]);
		if (i == fill)
			++fill;
	}
	for (int i = 1; i < fill; ++i)
		counter[i].merge(counter[i - 1]);
	swap(counter[fill - 1]);
}

template<class T>
inline void swap(list<T>& lhs, list<T>& rhs){
	lhs.swap(rhs);
}

template<class T>
bool operator==(const list<T>& lhs, const list<T>& rhs){
	auto it1 = lhs.cbegin(), it2 = rhs.cbegin();
	for (; it1 != lhs.cend() && it2 != rhs.cend(); ++it1, ++it2)
		if (*it1 != *it2) return false;
	return it1 == lhs.cend() && it2 == rhs.cend();
}

template<class T>
inline bool operator!=(const list<T>& lhs, const list<T>& rhs){
	return !(lhs == rhs);
}

}// end namespace::MiniSTL
