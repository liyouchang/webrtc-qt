#include "utility.h"
#ifndef SAFE_MODE
	#include <openssl/md5.h>
	#include <openssl/hmac.h>
#endif
int UTILITY_MD5(unsigned char *Src, unsigned char *dst, int len)
{
#ifndef SAFE_MODE
	if (!MD5(Src,len,dst))
	{
		return -1;
	}
#endif
	return 0;
}

int UTILITY_HMAC1_IOV(st_iov_t* iov,int iov_num,unsigned char* key,int key_len,unsigned char* dst)
{
#ifndef SAFE_MODE
	HMAC_CTX ctx;
	unsigned int md_len = 20;
	
	HMAC_CTX_init(&ctx);
	HMAC_Init(&ctx,key,key_len,EVP_sha1());
	int i = 0;
	for(i=0 ; i<iov_num; i++)
	{
		if (iov[i].iov_base==NULL)
			printf("i %d this iov_base is null\n",i);
		HMAC_Update(&ctx,iov[i].iov_base,iov[i].iov_len);
	}
	HMAC_Final(&ctx,dst,&md_len); /* HMAC-SHA1 is 20 bytes length */
	HMAC_CTX_cleanup(&ctx);
#endif	
	return 0;
}

int UTILITY_BASE64_Encode(unsigned char *src,int len,char *dst)
{
	char *p = base64_encode_kaer(src,len);
	if (p != NULL)
	{
		memcpy(dst,p,strlen(p));
		free(p);
	}
	return 0;
}

int UTILITY_MEDIA_Convert_Size(int resolution,int *width,int *high)
{
    switch(resolution)
    {
        case 1:                     //QCIF
        	*width = 176;
        	*high = 144;
        	break;
        case 2:                     //CIF
        	*width = 352;
        	*high = 288;
        	break;
        case 3:                     //HD
        	*width = 704;
        	*high = 288;
        	break;
        case 5:                     //QVGA
        	*width = 320;
        	*high = 240;
        	break;
        case 6:                     //VGA
        	*width = 640;
        	*high = 480;
        	break;
        case 7:						//720P
        	*width = 1280;
        	*high = 720;
        	break;
        case 8:                    //720P
        	*width = 1280;
        	*high = 960;
        	break;
        case 9:                    //1080P
        	*width = 1920;
        	*high = 1080;
        	break;
        default:                    //D1
        	*width = 704;
        	*high = 576;
            break;
    }	return 0;
}

int UTILITY_MEDIA_Convert_Format(int reso)
{
    int format;
    switch(reso)
    {
        case 1:
        case 2:
        case 3:
            format = reso;
            break;
        case 4:
            format = 0;
            break;
        case 5:
            format = 4;
            break;
        case 6:
            format = 7;
            break;
        case 7:
            format = 10;        //720P
            break;
        case 8:
            format = 12;        //960P
            break;
        case 9:
            format = 6;         //1080P
            break;
        default:
            format = 0;
            break;
    }
    return format;
}

int UTILITY_MEDIA_Convert_Resolution(int format)
{
    int cRet;
    switch(format)
    {
        case 1:                     //QCIF
        case 2:                     //CIF
        case 3:                     //HD
            cRet = format;
            break;
        case 4:                     //QVGA
            cRet = 5;
            break;
        case 7:
            cRet = 6;
            break;
        case 10:                    //720P
            cRet = 7;
            break;
        case 12:                    //960P
            cRet = 8;
            break;
        case 6:                     //1080P
            cRet = 9;
            break;
        default:                    //D1
            cRet = 4;
            break;
    }
    return cRet;
}

int UTILITY_String_IP(char *src,char *cip,int *iip)
{
	int i;
	int len;
	char *p;
	char *c;

	len = strlen(src);
	if ((len>16) || (len<=0))
		return -1;
	src[len] = '.';
	len++;
	src[len] = 0;
	i = 0;
	p = c = src;
	while (len)
	{
		if (*c == '.')
		{
			*c = 0;
			cip[i] = (char)atoi(p);
			i ++;
			if (i >= 4)
				break;
			p = c+1;
		}
		len--;
		c ++;
	}
	memcpy(iip,cip,4);
//	printf("\n%s:%d.%d.%d.%d iip=%08x",__FUNCTION__,cip[0],cip[1],cip[2],cip[3],*iip);
	return 0;
}

/*********************************************************
//名称:					cChar2Hex
//功能: 将一个字节的字符(16禁制数‘0’-‘9’'a'-'f''A'-'F'),转换成16进制数
*********************************************************/
char UTILITY_Char2Hex(char c)
{
    if ((c>='a') && (c<='f'))
        return (c-87);
    else if ((c>='A') && (c<='F'))
        return (c-55);
    else
        return (c-48);
}

int UTILITY_String_MAC(char *src,char *char_mac,char *hex_mac)
{
	char hexMac[16];
	char charMac[32];
	char charTmp[2];
	
	int i;
    for (i=0; i<(strlen(src)+1)/2; i++)
    {
    	charTmp[0] = UTILITY_Char2Hex(src[2*i]);
    	charTmp[1] = UTILITY_Char2Hex(src[2*i+1]);
    	sprintf(&charMac[i<<1],"%x%x",charTmp[0],charTmp[1]);
    	hexMac[i] = ((charTmp[0]<<4)+charTmp[1]);
    }
    charMac[i<<1]=0;    
    if (char_mac != NULL)
    	strcpy(char_mac,charMac);
    if (hex_mac != NULL)
    	memcpy(hex_mac,hexMac,i);
    
	return i;
}

int UTILITY_B64_ntop(unsigned char const * src,size_t src_len,char * dst,size_t dst_len)
{
#ifndef SAFE_MODE
	return lutil_b64_ntop(src,src_len,dst,dst_len);
#else
	return 0;
#endif
}
int UTILITY_B64_pton(char const * src,unsigned char * dst,size_t size)
{
#ifndef SAFE_MODE
	return lutil_b64_pton(src,dst,size);
#else
	return 0;
#endif
}


