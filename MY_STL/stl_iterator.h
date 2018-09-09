#ifndef __STL_CONFIG_H
# define __STL_CONFIG_H

struct input_iterator_tag{};
struct output_iterator_tag{};
struct forward_iterator_tag :public input_iterator_tag{};
struct bidirectional_iterator_tag :public forward_iterator_tag{};
struct random_access_iterator_tag :public bidirectional_iterator_tag{};

template <class T, class Distance> //����һ��ʲô����
struct ForwardIterator
{
	typedef forward_iterator_tag iterator_category;
	typedef T                    value_type;
	typedef Distance             difference_type;
	typedef T*                   pointer;
	typedef T&                   reference;
};


template <class Iterator>
struct iterator_traits {
	typedef typename Iterator::iterator_category iterator_category;
	typedef typename Iterator::value_type        value_type;
	typedef typename Iterator::difference_type   difference_type;
	typedef typename Iterator::pointer           pointer;
	typedef typename Iterator::reference         reference;
};

//��һ��vector��traits���ػ���

template <class T>
struct iterator_traits<T*>
{
	typedef random_access_iterator_tag iterator_category;
	typedef T                          value_type;
	typedef ptrdiff_t                  difference_type;
	typedef T*                         pointer;
	typedef T&                         reference;
};

template <class Iterator>
inline typename iterator_traits<Iterator>::value_type*  value_type(const Iterator&)
{
	return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
}

#endif //__STL_CONFIG_H;