
#include <cassert>
#include <string>
#include<vector>
#include<cstring>
using namespace std;
#ifdef OS_LINUX
#include<iconv.h>
#endif


#ifndef ENCODE_CONV_H_
#define ENCODE_CONV_H_

namespace encodeConv
{

	class CodingConv
	{
	public:
		static inline unsigned char ToHex(unsigned char x) 
		{ 
			return  x > 9 ? x + 55 : x + 48; 
		}

		static inline unsigned char FromHex(unsigned char x) 
		{ 
			unsigned char y;
			if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
			else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
			else if (x >= '0' && x <= '9') y = x - '0';
			else 
				assert(0);
			return y;
		}
		static string Unicode2Utf8(const wchar_t* pUnicode);
		static wstring Utf82Unicode(const char* pUtf8);
		static wstring s2ws(const string&s);
		static string ws2s(const wstring ws);
		static string ascii2Utf8(const char* pAscii);
		static string utf82Ascii(const char* pUtf8);
		static string Encode_GBK(const string& str);//±àÂëÎªGBK
		static string Decode_GBK(const string& str);//
		static string Encode_UTF8(const string& str);//±àÂëÎªUTF8
		static string Decode_UTF8(const string& str);//
/*		static wstring urlDecode(const wchar_t* pUnoode);
#ifndef OS_WIN32 
		static int gb2312_to_utf8(char *in, char *out, size_t size);
#endif
*/
	//static int gb2312_to_utf8(const char *in, char *out, size_t size);
	//static int utf8_to_gb2312(const char* sIn,char* sOut,size_t size);
	private:

		static int enc_unicode_to_utf8_one(unsigned long unic, unsigned char *pOutput,int outSize);
		static int my_utf8_to_unicode(vector<unsigned char>& strUnicode, unsigned char* utf8, int len);


}; 



	

};


#endif
