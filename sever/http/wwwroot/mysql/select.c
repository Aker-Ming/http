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
const char* sql="select * from TestTable";


//3，把sql语句发送到服务器
//使用遥控器把命令发给服务器
int ret=mysql_query(connect_fd,sql);
if(ret<0)
{
  fprintf(stderr,"mysql_query failed! %s\n",sql);
  return 1;
}

//4，读取并遍历服务器返回的结果
MYSQL_RES* result = mysql_store_result(connect_fd);
if(result<0)
{
 fprintf(stderr,"mysql_store_result feiled! \n");
 return 1;
 }
 //a>获取到表里面有几行几列
 int rows= mysql_num_rows(result);
 int fields= mysql_num_fields(result);
 //b>获取到集合的表结构
 MYSQL_FIELD* field= mysql_fetch_field(result);
 while(field!=NULL)
 {
  printf("%s\t",field->name);
  field=mysql_fetch_field(result);
 }
 printf("<br>");
 //c>获取到每个元素的具体值
    int i=0;
	for(i=0;i<rows;i++)
	{
	  MYSQL_ROW row=mysql_fetch_row(result);
	  int j=0;
	  for(j=0;j<fields;++j)
	  {
	    printf("%s\t",row[j]);
     } 
	    printf("<br>");
	}
	printf("<br>");
//5，断开连接
mysql_close(connect_fd);
 return 0;
}
