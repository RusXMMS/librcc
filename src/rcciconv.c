#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>

#include "internal.h"
#include "rcciconv.h"

#define RCC_MAX_ERRORS 3

static void rccIConvCopySymbol(char **in_buf, size_t *in_left, char **out_buf, size_t *out_left) {
    if ((out_left>0)&&(in_left>0)) {
/*	(**out_buf)=(**in_buf);
	(*out_buf)++;
	(*out_left)--;*/
	(*in_buf)++;
	(*in_left)--;
    }
}

static int rccIConvUTFBytes(unsigned char c) {
    int j;
    if (c<128) return 1;

    for (j=6;j>=0;j--)
	if ((c&(1<<j))==0) break;
	    
    if ((j==0)||(j==6)) return 1;
    return 6-j;
}


rcc_iconv rccIConvOpen(const char *to, const char *from) {
    rcc_iconv icnv;
    
    if ((!from)||(!to)||(!strcasecmp(from,to))) return NULL;
    
    icnv = (rcc_iconv)malloc(sizeof(rcc_iconv_s));
    if (!icnv) return icnv;
    
    icnv->icnv = iconv_open(to, from);
    return icnv;
}

void rccIConvClose(rcc_iconv icnv) {
    if (icnv) {
	if (icnv->icnv != (iconv_t)-1) iconv_close(icnv->icnv);
	free(icnv);
    }
}

int rccIConvGetError(rcc_iconv icnv) {
    if ((!icnv)||(icnv->icnv == (iconv_t)-1)) return -1;
    return 0;
}

size_t rccIConvRecode(rcc_iconv icnv, char *outbuf, size_t outsize, const char *buf, size_t size) {
    char *in_buf, *out_buf, err;
    size_t in_left, out_left;
    int ub, utf_mode=0;
    int errors=0;
    
    if ((!buf)||(!outbuf)||(!outsize)||(!icnv)||(icnv->icnv == (iconv_t)-1)) return (size_t)-1;
    if (iconv(icnv->icnv, NULL, NULL, NULL, NULL) == -1) return (size_t)-1;

    if (!size) size = strlen(buf);
        
loop_restart:
    errors = 0;
    in_buf = (char*)buf; /*DS*/
    in_left = size;
    out_buf = outbuf;
    out_left = outsize;

loop:
    err=iconv(icnv->icnv, &in_buf, &in_left, &out_buf, &out_left);
    if (err<0) {
        if (errno==E2BIG) {
    	    *(int*)(outbuf+(RCC_MAX_STRING_CHARS-sizeof(int)))=0;
	} else if (errno==EILSEQ) {
	    if (errors++<RCC_MAX_ERRORS) {
		for (ub=utf_mode?rccIConvUTFBytes(*in_buf):1;ub>0;ub--)
		    rccIConvCopySymbol(&in_buf, &in_left, &out_buf, &out_left);
		if (in_left>0) goto loop;
	    } else if (!utf_mode) {
		utf_mode = 1;
		goto loop_restart;
	    } else {
	        return (size_t)-1;
	    }
	} else {
	    return (size_t)-1;
	}
    }
    
    outbuf[outsize - out_left] = 0;

    return outsize - out_left;
}

char *rccIConv(rcc_iconv icnv, const char *buf, size_t len, size_t *rlen) {
    char *res;
    size_t size;
    char tmpbuffer[RCC_MAX_STRING_CHARS+1];

    size = rccIConvRecode(icnv, tmpbuffer, RCC_MAX_STRING_CHARS, buf, len);
    if (size != (size_t)-1) {
	res = (char*)malloc((size+1)*sizeof(char));
	if (!res) return res;
	
	if (rlen) *rlen = size;
	memcpy(res, tmpbuffer, size);
	res[size] = 0;

	return res;
    }

    return NULL;
}

size_t rccIConvInternal(rcc_context ctx, rcc_iconv icnv, const char *buf, size_t len) {
    if (!ctx) return (size_t)-1;
    return rccIConvRecode(icnv, ctx->tmpbuffer, RCC_MAX_STRING_CHARS, buf, len);
}

