
#ifndef __SGI_STL_INTERNAL_ALLOC_H
#define __SGI_STL_INTERNAL_ALLOC_H


#include<malloc.h>



//һ���ռ�������
template<int inst>
class _malloc_alloc_template
{
private:
	//һ�º������������ڴ治������
    // oom:out of memory
	static void *oom_malloc(size_t);
	static void *oom_realloc(size_t);
	static void(*__malloc_alloc_oom_handle)();//����Ӧ�Կռ䲻��ĺ���

public:
	static void * allocate(size_t n)
	{
		void *result = malloc(n);//һ���ռ�������ֱ����malloc
		if (0 == result)//����ʧ�ܣ�����oom����ȥѰ�ҿռ�
		{
			result = oom_malloc(n);
		}
		return result;
	}
	 
	static void deallocate(void *p, size_t /* n */)//??
	{
		free(p);//һ���ռ�������ֱ����free�ͷ�
	}

};

template<int inst>//һ��ʼ������ָ�븳0���û�Ӧ���Լ����Ӧ�Ժ���
void(*_malloc_alloc_template<inst>::__malloc_alloc_oom_handle)() = 0;

template<int inst>
void * _malloc_alloc_template<inst>::oom_malloc(size_t n)
{
	void(*my_malloc_headle)();
	void* result;
	for (;;)
	{
		my_malloc_headle = __malloc_alloc_oom_handle;
		if (my_malloc_headle == 0)
		{
			//__THROW_BAD_ALLOC;//û������Ӧ�Ժ���������ֱ�����쳣
		}
		(*my_malloc_headle)();//������Ӧ�Ժ�����ִ������
		result = malloc(n);//��ȥ��������ռ�
		if (result)//���뵽�˾ͷ���ֵΪָ�룬û�оͽ���ȥִ��Ӧ�Ժ���
			return result;
	}
}


//��װ��

template<class T, class Alloc>
class simple_alloc
{
public:
	static T * allocate(size_t n)
	{
		//�е��û��ᴫ0
		return 0 == n ? 0 : (T*)Alloc::allocate(n*sizeof(T));
	}

	static T *allocate()
	{
		return  Alloc::allocate(sizeof(T));
	}

	static void deallocate(T* p, size_t n)
	{
		if (n != 0)
		{
			Alloc::deallocate(p, n* sizeof(T));
		}
	}

	static void deallocate(T* p)
	{
		Alloc::deallocate(p, sizeof(T));
	}

};



//template<bool threads, int inst>
//typedef  default_malloc_alloc<false,0> alloc;//��ô��һ�ξͻ�������ռ�������
typedef  _malloc_alloc_template<0> malloc_alloc;
//typedef default_malloc_alloc<false, 0> alloc;

//���� �ռ�������

enum {__ALIGN=8};
enum {__MAX_BTYES=128};
enum { __NFREELISTS = __MAX_BTYES / __ALIGN };//��������ĸ���

template<bool threads,int inst>
class default_malloc_alloc
{
private:
	static size_t ROUND_UP(size_t bytes)//���ֽ������϶��뵽8��������
	{
		return (((bytes)+__ALIGN - 1)& ~(__ALIGN - 1));
	}

private:
	volatile union obj //��������Ľڵ�
	{
		union obj* free_list_link;
		char client_data[1];
	};

private:
	static obj* volatile free_list[__NFREELISTS];//��ϣ������
	//�ж�Ͱ��
	static size_t FREELIST_INDEX(size_t bytes)
	{
		return (((bytes)+__ALIGN - 1) / __ALIGN - 1);
	}
	//���ڴ�ؽ����� ���Ŀռ䣬�ҵ������ϵĺ���
	static void * refill(size_t n);

	//���ڴ������ռ�ĺ���
	static  char* chunk_alloc(size_t size, int & nobjs);

	static char *start_free;  //�ڴ�صĿ�ʼλ��
	static char *end_free;    //�ڴ�صĽ���λ��
	static size_t heap_size;

public:
	//�ռ����ú���
	
	 static void* allocate(size_t n)
	{
		obj* volatile   *    my_free_list;//����һ��ָ��ڵ�ָ���ָ��
		obj* result;

		//����128�͵���һ���ռ�������

		if (n > (size_t)__MAX_BTYES)
		{
			return (malloc_alloc::allocate(n));
		}
		//Ѱ��16��free_list �к��ʵ�һ��
		my_free_list = free_list + FREELIST_INDEX(n);//freeҲ��һ��ָ��������ָ���ָ��
		result = *my_free_list;
		if (result == 0)
		{
			void * r = refill(ROUND_UP(n));
			return r;
		}
		//���ռ����ȥ֮ǰ���Ƚ��ռ����һ���ڵ�ָ�룬�ŵ�Ͱͷ
		*my_free_list = result->free_list_link;
		return (result);
	}

	 //�ռ��ͷź���
	 static void deallocate(void *p, size_t n)
	 {
		obj* volatile   *    my_free_list;//����һ��ָ��ڵ�ָ���ָ��
		 obj *q = (obj*)p;

		 //����128�ֽھ͵���һ���ռ������������ٺ���
		 if (n > (size_t)__MAX_BTYES)
		 {
			 malloc_alloc::deallocate(p, n);
			 return;
		 }
		 //Ѱ�Ҷ�Ӧ����������
		 my_free_list = free_list + FREELIST_INDEX(n);
		 //��������������������
		 q->free_list_link = *my_free_list;
		 *my_free_list = q;

	 }

	static void* reallocate(void*p, size_t old_sz, size_t new_sz);
};

//�Գ�Ա���� ���г�ʼ��    ˼����private��Ա�������ʼ�� 

template<bool threads, int inst>
char* default_malloc_alloc<threads, inst>::start_free = 0;


template<bool threads, int inst>
char* default_malloc_alloc<threads, inst>::end_free= 0;

template<bool threads, int inst>
size_t default_malloc_alloc<threads, inst>::heap_size = 0;



template <bool threads, int inst>
typename  default_malloc_alloc<threads, inst>::obj * volatile
  default_malloc_alloc<threads, inst>::free_list
[__NFREELISTS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };






//��亯��
template<bool threads, int inst>
void*  default_malloc_alloc<threads, inst>::refill(size_t n)
{
	int nobjs = 20;
	char * chunk=chunk_alloc(n, nobjs);//�����������������ܣ����Ի�ı�nobjs��ֵ
	obj*  volatile * my_free_list;
	obj* result;
	obj* current_obj, *next_obj;
	int i;
	if (1 == nobjs)//�ڴ��ֻ����1���ռ䣬�Ǿ�ֱ�Ӹ��û�
		return chunk;

	//�������������������µĽڵ�
	my_free_list = free_list + FREELIST_INDEX(n);//��������λ��

	result = (obj*)chunk;//׼��һ�鷵�صĿռ�
	//�������ռ���ӵ�����������
	*my_free_list = next_obj = (obj*)(chunk + n);//��һ�ڵ��ѱ������ȥ��+n�ӵڶ��ڵ㿪ʼ�������Ϲ�

	//���µ�����ڵ����Ϲ�
	for (int i = 1;; i++)//ֻ�ܹ�nobjs-1���ڵ���
	{
		current_obj = next_obj;
		next_obj = (obj*)((char*)next_obj + n);
		if (nobjs - 1 == i)//��Ϊforѭ�����Ѿ�����һ�Σ����Ե�����ʱ�����ܹ��ˣ��õ�ǰ�ڵ�ָ���
		{
			current_obj->free_list_link = 0;
			break;
		}
		else
		{
			current_obj->free_list_link = next_obj;
		}

	}
	return result;
}



//���ڴ��������ĺ���
template<bool threads,int inst>
char*  default_malloc_alloc<threads, inst>::chunk_alloc(size_t size ,int& nobjs)
{
	char * result;
	size_t total_bytes = nobjs*size;//����Ŀռ�
	size_t bytes_left = end_free - start_free;//�ڴ����ʣ��ռ�

	//�ڴ�ؿռ��㹻
	if (bytes_left >= total_bytes)
	{
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	//�ռ䲻�㣬�����㹻һ�����Ͻڵ�Ĵ�С
	else if (bytes_left >= size)
	{
		nobjs = bytes_left / size;
		total_bytes = size*nobjs;
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	//��һ��ռ䶼û��
	else
	{
		size_t  bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
		//���ڴ���в������ͷ�ҵ�����������
		if (bytes_left > 0)
		{
			obj* volatile* my_free_list = free_list + FREELIST_INDEX(bytes_left);//��Ͱ��
			((obj*)start_free)->free_list_link = *my_free_list;
			*my_free_list = (obj*)start_free;
			//����û�н�start_free�����
		}


		//�������ֻ��ȥ����Ѱ�ҿռ���
		start_free = (char*)malloc(bytes_to_get);
		if (0 == start_free)
		{
			int i;
			obj* volatile* my_free_list, *p;//p ��������obj*
			//�ռ�����ʧ��
			//����������������Щ��δ�õ����飬�����㹻���
			for (int i = size; i <= __MAX_BTYES; i += __ALIGN)
			{
				my_free_list = free_list + FREELIST_INDEX(i);//��Ͱ��
				p = *my_free_list;
				if (0 != p)
				{
					//�п��õ�����ڵ�,�ѿ�������ڵ�Żص��ڴ��
					*my_free_list = p->free_list_link;
					start_free = (char*)p;
					end_free = start_free + i;
					//�����ڴ���пռ��ˣ��ٵ���chunk_alloc�ͻ�ɹ�
					return (chunk_alloc(size, nobjs));

				}

			}//end for

			end_free = 0;//����start_free==0,end_freeû�䣬����һ�κܴ�Ŀռ䣬
			//һ��malloc���쳣�������񣬲�������chunk_alloc���ͻ���ִ���
			//ȥ��һ���ռ��������������пռ䲻���Ӧ�Է���
			start_free = (char*)malloc_alloc::allocate(bytes_to_get);//���ܻ����쳣
			heap_size += bytes_to_get;
			end_free = start_free + bytes_to_get;
			//���µ����Լ�
			

		}//end if
		return (chunk_alloc(size, nobjs));
	}//end else
}

#endif //__SGI_STL_INTERNAL_ALLOC_H