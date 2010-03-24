#ifndef UTILS_XML_H_
#define UTILS_XML_H_

#include <libxml/parser.h>
#include <libxml/tree.h>

xmlDoc* OpenStorage(const char *filename, const char *fileversion);
void CloseStorage(xmlDoc *doc);
// to save storage use: xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);

xmlNode *AppendNode(xmlDoc *doc, const char *NodeName);
#define AppendHeader(doc) AppendNode(doc, "header")
#define AppendStep(doc) AppendNode(doc, "step")

xmlNode* GetLastNode(xmlDoc *doc, const char *NodeName);
#define GetLastHeader(doc) GetLastNode(doc, "header")
#define GetLastStep(doc) GetLastNode(doc, "step")
xmlNode* GetFirstNode(xmlDoc *doc, const char *NodeName);
#define GetFirstHeader(doc) GetFirstNode(doc, "header")
#define GetFirstStep(doc) GetFirstNode(doc, "step")
xmlNode* GetPrevNode(xmlNode *node);
xmlNode* GetNextNode(xmlNode *node);

int AppendNodeDouble(xmlNode *node, const char* caption, double value);
int AppendNodeInt(xmlNode *node, const char* caption, int value);
int AppendNodeString(xmlNode *node, const char* caption, const char* value);

int GetHeaderDouble(xmlNode *node, const char* caption, double *value);
int GetHeaderInt(xmlNode *node, const char* caption, int *value);
int GetHeaderString(xmlNode *node, const char* caption, char **value);
int GetStepDouble(xmlNode *node, const char* caption, double *value);
int GetStepInt(xmlNode *node, const char* caption, int *value);
int GetStepString(xmlNode *node, const char* caption, char **value);
bool CheckStepParameter(xmlNode *node, const char* caption);

int LoadVorticityFromStep(Space* S, xmlNode* node);
int LoadVorticityFromLastStep(Space* S, xmlDoc* doc);
int SaveVorticityToStep(Space *S, xmlNode* step);

#endif
