﻿#include"cgi_base.h"

int main ()
{
//1，调用获取到的函数，获取到的参数
char buf[1024*10]={0};
int ret=GetQueryString(buf);
if(ret<0)
{
  fprintf(stderr,"[CGI]GetQueryString failed\n");
  return 1;
}

  fprintf(stderr,"[CGI]\n");
//此时获取到的buf中的内容格式为 a=10&b=20
  int a,b;
  sscanf(buf,"a=%d&b=%d",&a,&b);
  int sum=a+b;
  //printf输出的内容会被读到用户 界面
  //作为http服务器，每次给客户端返回的字符串必须符合http 协议的格式
  //父进程已经将首行，header，空行都已经写回给了客户端
  //因此此时的CGI程序 只需要返回body就可以了，就是HTML的格式
  printf("<h1>sum=%d</h1>",sum);
return 0;
}
