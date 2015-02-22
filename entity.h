/* 
 * File:   entity.h
 * Author: maartyl
 *
 * Created on February 5, 2015, 11:35 PM
 */

#ifndef ENTITY_H
#define	ENTITY_H

#include <string>

#include "utils.h"

//forward all types of entities:
class dict;
class entity_int;
class text;
class view;
class action;
class bag; //set of entities

class elink; //for store : link to an entity

typedef entity_int eint;

class entity {
protected:
  entity() { }
public:
  using handle = std::shared_ptr<entity>;
  virtual dict& as_dict();
  virtual eint& as_int();
  virtual text& as_text();
  virtual view& as_view();
  virtual action& as_action();
  virtual bag& as_bag();

public: //shortcuts
  ///shortcut for dict[]
  ///syntax helper for chaining 
  entity& operator[](const std::string& name);
  bool set(std::string const& key, handle h);
  bool set(int val);
  handle trigger(entity& player);

public:
  virtual ~entity() { }
  virtual bool is_nil() const {
    return false;
  }
  explicit operator bool() const {
      return !is_nil();
  }
  bool operator!() const {
    return is_nil();
  }
};


#endif	/* ENTITY_H */

