/*
   xmlpp - an xml parser and validator written in C++
   (c) 2000 Michael Fink
   (c) 2001 Karl Pitrich

   $Id$
*/

/*! \file xmlcommon.hpp

  common typedefs and enums

*/

#ifndef __xmlcommom_h__
#define __xmlcommom_h__

//! namespace of the xmlpp project
namespace xmlpp {

//! dummy define		
#define XMLPP_API
		
//! handle to a tagname string in a tagname map
typedef int xmltagnamehandle;
//! maps the tagname string to a handle
typedef std::map<xmltagnamehandle,string> xmltagnamemap;
//! maps an entity to a string representation
typedef std::map<string,string> xmlentitymap;

		
//! xml parsing error codes enumeration
enum xmlerrorcode {
	//! unspecified or unknown error
	xml_unknown = 0,
	//! error in the infile stream
	xml_instream_error,
	//! expected an open tag literal '<'
	xml_opentag_expected,
	//! expected a '<' or cdata
	xml_opentag_cdata_expected,
	//! expected a '>' closing tag literal
	xml_closetag_expected,
	//! expected a processing instruction or doctype tag
	xml_pi_doctype_expected,
	//! expected a tag name after '<' or '</'
	xml_tagname_expected,
	//! expected a '/' after closing tag literal '<'
	xml_closetag_slash_expected,
	//! tag name from start and end tag mismatch
	xml_tagname_close_mismatch,
	//! expected '=' after attribute name
	xml_attr_equal_expected,
	//! expected value after an '=' in attribute
	xml_attr_value_expected,
	//! invalid nodetype encountered while saving
	xml_save_invalid_nodetype,
	//! dummy error code
	xml_dummy,
	//! requested name not found
	xml_name_not_found,
	//! node has no children
	xml_childlist_empty,
	//! no valid filename provided
	xml_filename_invalid,
	//! file could not be accessed
	xml_file_access	
};

/** node type enumeration */
enum xmlnodetype
{
	xml_nt_node,      /**< normal node, can contain subnodes */	
	xml_nt_leaf,      /**< a leaf node, which contains no further nodes, eg. <img/> */	
	xml_nt_document,  /**< document root node */	
	xml_nt_cdata,     /**< cdata node, which only contains char data */	
	xml_nt_dummy      /**< dummy node type */
};


}; // end namespace xmlpp

#endif
/* vi: set ts=3: */
