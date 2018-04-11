/*
  LibRCC - ISO8859-1/UTF-8 detection engine

  Copyright (C) 2005-2018 Suren A. Chilingaryan <csa@suren.me>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License version 2.1 or later
  as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
  for more details.

  You should have received a copy of the GNU Lesser General Public License 
  along with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <string.h>

#include <librcc.h>

#define bit(i) (1<<i)

/* 
 * Latin unicode subset:
 * 0x100 - 0x17E
 * 0x180 - 0x24F
 * 0x1E00 - 0x1EFF
 */

static rcc_autocharset_id AutoengineWestern(rcc_engine_context ctx, const char *sbuf, int len) {
    const unsigned char *buf = (const unsigned char*)sbuf;
    long i,j;
    int bytes=0,rflag=0;
    int res=0;

    if (!len) len = strlen((char*)buf);
    for (i=0;i<len;i++) {
	if (buf[i]<128) continue;
	
	if (bytes>0) {
	    if ((buf[i]&0xC0)==0x80) {
		if (rflag) {
		    // Western is 0x100-0x17e
		    res++;
		}
		bytes--;
	    } else {
		res--;
		bytes=1-bytes;
		rflag=0;
	    }
	} else {
	    for (j=6;j>=0;j--)
		if ((buf[i]&bit(j))==0) break;
	    
	    if ((j==0)||(j==6)) {
		if ((j==6)&&(bytes<0)) bytes++;
		else res--;
		continue;
	    }
	    bytes=6-j;
	    if (bytes==1) {
		// Western Languages (C2-C3)
		if (buf[i]==0xC2) rflag=1;
		else if (buf[i]==0xC3) rflag=2;
	    }
	}
	    
	if ((buf[i]==0xC0)||(buf[i]==0xC1)) {
	    if (i+1==len) break;
	    	    
	}
    }

    if (res > 0) return (rcc_autocharset_id)0;
    return (rcc_autocharset_id)1;
}

static rcc_engine western_engine = {
    "Western", NULL, NULL, &AutoengineWestern, {"UTF-8","ISO8859-1", NULL}
};

rcc_engine *rccGetInfo(const char *lang) {
    if (!lang) return NULL;

    return &western_engine;
}
