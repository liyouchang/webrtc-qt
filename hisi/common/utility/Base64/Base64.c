/* *****************************************************************************/
/*					 			Base64.c									   */
/* 					 Description : BASE64 encoding and decoding				   */
/*******************************************************************************/
#include "Base64.h"

#define BASE64_PAD64 '='

char base64_alphabet[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
	'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
	'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a',
	'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
	'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
	't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1',
	'2', '3', '4', '5', '6', '7', '8', '9', '+',
	'/'
};

unsigned char base64_suffix_map[256] = {
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255,  62, 255, 255, 255,  63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
	255, 255, 255, 255, 255,  0,   1,    2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
	15,   16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255, 255,  26,  27,  28,
	29,   30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
	49,   50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

static unsigned char cmove_bits(unsigned char src, unsigned lnum, unsigned rnum)
{
	src <<= lnum;
	src >>= rnum;
	return src;
}

//----------------------------------------------------------------------------------------
// 注意： 调用Base64编码函数时返回编码后数据的指针，其内存空间为动态分配，所以用完后要及时释放
// 参数： data:待编码数据缓冲区的指针
//		  len: 等编码数据的长度
// 		  return:ret为编码后数据的指针
//----------------------------------------------------------------------------------------
char *base64_encode_kaer(const unsigned char *data, int len)
{
	char *ret, *retpos;
	int m, padnum = 0, retsize, dlen = len;//strlen(data);
	if(dlen == 0) return NULL;
	/* Account the result buffer size and alloc the memory for it. */
	if((dlen % 3) != 0)
	{
		padnum = 3 - dlen % 3;
	}
	retsize = (dlen + padnum) + ((dlen + padnum) * 1/3) + 1;
	if((ret = malloc(retsize)) == NULL)
	{
		return NULL;
	}
	retpos = ret;
	/////////////////////////////////////////////////////////////////////////////
	for(m = 0; m < (dlen + padnum); m += 3)
	{
		/* When data is not suffice 24 bits then pad 0 and the empty place pad '='. */
		*(retpos) = base64_alphabet[cmove_bits(*data, 0, 2)];
		if(m == dlen + padnum - 3 && padnum != 0)
		{
			if(padnum == 1)
			{
				*(retpos + 1) = base64_alphabet[cmove_bits(*data, 6, 2) + cmove_bits(*(data + 1), 0, 4)];
				*(retpos + 2) = base64_alphabet[cmove_bits(*(data + 1), 4, 2)];
				*(retpos + 3) = BASE64_PAD64;
			}
			else if(padnum == 2)
			{
				*(retpos + 1) = base64_alphabet[cmove_bits(*data, 6, 2)];
				*(retpos + 2) = BASE64_PAD64;
				*(retpos + 3) = BASE64_PAD64;
			}
		}
		else
		{
			/* 24bit normal. */
			*(retpos + 1) = base64_alphabet[cmove_bits(*data, 6, 2) + cmove_bits(*(data + 1), 0, 4)];
			*(retpos + 2) = base64_alphabet[cmove_bits(*(data + 1), 4, 2) + cmove_bits(*(data + 2), 0, 6)];
			*(retpos + 3) = base64_alphabet[*(data + 2) & 0x3f];
		}
		retpos += 4;
		data += 3;
	}
	ret[retsize - 1] = 0;
	return ret;
}

//----------------------------------------------------------------------------------------
// Base64解码--将以Base64编码的数据还原
// 参数：buf: 解码后数据的缓冲区
//		 bdata:待解码的用Base64加密的数据缓冲区
//		 return:返回解码后数据的长度
//----------------------------------------------------------------------------------------
int base64_decode_kaer(unsigned char *buf, const char *bdata)
{
	char *retpos;
	int m, padnum = 0, retsize = 0, bdlen = strlen(bdata);
	if(bdlen == 0) return 0;
	if(bdlen % 4 != 0) return 0;
	/* Whether the data have invalid base-64 characters? */
	for(m = 0; m < bdlen; ++m)
	{
		if(bdata[m] != BASE64_PAD64 && base64_suffix_map[(unsigned char)bdata[m]] == 255)
			goto LEND;
	}
	if(bdata[bdlen - 1] == '=') padnum = 1;
	if(bdata[bdlen - 1] == '=' && bdata[bdlen - 2] ==  '=') padnum = 2;
	retsize = (bdlen - 4) - (bdlen - 4) / 4 + (3 - padnum) + 1;
	retpos = (char *)buf;
	/* Begging to decode. */
	for(m = 0; m < bdlen; m += 4)
	{
		*retpos = cmove_bits(base64_suffix_map[(unsigned char)*bdata], 2, 0) + cmove_bits(base64_suffix_map[(unsigned char)*(bdata + 1)], 0, 4);
		if(m == bdlen - 4 && padnum != 0)
		{
			if(padnum == 1)
				*(retpos + 1) = cmove_bits(base64_suffix_map[(unsigned char)*(bdata + 1)], 4, 0) + cmove_bits(base64_suffix_map[(unsigned char)*(bdata + 2)], 0, 2);
			retpos += 3 - padnum;
		}
		else
		{
			*(retpos + 1) = cmove_bits(base64_suffix_map[(unsigned char)*(bdata + 1)], 4, 0) + cmove_bits(base64_suffix_map[(unsigned char)*(bdata + 2)], 0, 2);
			*(retpos + 2) = cmove_bits(base64_suffix_map[(unsigned char)*(bdata + 2)], 6, 0) + base64_suffix_map[(unsigned char)*(bdata + 3)];
			retpos += 3;
		}
		bdata += 4;
	}
LEND:
	return retsize - 1;
}
