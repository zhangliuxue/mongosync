/*
模块名称：读取、写入XML文件
读取站点基本配置文件，主目录，数据库链接，文件系统连接，action方式连接等
作者：张留学
日期：2012-08-14
*/
#pragma once
#include <iostream>  
#include <string>  
#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/xml_parser.hpp>  
#include <boost/foreach.hpp>  
#include <set>
#include <map>
using namespace std;


typedef struct collink
{
	std::string dbstr;      //数据库名称
	std::string usrname;    //用户名称
	std::string pwd;        //密码
	string port;            //数据库服务端口
	string db;
	string col;
}COLLINK;
//web服务主要结构配置
typedef struct mainsturct
{
	COLLINK in_db;
	COLLINK out_db;
	string  onlycol;//仅仅转换的ｃｏｌ
	int tmout;
}MAINSTRUCT;


class xmlprocess
{
public:
	xmlprocess(void);
	~xmlprocess(void);
	void load(const std::string &filename);
	void save(const std::string &filename);
private:
	std::string m_file;         

};

extern	mainsturct m_mainstie;

