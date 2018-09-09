#ifndef __TYPE_TRAITS_H
#define __TYPE_TRAITS_H

typedef struct __true_type
{

}__true_type;

typedef struct __false_type
{

}__false_type;

//这是保守版本，所以定义为false
template<class type>
struct __type_traits
{
	typedef __false_type has_trivial_destructor;
	typedef __false_type is_POD_type;
};

//下面为几个特化版本
template<>
 struct __type_traits<int>
{
	typedef __true_type has_trivial_destructor;
	typedef __true_type is_POD_type;
};




#endif //__TYPE_TRAITS_H