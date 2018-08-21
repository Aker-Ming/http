#!/usr/bin/python
#coding:utf-8

import MySQLdb
import urllib2
import json
from bs4 import BeautifulSoup

#根据url获取服务器端响应内容
def OpenPage(url):
    Myheaders = {}
    req = urllib2.Request(url,headers=Myheaders)
    f = urllib2.urlopen(req)
    data = f.read()
    return data

def Test1():
    #url = "http://jy.51uns.com:8022/Pro_StudentEmploy/StudentJobFair/Zhaoping.aspx?WorkType=0"
    url = "http://jy.51uns.com:8022/Frame/Data/jdp.ashx?rnd=1533777957521&fn=GetZhaopinList&StartDate=2000-01-01&SearchKey=&InfoType=-1&CompanyAttr=&CompanyType=&Area=&City=&CompanyProvice=&Post=&Zhuanye=&XLkey=&Age=&start=0&limit=15&DateType=999&InfoState=1&WorkType=0&CompanyKey="
    print OpenPage(url)

#根据主页上获取到的招聘信息内容,获取招聘信息id
def ParseMainPage(page):
    #解压json格式数据
    data = json.loads(page)
    #获取招聘信息数据
    GetInfo = data["rows"]
    IdList = []
    #遍历招聘信息List
    for item in GetInfo:
        IdList.append(item["Id"])

    return IdList

#获取招聘信息详情页的响应数据
def ParseDetailPage(page):
    data = json.loads(page)
    Info = data["Data"]
    Id = Info["Id"]
    Title = Info["CompanyTitle"]
    Price = Info["WorkPrice"]
    Position = Info["WorkPositon"]
    detail = Info["EmployContent"]
    soup = BeautifulSoup(detail,"html.parser")
    #查询所有的p标签，返回一个list
    GetList = soup.find_all("p")
    GetData = [item.get_text() for item in GetList]
    Content = "\n".join(GetData)
    return Id,Title,Price,Position,Content

#创建一个数据库，创建一个table
#Id Title Price Position Content text
#CREATE TABLE '' (
#    'id' text,
#    'company' text,
#    'price' text,
#   'position' text,
#    'cy8ontent' text
#)ENGINE= InnoDB DEFAULT CHARSET=utf8
#import MySQLdb
import base64
#保存数据进Mysql数据库
def WriteDataToMySQL(data):
    #封装存储过程
    #table.save(1,2,3,4,5)
    #构造数据库连接
    db = MySQLdb.connect(host="39.104.50.13",user="root",passwd="123",db="ClawerSchool",charset="utf8")
    #建立连接
    Connect = db.cursor()
    #Content内容来自html标签内，可能包含各种特殊字符，影响我们插入mysql数据库，所以要进行base64编码转换
    Content = base64.b64encode(data[4])
    #构造插入sql语句
    sql = "insert into Job values('%s','%s','%s','%s','%s')" % (data[0],data[1],data[2],data[3],Content)

    try:
        #执行sql语句
        Connect.execute(sql)
        db.commit()
    except Exception,e:
        #插入失败，则进行回滚操作
        db.rollback()
        print str(e)

def Test5():
    test_data = ("10086","比特科技","8-16k","班主任","人美声甜")
    WriteDataToMySQL(test_data)

def WriteDataToFile(data):
    f = open("output.txt","a+")
    data = "\n".join(data).encode("utf-8")

    f.write(data)
    f.close()

if __name__ == "__main__":
    url = "http://jy.51uns.com:8022/Frame/Data/jdp.ashx?rnd=1533777957521&fn=GetZhaopinList&StartDate=2000-01-01&SearchKey=&InfoType=-1&CompanyAttr=&CompanyType=&Area=&City=&CompanyProvice=&Post=&Zhuanye=&XLkey=&Age=&start=0&limit=15&DateType=999&InfoState=1&WorkType=0&CompanyKey="
   # 获取主页服务器端响应内容
    homePage = OpenPage(url)
    #获取招聘信息的Id列表
    GetList = ParseMainPage(homePage)
    for item in GetList:
        Detailurl = "http://jy.51uns.com:8022/Frame/Data/jdp.ashx?rnd=1533780257782&fn=GetOneZhaopin&StartDate=2000-01-01&JobId=" + item
        DetailPage = OpenPage(Detailurl)
        data = ParseDetailPage(DetailPage)
        #存进数据库中
        WriteDataToMySQL(data)
        #WriteDataToFile(data)
    print "Done"


