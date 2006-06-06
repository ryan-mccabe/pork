/*
** Copyright (C) 2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License version 2,
** as published by the Free Software Foundation.
*/

#ifndef __XML_C_WRAPPER_H
#define __XML_C_WRAPPER_H

#include "xmlCSafe.h"

typedef void *xml_node;
typedef void *xml_node_contents;

LPTSTR str_to_xml(LPCTSTR src);
LPTSTR strn_to_xml(LPTSTR *dest, LPCTSTR src, int len);
LPTSTR xml_to_str(LPCTSTR src, int len);
LPTSTR xml_node_create_string(xml_node node, int nFormat, int *pnSize);

LPCTSTR xml_get_error(xml_node node, XMLError err);
LPCTSTR xml_node_add_text(xml_node node, LPCTSTR lpszValue);
LPCTSTR xml_node_get_text(xml_node node);
LPCTSTR xml_node_get_nth_attr(xml_node node, LPCTSTR name, int j);
LPCTSTR xml_node_get_attr(xml_node node, LPCTSTR name);
LPCTSTR xml_node_get_name(xml_node node);
LPCTSTR xml_node_get_nth_text(xml_node node, int i);

void xml_node_set_name(xml_node node, LPCTSTR lpszName);
void xml_node_del_attr(xml_node node, XMLAttribute *a);
void xml_node_nth_del_attr(xml_node node, int i);
void xml_node_name_del_attr(xml_node node, LPCTSTR lpszName);
void xml_node_name_del_attr_relaxed(xml_node node, LPCTSTR lpszName);
void xml_node_del_content(xml_node node);
void xml_node_del_clear(xml_node node, XMLClear *clear);
void xml_node_del_nth_clear(xml_node node, int i);
void xml_node_name_del_clear(xml_node node, LPCTSTR lpszValue);
void xml_node_del_nth_text(xml_node node, int i);
void xml_node_del_text(xml_node node, LPCTSTR lpszValue);

int xml_strlen(LPCTSTR source);
int xml_node_num_text(xml_node node);
int xml_node_name_num_children(xml_node node, LPCTSTR name);
int xml_node_num_children(xml_node node);
int xml_node_num_attr(xml_node node);
int xml_node_num_clear(xml_node node);
int xml_node_num_elements(xml_node node);

char xml_node_set_attr(xml_node node, LPCTSTR lpszAttrib);
char xml_node_is_declaration(xml_node node);
char xml_node_is_empty(xml_node node);

XMLClear xml_node_get_nth_clear(xml_node node, int i);
XMLClear *xml_node_add_clear(	xml_node node,
								LPCTSTR lpszValue,
								LPCTSTR lpszOpen,
								LPCTSTR lpszClose);


XMLAttribute xml_node_get_nth_attribute(xml_node node, int i);
XMLAttribute *xml_node_add_attr(xml_node node,
								LPCTSTR lpszName,
								LPCTSTR lpszValuev);


xml_node xml_node_get_child(xml_node node, LPCTSTR name);
xml_node xml_node_get_nth_child(xml_node node, int i);
xml_node xml_create_top_node(void);

xml_node xml_open_file_helper(const char *lpszXML, LPCTSTR tag);

xml_node xml_parse_file(	const char *filename,
							LPCTSTR tag,
							XMLResults *pResults);

xml_node xml_parse_str(LPCTSTR lpszXML,
						LPCTSTR tag,
						XMLResults *pResults);

xml_node xml_node_name_add_child(	xml_node node,
									LPCTSTR lpszName,
									int isDeclaration);

xml_node_contents xml_nth_node_content(xml_node node, int i);

#else
#	warn "included multiple times"
#endif
