#include <iostream>
#include "libVVHD/core.h"
#include "utils.h"
#include "malloc.h"

#include "string.h"

using namespace std;

namespace {

	void three2four(char *src, char *dst);
	char index(char A);
	void four2three(char *src, char *dst);

} //end of namespace

int PrintBody(std::ostream& os, Space *S)
{
	int i, lsize;
	TList *list = S->BodyList;
	if (!list) return -1;
	lsize = list->size;
	TVortex *Vort = list->Elements;
	for ( i=0; i<lsize; i++ )
	{
		os << (*Vort) << endl;
		Vort++;
	}
	return 0;
}

int PrintVorticity(std::ostream& os, Space *S, bool PrintSpeed)
{
	int i, lsize;
	TList *list = S->VortexList;
	if (!list) return -1;
	lsize = list->size;
	TVortex *Vort = list->Elements;
	if ( !PrintSpeed )
		for ( i=0; i<lsize; i++ )
		{
			os << *Vort << endl;
			Vort++;
		}
	else
		for ( i=0; i<lsize; i++ )
		{
			os << *Vort << "\t" << Vort->vx << "\t" << Vort->vy << endl;
			Vort++;
		}
	return 0;
}

int PrintHeat(std::ostream& os, Space *S, bool PrintSpeed)
{
	int i, lsize;
	TList *list = S->HeatList;
	if (!list) return -1;
	lsize = list->size;
	TVortex *Vort = list->Elements;
	if ( !PrintSpeed )
		for ( i=0; i<lsize; i++ )
		{
			os << *Vort << endl;
			Vort++;
		}
	else
		for ( i=0; i<lsize; i++ )
		{
			os << *Vort << "\t" << Vort->vx << "\t" << Vort->vy << endl;
			Vort++;
		}
	return 0;
}

/************************ XML PART **************************************/

namespace {
static const char *base64_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
}

namespace {
void three2four(const char *src, char *dst)
{
	dst[0] = base64_alphabet[(src[0]&0xfc) >> 2];
	dst[1] = base64_alphabet[((src[0]&0x03) << 4) | ((src[1]&0xf0) >> 4)];
	dst[2] = base64_alphabet[((src[1]&0x0f) << 2) | ((src[2]&0xc0) >> 6)];
	dst[3] = base64_alphabet[(src[2]&0x3f)];
	return;
}}

char* BinToBase64(const char *src, int srclen)
{
	if (srclen%3) return NULL;
	char *result = (char*)malloc(srclen/3*4*sizeof(char)+1);
	const char *l3 = src;
	char *l4 = result;
	for (int i=0; i<srclen/3; i++)
	{
		three2four(l3, l4);
		l3+= 3;
		l4+= 4;
	}
	*l4 = 0;
	return result;
}

namespace {
char index(char A)
{
	if ( (A>='A') && (A<='Z') ) return A-'A'; else
	if ( (A>='a') && (A<='z') ) return A-'a'+26; else
	if ( (A>='0') && (A<='9') ) return A-'0'+52; else
	if ( A=='+' ) return 62; else
	if ( A=='/' ) return 63; else
	return -1;
}}

namespace {
void four2three(const char *src, char *dst)
{
	char c0=index(src[0]), c1=index(src[1]), c2=index(src[2]), c3=index(src[3]);
	dst[0] = (c0 << 2) | ((c1&0x30) >> 4);
	dst[1] = ((c1&0xF)<<4) | ((c2&0x3C) >> 2);
	dst[2] = ((c2&0x3)<<6) | c3;
}}

char* Base64ToBin(const char *src, int srclen)
{
	if (srclen%4) return NULL;
	char *result = (char*)malloc(srclen/4*3*sizeof(char));
	const char *l4 = src; 
	char *l3 = result;
	for (int i=0; i<srclen/4; i++)
	{
		four2three(l4, l3);
		l3+= 3;
		l4+= 4;
	}
	return result;
}
