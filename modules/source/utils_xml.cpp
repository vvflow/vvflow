#include <iostream>
#include "libVVHD/core.h"
#include "utils.h"
#include "string.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
using namespace std;

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
