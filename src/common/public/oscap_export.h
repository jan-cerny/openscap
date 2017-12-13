/*
 * Copyright 2017 Red Hat Inc., Durham, North Carolina.
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
 *    Jan Černý <jcerny@redhat.com>
 */

#ifndef OSCAP_EXPORT_H
#define OSCAP_EXPORT_H

/**
 * OSCAP_EXPORT macro
 *
 * OSCAP_EXPORT is a macro that marks every public function in OpenSCAP API.
 * Functions marked with OSCAP_EXPORT will be exported to the shared library
 * interface.
 */
#if defined(_WIN32) && defined(_MSC_VER)
#define OSCAP_EXPORT __declspec(dllexport)
#else
#define OSCAP_EXPORT
#endif
