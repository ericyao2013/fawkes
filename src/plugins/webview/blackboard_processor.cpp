
/***************************************************************************
 *  blackboard_processor.cpp - Web request processor for BlackBoard info
 *
 *  Created: Thu Oct 23 16:10:21 2008
 *  Copyright  2006-2008  Tim Niemueller [www.niemueller.de]
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include "blackboard_processor.h"
#include <webview/page_reply.h>
#include <webview/file_reply.h>
#include <webview/error_reply.h>

#include <core/exceptions/software.h>

#include <blackboard/blackboard.h>
#include <interface/interface.h>
#include <interface/field_iterator.h>
#include <interface/interface_info.h>
#include <utils/time/time.h>
#include <utils/misc/string_split.h>
#include <utils/misc/string_conversions.h>

#include <string>
#include <cstring>
#include <cstdlib>
#include <limits>

#include <set>
#include <sstream>
#include <algorithm>
#ifdef HAVE_GRAPHVIZ
#  include <gvc.h>
#  include <gvcjob.h>
#endif


using namespace fawkes;

/** @class WebviewBlackBoardRequestProcessor "blackboard_processor.h"
 * BlackBoard web request processor.
 * Provides access to BlackBoard introspection features.
 * @author Tim Niemueller
 */

/** Constructor.
 * @param baseurl Base URL where processor is mounted
 * @param blackboard BlackBoard instance
 * @param enable_msgs whether to allow sending msgs via webview
 */
WebviewBlackBoardRequestProcessor::WebviewBlackBoardRequestProcessor(const char *baseurl,
								     BlackBoard *blackboard, bool enable_msgs)
{
  __baseurl     = strdup(baseurl);
  __baseurl_len = strlen(__baseurl);
  __blackboard  = blackboard;
  __enable_msgs = enable_msgs;
}


/** Destructor. */
WebviewBlackBoardRequestProcessor::~WebviewBlackBoardRequestProcessor()
{
  free(__baseurl);
  for (__ifi = __interfaces.begin(); __ifi != __interfaces.end(); ++__ifi) {
    __blackboard->close(__ifi->second);
  }
  __interfaces.clear();
}


WebReply *
WebviewBlackBoardRequestProcessor::process_request(const fawkes::WebRequest *request)
{
  if ( strncmp(__baseurl, request->url().c_str(), __baseurl_len) == 0 ) {
    // It is in our URL prefix range
    std::string subpath = request->url().substr(__baseurl_len);

    if (subpath.find("/graph/graph.png") == 0) {
#if defined(HAVE_GRAPHVIZ) && ((defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))) || defined(__clang__))
      std::string graph_node = request->get_value("for");
      std::string graph = generate_graph(graph_node);

      FILE *f = tmpfile();
      if (NULL == f) {
	return new WebErrorPageReply(WebReply::HTTP_INTERNAL_SERVER_ERROR,
				     "Cannot open temp file: %s", strerror(errno));
      }

      GVC_t* gvc = gvContext(); 
      Agraph_t* G = agmemread((char *)graph.c_str());
      gvLayout(gvc, G, (char *)"dot");
      gvRender(gvc, G, (char *)"png", f);
      gvFreeLayout(gvc, G);
      agclose(G);    
      gvFreeContext(gvc);

      try {
	DynamicFileWebReply *freply = new DynamicFileWebReply(f);
	return freply;
      } catch (fawkes::Exception &e) {
	return new WebErrorPageReply(WebReply::HTTP_INTERNAL_SERVER_ERROR, *(e.begin()));
      }
#else
      return new WebErrorPageReply(WebReply::HTTP_INTERNAL_SERVER_ERROR,
				   "BlackBoard processor was not built with Graphviz support");
#endif
    } else {

      WebPageReply *r = new WebPageReply("BlackBoard");
      r->set_html_header("  <link type=\"text/css\" href=\""
			 "/static/css/jqtheme/jquery-ui.custom.css\" rel=\"stylesheet\" />\n"
                   "  <script type=\"text/javascript\" src=\"/static/js/"
                   "jquery.min.js\"></script>\n"
                   "  <script type=\"text/javascript\" src=\"/static/js/"
                   "jquery-ui.custom.min.js\"></script>\n"
			 "  <link type=\"text/css\" href=\""
			 "/static/css/blackboard.css\" rel=\"stylesheet\" />\n");


      if (subpath.find("/view/") != 0 && subpath.find("/graph") != 0) {
	*r += "\n\n<h2>Select Interface</h2>\n"
	  "<div id=\"blackboard-interfaces-mainpart\">\n";
      } else {
	*r +=  "\n\n  <div id=\"blackboard-interfaces\">\n";
      }

      bool found_some = false;
      InterfaceInfoList *iil = __blackboard->list_all();
      iil->sort();
      for (InterfaceInfoList::iterator i = iil->begin(); i != iil->end(); ++i) {
	if (! found_some) {
	  *r += "<table>\n";
	  *r += "<tr><th>Interface</th><th>Reader(s)</th><th>Writer</th></tr>\n";
	  found_some = true;
	}
	r->append_body("<tr><td><a href=\"%s/view/%s::%s\">%s::%s</a></td><td>%u</td><td style=\"color:%s\">%s</td></tr>\n",
		       __baseurl, i->type(), i->id(), i->type(), i->id(),
		       i->num_readers(), i->has_writer() ? "green" : "red", i->has_writer() ? i->writer().c_str() : "no");
      }
      delete iil;

      if (found_some) {
	*r += "</table>\n";
      }

#ifdef HAVE_GRAPHVIZ
      if (subpath.find("/graph") != 0) {
	r->append_body("  <div class=\"blackboard-graph-div\">"
		       "<a href=\"%s/graph\" class=\"blackboard-graph-link\">Graph</a></div>\n", __baseurl);
      }
#endif

      *r += "  </div>\n";

      if (! found_some) {
	*r += "<p><b>No interfaces found.</b></p>\n";
      }

      std::list<const char *> all_msgs; // needed for collapsible msg forms
      if (subpath.find("/view/") == 0) {
        std::string iuid = "";
        try {
          std::size_t tmp = subpath.find_first_not_of("/", std::string("/view/").length());
          if (tmp == std::string::npos) { throw IllegalArgumentException("String ended after \"/view/\"."); }
          iuid = subpath.substr(tmp);
        } catch (Exception &e) {
          return new WebErrorPageReply(WebReply::HTTP_NOT_FOUND,
                           "Could not parse interface id: %s<br>\n<a href=\"%s\">Back to BlackBoard</a>", e.what(), __baseurl);
        } catch (std::exception &e) {
          return new WebErrorPageReply(WebReply::HTTP_NOT_FOUND,
                           "Could not parse interface id: %s<br>\n<a href=\"%s\">Back to BlackBoard</a>", e.what(), __baseurl);
        }
        std::string message_type;
        bool send = false;
        if (iuid.find("/send/") != std::string::npos) {
          std::string::size_type slash_pos1 = iuid.find("/");
          std::string::size_type slash_pos2 = iuid.find("/", slash_pos1+1);
          message_type = iuid.substr(slash_pos2+1,iuid.length());
          iuid = iuid.substr(0,slash_pos1);
          send = true;
      }
	std::string iftype = iuid.substr(0, iuid.find("::"));
	std::string ifname = iuid.substr(iuid.find("::") + 2);

	r->append_body("<h2>Showing %s</h2>\n", iuid.c_str());
	if (__interfaces.find(iuid) == __interfaces.end()) {
	  try {
	    Interface *iface = __blackboard->open_for_reading(iftype.c_str(), ifname.c_str());
	    __interfaces[iuid] = iface;
	  } catch (Exception &e) {
	    r->append_body("Failed to open interface: %s\n", e.what());
	  }
	}
	if (__interfaces.find(iuid) != __interfaces.end()) {
	  Interface *iface = __interfaces[iuid];
	  iface->read();

        if(send){
          // handle send message request
          Message* msg_to_send;
          try {
            msg_to_send = iface->create_message(message_type.c_str());
          } catch (Exception &e) {
            return new WebErrorPageReply(WebReply::HTTP_NOT_FOUND,
                           "Could not parse message name: %s<br>\n <a href=\"%s/view/%s::%s\">Back to %s::%s</a>", e.what(), __baseurl, iftype.c_str(), ifname.c_str(), iftype.c_str(), ifname.c_str());
          }
          bool no_errors = true;
          if (!iface->has_writer()){
            r->append_body("<font color=\"red\">Error: Cannot send message because the interface has no writer!</font>\n");
            no_errors = false;
          }else{
            for (InterfaceFieldIterator mfi = msg_to_send->fields(); mfi != msg_to_send->fields_end(); ++mfi) {
              if(!request->post_value(mfi.get_name()).empty()){

                if (mfi.get_length() > 1 && mfi.get_type() != IFT_STRING){
                  r->append_body("<font color=\"red\">Error: Type array is currently not supported by webview send message functionality.</font><br>\n");
                  no_errors = false;
                } else {
                  switch(mfi.get_type()){
                    case IFT_STRING: {
                        std::string str = request->post_value(mfi.get_name());
                        if (str.length() > mfi.get_length()) {
                          r->append_body("<font color=\"red\">Error: The entered string is longer than %i! </font><br>\n", mfi.get_length());
                          no_errors = false;
                        } else {
                          mfi.set_string(str.c_str());
                          printf("received string, it is at most of length: %lu<br>\n", mfi.get_length());
                        }
                      break;
                    }
                    case IFT_ENUM:
                      mfi.set_enum_string(request->post_value(mfi.get_name()).c_str());
                      break;
                    case IFT_BOOL:
                      mfi.set_bool( (strcmp("true",request->post_value(mfi.get_name()).c_str()) != 0) ? true : false);
                      break;
                    case IFT_INT8: {
                      int i = StringConversions::to_int(request->post_value(mfi.get_name()));
                      mfi.set_int8(i);
                      break;
                    }
                    case IFT_UINT8: {
                      unsigned int i = StringConversions::to_uint(request->post_value(mfi.get_name()));
                      mfi.set_uint8(i);
                      break;
                    }
                    case IFT_INT16: {
                      int i = StringConversions::to_int(request->post_value(mfi.get_name()));
                      mfi.set_int16(i);
                      break;
                    }
                    case IFT_UINT16: {
                      unsigned int i = StringConversions::to_uint(request->post_value(mfi.get_name()));
                      mfi.set_uint16(i);
                      break;
                    }
                    case IFT_INT32: {
                      int i = StringConversions::to_int(request->post_value(mfi.get_name()));
                      mfi.set_int32(i);
                      break;
                    }
                    case IFT_UINT32: {
                      unsigned int i = StringConversions::to_uint(request->post_value(mfi.get_name()));
                      mfi.set_uint32(i);
                      break;
                    }
                    case IFT_INT64: {
                      int i = StringConversions::to_int(request->post_value(mfi.get_name()));
                      mfi.set_int64(i);
                      break;
                    }
                    case IFT_UINT64: {
                      unsigned int i = StringConversions::to_uint(request->post_value(mfi.get_name()));
                      mfi.set_uint64(i);
                      break;
                    }
                    case IFT_FLOAT: {
                      float f = StringConversions::to_float(request->post_value(mfi.get_name()));
                      mfi.set_float(f);
                      break;
                    }
                    case IFT_DOUBLE: {
                      double d = StringConversions::to_double(request->post_value(mfi.get_name()));
                      mfi.set_float(d);
                      break;
                    }
                    default:
                      r->append_body("<font color=\"red\">Error: Type %s is currently not supported by webview send message functionality.</font>\n", mfi.get_typename());
                      no_errors = false;
                  }
                  r->append_body("Received value %s for the field \"%s\".\n",request->post_value(mfi.get_name()).c_str(), mfi.get_name());
                }
              }
            }
          }
          if (no_errors){
            unsigned int msg_id = iface->msgq_enqueue(msg_to_send);
             r->append_body("Sent Message of type %s with id %d\n",message_type.c_str(), msg_id);
          }
        }

	  /*
	   *r += "<script type=\"text/javascript\">\n"
	   "  $(function(){\n"
	   "    $(\"#blackboard-interface-details-title\").click(function(){\n"
	   "	     if ( $(\"#blackboard-interface-details\").is(\":visible\") ) {\n"
	   "        $(\"#blackboard-interface-details\").hide(\"blind\");\n"
	   "        $(\"#blackboard-interfaces-icon\").attr(\"src\", "
	   "\"/static/images/icon-triangle-e.png\");\n"
	   "      } else {\n"
	   "	       $(\"#blackboard-interface-details\").show(\"blind\");\n"
	   "        $(\"#blackboard-interfaces-icon\").attr(\"src\", "
	   "\"/static/images/icon-triangle-s.png\");\n"
	   "      }\n"
	   "    });\n"
	   "    $(\"#blackboard-interface-details\").hide();\n"
	   "  });\n"
	   "</script>\n"
	   "<div id=\"blackboard-box\">\n"
	   "  <div><a id=\"blackboard-interface-details-title\" href=\"#\">"
	   "<img id=\"blackboard-interfaces-icon\" "
	   "class=\"blackboard-interfaces-icon\" "
	   "src=\"/static/images/icon-triangle-e.png\" />"
	   "Interface details</a></div>\n"
	   "  <div id=\"blackboard-interface-details\">\n";
	  */

	  std::string writer;
	  if (iface->has_writer()) {
	    try {
	      writer = iface->writer();
	    } catch (Exception &e) {}
	  }
	  std::string readers;
	  try {
	    readers = str_join(iface->readers(), ", ");
	  } catch (Exception &e) {}

	  r->append_body("<table>\n"
			 " <tr><td><b>Type:</b></td><td>%s</td></tr>\n"
			 " <tr><td><b>ID:</b></td><td>%s</td></tr>\n"
			 " <tr><td><b>Writer:</b></td><td><span class=\"blackboard-writer-%s\">%s</span></td></tr>\n"
			 " <tr><td><b>Readers:</b></td><td>%s (%u)</td></tr>\n"
			 " <tr><td><b>Serial:</b></td><td>%u</td></tr>\n"
			 " <tr><td><b>Data size:</b></td><td>%u</td></tr>\n"
			 " <tr><td><b>Hash:</b></td><td>%s</td></tr>\n"
			 " <tr><td><b>Data changed:</b></td>"
			 "<td>%s (last at %s)</td></tr>\n"
			 "</table>\n",
			 iface->type(), iface->id(),
			 iface->has_writer() ? "exists" : "none",
			 iface->has_writer() ? writer.c_str() : "none",
			 iface->num_readers() > 0 ? readers.c_str() : "none",
			 iface->num_readers(),
			 iface->serial(),
			 iface->datasize(), iface->hash_printable(),
			 iface->changed() ? "yes" : "no", iface->timestamp()->str());

	  /*
	   *r += "  </div>\n"
	   "</div>\n";
	  */

	  r->append_body("<table>\n"
			 " <tr>\n"
			 "  <th>Name</th><th>Type</th><th>Value</th>\n"
			 " </tr>\n");
	  for (InterfaceFieldIterator fi = iface->fields(); fi != iface->fields_end(); ++fi) {
	    bool is_string = (fi.get_type() == IFT_STRING);
	    *r += " <tr>\n";
	    if ( fi.get_length() > 1 ) {
	      r->append_body("  <td>%s</td><td>%s [%zu]</td><td>%s%s%s</td>\n",
			     fi.get_name(), fi.get_typename(),
			     fi.get_length(), is_string ? "<pre>" : "",
			     fi.get_value_string(), is_string ? "</pre>" : "");
	    } else {
	      r->append_body("  <td>%s</td><td>%s</td><td>%s%s%s</td>\n",
			     fi.get_name(), fi.get_typename(), is_string ? "<pre>" : "",
			     fi.get_value_string(), is_string ? "</pre>" : "");
	    }
	    *r += " </tr>\n";
	  }
	  r->append_body("</table>\n");

        // Show possible messages
        std::list<const char *> msg_types = iface->get_message_types();
        if (!msg_types.empty() && __enable_msgs){
          *r +="<h3>Messages</h3>\n";

          /*r->append_body("<table>\n"
                   " <tr>\n"
                   "  <th>Name</th><th>Field Name</th><th>Type</th><th>Value</th>\n"
                   " </tr>\n");
        */
          for (std::list<const char *>::iterator msgit = msg_types.begin(); msgit != msg_types.end(); ++msgit) {
            all_msgs.push_front(*msgit);
            r->append_body("<div class=\"msg-name-%s\"><h5 class=\"close\">%s</h5></div>\n", *msgit, *msgit);
            r->append_body("<div class=\"msg-form-%s\" id=\"msg-form-%s\">\n<form action=\"%s/view/%s/send/%s\" method=\"post\">\n",
                        *msgit, *msgit, __baseurl, iuid.c_str(), *msgit);
            Message* ifmsg = iface->create_message(*msgit);
            for (InterfaceFieldIterator mfi = ifmsg->fields(); mfi != ifmsg->fields_end(); ++mfi) {
              //*r += " <tr>\n";
              if ( mfi.get_length() > 1 ) {
                if (mfi.get_type() == IFT_STRING) {
                  r->append_body("%s (%s [%zu]): <input type=\"text\" name=\"%s\" maxlength=\"%i\"><br>\n",
                          mfi.get_name(), mfi.get_typename(),
                          mfi.get_length(), mfi.get_name(), mfi.get_length());
                } else {
                  r->append_body("%s (%s [%zu]): <input type=\"text\" name=\"%s\" ><br>\n",
                          mfi.get_name(), mfi.get_typename(),
                          mfi.get_length(), mfi.get_name());
                }
              } else {
                switch (mfi.get_type()) {
                  case IFT_DOUBLE:
                  case IFT_FLOAT:
                    r->append_body("%s (%s): <input type=\"text\" name=\"%s\" pattern=\"[0-9]*.[0-9]*\"><br>\n",
                          mfi.get_name(), mfi.get_typename(),
                          mfi.get_name());
                    break;
                  case IFT_BOOL:
                    r->append_body("%s (%s):\n  <input type=\"radio\" name=\"%s\" value=\"true\" checked>true\n"
                          "  <input type=\"radio\" name=\"%s\" value=\"false\">false<br>\n",
                          mfi.get_name(), mfi.get_typename(), mfi.get_name(), mfi.get_name());
                    break;
                  case IFT_ENUM:
                    {
                      std::list<const char*> enum_values = mfi.get_enum_valuenames();
                      r->append_body("%s:<br>\n", mfi.get_name());
                      for (std::list<const char*>::iterator enum_it = enum_values.begin(); enum_it != enum_values.end(); ++enum_it) { // TODO improve formatting
                        r->append_body("  <input type=\"radio\" name=\"%s\" value=\"%s\">%s<br>\n",
                            mfi.get_name(), *enum_it, *enum_it);
                      }
                      break;
                    }
                  case IFT_INT8:
                    {
                      std::string form = get_input_form_string<int8_t>(*msgit, mfi.get_name(), mfi.get_typename());
                      r->append_body(form.c_str());
                      break;
                    }
                  case IFT_BYTE:
                  case IFT_UINT8:
                    {
                      std::string form = get_input_form_string<uint8_t>(*msgit, mfi.get_name(), mfi.get_typename());
                      r->append_body(form.c_str());
                      break;
                    }
                  case IFT_INT16:
                    {
                      std::string form = get_input_form_string<int16_t>(*msgit, mfi.get_name(), mfi.get_typename());
                      r->append_body(form.c_str());
                      break;
                    }
                  case IFT_UINT16:
                    {
                      std::string form = get_input_form_string<uint16_t>(*msgit, mfi.get_name(), mfi.get_typename());
                      r->append_body(form.c_str());
                      break;
                    }
                  case IFT_INT32:
                    {
                      std::string form = get_input_form_string<int32_t>(*msgit, mfi.get_name(), mfi.get_typename());
                      r->append_body(form.c_str());
                      break;
                    }
                  case IFT_UINT32:
                    {
                      std::string form = get_input_form_string<uint32_t>(*msgit, mfi.get_name(), mfi.get_typename());
                      r->append_body(form.c_str());
                      break;
                    }
                  case IFT_INT64:
                    {
                      std::string form = get_input_form_string<int64_t>(*msgit, mfi.get_name(), mfi.get_typename());
                      r->append_body(form.c_str());
                      break;
                    }
                  case IFT_UINT64:
                    {
                      std::string form = get_input_form_string<uint64_t>(*msgit, mfi.get_name(), mfi.get_typename());
                      r->append_body(form.c_str());
                      break;
                    }

                  default:
                    r->append_body("%s (%s): <input type=\"text\" name=\"%s\" ><br>\n",
                          mfi.get_name(), mfi.get_typename(), mfi.get_name());
                }
              }
            }
            *r += "  <input type=\"submit\" value=\"Send\" />\n</form>\n</div>\n";
          }
        }
	  r->append_body("<p><a href=\"%s\">Clear detailed</a></p>\n", __baseurl);
      }
      } else if (subpath.find("/graph") == 0) {
	std::string graph_baseurl("/graph/");
	std::string graph_node =
	  subpath.length() > graph_baseurl.length() ? subpath.substr(graph_baseurl.length()) : "";
#if defined(HAVE_GRAPHVIZ) && ((defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))) || defined(__clang__))
	std::string graph = generate_graph(graph_node);
	char *map;
	unsigned int map_length;

	GVC_t* gvc = gvContext(); 
	Agraph_t* G = agmemread((char *)graph.c_str());
	gvLayout(gvc, G, (char *)"dot");
	gvRenderData(gvc, G, (char *)"cmapx", &map, &map_length);
 	r->append_body("\n%s\n", map);
#if GRAPHVIZ_VERSION >= 23200
	gvFreeRenderData(map);
#else
	free(map);
#endif
	gvFreeLayout(gvc, G);
	agclose(G);    
	gvFreeContext(gvc);

 	r->append_body("<p><img src=\"%s/graph/graph.png%s%s\" usemap=\"#bbmap\" /></p>\n",
		       __baseurl,
		       graph_node.empty() ? "" : "?for=",
		       graph_node.empty() ? "" : graph_node.c_str());
	r->append_body("<!-- DOT Graph:\n\n%s\n\n-->\n\n", graph.c_str());
#else
	r->append_body("<p>No graphviz support at compile time</p>\n");
#endif
	if (! graph_node.empty()) {
	  r->append_body("<p><a href=\"%s/graph\">Full Graph</a></p>\n\n", __baseurl);
	}
      }
      r->append_body("<script>\n");
      for (std::list<const char *>::iterator it = all_msgs.begin(); it != all_msgs.end(); ++it){
        r->append_body( "  $('.msg-name-%s').click(function(){\n    $('.msg-form-%s').slideToggle('slow');\n  });\n", *it, *it);
        //r->append_body("  $('.msg-form-%s').style.display=\"none\";", *it);
        r->append_body("  document.getElementById(\"msg-form-%s\").style.display=\"none\";", *it);
      }
      r->append_body("\n</script>");
      return r;
    }

  } else {
    return NULL;
  }
}

template<typename T> std::string
WebviewBlackBoardRequestProcessor::get_input_form_string(std::string msgit, const char * fieldname, std::string fieldtype)
{
  std::stringstream ss;
  ss << fieldname << " (" << fieldtype << "):\n <input type=\"number\" min=\"" <<
                 std::numeric_limits<T>::min() << "\" max=\"" << 
                 std::numeric_limits<T>::max() << "\" step=\"1\" id=\"" << msgit <<
                 "-" << fieldname << "\" name=\"" << fieldname << "\" pattern=\"\\d+\"><br>\n";
  return ss.str();
}


#if defined(HAVE_GRAPHVIZ) && ((defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))) || defined(__clang__))
std::string
WebviewBlackBoardRequestProcessor::generate_graph(std::string for_owner)
{
  InterfaceInfoList *iil = __blackboard->list_all();
  iil->sort();

  std::stringstream mstream;
  mstream << "digraph bbmap {" << std::endl
	  << "  graph [fontsize=12,rankdir=LR];" << std::endl;

  std::set<std::string> owners;

  InterfaceInfoList::iterator ii;
  for (ii = iil->begin(); ii != iil->end(); ++ii) {
    const std::list<std::string> readers = ii->readers();
    
    if (for_owner == "" ||
	ii->writer() == for_owner ||
	std::find_if(readers.begin(), readers.end(),
		     [&for_owner](const std::string &o)->bool { return for_owner == o; })
	!= readers.end())
    {
      if (ii->has_writer()) {
	const std::string writer = ii->writer();
	if (! writer.empty())  owners.insert(writer);
      }
      std::list<std::string>::const_iterator r;
      for (r = readers.begin(); r != readers.end(); ++r) {
	owners.insert(*r);
      }
    }
  }

  mstream << "  node [fontsize=12 shape=box width=4 margin=0.05];" << std::endl
	  << "  { rank=same; " << std::endl;
  std::set<std::string>::iterator i;
  for (ii = iil->begin(); ii != iil->end(); ++ii) {
    const std::list<std::string> readers = ii->readers();
    if (for_owner == "" ||
	ii->writer() == for_owner ||
	std::find_if(readers.begin(), readers.end(),
		     [&for_owner](const std::string &o)->bool { return for_owner == o; })
	!= readers.end())
    {
      mstream << "    \"" << ii->type() << "::" << ii->id() << "\""
	      << " [href=\"" << __baseurl << "/view/" << ii->type() << "::" << ii->id() << "\"";


      if (! ii->has_writer()) {
	mstream << " color=red";
      } else if (ii->writer().empty()) {
	mstream << " color=purple";
      }
      mstream << "];" << std::endl;
    }
  }
  mstream << "  }" << std::endl;

  mstream << "  node [fontsize=12 shape=octagon width=3];" << std::endl;
  for (i = owners.begin(); i != owners.end(); ++i) {
    mstream << "  \"" << *i << "\""
	    << " [href=\"" << __baseurl << "/graph/" << *i << "\"];"
	    << std::endl;
  }

  for (ii = iil->begin(); ii != iil->end(); ++ii) {
    const std::list<std::string> readers = ii->readers();
    if (for_owner == "" ||
	ii->writer() == for_owner ||
	std::find_if(readers.begin(), readers.end(),
		     [&for_owner](const std::string &o)->bool { return for_owner == o; })
	!= readers.end())
    {
      std::list<std::string> quoted_readers;
      std::for_each(readers.begin(), readers.end(),
		    [&quoted_readers](const std::string &r) {
		      quoted_readers.push_back(std::string("\"")+r+"\"");
		    });
      std::string quoted_readers_s = str_join(quoted_readers, ' ');
      mstream << "  \"" << ii->type() << "::" << ii->id() << "\" -> { "
	      << quoted_readers_s << " } [style=dashed arrowhead=dot arrowsize=0.5 dir=both];" << std::endl;

      if (ii->has_writer()) {
	mstream << "  \"" << (ii->writer().empty() ? "???" : ii->writer()) << "\" -> \""
		<< ii->type() << "::" << ii->id() << "\""
		<< (ii->writer().empty() ? " [color=purple]" : " [color=\"#008800\"]")
		<< ";" << std::endl;
      }
    }
  }

  delete iil;

  mstream << "}";
  return mstream.str();
}
#endif
