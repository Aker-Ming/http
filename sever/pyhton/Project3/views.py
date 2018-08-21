#!/usr/bin/python
#coding:utf-8
'''
使用flask框架建立web网站
'''
#pip install flask
from flask import Flask
#引入flask框架为我们提供的调用模板函数
from flask import render_template,request
#用Flask方法建立app对象，一般默认把本文件设置为app对象
app = Flask(__name__)
import sys
reload(sys)
sys.setdefaultencoding("utf8")

#web网站初尝试之Hello World
#route路由 将url和函数进行一对一映射，访问指定url时,就会被路由转发到指定函数进行响应
#route map
@app.route("/hello/<name>")
def Hello():
    #return "<h1>Hello World</h1>"
    data = [
         ("10086","比特科技","8-16k","班主任","颜值担当"),
         ("10010","比特科技","8-16k","班主任","颜值担当"),
         ("10000","比特科技","8-16k","班主任","颜值担当"),
    ]
    return render_template("main.html",Job=data)

@app.route("/main/")
def Main():
    return "This is main page"

#GET方法通过url后面追加键值对 key=value
#request.args.get(key)
#POST方法通过form表单提交数据
#request.form.get(name)

@app.route("/login/",methods=["GET","POST"])
def Login():
    if request.method == "POST":
#        print request.args
        getName = request.form.get("username")
        getPsw = request.form.get("userpsw")

        return getName + getPsw
    return render_template("login.html")


if __name__ == "__main__":
    app.run(host="0.0.0.0",port=40000)

