#ifndef __SGI_STL_INTERNAL_VECTOR_H
#define __SGI_STL_INTERNAL_VECTOR_H

#include"stl_alloc.h"

template<class T, class Alloc = default_malloc_alloc<false,0>>
class vector
{
public:
	//�Ƚ������Ĺ����������ܣ��ռ�������ͷ� �����������ܸ���
	typedef T value_type;
	typedef T* iterator;//������

	//���캯��
	typedef simple_alloc<value_type, Alloc> data_allocator;
	iterator start;
	iterator finish;
	iterator end_of_storage;


	//���캯��
	vector() :start(0), finish(0), end_of_storage(0){}

	vector(size_t n, const T& value)
	{
		fill_initialized(n, value);
	}
	void fill_initialized(size_t n, const T& value)
	{
		start = allocate_and_fill(n, value);
		finish = start + n;
		end_of_storage = finish;
	}
	iterator allocate_and_fill(size_t n, const T& x)
	{
		iterator result = data_allocator::allocate(n);//����ռ�
		uninitialized_fill_n(result, n, x);//���ռ�
		return result;
	}

	//��������
	~vector() 
	{
		destory(start, finish);
		deallocate();
	}

	void deallocate()
	{
		if (start)
			data_allocator::deallocate(start, end_of_storage - start);
	}


};
#endif //__SGI_STL_INTERNAL_VECTOR_H
