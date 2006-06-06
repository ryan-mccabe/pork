/*
** Copyright (C) 2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License version 2,
** as published by the Free Software Foundation.
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "xmlParser.h"

typedef void *xml_node;
typedef void *xml_node_contents;

LPTSTR fromXMLString(LPCTSTR s, int lo);
int lengthXMLString(LPCTSTR source);

LPTSTR str_to_xml(LPCTSTR src) {
	return (toXMLString(src));
}

LPTSTR strn_to_xml(LPTSTR dest, LPCTSTR src, int len) {
	return (toXMLStringFast(&dest, &len, src));
}

LPTSTR xml_to_str(LPCTSTR src, int len) {
	return (fromXMLString(src, len));
}

LPCTSTR xml_get_error(xml_node node, XMLError err) {
	return ((XMLNode *) node)->getError(err);
}

int xml_strlen(LPCTSTR source) {
	return (lengthXMLString(source));
}

void xml_node_set_name(xml_node node, LPCTSTR lpszName) {
	((XMLNode *) node)->setName(lpszName);
}

xml_node xml_node_name_add_child(	xml_node node,
									LPCTSTR lpszName,
									int isDeclaration)
{
	((XMLNode *) node)->addChild(lpszName, isDeclaration);
	return (node);
}

xml_node xml_create_top_node(void) {
	XMLNode *x = new XMLNode;
	*x = XMLNode::createXMLTopNode();
	return (x);
}

xml_node_contents xml_nth_node_content(xml_node node, int i) {
	XMLNodeContents *nc = new XMLNodeContents;
	*nc = ((XMLNode *) node)->enumContents(i);
	return (nc);
}

LPCTSTR xml_node_add_text(xml_node node, LPCTSTR lpszValue) {
	return ((XMLNode *) node)->addText(lpszValue);
}

XMLClear *xml_node_add_clear(	xml_node node,
								LPCTSTR lpszValue,
								LPCTSTR lpszOpen,
								LPCTSTR lpszClose)
{
	return ((XMLNode *) node)->addClear(lpszValue, lpszOpen, lpszClose);
}

XMLAttribute *xml_node_add_attr(xml_node node,
								LPCTSTR lpszName,
								LPCTSTR lpszValuev)
{
	return ((XMLNode *) node)->addAttribute(lpszName, lpszValuev);
}

void xml_node_del_attr(xml_node node, XMLAttribute *a) {
	return ((XMLNode *) node)->deleteAttribute(a);
}

void xml_node_nth_del_attr(xml_node node, int i) {
	((XMLNode *) node)->deleteAttribute(i);
}

void xml_node_name_del_attr(xml_node node, LPCTSTR lpszName) {
	((XMLNode *) node)->deleteAttribute(lpszName);
}

void xml_node_name_del_attr_relaxed(xml_node node, LPCTSTR lpszName) {
	((XMLNode *) node)->deleteAttributeRelaxed(lpszName);
}

xml_node xml_parse_str(LPCTSTR lpszXML,
						LPCTSTR tag,
						XMLResults *pResults)
{
	XMLNode *node = new XMLNode;
	*node = XMLNode::parseString(lpszXML, tag, pResults);
	return (node);
}

xml_node xml_parse_file(	const char *filename,
							LPCTSTR tag,
							XMLResults *pResults)
{
	XMLNode *node = new XMLNode;
	*node = XMLNode::parseFile(filename, tag, pResults);
	return (node);
}

xml_node xml_open_file_helper(const char *lpszXML, LPCTSTR tag) {
	XMLNode *node = new XMLNode;
	*node = XMLNode::openFileHelper(lpszXML, tag);
	return (node);
}

LPTSTR xml_node_create_string(xml_node node, int nFormat, int *pnSize) {
	return ((XMLNode *) node)->createXMLString(nFormat, pnSize);
}

void xml_node_del_content(xml_node node) {
	((XMLNode *) node)->deleteNodeContent();
}

char xml_node_set_attr(xml_node node, LPCTSTR lpszAttrib) {
	return ((XMLNode *) node)->isAttributeSet(lpszAttrib);
}

LPCTSTR xml_node_get_nth_attr(xml_node node, LPCTSTR name, int j) {
	return ((XMLNode *) node)->getAttribute(name, j);
}

LPCTSTR xml_node_get_attr(xml_node node, LPCTSTR name) {
	return ((XMLNode *) node)->getAttribute(name);
}

LPCTSTR xml_node_get_name(xml_node node) {
	return ((XMLNode *) node)->getName();
}

void xml_node_del_clear(xml_node node, XMLClear *clear) {
	((XMLNode *) node)->deleteClear(clear);
}

void xml_node_del_nth_clear(xml_node node, int i) {
	((XMLNode *) node)->deleteClear(i);
}

void xml_node_name_del_clear(xml_node node, LPCTSTR lpszValue) {
	((XMLNode *) node)->deleteClear(lpszValue);
}

void xml_node_del_nth_text(xml_node node, int i) {
	((XMLNode *) node)->deleteText(i);
}

void xml_node_del_text(xml_node node, LPCTSTR lpszValue) {
	((XMLNode *) node)->deleteText(lpszValue);
}

int xml_node_num_text(xml_node node) {
	return ((XMLNode *) node)->nText();
}

int xml_node_name_num_children(xml_node node, LPCTSTR name) {
	return ((XMLNode *) node)->nChildNode(name);
}

int xml_node_num_children(xml_node node) {
	return ((XMLNode *) node)->nChildNode();
}

int xml_node_num_attr(xml_node node) {
	return ((XMLNode *) node)->nAttribute();
}

int xml_node_num_clear(xml_node node) {
	return ((XMLNode *) node)->nClear();
}

int xml_node_num_elements(xml_node node) {
	return ((XMLNode *) node)->nElement();
}

char xml_node_is_declaration(xml_node node) {
	return ((XMLNode *) node)->isDeclaration();
}

char xml_node_is_empty(xml_node node) {
	return ((XMLNode *) node)->isEmpty();
}

LPCTSTR xml_node_get_text(xml_node node) {
	return ((XMLNode *) node)->getText();
}

LPCTSTR xml_node_get_nth_text(xml_node node, int i) {
	return ((XMLNode *) node)->getText(i);
}

XMLClear xml_node_get_nth_clear(xml_node node, int i) {
	return ((XMLNode *) node)->getClear(i);
}

XMLAttribute xml_node_get_nth_attribute(xml_node node, int i) {
	return ((XMLNode *) node)->getAttribute(i);
}

xml_node xml_node_get_nth_child(xml_node node, int i) {
	return (xml_node) ((XMLNode *) node)->getChildNodePtr(i);
}

xml_node xml_node_get_child(xml_node node, LPCTSTR name) {
	return (xml_node) ((XMLNode *) node)->getChildNodePtr(name);
}
