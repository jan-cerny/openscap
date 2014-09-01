/*
 * Copyright 2009--2014 Red Hat Inc., Durham, North Carolina.
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
 *      Lukas Kuklinek <lkuklinek@redhat.com>
 *      Peter Vrabec <pvrabec@redhat.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libexslt/exslt.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "public/oscap.h"
#include "_error.h"
#include "util.h"
#include "list.h"
#include "elements.h"
#include "assume.h"
#include "debug_priv.h"
#include "oscap_source.h"
#include "source/validate_priv.h"
#include "source/xslt_priv.h"

#ifndef OSCAP_DEFAULT_SCHEMA_PATH
const char * const OSCAP_SCHEMA_PATH = "/usr/local/share/openscap/schemas";
#else
const char * const OSCAP_SCHEMA_PATH = OSCAP_DEFAULT_SCHEMA_PATH;
#endif

#ifndef OSCAP_DEFAULT_XSLT_PATH
const char * const OSCAP_XSLT_PATH = "/usr/local/share/openscap/xsl";
#else
const char * const OSCAP_XSLT_PATH = OSCAP_DEFAULT_XSLT_PATH;
#endif

#ifndef OSCAP_DEFAULT_CPE_PATH
const char * const OSCAP_CPE_PATH = "/usr/local/share/openscap/cpe";
#else
const char * const OSCAP_CPE_PATH = OSCAP_DEFAULT_CPE_PATH;
#endif

/* return default path if pathvar is not defined */
static const char * oscap_path_to(const char *pathvar, const char *defpath) {
	const char *path = NULL;

	if (pathvar != NULL)
		path = getenv(pathvar);

	if (path == NULL || oscap_streq(path, ""))
		path = defpath;

	return path;
}

const char * oscap_path_to_schemas() {
	return oscap_path_to("OSCAP_SCHEMA_PATH", OSCAP_SCHEMA_PATH);
}

static inline const char *oscap_path_to_xslt(void)
{
	return oscap_path_to("OSCAP_XSLT_PATH", OSCAP_XSLT_PATH);
}

OSCAP_DEPRECATED(
const char * oscap_path_to_schematron() {
	// It has never returned correct path to schematron files.
	return oscap_path_to_xslt();
}
)

const char * oscap_path_to_cpe() {
	return oscap_path_to("OSCAP_CPE_PATH", OSCAP_CPE_PATH);
}

void oscap_init(void)
{
    xmlInitParser();
    xsltInit();
    exsltRegisterAll();
}

void oscap_cleanup(void)
{
	oscap_clearerr();
	xsltCleanupGlobals();
	xmlCleanupParser();
}

const char *oscap_get_version(void) { return VERSION; }

int oscap_validate_document(const char *xmlfile, oscap_document_type_t doctype, const char *version, xml_reporter reporter, void *arg)
{
	if (xmlfile == NULL) {
		oscap_seterr(OSCAP_EFAMILY_OSCAP, "'xmlfile' == NULL");
		return -1;
	}

	if (access(xmlfile, R_OK)) {
		oscap_seterr(OSCAP_EFAMILY_GLIBC, "%s '%s'", strerror(errno), xmlfile);
		return -1;
	}

	struct oscap_source *source = oscap_source_new_from_file(xmlfile);
	int ret = oscap_source_validate_priv(source, doctype, version, reporter, arg);
	oscap_source_free(source);
	return ret;
}

/*
 * Apply stylesheet on XML file.
 * If xsltfile is an absolute path to the stylesheet, path_to_xslt will not be used.
 *
 */
static int oscap_apply_xslt_path(const char *xmlfile, const char *xsltfile,
				 const char *outfile, const char **params, const char *path_to_xslt)
{
	struct oscap_source *source = oscap_source_new_from_file(xmlfile);
	int ret = oscap_source_apply_xslt_path(source, xsltfile, outfile, params, path_to_xslt);
	oscap_source_free(source);
	return ret;
}

struct oscap_schema_table_entry OSCAP_SCHEMATRON_TABLE[] = {
        {OSCAP_DOCUMENT_OVAL_DEFINITIONS,       "5.3",		"oval/5.3/oval-definitions-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_DEFINITIONS,       "5.4",  	"oval/5.4/oval-definitions-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_DEFINITIONS,       "5.5",  	"oval/5.5/oval-definitions-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_DEFINITIONS,       "5.6",  	"oval/5.6/oval-definitions-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_DEFINITIONS,       "5.7",  	"oval/5.7/oval-definitions-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_DEFINITIONS,       "5.8",  	"oval/5.8/oval-definitions-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_DEFINITIONS,       "5.9",  	"oval/5.9/oval-definitions-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_DEFINITIONS,       "5.10",  	"oval/5.10/oval-definitions-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_DEFINITIONS,       "5.10.1",	"oval/5.10.1/oval-definitions-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_SYSCHAR,		"5.3",		"oval/5.3/oval-system-characteristics-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_SYSCHAR,		"5.4",		"oval/5.4/oval-system-characteristics-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_SYSCHAR,		"5.5",		"oval/5.5/oval-system-characteristics-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_SYSCHAR,		"5.6",		"oval/5.6/oval-system-characteristics-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_SYSCHAR,		"5.7",		"oval/5.7/oval-system-characteristics-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_SYSCHAR,		"5.8",		"oval/5.8/oval-system-characteristics-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_SYSCHAR,		"5.9",		"oval/5.9/oval-system-characteristics-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_SYSCHAR,		"5.10",		"oval/5.10/oval-system-characteristics-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_SYSCHAR,		"5.10.1",	"oval/5.10.1/oval-system-characteristics-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_RESULTS,		"5.3",		"oval/5.3/oval-results-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_RESULTS,		"5.4",		"oval/5.4/oval-results-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_RESULTS,		"5.5",		"oval/5.5/oval-results-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_RESULTS,		"5.6",		"oval/5.6/oval-results-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_RESULTS,		"5.7",		"oval/5.7/oval-results-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_RESULTS,		"5.8",		"oval/5.8/oval-results-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_RESULTS,		"5.9",		"oval/5.9/oval-results-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_RESULTS,		"5.10",		"oval/5.10/oval-results-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_RESULTS,		"5.10.1",	"oval/5.10.1/oval-results-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_VARIABLES,		"5.3",		"oval/5.3/oval-variables-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_VARIABLES,		"5.4",		"oval/5.4/oval-variables-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_VARIABLES,		"5.5",		"oval/5.5/oval-variables-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_VARIABLES,		"5.6",		"oval/5.6/oval-variables-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_VARIABLES,		"5.7",		"oval/5.7/oval-variables-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_VARIABLES,		"5.8",		"oval/5.8/oval-variables-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_VARIABLES,		"5.9",		"oval/5.9/oval-variables-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_VARIABLES,		"5.10",		"oval/5.10/oval-variables-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_VARIABLES,		"5.10.1",	"oval/5.10.1/oval-variables-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_DIRECTIVES,	"5.8",		"oval/5.8/oval-directives-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_DIRECTIVES,	"5.9",		"oval/5.9/oval-directives-schematron.xsl"},
        {OSCAP_DOCUMENT_OVAL_DIRECTIVES,	"5.10",		"oval/5.10/oval-directives-schematron.xsl"},
	{OSCAP_DOCUMENT_OVAL_DIRECTIVES,        "5.10.1",       "oval/5.10.1/oval-directives-schematron.xsl"},
	{OSCAP_DOCUMENT_XCCDF,                  "1.2",          "xccdf/1.2/xccdf_1.2-schematron.xsl"},
};

int oscap_schematron_validate_document(const char *xmlfile, oscap_document_type_t doctype, const char *version, const char *outfile) {

        struct oscap_schema_table_entry *entry;
	const char *params[] = { NULL };

        if (xmlfile == NULL) {
                oscap_seterr(OSCAP_EFAMILY_OSCAP, "'xmlfile' == NULL");
                return -1;
        }

        if (access(xmlfile, R_OK)) {
                oscap_seterr(OSCAP_EFAMILY_GLIBC, "%s '%s'", strerror(errno), xmlfile);
                return -1;
        }

        if (version == NULL) {
                oscap_seterr(OSCAP_EFAMILY_OSCAP, "'version' == NULL");
                return -1;
        }

	/* find a right schematron file */
	for (entry = OSCAP_SCHEMATRON_TABLE; entry->doc_type != 0; ++entry) {
		if (entry->doc_type != doctype || strcmp(entry->schema_version, version))
			continue;

		/* validate */
                return oscap_apply_xslt_path(xmlfile, entry->schema_path, NULL, params, oscap_path_to_schemas());
	}

	/* schematron not found */
	if (entry->doc_type == 0) {
		oscap_seterr(OSCAP_EFAMILY_OSCAP, "Schematron rules not found when trying to validate '%s'", xmlfile);
		return -1;
	}

	/* we shouldn't get here */
	return -1;
}

int oscap_apply_xslt(const char *xmlfile, const char *xsltfile, const char *outfile, const char **params)
{
	return oscap_apply_xslt_path(xmlfile, xsltfile, outfile, params, oscap_path_to_xslt());
}

int oscap_determine_document_type(const char *document, oscap_document_type_t *doc_type) {
	struct oscap_source *source = oscap_source_new_from_file(document);
	*doc_type = oscap_source_get_scap_type(source);
	oscap_source_free(source);
	return (*doc_type == 0) ? -1 : 0;
}

const char *oscap_document_type_to_string(oscap_document_type_t type)
{
	switch (type) {
	case OSCAP_DOCUMENT_OVAL_DEFINITIONS:
		return "OVAL Definition";
	case OSCAP_DOCUMENT_OVAL_DIRECTIVES:
		return "OVAL Directives";
	case OSCAP_DOCUMENT_OVAL_RESULTS:
		return "OVAL Results";
	case OSCAP_DOCUMENT_OVAL_SYSCHAR:
		return "OVAL System Characteristics";
	case OSCAP_DOCUMENT_OVAL_VARIABLES:
		return "OVAL Variables";
	case OSCAP_DOCUMENT_SDS:
		return "SCAP Source Datastream";
	case OSCAP_DOCUMENT_XCCDF:
		return "XCCDF Checklist";
	case OSCAP_DOCUMENT_SCE_RESULT:
		return "SCE Results";
	case OSCAP_DOCUMENT_CPE_DICTIONARY:
		return "CPE Dictionary";
	case OSCAP_DOCUMENT_CPE_LANGUAGE:
		return "CPE Language";
	case OSCAP_DOCUMENT_ARF:
		return "ARF Result Datastream";
	case OSCAP_DOCUMENT_CVE_FEED:
		return "CVE NVD Feed";
	default:
		return NULL;
	}
}
