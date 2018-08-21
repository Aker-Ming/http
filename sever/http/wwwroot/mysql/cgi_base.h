#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>


//分GET/POST 两种情况读取计算的参数
//1.GET从query_string读取
//2.POST 从 body 中读取
//读取的结果放到buf缓冲区中
int GetQueryString (char buf[])
{
  fprintf(stderr,"[CGI]error\n");
 //1,从环境变量中获取方法
 char* method = getenv("REQUEST_METHOD");
 if(method==NULL)
 {
  //不能直接printf，因为子进程的标准输出被重定向到了管道中
  //为了避免让程序内部的错误暴露给普通用户，通过stderr来
  //作为输出日志的手段
  fprintf(stderr,"[CGI]error\n");
  return -1;
 }
 //判定方法是GET 还是POST 
 //如果是GET 就从环境变量里面读取	QUERY_STRING 
 //如果是 POST 就需要从坏境变量读   CONTENT_LENGTH
 if(strcasecmp(method,"GET")==0)
 {
   char* query_string=getenv("QUERY_STRING");
   if(query_string ==NULL)
   {
    fprintf(stderr,"[CGI]query_string is  NULL\n");
    return -1;
   }
   //拷贝完成之后，buf里面的内容形如
   //a=10&b=20
   strcpy(buf,query_string);//复制到缓冲区
 }


 else//是POST 请求
 {
  char* content_length_str =getenv("CONTENT_LENGTH");
  if(content_length_str==NULL)
  {
   fprintf(stderr,"[CGI]content_length is  NULL\n");
   return -1;
  }
  int content_length=atoi(content_length_str);
  int i=0;
  for(i=0;i<content_length;i++)
  {
  //此处父进程把body写入了管道中
  //子进程又把0号文件描述符重定向到了管道
  //此时从标准输入读，也就读到了管道中的数据
   read(0,&buf[i],1);//从标准输入0读，就是从管道中读
  }
  //此循环读完后 ，buf里面的内容形如
  //a=10&b=20
  buf[i]='\0';
 }

 return 0;
}
