#include <iostream>
#include "libVVHD/core.h"
#include "utils.h"
#include "malloc.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
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

/********************************** XML *******************************************/

xmlDoc* OpenStorage(const char *filename, const char *fileversion) //открываем хранилку с XMLем
{
	LIBXML_TEST_VERSION

	xmlDoc *doc = xmlReadFile(filename, NULL, XML_PARSE_NOWARNING);
	if (!fileversion) return doc;
	if (!doc)
	{
		doc = xmlNewDoc(BAD_CAST "1.0");
		if (!doc) return NULL;
		xmlNode *root = xmlNewNode(NULL, BAD_CAST "root");
		xmlDocSetRootElement(doc, root);
		xmlNewChild(root, NULL, BAD_CAST "version", BAD_CAST fileversion);
		xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
	} else
	{
		xmlNode *root = xmlDocGetRootElement(doc);
		for ( xmlNode *n=root->children; n; n=n->prev )
		{
			if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, "version") )
				if ( strcmp((const char*) root->children->children->content, fileversion) )
					{ cout << "Wrong fileversion. Expected is \"" << fileversion << "\"\n"; xmlFreeDoc(doc); return NULL; }
		}
	}
	return doc;
}

void CloseStorage(xmlDoc *doc) //закрываем хранилку
{
	xmlFreeDoc(doc);
	xmlCleanupParser();
}

xmlNode *AppendNode(xmlDoc *doc, const char *NodeName)
{
	xmlNode *root = xmlDocGetRootElement(doc);
	if (!root) return NULL;
	return xmlNewChild(root, NULL, BAD_CAST NodeName, NULL);
}

xmlNode* GetLastNode(xmlDoc *doc, const char *NodeName)
{
	xmlNode *root = xmlDocGetRootElement(doc);
	if (!root) return NULL;
	for ( xmlNode *n=root->last; n; n=n->prev )
	{
		if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, NodeName) )
			return n;
	}
	return NULL;
}

xmlNode* GetFirstNode(xmlDoc *doc, const char *NodeName)
{
	xmlNode *root = xmlDocGetRootElement(doc);
	if (!root) return NULL;
	for ( xmlNode *n=root->children; n; n=n->next )
	{
		if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, NodeName) )
			return n;
	}
	return NULL;
}

xmlNode* GetPrevNode(xmlNode *node)
{
	const char *NodeName = (const char*)node->name;
	for ( xmlNode *n=node->prev; n; n=n->prev )
	{
		if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, NodeName) )
			return n;
	}
	return NULL;
}

xmlNode* GetNextNode(xmlNode *node)
{
	const char *NodeName = (const char*)node->name;
	for ( xmlNode *n=node->next; n; n=n->next )
	{
		if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, NodeName) )
			return n;
	}
	return NULL;
}

int AppendNodeDouble(xmlNode *node, const char* caption, double value)
{
	if (!node) return -1;

	char *dbl = (char*)malloc(16); sprintf(dbl, "%.6lf", value);
	xmlNewChild(node, NULL, BAD_CAST caption, BAD_CAST dbl);
	return 0;
}

int AppendNodeInt(xmlNode *node, const char* caption, int value)
{
	if (!node) return -1;

	char *i = (char*)malloc(16); sprintf(i, "%d", value);
	xmlNewChild(node, NULL, BAD_CAST caption, BAD_CAST i);
	return 0;
}

int AppendNodeString(xmlNode *node, const char* caption, const char* value)
{
	if (!node) return -1;

	xmlNewChild(node, NULL, BAD_CAST caption, BAD_CAST value);
	return 0;
}


int GetHeaderDouble(xmlNode *node, const char* caption, double *value)
{
	if (!node) return -1;

	for ( xmlNode *n=node->children; n; n=n->next )
	{
		if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, caption) ) 
			{
				sscanf((const char*) n->children->content, "%lf", value);
				return 0;
			}
	}
	return GetHeaderDouble(GetPrevNode(node), caption, value);
}

int GetHeaderInt(xmlNode *node, const char* caption, int *value)
{
	if (!node) return -1;

	for ( xmlNode *n=node->children; n; n=n->next )
	{
		if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, caption) ) 
			{
				sscanf((const char*) n->children->content, "%d", value);
				return 0;
			}
	}
	return GetHeaderInt(GetPrevNode(node), caption, value);
}

int GetHeaderString(xmlNode *node, const char* caption, char **value)
{
	if (!node) return -1;

	for ( xmlNode *n=node->children; n; n=n->next )
	{
		if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, caption) ) 
			{
				*value = (char*)malloc(strlen((const char*)n->children->content)+1);
				sprintf(*value, "%s", (const char*)n->children->content);
				return 0;
			}
	}
	return GetHeaderString(GetPrevNode(node), caption, value);
}

int GetStepDouble(xmlNode *node, const char* caption, double *value)
{
	if (!node) return -1;

	for ( xmlNode *n=node->children; n; n=n->next )
	{
		if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, caption) ) 
			{
				sscanf((const char*) n->children->content, "%lf", value);
				return 0;
			}
	}
	return -1;
}

int GetStepInt(xmlNode *node, const char* caption, int *value)
{
	if (!node) return -1;

	for ( xmlNode *n=node->children; n; n=n->next )
	{
		if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, caption) ) 
			{
				sscanf((const char*) n->children->content, "%d", value);
				return 0;
			}
	}
	return -1;
}

int GetStepString(xmlNode *node, const char* caption, char **value)
{
	if (!node) return -1;

	for ( xmlNode *n=node->children; n; n=n->next )
	{
		if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, caption) ) 
			{
				*value = (char*)malloc(strlen((const char*)n->children->content)+1);
				sprintf(*value, "%s", (const char*)n->children->content);
				return 0;
			}
	}
	return -1;
}

bool CheckStepParameter(xmlNode *node, const char* caption)
{
	if (!node) return false;

	for ( xmlNode *n=node->children; n; n=n->next )
	{
		if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, caption) ) 
			{
				return true;
			}
	}
	return false;
}

int LoadVorticityFromStep(Space* S, xmlNode* node)
{
	char *EncodedVorticity=NULL;
	if(GetStepString(node, "VortexField", &EncodedVorticity)) return -2;

	int len = strlen(EncodedVorticity);
	int lsize = len/4/sizeof(double);
	double *DecodedVorticity = (double*)Base64ToBin((const char*)EncodedVorticity, len);
	double *Pos = DecodedVorticity;
	TVortex vort; ZeroVortex(vort);
	for (int i=0; i<lsize; i++)
	{
		vort.rx = *Pos++;
		vort.ry = *Pos++;
		vort.g = *Pos++;
		S->VortexList->Add(vort);
	}
	
	GetStepDouble(node, "Time", &S->Time);
	free(DecodedVorticity);
	free(EncodedVorticity);
	return 0;
}

int LoadVorticityFromLastStep(Space* S, xmlDoc* doc)
{
	if (!S || !doc) return -1;

	char *EncodedVorticity=NULL;

	xmlNode* step=GetLastNode(doc, "step"), *ul;

	while(step && GetStepString(step, "VortexField", &EncodedVorticity))
	{
		ul = step;
		step = GetPrevNode(step);
		xmlUnlinkNode(ul);
	}

	if (!EncodedVorticity) return -2;
	int len = strlen(EncodedVorticity);
	int lsize = len/4/sizeof(double);
	double *DecodedVorticity = (double*)Base64ToBin((const char*)EncodedVorticity, len);
	double *Pos = DecodedVorticity;
	TVortex vort; ZeroVortex(vort);
	for (int i=0; i<lsize; i++)
	{
		vort.rx = *Pos++;
		vort.ry = *Pos++;
		vort.g = *Pos++;
		S->VortexList->Add(vort);
	}
	
	GetStepDouble(step, "Time", &S->Time);
	free(DecodedVorticity);
	free(EncodedVorticity);
	return 0;
}

int SaveVorticityToStep(Space *S, xmlNode* step)
{
	if (!S || !S->VortexList || !step) return -1;
	TList* vlist = S->VortexList;
	int vlsize = vlist->size;
	TVortex *v=vlist->Elements;

	double *DecodedVorticity = (double*) malloc(vlsize*4*sizeof(double));
	double *Pos = DecodedVorticity;
	for (int i=0; i<vlsize; i++)
	{
		*Pos = v->rx; Pos++;
		*Pos = v->ry; Pos++;
		*Pos = v->g; Pos++;
		v++;
	}

	char *EncodedVorticity = BinToBase64((const char*)DecodedVorticity, vlsize*3*sizeof(double));

	xmlNewChild(step, NULL, BAD_CAST "VortexField", BAD_CAST EncodedVorticity);
	free(EncodedVorticity);
	free(DecodedVorticity);

	return 0;
}
