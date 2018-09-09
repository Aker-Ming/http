#ifndef __SGI_STL_INTERNAL_UNINITIALIZED_H
#define __SGI_STL_INTERNAL_UNINITIALIZED_H

template<class ForwordIterator  ,class Size,class T>
inline ForwordIterator uninitialized_fill_n(ForwordIterator first, Size n, const T& x)
{
	return __uninitialized_fill_n(first, n, x, value_type(first));//������
}

template<class ForwardIterator ,class Size ,class T,class T1>
inline ForwardIterator __uninitialized_fill_n(ForwardIterator first, Size  n, const T& x, T1*)
{
	typedef typename __type_traits<T1>::is_POD_type is_POD;//Ϊɶ�ǵ���T1����T��������
	return __uninitialized_fill_n_aux(first, n, x, is_POD());
}

template<class ForwardIterator, class Size, class T>
//�������ͣ����ø�Ч��䷽ʽ
inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size  n, const T& x,
																				__true_type)
{
	return fill_n(first, n, x);
}

template<class ForwardIterator, class Size, class T>
//�Զ������ͣ�����������䷽ʽ
inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size  n, 
														const T& x, __false_type)
{
	ForwardIterator cur = first;
	for (; n > 0; n--,++cur)
	{
		construct(&*cur, x);//??�Ƚ�������ȡ��ַ��ʲô��˼��
	}
	return cur;
}





#endif //__SGI_STL_INTERNAL_UNINITIALIZED_H