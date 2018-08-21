#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include<unistd.h>

int main ()
{
  fprintf(stderr,"进入交互页面！\n");
 //打开数据库文件，把新闻链接往html中写
  int fd_html=open("/home/wm/code/http/http/wwwroot/index1.html",O_RDWR);
  //printf("fd_html=%d",fd_html);
  if(fd_html<0)
  {
    perror("open index1.html");
  }
  int fd_txt=open("/home/wm/code/python/tmp.txt",O_RDWR);
  //printf("fd_txt=%d",fd_txt);
  if(fd_txt<0)
  {
    printf("open mysql.txt filed!");
  }
 



  char c ='\0';
  //int ret=read(fd_html,&c,1);
  //printf("ret=%d",ret);
  while(read(fd_html,&c,1)>0)
  {
  if(c=='')
  {
    char c1='\0';
    while(read(fd_txt,&c1,1)>0)
    {
      if(c1=='\n' || c1==EOF)
        break;
      printf("%c",c1);
    }
    continue;
  }
      printf("%c",c);
  }

  close(fd_html);
  close(fd_txt);

  return 0;
}
