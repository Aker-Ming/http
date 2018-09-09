#ifndef __SGI_STL_INTERNAL_CONSTRUCT_H
#define __SGI_STL_INTERNAL_CONSTRUCT_H

#include<new.h>
#include"type_traits.h"




template<class T1,class T2>
inline void construct(T1*p, const T2& value)
{
	new(p)T1(value);
}

template<class ForwordIterator>
inline void destory(ForwordIterator first, ForwordIterator last)
{
	__destory(first, last, value_type(first));//
}

template<class ForwordIterator,class T>
inline void __destory(ForwordIterator first, ForwordIterator last, T*)
{
	typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
	__destory_aux(first, last, trivial_destructor());
}


template<class ForwordIterator,class T>
inline void __destory_aux(ForwordIterator first, ForwordIterator last, __false_type)
{
	for (; first < last; ++first)
	{
		destory(&*first);
	}
}

template<class T>
void destroy(T* pointer)
{
	pointer->~T();//这个自定义类型有自己的析构函数
}

template<class ForwordIterator>
inline void __destory_aux(ForwordIterator first, ForwordIterator last, __true_type)
{
	//不是自定义类型就啥都不做，直接释放
}

#endif //__SGI_STL_INTERNAL_CONSTRUCT_H