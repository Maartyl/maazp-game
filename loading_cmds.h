/* 
 * File:   loading_cmds.h
 * Author: maartyl
 *
 * Created on February 8, 2015, 3:49 AM
 */

#ifndef LOADING_CMDS_H
#define	LOADING_CMDS_H

#include <string>
#include <vector>
#include <stdexcept>

#include "cmds.h"

///simple means: can be just a function; not necessarily a functor
class defs_simple {
public:
  static void def_dict(store::id const& id, std::string const& args) {
    auto words = parser::words(args);
    dict_assoc_check_even(words, "def dict[" + id + "]: requires even number of forms: " + args);
    dict_assoc(store::deref(store::emplace<dict>(id)).as_dict(), words); //add |> deref |> assoc words
  }
  static void def_text(store::id const& id, std::string const& args) {
    store::emplace<text>(id, args);
  }
  static void def_int(store::id const& id, std::string const& args) {
    try {
      size_t end_pos; //ignore; only needed to get to base argument
      int val = parser::trim(args).size() == 0 ? 0 : std::stoi(args, &end_pos, 0); //0 base: auto detect
      store::emplace<eint>(id, val);
    } catch (std::invalid_argument& e) {
      throw std::invalid_argument("defint[" + id + "] invalid int: " + args);
    }
  }
  ///associate (property of dict to store::handle (by some id))
  static void assoc(store::id const& id, std::string const& args) {
    auto words = parser::words(args);
    dict_assoc_check_even(words, "assoc[" + id + "]: requires even number of forms");
    if (words.size() == 0)
      throw std::logic_error("assoc[" + id + "]: no forms to associate");
    
    if (REF d = store::deref(id).as_dict())
      dict_assoc(d, words);
    else throw std::logic_error("assoc[" + id + "]: not a dict");
  }
private:
  static bool dict_assoc_check_even(str_vec const& forms, std::string const& throw_ = "") {
    if (forms.size() % 2 != 0)
      if (throw_.size() != 0) throw std::logic_error(throw_);
      else return false;
    return true;
  }
  static void dict_assoc(dict& d, str_vec const& forms) {
    //[requires] already checked that size is even.
    for (auto it = std::begin(forms); it != std::end(forms); ++++it)
      d.assoc(*it, store::handle_of(it[1]));
  }
};


#endif	/* LOADING_CMDS_H */

