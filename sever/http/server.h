#pragma once

#define SIZE 10240
typedef struct Request
{
  char first_line[SIZE];
  char* method ;
  char* url;
  char* url_path;//路径
  char* query_string;//搜索字
 // char* version;
 //简易版本，只保留Content-Liength
 int content_length;
}Request;
