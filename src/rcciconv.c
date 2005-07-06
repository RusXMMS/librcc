#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>

#include "internal.h"
#include "rcciconv.h"

static void rccIConvCopySymbol(char **in_buf, int *in_left, char **out_buf, int *out_left) {
    if ((out_left>0)&&(in_left>0)) {
	(**out_buf)=(**in_buf);
	(*out_buf)++;
	(*in_buf)++;
	(*in_left)--;
	(*out_left)--;
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

size_t rccIConv(rcc_context ctx, iconv_t icnv, const char *buf, size_t len) {
    char *in_buf, *out_buf, *res, err;
    int in_left, out_left, olen;
    int ub, utf_mode=0;
    int errors=0;
    
    if ((!buf)||(!ctx)||(icnv == (iconv_t)-1)) return (size_t)-1;
    
    len = STRNLEN(buf,len);
    
    if (iconv(icnv, NULL, NULL, NULL, NULL) == -1) return (size_t)-1;
    
loop_restart:
    errors = 0;
    in_buf = (char*)buf; /*DS*/
    in_left = len;
    out_buf = ctx->tmpbuffer;
    out_left = RCC_MAX_STRING_CHARS;

loop:
    err=iconv(icnv, &in_buf, &in_left, &out_buf, &out_left);
    if (err<0) {
        if (errno==E2BIG) {
    	    *(int*)(ctx->tmpbuffer+(RCC_MAX_STRING_CHARS-sizeof(int)))=0;
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
    
    ctx->tmpbuffer[RCC_MAX_STRING_CHARS - out_left] = 0;

    return RCC_MAX_STRING_CHARS - out_left;
}
