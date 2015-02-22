/* 
 * File:   template_view.h
 * Author: maartyl
 *
 * Created on February 22, 2015, 4:22 AM
 */

#ifndef TEMPLATE_VIEW_H
#define	TEMPLATE_VIEW_H

#include "entity_defs.h" //this file is essentially part of it

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
 */
class templateview : public textview {
  templateview(const std::string& txt) : textview(txt) { } //same constructors as textview
  textview(const std::string& txt, bool valid) : textview(txt, valid) { }
  virtual std::string print(const entity& subject, const entity& object) const {
    return text::value();
  }
};


#endif	/* TEMPLATE_VIEW_H */

