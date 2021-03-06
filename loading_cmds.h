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
#include "template_view.h"

///simple means: can be just a function; not necessarily a functor

class defs_simple {
public:
  static void alias(store::id const& id, std::string const& args) {
    //assert args.size == 1
    CREF p = parser::first_and_rest(args);
    if (args.size() == 0 || p.second.size() != 0) throw std::invalid_argument("alias: expects <alias> <existing id>; got: " + id + " " + args);
    store::add_alias(id, p.first); //exception if problem
  }
  static void aliases(store::id const& id, std::string const& args) {
    for (REF a : parser::words(args))
      store::add_alias(a, id); //exception if problem
  }
  static void link(store::id const& id, std::string const& args) {
    CREF p = parser::first_and_rest(args); //assert 1 arg
    if (args.size() == 0 || p.second.size() != 0) throw std::invalid_argument("link: expects <id> <query>; got: " + id + " " + args);
    store::emplace<elink>(id, p.first);
  }
  ///can copy simple dict, bag; nothing else / smart ... (for template objects; "shallow" copy)
  static void copy(store::id const& id, std::string const& args) {
    //assert args.size == 1
    CREF p = parser::first_and_rest(args);
    if (p.second.size() != 0) throw std::invalid_argument("copy: expects <new id> <existing id>; got: " + id + " " + args);
    REF e = store::deref(p.first);
    if (REF x = e.as_dict()) {
      store::emplace<dict>(id, x); //copy .ctor
      return;
    }
    if (REF x = e.as_bag()) {
      store::emplace<bag>(id, x);
      return;
    }
    throw std::invalid_argument("copy: can only copy simple {dict, bag}.");
  }

public:
  static void def_dict(store::id const& id, std::string const& args) {
    auto words = parser::words(args);
    dict_assoc_check_even(words, "def dict[" + id + "]: requires even number of forms: " + args);
    dict_assoc(store::deref(store::emplace<dict>(id)).as_dict(), words); //add |> deref |> assoc words
  }
  static void def_text(store::id const& id, std::string const& args) {
    store::emplace<textview>(id, args);
  }
  static void def_view(store::id const& id, std::string const& args) {
    store::emplace<templateview>(id, args);
  }
  static void def_err(store::id const& id, std::string const& args) {
    store::emplace<templateview>(id, args, false);
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
  static void def_bag(store::id const& id, std::string const& args) {
    CREF ids = parser::words(args);
    REF b = store::deref(store::emplace<bag>(id)).as_bag();
    for (CREF id : ids)
      b.insert(store::handle_of(id));
  }
  static void def_handler(store::id const& id, std::string const& args) {
    CREF ids = parser::words(args);
    //don't check for 'non-actions' :: they will just not be called
    //would be problem with links...
    //maybe handle while *calling*: deref non-action? throw...
    REF b = store::deref(store::emplace<actionbag>(id)).as_bag();
    for (CREF id : ids)
      b.insert(store::handle_of(id));
  }
public:
  ///dict: associate (property of dict to store::handle (by some id))
  static void assoc(store::id const& id, std::string const& args) {
    auto words = parser::words(args); //(name <id>)+
    dict_assoc_check_even(words, "assoc[" + id + "]: requires even number of forms");
    if (words.size() == 0)
      throw std::logic_error("assoc[" + id + "]: no forms to associate");
    
    if (REF d = store::deref(id).as_dict())
      dict_assoc(d, words);
    else throw std::logic_error("assoc[" + id + "]: not a dict");
  }

  //bag: conjoin (add) //? insert could be better name ...
  static void conj(store::id const& id, std::string const& args) {
    CREF ids = parser::words(args);
    if (ids.size() == 0)
      throw std::logic_error("conj[" + id + "]: no forms to conjoin");

    if (REF b = store::deref(id).as_bag())
      for (CREF id : ids)
        b.insert(store::handle_of(id));
    else throw std::logic_error("conj[" + id + "]: not a bag");
  }
private:
  static bool dict_assoc_check_even(str_vec const& forms, std::string const& throw_ = "") {
    if (forms.size() % 2 != 0) 
      if (throw_.size() != 0) throw std::logic_error(throw_);
      else return false;
    else return true;
  }
  static void dict_assoc(dict& d, str_vec const& forms) {
    //[requires] already checked that size is even.
    for (auto it = std::begin(forms); it != std::end(forms); ++++it)
      d.assoc(*it, store::handle_of(it[1]));
  }
};


#endif	/* LOADING_CMDS_H */

