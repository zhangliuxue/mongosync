/*
 * main.cpp
 *  根据日志做数据库的增量同步
 *  Created on: Jul 2, 2013
 *      Author: zlx
 */
#include "xmlprocess.h"

#include "utf8.h"

#include "pch.h"
#include "common.h"
#include "connpool.h"
#include "boost/format.hpp"


#include <stdio.h>
#include "dbclient.h"
#include "dbclientinterface.h"
#include <boost/shared_ptr.hpp>
#include <dbclient_rs.h>
#include "pch.h"

using namespace std;
using namespace bson;
using namespace boost;
using namespace mongo;



//线程池
std::vector<boost::shared_ptr<boost::thread> > threads;

std::ofstream fout;
//系统信号处理
void sig_term(int signo) {


	cout << "program terminated,wait for all threads over!" << endl;

	cout << "reason:" << signo << endl;
	std::ofstream fout("./mongotrans.bug", ios::out | ios::app);
	boost::posix_time::ptime now =
			boost::posix_time::microsec_clock::local_time();
	fout << "time:" << now.zone_as_posix_string() << endl << "sig_term:" << signo << endl;
	fout.close();

	exit(EXIT_FAILURE);

}
/*
 * { "ts" : { "t" : 1374854080, "i" : 5 }, "op" : "u", "ns" : "Weather.WeatherDetail", "o2" : { "_id" : ObjectId("51f14d4386a44d8694650044") }, "o" : { "$set" : { "code" : "101080505", "date" : NumberLong(1374768000), "name" : "青龙山", "wind" : "南风3-4级", "winddirection" : "南风", "windspeed" : "3-4级" } } }
 *
 *
 *Oplog日志中：
 ts:Timestamp 这个操作的时间戳
 op:operation 操作
 i – insert
 d – delete
 u – update
 c – command
 n – no-op
 ns:Namespace也就是操作的collection name
 o:Document
 * */
/**
 *db.oplog.rs.find({ ts: {$gte: Timestamp(1335100998000, 1)}});
 db.oplog.rs.find({ ts: {$lte: Timestamp(1335900998000, 1)}});
 */

void startMongoTrans() {
	long long idstr;
	std::ifstream fin("./mongoTrans.ini", ios::in);
	fin >> idstr;
	fin.close();
	fout.open("./mongoTrans.ini", ios::out);
	if (idstr <= 0)
		idstr = 1;
	string replicaset_name, username, password, ns, errmsg, db;
	string mongo_conn_str;
	format fm("{ ts: {$gt: Timestamp(%d, %d)}}");
	fm % (idstr / 1000) % (idstr % 1000);
	mongo::BSONObj qobj;
	cout << fm.str() << endl;
	qobj = mongo::fromjson(fm.str());

	mongo_conn_str = m_mainstie.in_db.dbstr + ":" + m_mainstie.in_db.port;

	string outconstr = m_mainstie.out_db.dbstr + ":" + m_mainstie.out_db.port;

	Query query(qobj);
	cout << query.toString() << endl;
	username = m_mainstie.in_db.usrname;
	db = m_mainstie.in_db.db;
	password = m_mainstie.in_db.pwd;
	ns = m_mainstie.in_db.col;
	{
		ns = db + "." + ns;
		long long snum = 0;
		while (1) {
			ScopedDbConnection conn(mongo_conn_str);
			ScopedDbConnection wconn(outconstr);
			fm % (idstr / 1000) % (idstr % 1000);
			mongo::BSONObj qobj;
			cout << fm.str() << endl;
			qobj = mongo::fromjson(fm.str());
			Query query(qobj);

			try {

				bool bauth = false;
				DBClientBase &conno = conn.conn();
				if (!username.empty() && !password.empty()) {
					if (db.empty())
						db = "admin";
					bauth = conno.auth("admin", username, password, errmsg);
				}

				snum = conno.count(ns,query.obj);
				//query.sort("$natural", -1);
				auto_ptr<DBClientCursor> m_cur = conno.query(ns, query);
				DBClientBase &wconno = wconn.conn();
				if (!m_mainstie.out_db.usrname.empty()
						&& !m_mainstie.out_db.pwd.empty()) {
					bauth = wconno.auth("admin", m_mainstie.out_db.usrname,
							m_mainstie.out_db.pwd, errmsg);
					cout << errmsg << endl;
				}
				long long pnum = 0;
				while (m_cur->more()) {

					mongo::BSONObj obj = m_cur->nextSafe();
					string ins = obj.getStringField("ns");
					//如果为空则传输所有的，如果为*则不传admin local
					if(!m_mainstie.onlycol.empty())
					{
						//复制数据库
						if(m_mainstie.onlycol=="*")
						{
							if(ins=="admin" || ins=="local")
							{
								continue;
							}
						}
						size_t pos = m_mainstie.onlycol.find(".*");
						string dbstr;
						if(pos != string::npos)
						{
							dbstr = m_mainstie.onlycol.substr(0,pos);
						}
						if(!dbstr.empty())
						{
							if(ins.find(dbstr) == string::npos)
							{
								continue;
							}
						}else
						{
							if(ins != m_mainstie.onlycol)
								continue;
						}
					}
					string op = obj.getStringField("op");
					idstr = obj.getFieldDotted("ts").timestampTime();
					cout << ins << ":" << idstr << endl;

					if (op == "u") {
						mongo::BSONObj o2 = obj.getObjectField("o2");
						mongo::BSONObj o = obj.getObjectField("o");

						wconno.update(ins, o2, o, true, false);
					} else if (op == "i") {

						mongo::BSONObj o = obj.getObjectField("o");
						wconno.insert(ins, o, false);

					} else if (op == "d") {
						mongo::BSONObj o = obj.getObjectField("o");

						bool b = obj.getBoolField("b");
						wconno.remove(ins, o, b);

					} else if (op == "c") {
						mongo::BSONObj o = obj.getObjectField("o");
						size_t st = ins.find(".$cmd");
						if (st != string::npos) {
							mongo::BSONObj info;
							ins = ins.substr(0, st);
							wconno.runCommand(ins, o, info);
						}
					}
					cout << obj.toString() << endl;
					fout.seekp(ios_base::beg);
					fout << idstr << endl;

					Date_t d = idstr;
					pnum ++;
					cout << d.toString() << ":process mongodb trans,ID:"
							<< snum<<":"<<pnum << endl;
					fout.flush();
				}
				wconn.done();
				fout.seekp(ios_base::beg);

				fout << idstr << endl;
				cout << "process mongodb trans,ID:"
											<< snum<<":"<<pnum << endl;
				fout.flush();
				conn.done();
				sleep(10);

			} catch (const std::exception & ex) {
				std::cerr << ex.what() << std::endl;
				fout.seekp(ios_base::beg);
				fout << idstr << endl;
				cout << "process mongodb trans,ID:" << idstr << endl;
				fout.flush();
				wconn.done();
				conn.done();
			}
		}

	}

}
void createsolrTransTheadPool(const long long &t) {

	try {
		boost::shared_ptr<boost::thread> thread(
				new boost::thread(&startMongoTrans));

		thread->join();
		cout << "all thread over!" << endl;

	} catch (std::exception& e) {
		cout << "boost::thread error:" << e.what() << endl;

	}
}
#define DEBUG

int main(int argc, char* argv[]) {
#ifndef DEBUG
	int pid;

	if ((pid = fork()))

	exit(0); //是父进程，结束父进程

	else if (pid < 0)

	exit(1);//fork失败，退出

//是第一子进程，后台继续执行

	setsid();//第一子进程成为新的会话组长和进程组长

//并与控制终端分离

	if ((pid = fork()))

	exit(0);//是第一子进程，结束第一子进程

	else if (pid < 0)

	exit(1);//fork失败，退出

//是第二子进程，继续

//第二子进程不再是会话组长
	signal(SIGTERM, sig_term); /* arrange to catch the signal */
	signal(SIGKILL, sig_term); /* arrange to catch the signal */
	signal(SIGSEGV, sig_term); /* arrange to catch the signal */
	signal(SIGFPE, sig_term); /* arrange to catch the signal */
	signal(SIGILL, sig_term); /* arrange to catch the signal */
	signal(SIGABRT, sig_term); /* arrange to catch the signal */

#endif

	char buf[2000];
	if (getcwd(buf, sizeof(buf) - 1) != NULL) {
		printf(buf);
		printf("\n");
	} else {
		printf("error \n");
	}
	string str = buf;
	str += "/main.xml";
	xmlprocess m_xml;
	cout << "Load xml!" << str << endl;
	m_xml.load(str.c_str());

	startMongoTrans();
	return 1;
}

