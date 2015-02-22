/* 
 * File:   template_view.h
 * Author: maartyl
 *
 * Created on February 22, 2015, 4:22 AM
 */

#ifndef TEMPLATE_VIEW_H
#define	TEMPLATE_VIEW_H

#include "entity_defs.h" //this file is essentially part of it
#include "store_context_frame.h"

/*like text view but:
 * - can interpret special words starting with '^'
 * - considers them to be queries
 * - evaluates queries and replaces them with result:
 * -- view result: prints in place of query
 * -- int: converts to string ...
 * -- etc.
 * 
 * can use extended queries: suffix: \<type>
 * if type is not matched: throws exception
 * if type is omitted, is tried in this order:
 *  - view
 *  - text
 *  - int
 *  - TODO: complete
 *
 * some types are not printable ... or are they? (dict, bag...)
 *  - could be useful to see what's inside ...
 *  - possibly
 *
 * + there are special names for view.print arguments:
 *  - $&subject; $&1
 *  - $&object;  $&2
 *
 */
class templateview : public textview {
public:
  templateview(const std::string& txt) : textview(txt) { } //same constructors as textview
  templateview(const std::string& txt, bool valid) : textview(txt, valid) { }
  virtual std::string print(entity& subject, entity& object) const {
    auto text = text::value();
    std::string qry;
    std::string::size_type pos = std::string::npos;
    CREF h1 = store::transient<eref>(&subject);
    CREF h2 = store::transient<eref>(&object);
    store_context_frame({
      {"$&subject", h1},
      {"$&1", h1},
      {"$&object", h2},
      {"$&2", h2}
    });
    while (next_query(text, /*out*/ qry, /*out*/ pos)) 
      text.replace(pos, qry.size(), to_string(store::deref(qry), qry));
    
    return text;
  }
private:
  static bool next_query(std::string const& body, std::string& qry, std::string::size_type pos) {
    auto start = body.find('^');
    if (start != std::string::npos) {
      auto end = body.find_first_of(parser::delims); //TODO: is this ugly? ... (parser reference ... probably not so much)
      auto len = end == std::string::npos ? end : end - start; //not found: substr until end
      pos = start;
      qry = body.substr(start, len);
      return true;
    } else return false;
  }
  static std::string to_string(entity& e, std::string const& queryInfo) {
    if (REF v = e.as_view())
      return v.print(); //args?
    if (REF v = e.as_text())
      return v.value();
    if (REF v = e.as_int())
      return std::to_string(v.value());
    throw std::logic_error("templateview: unsupported entity for query: " + queryInfo);
  } 
};


#endif	/* TEMPLATE_VIEW_H */

