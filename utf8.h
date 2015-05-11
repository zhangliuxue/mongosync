#ifndef _UTF8_H
#define _UTF8_H
#include <string>
using namespace std;
int utf8_to_gb ( char* src, char* dst, int len );
int gb_to_utf8( char* src, char* dst, int len );
void decode_uri( char* input );
int if_UTF8(char *str);
string strgb_to_utf8str(const string instring);
string strutf8_to_gbstr(const string instring);
char *url_decode(char *str);
char *url_encode(char *str);
void convert(char *src, size_t nsrc, char **dst, int *ndst,const char *codefrom, const char *codeto);
int isgbk(const char *s, size_t ns);
int isutf8(const char *s, size_t ns);
int isgbk1(const char * s,size_t ns);
int charconv(char *from, char *to,const char *input, int inlen, char **output, int *outlen);
string& replace_all_distinct(string& str, const string& old_value,
		const string& new_value);
string& replace_all_t(string& str, const string& old_value,
		const string& new_value);
string utojson(string&  str);
string sutojson(string& str);
std::wstring GBK_To_Unicode(const char* s);
time_t FormatTime2(const char * szTime);
time_t BaiDuFormatTime2(const char * szTime);
#endif
