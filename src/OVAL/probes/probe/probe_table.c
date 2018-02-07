/*
 * Copyright 2018 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 *      "Jan Cerny" <jcerny@redhat.com>
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "probe_table.h"
#include "oval_definitions.h"
#include "oval_definitions_impl.h"

typedef struct probe_table_entry {
    oval_subtype_t type;
    probe_function_t function;
} probe_table_entry_t;

static const probe_table_entry_t probe_table[] = {
    {OVAL_INDEPENDENT_FAMILY, family_probe_main},
    {OVAL_INDEPENDENT_SYSCHAR_SUBTYPE, system_info_probe_main},
    {OVAL_INDEPENDENT_TEXT_FILE_CONTENT_54, textfilecontent54_probe_main},
    {OVAL_SUBTYPE_UNKNOWN, NULL}
};

probe_function_t probe_table_get(oval_subtype_t type)
{
    probe_table_entry_t *entry = probe_table;
    while (entry->function != NULL && entry->type != type) {
        entry++;
    }
    return entry->function;
}