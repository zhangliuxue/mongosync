/*
 * * 读取配置文件
 * */
#include "xmlprocess.h"
#include "boost/format.hpp"
using namespace boost;
mainsturct m_mainstie;
xmlprocess::xmlprocess(void) {
}

xmlprocess::~xmlprocess(void) {
}

void xmlprocess::load(const std::string &filename) {
	using boost::property_tree::ptree;
	ptree pt;
	read_xml(filename, pt);
	m_mainstie.onlycol = pt.get<string>("main.onlycollections");
	ptree childtt = pt.get_child("main.indb");

	m_mainstie.in_db.col = childtt.get<string>("oplog");
	m_mainstie.in_db.dbstr = childtt.get<string>("dbstr");
	m_mainstie.in_db.port = childtt.get<string>("port");
	m_mainstie.in_db.pwd = childtt.get<string>("pwd");
	m_mainstie.in_db.usrname = childtt.get<string>("usr");
	m_mainstie.in_db.db = childtt.get<string>("db");

	childtt = pt.get_child("main.outdb");


	m_mainstie.out_db.dbstr = childtt.get<string>("dbstr");
	m_mainstie.out_db.port = childtt.get<string>("port");
	m_mainstie.out_db.pwd = childtt.get<string>("pwd");
	m_mainstie.out_db.usrname = childtt.get<string>("usr");

}

void xmlprocess::save(const std::string &filename) {

}
