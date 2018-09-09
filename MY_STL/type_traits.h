#ifndef __TYPE_TRAITS_H
#define __TYPE_TRAITS_H

typedef struct __true_type
{

}__true_type;

typedef struct __false_type
{

}__false_type;

//���Ǳ��ذ汾�����Զ���Ϊfalse
template<class type>
struct __type_traits
{
	typedef __false_type has_trivial_destructor;
	typedef __false_type is_POD_type;
};

//����Ϊ�����ػ��汾
template<>
 struct __type_traits<int>
{
	typedef __true_type has_trivial_destructor;
	typedef __true_type is_POD_type;
};




#endif //__TYPE_TRAITS_H