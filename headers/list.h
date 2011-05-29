#ifndef LIST_H_
#define LIST_H_
#include <stdlib.h>
#include <iostream>

#define const_for(list, it) for (auto it=list->begin(); it<list->end(); it++)

#define list vector

template <class T>
class list
{
	public:
		typedef const T* const_iterator;
		typedef T* iterator;

		list();
		~list();

		void push_back(const T &item);
		void erase(T* item);
		void clear();
		T& at(long i);
		T* begin();
		T* end();
		long size();
		long size_safe();

	private:
		long size_;
		long maxsize;
		T* begin_;
		T* end_;

		void realloc_();
};

template <class T> inline
list<T>::list()
{
	maxsize = 4;
	begin_ = (T*)malloc(maxsize*sizeof(T));
	size_ = 0;
	end_ = begin_;
}

template <class T> inline
list<T>::~list() { free(begin_); }

template <class T> inline
void list<T>::push_back(const T &item)
{
	if ( size_ == maxsize ) { realloc_(); }
	*end_ = item;
	size_++;
	end_++;
}

template <class T> inline
void list<T>::erase(T* item)
{
	size_--;
	end_--;
	*item = *end_;
	return;
}

template <class T> inline
void list<T>::clear() { size_=0; end_ = begin_; }

template <class T> inline
T& list<T>::at(long i) { return begin_[i]; }

template <class T> inline
T* list<T>::begin() { return begin_; }

template <class T> inline
T* list<T>::end() { return end_; }

template <class T> inline
long list<T>::size() { return size_; }

template <class T> inline
long list<T>::size_safe() { return this ? size_ : 0; }


template <class T>
void list<T>::realloc_()
{
		maxsize*= 2;
		begin_ = (T*)realloc(begin_, maxsize*sizeof(T));
		end_ = begin_ + size_;
}

#undef list

#endif /*LIST_H_*/

