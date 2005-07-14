/*
** pork_irc_cmd.h - IRC specific user commands
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_IRC_CMD_H
#define __PORK_IRC_CMD_H

void irc_cmd_setup(struct pork_proto *proto);

#else
#	warning "included multiple times"
#endif
