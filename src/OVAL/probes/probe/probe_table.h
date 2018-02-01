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

#ifndef OPENSCAP_PROBE_TABLE_H
#define OPENSCAP_PROBE_TABLE_H

#include "probe-api.h"


typedef int (*probe_function_t) (probe_ctx *ctx, void *arg);

/*
 * Get a pointer to main probe function
 */
probe_function_t probe_table_get(oval_subtype_t type);


/* Probe function prototypes */
int family_probe_main(probe_ctx *ctx, void *arg);


#endif //OPENSCAP_PROBE_TABLE_H
