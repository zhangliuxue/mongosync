#include <iconv.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utf8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <iostream>
#include <boost/algorithm/string.hpp>
using namespace std;

time_t BaiDuFormatTime2(const char * szTime) {
	struct tm tm1;
	time_t time1;
	string str = szTime;
	str = str.substr(0, 16);
	sscanf(str.c_str(), "%4d-%2d-%2d %2d:%2d", &tm1.tm_year, &tm1.tm_mon,
			&tm1.tm_mday, &tm1.tm_hour, &tm1.tm_min);

	tm1.tm_sec = 0;
	tm1.tm_year -= 1900;
	tm1.tm_mon--;

	tm1.tm_isdst = -1;

	time1 = mktime(&tm1);
	return time1;
}
time_t FormatTime2(const char * szTime) {
	struct tm tm1;
	time_t time1;
	string str = szTime;
	str = str.substr(0, 10);
	sscanf(str.c_str(), "%4d-%2d-%2d", &tm1.tm_year, &tm1.tm_mon, &tm1.tm_mday);

	tm1.tm_hour = 0;
	tm1.tm_min = 0;
	tm1.tm_sec = 1;
	tm1.tm_year -= 1900;
	tm1.tm_mon--;

	tm1.tm_isdst = -1;

	time1 = mktime(&tm1);
	return time1;
}
string utojson(string& str) {
	const char * p = str.c_str();
	size_t len = str.length();
	char * rp = new char[2 * len + 1];

	memset(rp, 0, len * 2 + 1);
	size_t num = 0;
	for (size_t i = 0; i < len; i++) {

		if (p[i] == '"' || p[i] == '\\' || p[i] == '/' || p[i] == '\b'
				|| p[i] == '\f' /* || p[i] == '\r' || p[i] == '\n'*/
				|| p[i] == '\t') {
			rp[num] = '\\';
			num++;
			rp[num] = p[i];
			num++;
		} else if (p[i] >= 1 && p[i] <= 31)
		{

		} else {
			rp[num] = p[i];
			num++;
		}
	}
	string rstr;
	rstr.append(rp);

	delete[] rp;
	return rstr;
}
//通过curl提交到solr是不要处理回车和换行　２０１３－５－２２
string sutojson(string& str) {
	boost::trim(str);
	const char * p = str.c_str();
	size_t len = str.length();
	char * rp = new char[2 * len + 1];
	//std::cout<<str<<std::endl;
	memset(rp, 0, len * 2 + 1);
	size_t num = 0;
	for (size_t i = 0; i < len; i++) {
		if (p[i] == '"' || p[i] == '\\' || p[i] == '/' ) {
			rp[num] = '\\';
			num++;
			rp[num] = p[i];
			num++;
		} else if (p[i] >= 1 && p[i] <= 31 && p[i]!='\r' && p[i]!='\n' && p[i] != '\b'
				&& p[i] != '\f'  && p[i] != '\t')
		{

		} else {
			rp[num] = p[i];
			num++;
		}
	}
	string rstr;
	rstr.append(rp);
	//std::cout<<rstr<<std::endl;
	delete[] rp;
	return rstr;
}

int charconv(char *from, char *to, const char *input, int inlen, char **output,
		int *outlen) {
	char *inbuf;
	char *outbuf;
	size_t inleft;
	size_t outleft;
	iconv_t cd;
	size_t result;
	cd = iconv_open(to, from);

	if (cd == (iconv_t) (-1)) {
		*outlen = -1;
		*output = NULL;
		return -1;
	}

	if (inlen == 0)
		inlen = strlen(input);
	*outlen = 4 * inlen;
	inbuf = (char *) input;
	outbuf = (char *) malloc(*outlen);
	inleft = inlen;
	outleft = *outlen;
	*output = outbuf;
	result = iconv(cd, (char **) &inbuf, &inleft, &outbuf, &outleft);
	iconv_close(cd);
	*outlen = *outlen - outleft;
	(*output)[*outlen] = 0;

	return inlen - inleft;

}

int isutf8(const char *s, size_t ns) {
	uint8_t x = 0, i = 0, j = 0, nbytes = 0, n = 0;
	for (i = 1; i < 7; i++) {
		x = (uint8_t) (255 << i);
		if (((uint8_t) *s & x) == x) {
			n = nbytes = (8 - i);
			for (j = 0; (j < nbytes && j < ns); j++) {
				if ((uint8_t) s[j] <= 0x80 && (uint8_t) s[j] >= 0xc0)
					break;
				else
					n--;
			}
			if (n == 0)
				return nbytes;
		}
	}
	return 0;
}

int isgbk(const char *s, size_t ns) {
	if (ns > 2 && (uint8_t) *s >= 0x81 && (uint8_t) *s <= 0xfe
			&& (((uint8_t) *(s + 1) >= 0x80 && (uint8_t) *(s + 1) <= 0x7e)
					|| ((uint8_t) *(s + 1) >= 0xa1 && (uint8_t) *(s + 1) <= 0xfe))) {
		return 1;
	}
	return 0;
}
int isgbk1(const char * s, size_t ns) {
	size_t n = 0;
	size_t x = 0, nbytes = 0;
	while (ns > 0) {
		n = ns;
		if ((nbytes = isutf8(s, ns)) > 0) {
			s += nbytes;
			ns -= nbytes;
		} else if (isgbk(s, ns)) {
			x = 2;
			ns -= 2;
			s += 2;
			return 1;
		} else {
			ns--;
			s++;
		}
		if (ns == n)
			break;
	}
	return 0;
}
void convert(char *src, size_t nsrc, char **dst, int *ndst,
		const char *codefrom, const char *codeto) {
	char *s = src, *d = (*dst), *p = NULL;
	iconv_t handler;
	size_t n = 0, ns = nsrc, nd = (*ndst), result = 0;
	size_t x = 0, nbytes = 0, nbuf = 16;
	char buf[16];

	handler = iconv_open(codeto, codefrom);
	while (ns > 0) {
		n = ns;
		if ((nbytes = isutf8(s, ns)) > 0) {
			memcpy(d, s, nbytes);
			s += nbytes;
			d += nbytes;
			ns -= nbytes;
			nd -= nbytes;
		} else if (isgbk(s, ns)) {
			memset(buf, 0, nbuf);
			memcpy(buf, s, 2);
			x = 2;
			p = buf;
			result = iconv(handler, (char**) &p, &x, &d, &nd);
			ns -= 2;
			s += 2;

		} else {
			*d++ = *s++;
			ns--;
			nd--;
		}
		if (ns == n)
			break;
	}
	iconv_close(handler);
}
#ifndef ICONV_CONST
# define ICONV_CONST const
#endif

/*!
 对字符串进行语言编码转换
 param from  原始编码，比如"GB2312",的按照iconv支持的写
 param to      转换的目的编码
 param save  转换后的数据保存到这个指针里，需要在外部分配内存
 param savelen 存储转换后数据的内存大小
 param src      原始需要转换的字符串
 param srclen    原始字符串长度
 */
int z_convert(const char *from, const char *to, char* save, int savelen,
		char *src, int srclen) {
	iconv_t cd;
	char *inbuf = src;
	char *outbuf = save;
	size_t outbufsize = savelen;
	int status = 0;
	size_t savesize = 0;
	size_t inbufsize = srclen;
	const char* inptr = inbuf;
	size_t insize = inbufsize;
	char* outptr = outbuf;
	size_t outsize = outbufsize;

	cd = iconv_open(to, from);

	if (inbufsize == 0) {
		status = -1;
		goto done;
	}
	while (insize > 0) {
		size_t res = iconv(cd, (char**) &inptr, &insize, &outptr, &outsize);
		if (outptr != outbuf) {
			int saved_errno = errno;
			int outsize = outptr - outbuf;
			strncpy(save + savesize, outbuf, outsize);
			errno = saved_errno;
		}
		if (res == (size_t) (-1)) {
			if (errno == EILSEQ) {
				int one = 1;
				iconvctl(cd, ICONV_SET_DISCARD_ILSEQ, &one);
				status = -3;
			} else if (errno == EINVAL) {
				if (inbufsize == 0) {
					status = -4;
					goto done;
				} else {
					break;
				}
			} else if (errno == E2BIG) {
				status = -5;
				goto done;
			} else {
				status = -6;
				goto done;
			}
		}
	}
	status = strlen(save);
	done:
	iconv_close(cd);
	return status;

}

int utf8_to_gb(char* src, char* dst, int len) {
	int ret = 0;
	size_t inlen = strlen(src);
	size_t outlen = len;
	char* inbuf = src;
	char* outbuf = dst;
	iconv_t cd;
	cd = iconv_open("GBK", "UTF-8");

	if (cd != (iconv_t) -1) {
		ret = iconv(cd, (char**) &inbuf, &inlen, &outbuf, &outlen);
		if (ret != 0) {
			printf("iconv failed err: %s\n", strerror(errno));
			return -1;
		}
		iconv_close(cd);
		return 0;
	}
	return -1;
}
int gb_to_utf8(char* src, char* dst, int len) {
	int ret = 0;
	size_t inlen = strlen(src);
	size_t outlen = len;
	char* inbuf = src;
	char* outbuf2 = NULL;
	char* outbuf = dst;
	iconv_t cd;

	if (src == dst) {
		outbuf2 = (char*) malloc(len);
		memset(outbuf2, 0, len);
		outbuf = outbuf2;
	}
	cd = iconv_open("UTF-8", "GBK");

	if (cd != (iconv_t) -1) {
		ret = iconv(cd, (char**) &inbuf, &inlen, &outbuf, &outlen);
		if (ret != 0)
		{
			free(outbuf2);
			printf("iconv failed err: %s\n", strerror(errno));
			return -1;
		}
		if (outbuf2 != NULL) {
			strcpy(dst, outbuf2);
			free(outbuf2);
		}

		iconv_close(cd);
		return 0;
	}
	return -1;
}
string strgb_to_utf8str(const string instring) {
	char * dst;
	char *src = (char*) instring.c_str();
	int len = strlen(src);
	if (len <= 0)
		return "";
	string outstring;

	dst = (char*) malloc(len * 4);
	memset(dst, 0, 4 * len);
	if(z_convert("GBK","UTF-8",dst,4*len,src,len)>0)
	//if (gb_to_utf8(src, dst, len * 4) == 0)
		outstring = dst;
	else
	{
		free(dst);
		return instring;
	}
	free(dst);
	return outstring;
}

string strutf8_to_gbstr(const string instring) {
	char * dst;
	char *src = (char*) instring.c_str();
	int len = strlen(src);
	if (len <= 0)
		return "";
	string outstring;

	dst = (char*) malloc(len * 4);
	memset(dst, 0, 4 * len);

	if(z_convert("UTF-8","GBK",dst,4*len,src,len)>0)
		outstring = dst;
	else
	{
		free(dst);
		return instring;
	}
	free(dst);
	return outstring;
}
/* Converts a hex character to its integer value */
char from_hex(char ch) {
	return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str) {
	char *pstr = str, *buf = (char*) malloc(strlen(str) * 3 + 1), *pbuf = buf;
	while (*pstr) {
		if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.'
				|| *pstr == '~')
			*pbuf++ = *pstr;
		else if (*pstr == ' ')
			*pbuf++ = '+';
		else
			*pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(
					*pstr & 15);
		pstr++;
	}
	*pbuf = '\0';
	return buf;
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str) {
	char *pstr = str, *buf = (char*) malloc(strlen(str) + 1), *pbuf = buf;
	while (*pstr) {
		if (*pstr == '%') {
			if (pstr[1] && pstr[2]) {
				*pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
				pstr += 2;
			}
		} else if (*pstr == '+') {
			*pbuf++ = ' ';
		} else {
			*pbuf++ = *pstr;
		}
		pstr++;
	}
	*pbuf = '\0';
	return buf;
}
string& replace_all_t(string& str, const string& old_value,
		const string& new_value) {
	while (true) {
		string::size_type pos(0);
		if ((pos = str.find(old_value)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else
			break;
	}
	return str;
}

string& replace_all_distinct(string& str, const string& old_value,
		const string& new_value) {
	for (string::size_type pos(0); pos != string::npos; pos +=
			new_value.length()) {
		if ((pos = str.find(old_value, pos)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else
			break;
	}
	return str;
}

std::wstring GBK_To_Unicode(const char* s) {
	if (s == NULL)
		return L"";
	std::string curLocale = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "chs");
	const char* _Source = s;
	size_t _Dsize = strlen(s) + 1;
	wchar_t *_Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest, _Source, _Dsize);
	std::wstring result = _Dest;
	delete[] _Dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

