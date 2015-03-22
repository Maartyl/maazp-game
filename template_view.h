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
 * (//not yet
 * can use extended queries: suffix: \<type>
 * if type is not matched: throws exception
 * if type is omitted, is tried in this order:
 *  - view
 *  - text
 *  - int
 *  - TODO: complete ...?
 * )
 * some types are not printable ... or are they? (dict, bag...)
 *  - could be useful to see what's inside ...
 *  - possibly
 *
 * + there are special names for view.print arguments:
 *  - first: $&subject; $&1
 *  - second: $&object; $&2
 *
 */
class templateview : public textview {
public:
  templateview(const std::string& txt) : textview(txt) { } //same constructors as textview
  templateview(const std::string& txt, bool valid) : textview(txt, valid) { }
  virtual std::string print(entity& subject, entity& object) const {
    auto text = text::value();
    std::string qry;
    std::string::size_type pos = 0;
    CREF h1 = store::transient<eref>(&subject);
    CREF h2 = store::transient<eref>(&object);
    store_context_frame cfArgs({//frame: name required
      {"$&subject", h1},
      {"$&1", h1},
      {"$&object", h2},
      {"$&2", h2}
    });
    while (next_query(text, /*out*/ qry, /*out*/ pos)) {
      qry = parser::trimr(qry, ".!?'\""); //sentence period / otherwise illogical
      //prn("qry: \"" << qry << "\" at: " << pos);
      text.replace(pos++, qry.size(), to_string(store::deref(qry, true), qry));
    }
    
    return text;
  }
private:
  static bool next_query(std::string const& body, std::string& qry, std::string::size_type& start) {
    //continue from last pos (+1:pos++) : prevents infinite loop + faster + won't expand repeatedly
    if ((start = body.find('^', start)) != std::string::npos) {
      auto end = body.find_first_of(parser::delims, start);
      auto len = (end == std::string::npos) ? end : (end - start); //not found: substr until end
      qry = body.substr(start, len);
      return true;
    } else return false;
  }
  static std::string to_string(entity& e, std::string const& queryInfo) {
    if (!e)
      return queryInfo; // didn't match anything: leave as itself... / throw? / ""?
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

