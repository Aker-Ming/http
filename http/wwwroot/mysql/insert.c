﻿#include<stdio.h>
#include<mysql/mysql.h>
#include"cgi_base.h"

int main ()
{
//1,获取query_string 
char buf[1024*10]={0};
if(GetQueryString(buf)<0)
{
 fprintf(stderr,"GetQueryString failed\n");
 return 1;
}

//约定客户端传过来的参数是： id=123&name=hehe
int id=0;
char name[1024]={0};
sscanf(buf,"id=%d&name=%s",&id,name);
fprintf(stderr,"[CGI] id=%d,name=%s\n",id,name);

/////////////////////////////////////////////////////////////////////////////
//接下来把要进行数据库查找
//直接把一个数据库表中的所有数据都一股脑的查出来 
//1.连接到数据库
//2，拼装sql语句
//3，把sql语句发送到服务器
//4，读取并遍历服务器返回的结果
//5，断开连接
////////////////////////////////////////////////////////////////////////////

//1，连接到数据库
MYSQL*connect_fd=mysql_init(NULL);
//拿遥控器和数据库进行配对
MYSQL* connect_ret=mysql_real_connect(connect_fd,"127.0.0.1","root","123456","http",3306,NULL,0);
if(connect_ret==NULL)
{
 fprintf(stderr,"mysql connect failed\n");
 return 1;
}
fprintf(stderr,"mysql connect ok!\n");
//2.拼装sql语句
//组织命令
char sql[1024*10]={0};
sprintf(sql,"insert into TestTable values(%d,'%s')",id,name);

//3，把sql语句发送到服务器
//使用遥控器把命令发给服务器
int ret=mysql_query(connect_fd,sql);
if(ret<0)
{
  fprintf(stderr,"mysql_query failed! %s\n",sql);
  return 1;
}

//4,想客户端反馈插入的结果
 if(ret==0)
 {  
  printf("<html><h1>插入成功</h1></html>");
 }
 else
 {
  printf("<html><h1>插入失败</h1></html>");
 }
//5，断开连接
mysql_close(connect_fd);
 return 0;
}
