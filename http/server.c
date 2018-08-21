#include <sys/wait.h>

#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include "server.h"

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

//一次从socket中读取一行数据
//把数据放到buf缓冲区中
//读取失败就返回-1
//\n \r  \r\n 三种情况
int ReadLine(int sock,char  buf[],ssize_t size)
{
 //1，从socket中一次读一个字符
  char c='\0';
  ssize_t i=0;
  //结束条件：
  //1，读的长度太长
  //2，读到‘\n’ 此处我要兼容 \r \r\n的情况
  //   遇到\r 或者 \r\n想办法转换成 \n
 while(i<size-1 && c!='\n')
 {
   ssize_t read_size=recv(sock,&c,1,0);
   if(read_size<0)
   {
    return -1;
   }
   if(read_size==0)
   {
   //预期是要读到 \n 这样的换行符，结果还没读到，
   //就先读到了 EOF ,这样的情况我们也暂时认为是失败的
    return -1;
   }
   if(c == '\r')
   {
     //当前遇到了\r ,但是还需要确定下一个字符是不是\n
	 //MSG_PEEK 选项从内核的缓冲区中读数据
	 //但是读到的数据不会从缓冲区中删除点
	 recv(sock,&c,1,MSG_PEEK);
	 if(c=='\n')
	 {
	   //此时整个分隔符就是 \r\n
	   recv(sock,&c,1,0);
	 }
	 else
	 {
	 //当前分隔符就是 \r ,此时把分隔符转换成 \n
	 c='\n';
	 }
   }

 //只要上面 c 读到的是 \r ,那么 if 结束后，c都变成了 \n
 //这种方式就是把 \r \r\n 都统一成了 \n
 buf[i++]=c;
 }
 buf[i]='\0';
 return i;//真正往缓冲区中放置的字符个数
}

int Split(char input[],char *split_char,char * output[],int output_size)
{
   //使用strtok
   //因为strtok函数使用过静态全局变量来保存，上一次分割的终点
   //所以strtok是线程不安全函数
   //使用strtok_代替strtok
   
   int i=0;
   char * pch;
   char * tmp=NULL;
   pch=strtok_r(input,split_char,&tmp);
   while(pch!=NULL)
   {
   if(i>=output_size)
   {
    return i;
   }
     output[i++]=pch;
	 pch=strtok_r(NULL,split_char,&tmp);
   }
   return i;
}

//解析首行函数
//为什么传二级指针，想在内部函数改变变量 
int ParseFirstLine(char first_line[],char **p_url,char** p_method)
{
  //把首行按照空格进行字符串切分
  char *tok[10];
  //把first_line按照空格进行字符串切分
  //切分得到的每一个部分，就放到tok数组之中
  //返回值就是tok数组中包含几个元素
  //最后一个参数就是 10 表示tok数组最多能放几个数组
  int tok_size = Split(first_line," ",tok,10);
  if(tok_size != 3)
  {
    printf("Split failed! tok_size=%d\n",tok_size);
	return -1;
  }
  *p_method=tok[0];
  *p_url=tok[1];
  return 0;
}

int ParseQueryString(char *url,char** p_url_path,char** p_query_string)
{
 *p_url_path=url;
 char*p=url;
 for(;*p!='\0';p++)
 {
   if(*p=='?')
   {
   *p='\0';
   *p_query_string=p+1;
   return 0;
   }
 }
 //循环结束都没找到，这个请求不带query_string
 *p_query_string=NULL;
 return 0;
}


int ParseHeader(int sock,int* content_length)
{
 char buf[SIZE]={0};
 //1,循环从socket中读取一行
 while(1)
 {
  ssize_t read_size=ReadLine(sock,buf,sizeof(buf));
  if(read_size<=0)
  {
  return -1;
  }
  //处理读完的情况
  if(strcmp(buf,"\n")==0)
  {
   return 0;
  }
  
 //2，判定当前行是不是Content-Length 
 //    如果是Content-Length,就把value读取出来
 //    如果不是就直接丢弃
 const char* content_length_str="Content-Length: ";

 if(content_length!=NULL && strncmp(buf,content_length_str,
				strlen(content_length_str))==0)
 {
  *content_length=atoi(buf+strlen(content_length_str));

 }

 }
  return 0;
}

void  Handler404(int sock)
{
//构造完整的响应
//状态码是404
//body部分应该也是一个404 相关的错误页面
const char* type_line="Content-Type: text/html; charset=UTF-8\n";
const char* first_line="HTTP/1.1 404 Not Found\n";
const char* blank_line="\n";
const char* html="<head><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\"></head>"
"<h1>您的页面被喵星人吃掉了</h1>";


send(sock,first_line,strlen(first_line),0);
send(sock,type_line,strlen(type_line),0);
send(sock,blank_line,strlen(blank_line),0);
send(sock,html,strlen(html),0);
printf("404函数内部\n");
return ;

}

void PrintRequest(Request* req)
{
 
  printf("method: %s\n",req->method);
  printf("url_path: %s\n",req->url_path);
  printf("query_string: %s\n",req->query_string);
  printf("content_length: %d\n",req->content_length);
  return ;
}

int IsDir(const char* file_path)
{
 struct stat st;
 int ret = stat(file_path,&st);
 if(ret<0)
 {
  return 0;
 }
 if(S_ISDIR(st.st_mode))
 {
  return 1;
 }
 return 0;
}



void HandlerFilePath(const char* url_path,char file_path[])
{
 //a）给 url_path 加上前缀（Http服务器的根目录）
 //url_path=>  /index.html
 //file_path => ./wwwroot/index.html
   sprintf(file_path,"./wwwroot%s",url_path);
 //b）例如 url_path ==> 此时的url_path是一个目录
 //   如果是目录的话，就给这个目录之中追加一个index.html
 //   url_path是  / 或者 /image/
	if(file_path[strlen(file_path)-1]=='/')
	{
	 strcat(file_path,"index.html");
	}
 //c）例如 url_path=> /image
	if(IsDir(file_path))
	{
	  strcat(file_path,"/index.html");
	}
	return ;
}

ssize_t GetFileSize(const char* file_path)
{
  struct stat st;
  int ret=stat(file_path,&st);
      if(ret<0)
      {
       //打开文件失败，很可能是文件不存在
       //返回文件长度为0
       return 0;
      }
         printf("count=%d\n",st.st_size);
      return st.st_size;
    }

int WriteStaticFile(int sock,const  char* file_path)
{
  //1,打开文件
  //什么情况下会打开文件失败？1，文件描述符不够用了 2，文件不存在
  int fd=open (file_path,O_RDONLY);
  if(fd<0)
  {

    //debug
    printf("错误1\n");

    perror("open");
    return 404;
  }
  //2，把构造出来的HTTP响应文件内容写入socket中

  //  a）写入首行
  const char* first_line = "HTTP/1.1 200 OK\n";
  send(sock,first_line,strlen(first_line),0);

  //  c）写入header
  const char* type_line="Content-Type: text/html; charset=UTF-8\n";
  //const char* type_line="Content-Type: image/jpeg; charset=UTF-8\n";
  send(sock,type_line,strlen(type_line),0);
  //  b）写入空行
  const char* blank_line="\n";
  send(sock,blank_line,strlen(blank_line),0);
  //  d）写入body（文件内容）
  ////直接将数据从文件中写入到sock中，实际是两个文件描述符的传递

  sendfile(sock,fd,NULL,GetFileSize(file_path));
  //3，关闭文件
  printf("WriteStaticFile函数内部\n");
  close(fd);
  return 200;
}

    int HandlerStaticFile(int sock,Request* req)
    {
      //1,根据 url_path获取到文件在服务器上的真实路径
      char file_path[SIZE]={0};
      HandlerFilePath(req->url_path,file_path);
  //2，读取文件，把文件的内容直接写到socket之中
  int err_code=WriteStaticFile(sock,file_path);

  return err_code;
}

int   HandlerCGIFather(int new_sock,int father_read,int father_write,int child_pid,Request* req)
{
 //1,是post请求，就把body写入到管道中
 if(strcasecmp(req->method,"POST")==0)
 {
  int i=0;
  char c='\0';
  for(;i<req->content_length;++i)
  {
 //前面已经把body前面的部分都已经读完了
 //再读就必然是body
   read(new_sock,&c,1);
  fprintf(stderr,"body=%s",&c);
   write(father_write,&c,1);
  }
 }
 //2.构造http响应
   //  a）写入首行
      const char* first_line = "HTTP/1.1 200 OK\n";
	  send(new_sock,first_line,strlen(first_line),0);
      
   //  c）写入header
	  const char* type_line="Content-Type: text/html; charset=UTF-8\n";
	  send(new_sock,type_line,strlen(type_line),0);
   //  b）写入空行
	  const char* blank_line="\n";
	  send(new_sock,blank_line,strlen(blank_line),0);
 //3.循环从管道中读取数据并写入socket
     char c ='\0';
	 while(read(father_read,&c,1)>0)
	 {

	   send(new_sock,&c,1,0);
	 }
 //4,回收子进程 资源
     waitpid(child_pid,NULL,0);

   return 200;

}

int HandlerCGIChild(int child_read,int child_write,Request* req)
{
  
   //1,设置必要的环境变量
   char method_env[SIZE]={0};
   sprintf(method_env,"REQUEST_METHOD=%s",req->method);
   putenv(method_env);
   //还需要设置 query_string 或者 content_length
   if(strcasecmp(req->method,"GET")==0)
   {
    char query_string_env[SIZE]={0};
	sprintf(query_string_env,"QUERY_STRING=%s",req->query_string);
	putenv(query_string_env);
   }
   else
   {
    char content_length_env[SIZE]={0};
	sprintf(content_length_env,"CONTENT_LENGTH=%d",req->content_length);
    putenv(content_length_env);
   }
   

   //2，把标准输入输出重定向到管道中
   dup2(child_read,0);
   dup2(child_write,1);
   //3，对子进程执行程序替换
   //url_path: /cgi-bin/test
   //file_path: ./wwwroot/cgi-bin/test
   char file_path[SIZE]={0};
   HandlerFilePath(req->url_path,file_path);
   printf("file_path=%s\n",file_path);
   int ret=execl(file_path,file_path,NULL);

   printf("exec ret=%d\n",ret);
   exit(1);
   return 200;

}

int HandlerCGI(int sock,Request* req)
{
	int err_code=200;
	//1，创建一对匿名管道
	int fd1[2],fd2[2];
	int ret=pipe(fd1);
	if(ret<0)
	{
	 return 404;
	}
		ret=pipe(fd2);
	if(ret<0)
	{
	 close(fd1[0]);
	 close(fd1[1]);
	 return 404;
	}
	//fd1 fd2 这样变量名描述性太差，所以用更加明确的变量名来描述
	//文件描述符的作用
	int father_read=fd1[0];
	int child_write=fd1[1];
	int father_write=fd2[1];
	int child_read=fd2[0];

	//2，创建子进程
	ret=fork();
	//3，父子进程执行各自的逻辑
	if(ret>0)
	{
	 //father
	 //此处父进程优先关闭两个管道的文件描述符，
	 //是为了后续父进程从子进程这里能够读到EOF,对于管道来说，所有的写端关闭
	 //继续读，才有EOF,而此处的所有写端，一方面是父进程需要关闭，另一方面是
	 //子进程需要关闭
	 //父子进程都拥有四个文件描述符，父进程读端，对应着父 子进程的child_write
	 close(child_read);
	 close(child_write);

	err_code= HandlerCGIFather(sock,father_read,father_write,ret,req);
	}
	else if(ret==0)
	{
	//child
	close(father_read);
	close(father_write);

	err_code=HandlerCGIChild(child_read,child_write,req);
	}
	else
	{
	 perror("fork");
	 err_code=404;
	 goto END;
	}
	//4，收尾工作和错误处理
	END:
	 close(fd1[0]);
	 close(fd1[1]);
	 close(fd2[0]);
	 close(fd2[1]);
	 return err_code;
}


void  HandlerRequest(int new_sock)
{
  int  err_code=200;
  //1,读取并解析请求（反序列化）
  Request req;
  memset(&req,0,sizeof(req));
  //a>从socket中读取首行
  if(ReadLine(new_sock,req.first_line,sizeof(req.first_line))<0)
  {
	printf("错误3\n");
   err_code=404;
   goto END;
  }
  //b>解析首行，从首行中解析出 URL 和 method
  if(ParseFirstLine(req.first_line ,&req.url,&req.method))
  {
	printf("错误4\n");
   err_code=404;
   goto END;
   
  }
  //c>解析url，从URL中解析出url_path，query_string
  if(ParseQueryString(req.url,&req.url_path,&req.query_string))
  {
   err_code=404;
   goto END;

  }
  //d>解析header部分
  if(ParseHeader(new_sock,&req.content_length))
  {
   err_code=404;
   goto END;
  }
  PrintRequest(&req);
 

  //2,静态/动态生成页面3,把生成结果写回到客户端上
     //a>   如果请求是GET 请求，并且没有query_string,
	 //     那么就返回静态页面
	 if((strcasecmp(req.method,"GET")==0)&&(req.query_string==NULL))
	 {
	 
	 err_code=HandlerStaticFile(new_sock,&req);

	 }
	 //b>   如果请求是GET 请求，有query_string
	 //		那么返回动态页面
	 else if((strcmp(req.method,"GET")==0)&&(req.query_string!=NULL))
	 {
	       err_code=HandlerCGI(new_sock,&req);
	 }
	 //c>   如果请求是POST请求（一定是带参数的，参数是通过body
	 //		来传给服务器的），也是返回动态页面
     else if((strcmp(req.method,"POST")==0)&&(req.query_string==NULL))
	 {
		err_code=HandlerCGI(new_sock,&req);
	 }
	 else
	 {
    	err_code=404;
    	goto END;
    
	 }
	 //错误处理：直接返回一个404 错误页面
	 END:
	 if(err_code!=200)
	 {
	   printf("进入404函数了\n");
	   Handler404(new_sock);
	 }
	 close(new_sock);
	 return ;
}
  
//线程函数
void * ThreadEntry(void *arg)
{
  
  long new_sock=(long)arg;
  //此处使用HandlerRequest函数进行完成具体的处理请求过程
  //这个过程单独提取出来是为了解耦和
  //一旦需要把服务器改成多进程或IO多路复用的形式
  //整体代码的改动是比较小的
  HandlerRequest(new_sock);
  return NULL;
}

//服务器启动
void HttpServerStart (const char* ip,short port)
{
  int listen_sock=socket(AF_INET,SOCK_STREAM,0);
  if(listen_sock < 0)
  {
    perror("socket");
	return ;
  }
  int opt=1;
  //这个函数用来将sock的属性重新设置，使不会出现大量的wait_time
  //重用time_wait连接
  setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

  sockaddr_in addr;
  addr.sin_family =AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_port = htons(port);

  int ret=bind(listen_sock,(sockaddr*)&addr,sizeof(addr));
  if(ret<0)
  {
    perror("bind");
	return ;
  }
  ret=listen(listen_sock,5);
  if(ret<0)
  {
    perror("listen");
	return ;
  }
  printf("ServerInit OK\n");
  while(1)
  {
    sockaddr_in peer;
	socklen_t len =sizeof(peer);
	long new_sock = accept(listen_sock,(sockaddr*)&peer,&len);
    if(new_sock<0)
	{
	  perror("accept");
	  continue ;
	}
    //使用多线程方式实现TCP服务器
	pthread_t tid;
	pthread_create(&tid,NULL,ThreadEntry,(void*)new_sock);
	pthread_detach(tid);

  }
}

int main (int arg,char *argv[])//让main函数简短，可使其他函数易于遍成库函数
{
  if(arg!=3)
  {
    return 1;
  }
  HttpServerStart(argv[1],atoi(argv[2]));
  return 0;
}
