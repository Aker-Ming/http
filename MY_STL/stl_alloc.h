
#ifndef __SGI_STL_INTERNAL_ALLOC_H
#define __SGI_STL_INTERNAL_ALLOC_H


#include<malloc.h>



//一级空间配置器
template<int inst>
class _malloc_alloc_template
{
private:
	//一下函数用来处理内存不足的情况
    // oom:out of memory
	static void *oom_malloc(size_t);
	static void *oom_realloc(size_t);
	static void(*__malloc_alloc_oom_handle)();//定义应对空间不足的函数

public:
	static void * allocate(size_t n)
	{
		void *result = malloc(n);//一级空间配置器直接用malloc
		if (0 == result)//申请失败，调用oom函数去寻找空间
		{
			result = oom_malloc(n);
		}
		return result;
	}
	 
	static void deallocate(void *p, size_t /* n */)//??
	{
		free(p);//一级空间配置器直接用free释放
	}

};

template<int inst>//一开始给函数指针赋0，用户应该自己设计应对函数
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
			//__THROW_BAD_ALLOC;//没有设置应对函数，所以直接抛异常
		}
		(*my_malloc_headle)();//设置了应对函数，执行以下
		result = malloc(n);//再去尝试申请空间
		if (result)//申请到了就返回值为指针，没有就接着去执行应对函数
			return result;
	}
}


//封装类

template<class T, class Alloc>
class simple_alloc
{
public:
	static T * allocate(size_t n)
	{
		//有得用户会传0
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
//typedef  default_malloc_alloc<false,0> alloc;//那么第一次就会调二级空间配置器
typedef  _malloc_alloc_template<0> malloc_alloc;
//typedef default_malloc_alloc<false, 0> alloc;

//二级 空间配置器

enum {__ALIGN=8};
enum {__MAX_BTYES=128};
enum { __NFREELISTS = __MAX_BTYES / __ALIGN };//自由链表的个数

template<bool threads,int inst>
class default_malloc_alloc
{
private:
	static size_t ROUND_UP(size_t bytes)//将字节数向上对齐到8的整数倍
	{
		return (((bytes)+__ALIGN - 1)& ~(__ALIGN - 1));
	}

private:
	volatile union obj //自由链表的节点
	{
		union obj* free_list_link;
		char client_data[1];
	};

private:
	static obj* volatile free_list[__NFREELISTS];//哈希表数组
	//判断桶号
	static size_t FREELIST_INDEX(size_t bytes)
	{
		return (((bytes)+__ALIGN - 1) / __ALIGN - 1);
	}
	//给内存池将申请 到的空间，挂到链表上的函数
	static void * refill(size_t n);

	//给内存池申请空间的函数
	static  char* chunk_alloc(size_t size, int & nobjs);

	static char *start_free;  //内存池的开始位置
	static char *end_free;    //内存池的结束位置
	static size_t heap_size;

public:
	//空间配置函数
	
	 static void* allocate(size_t n)
	{
		obj* volatile   *    my_free_list;//创建一个指向节点指针的指针
		obj* result;

		//大于128就调用一级空间配置器

		if (n > (size_t)__MAX_BTYES)
		{
			return (malloc_alloc::allocate(n));
		}
		//寻找16个free_list 中合适的一个
		my_free_list = free_list + FREELIST_INDEX(n);//free也是一个指向联合体指针的指针
		result = *my_free_list;
		if (result == 0)
		{
			void * r = refill(ROUND_UP(n));
			return r;
		}
		//将空间给出去之前，先将空间的下一个节点指针，放到桶头
		*my_free_list = result->free_list_link;
		return (result);
	}

	 //空间释放函数
	 static void deallocate(void *p, size_t n)
	 {
		obj* volatile   *    my_free_list;//创建一个指向节点指针的指针
		 obj *q = (obj*)p;

		 //大于128字节就调用一级空间配置器的销毁函数
		 if (n > (size_t)__MAX_BTYES)
		 {
			 malloc_alloc::deallocate(p, n);
			 return;
		 }
		 //寻找对应的自由链表
		 my_free_list = free_list + FREELIST_INDEX(n);
		 //调整自由链表，回收区块
		 q->free_list_link = *my_free_list;
		 *my_free_list = q;

	 }

	static void* reallocate(void*p, size_t old_sz, size_t new_sz);
};

//对成员变量 进行初始化    思考：private成员在类外初始化 

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






//填充函数
template<bool threads, int inst>
void*  default_malloc_alloc<threads, inst>::refill(size_t n)
{
	int nobjs = 20;
	char * chunk=chunk_alloc(n, nobjs);//将来会用引用来接受，所以会改变nobjs的值
	obj*  volatile * my_free_list;
	obj* result;
	obj* current_obj, *next_obj;
	int i;
	if (1 == nobjs)//内存池只给了1个空间，那就直接给用户
		return chunk;

	//调整子自由链表，纳入新的节点
	my_free_list = free_list + FREELIST_INDEX(n);//计算链表位置

	result = (obj*)chunk;//准备一块返回的空间
	//将其他空间添加到自由链表中
	*my_free_list = next_obj = (obj*)(chunk + n);//第一节点已被分配出去，+n从第二节点开始往链表上挂

	//将新的链表节点往上挂
	for (int i = 1;; i++)//只能挂nobjs-1个节点了
	{
		current_obj = next_obj;
		next_obj = (obj*)((char*)next_obj + n);
		if (nobjs - 1 == i)//因为for循环外已经挂了一次，所以到这里时，不能挂了，让当前节点指向空
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



//从内存池中申请的函数
template<bool threads,int inst>
char*  default_malloc_alloc<threads, inst>::chunk_alloc(size_t size ,int& nobjs)
{
	char * result;
	size_t total_bytes = nobjs*size;//申请的空间
	size_t bytes_left = end_free - start_free;//内存池中剩余空间

	//内存池空间足够
	if (bytes_left >= total_bytes)
	{
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	//空间不足，但是足够一个以上节点的大小
	else if (bytes_left >= size)
	{
		nobjs = bytes_left / size;
		total_bytes = size*nobjs;
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	//连一块空间都没有
	else
	{
		size_t  bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
		//将内存池中残余的零头挂到自由链表上
		if (bytes_left > 0)
		{
			obj* volatile* my_free_list = free_list + FREELIST_INDEX(bytes_left);//找桶号
			((obj*)start_free)->free_list_link = *my_free_list;
			*my_free_list = (obj*)start_free;
			//好像并没有将start_free向后走
		}


		//到这里，就只能去堆上寻找空间了
		start_free = (char*)malloc(bytes_to_get);
		if (0 == start_free)
		{
			int i;
			obj* volatile* my_free_list, *p;//p 的类型是obj*
			//空间申请失败
			//在自由链表中找那些，未用的区块，而且足够大的
			for (int i = size; i <= __MAX_BTYES; i += __ALIGN)
			{
				my_free_list = free_list + FREELIST_INDEX(i);//找桶号
				p = *my_free_list;
				if (0 != p)
				{
					//有可用的链表节点,把可用链表节点放回到内存池
					*my_free_list = p->free_list_link;
					start_free = (char*)p;
					end_free = start_free + i;
					//这下内存池有空间了，再调用chunk_alloc就会成功
					return (chunk_alloc(size, nobjs));

				}

			}//end for

			end_free = 0;//现在start_free==0,end_free没变，这是一段很大的空间，
			//一旦malloc抛异常，被捕获，并且重新chunk_alloc，就会出现错乱
			//去找一级空间配置器，这里有空间不足的应对方法
			start_free = (char*)malloc_alloc::allocate(bytes_to_get);//可能会抛异常
			heap_size += bytes_to_get;
			end_free = start_free + bytes_to_get;
			//重新调用自己
			

		}//end if
		return (chunk_alloc(size, nobjs));
	}//end else
}

#endif //__SGI_STL_INTERNAL_ALLOC_H