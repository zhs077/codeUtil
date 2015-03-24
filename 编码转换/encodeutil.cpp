#include"encodeutil.h"
#include<stdlib.h>

namespace encodeConv
{


int CodingConv::enc_unicode_to_utf8_one(unsigned long unic, unsigned char *pOutput,int outSize)
{
	/*char* pchar = (char*)&unic;

	pOutput[0] = (0XE0 |((pchar[1] & 0XF0) >> 4));
	pOutput[1] = (0X80 |((pchar[1] & 0X0F) <<2)) + ((pchar[0] & 0xC0) >> 6);
	pOutput[2] = (0X80 |((pchar[0] & 0X3F) ));*/

	assert(outSize >= 6);
	if ( unic <= 0x0000007F )
	{
		// * U-00000000 - U-0000007F:  0xxxxxxx
		*pOutput     = (unic & 0x7F);
		return 1;
	}
	else if ( unic >= 0x00000080 && unic <= 0x000007FF )
	{
		// * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
		*(pOutput+1) = (unic & 0x3F) | 0x80;
		*pOutput     = ((unic >> 6) & 0x1F) | 0xC0;
		return 2;
	}
	else if ( unic >= 0x00000800 && unic <= 0x0000FFFF )
	{
		// * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
		*(pOutput+2) = (unic & 0x3F) | 0x80;
		*(pOutput+1) = ((unic >>  6) & 0x3F) | 0x80;
		*pOutput     = ((unic >> 12) & 0x0F) | 0xE0;
		return 3;
	}
	else if ( unic >= 0x00010000 && unic <= 0x001FFFFF )
	{
		// * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		*(pOutput+3) = (unic & 0x3F) | 0x80;
		*(pOutput+2) = ((unic >>  6) & 0x3F) | 0x80;
		*(pOutput+1) = ((unic >> 12) & 0x3F) | 0x80;
		*pOutput     = ((unic >> 18) & 0x07) | 0xF0;
		return 4;
	}
	else if ( unic >= 0x00200000 && unic <= 0x03FFFFFF )
	{
		// * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		*(pOutput+4) = (unic & 0x3F) | 0x80;
		*(pOutput+3) = ((unic >>  6) & 0x3F) | 0x80;
		*(pOutput+2) = ((unic >> 12) & 0x3F) | 0x80;
		*(pOutput+1) = ((unic >> 18) & 0x3F) | 0x80;
		*pOutput     = ((unic >> 24) & 0x03) | 0xF8;
		return 5;
	}
	else if ( unic >= 0x04000000 && unic <= 0x7FFFFFFF )
	{
		// * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		*(pOutput+5) = (unic & 0x3F) | 0x80;
		*(pOutput+4) = ((unic >>  6) & 0x3F) | 0x80;
		*(pOutput+3) = ((unic >> 12) & 0x3F) | 0x80;
		*(pOutput+2) = ((unic >> 18) & 0x3F) | 0x80;
		*(pOutput+1) = ((unic >> 24) & 0x3F) | 0x80;
		*pOutput     = ((unic >> 30) & 0x01) | 0xFC;
		return 6;
	}

	return 0;
}



int CodingConv:: my_utf8_to_unicode(vector<unsigned char> & strUnicode, unsigned char* utf8, int len)
{


	int length=0;
	unsigned int code_int;
	int i;
	for(i = 0; i < len ; i++)
	{

		if(utf8[i] <= 0x7f)
		{
		
			strUnicode[length] =0;
			length++;
			strUnicode[length] =utf8[i];
			length++;
			  
		}

		else if(utf8[i] <= 0xdf)
		{
			code_int = (((unsigned char) utf8[i] & 0x1f) << 6) + ((unsigned char) utf8[i+1] & 0x3f) ;
			
			strUnicode[length] = (unsigned char) (code_int >> 8);
			i +=1;
			strUnicode[length+1] = (unsigned char) code_int;
			
			strUnicode[length] = (unsigned char) (code_int >> 8);
			length += 2;
		} 
		else 
		{

			code_int = ((int) ((unsigned char) utf8[i] & 0x0f) << 12)+(((unsigned char) utf8[i+1] & 0x3f) << 6)+
				((unsigned char) utf8[i+2] & 0x3f);
			i +=2;
			strUnicode[length+1] = (unsigned char) code_int;
			strUnicode[length] = (unsigned char) (code_int >> 8);
			length += 2;

		}

	}

	return (length);

}


string CodingConv::Unicode2Utf8(const wchar_t* pUnicode)
{
	assert(pUnicode);
	string strUtf8;
	unsigned char buf[7];
	int bytes = 0;

	int length = wcslen(pUnicode);
	for (int i = 0; i < length; i++)
	{
		memset(buf,0,sizeof(buf));
		bytes = enc_unicode_to_utf8_one(pUnicode[i],buf,sizeof(buf));
		strUtf8 += (char*)buf;

	}

	return strUtf8;


}

wstring CodingConv::Utf82Unicode(const char* pUtf8)
{
	// 
	assert(pUtf8);
	int len = strlen(pUtf8);
	if (len  == 0)
	{
		return L"";
	}
	int bytes;
	wstring strUnicode;
	string strUtf8;
	vector<unsigned char> strUnicode_;
	unsigned char* pUnicode = new unsigned char[len * 2 +1];
	memset(pUnicode,0,len * 2+1);
	strUnicode_.resize(len* 2 +1);
	bytes = my_utf8_to_unicode(strUnicode_,(unsigned char*)pUtf8,len);
	assert(bytes !=0 );
	//strUnicode.resize(bytes>>2);
	//è¿éåäž€äžªå­è?
	len =strUnicode_.size();
	unsigned int iValue = 0;
	//int j=0;

	for(int i = 0; i < bytes;i=i+2)
	{
		iValue = strUnicode_[i+1];
		iValue += strUnicode_[i]<<8;
		//printf("%02X",iValue);
		strUnicode.push_back(iValue);
	}

	return strUnicode;

}


string CodingConv::ws2s(const wstring ws)
{
	string curLocale = setlocale(LC_ALL,NULL);
	setlocale(LC_ALL,"chs");
	const wchar_t* Sourse = ws.c_str();
	size_t dSize = ws.size() * 2 + 1;
	size_t i=0;
	char* dest = new char[dSize];
	memset(dest,0,dSize);
	wcstombs_s(&i,dest,dSize,Sourse,dSize);
	string result = dest;
	delete[] dest;
	setlocale(LC_ALL,curLocale.c_str());
	return result;



}

wstring CodingConv:: s2ws(const string&s)
{
	string curLocale = setlocale(LC_ALL,NULL);
	setlocale(LC_ALL,"chs");
	const char* source = s.c_str();
	size_t dSize = s.size()+1;
	size_t i=0;
	wchar_t* dest = new wchar_t[dSize];
	memset(dest,0,dSize);
	mbstowcs_s(&i,dest,dSize,source,dSize);
	wstring result = dest;
	delete[] dest;
	setlocale(LC_ALL,curLocale.c_str());
	return result;
}
string CodingConv::ascii2Utf8(const char* pAscii)
{
	wstring unicode = s2ws(pAscii);
	return Unicode2Utf8(unicode.c_str());
}

string CodingConv::utf82Ascii(const char* pAscii)
{
	wstring  unicode = Utf82Unicode(pAscii);
	return ws2s(unicode);
}


string CodingConv::Encode_GBK(const string& str)
{
	std::string strTemp = "";  
	size_t length = str.length();  
	for (size_t i = 0; i < length; i++)  
	{  
		if (isalnum((unsigned char)str[i]) ||   
			(str[i] == '-') ||  
			(str[i] == '_') ||   
			(str[i] == '.') ||   
			(str[i] == '~'))  
			strTemp += str[i];  
		else if (str[i] == ' ')  
			strTemp += "+";  
		else  
		{  
			strTemp += '%';  
			strTemp += ToHex((unsigned char)str[i] >> 4);  
			strTemp += ToHex((unsigned char)str[i] % 16);  
		}  
	}  
	return strTemp;  

}
string CodingConv::Decode_GBK(const string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (str[i] == '+') strTemp += ' ';
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high*16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}

string CodingConv::Encode_UTF8(const string& str)
{
	
	string utf8 = ascii2Utf8(str.c_str());
	
	return  Encode_GBK(utf8);

}
string CodingConv:: Decode_UTF8(const string& str)
{
	string ascii =Decode_GBK(str);
	return utf82Ascii(ascii.c_str());
}

}
