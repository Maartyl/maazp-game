/* 
 * File:   entity_defs.h
 * Author: maartyl
 *
 * Created on February 6, 2015, 1:25 AM
 */

#ifndef ENTITY_DEFS_H
#define	ENTITY_DEFS_H

#include <string>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>
#include <functional>

#include "entity.h"
#include "store.h"

class dict : public virtual entity {
public:
  typedef std::map<std::string, store::handle> d_map;
private:
  d_map dict_{};
public:
  dict() { }
  dict(d_map const& dm) : dict_(dm) { }
  dict(d_map&& dm) : dict_(std::move(dm)) { }
  typedef d_map::value_type kv_pair;
  //  ///syntax helper for chaining
  //  entity& operator[](const std::string& name);
  
  ///getter of actual handle
  store::handle at(const std::string& name) {
    auto it = dict_.find(name);
    if (it != std::end(dict_))
      return (it->second);

    return store::handle_of();
    //throw std::invalid_argument("€dict: not found: " + name);
  }
  //also updates
  void assoc(std::string const& prop, store::handle entity) {
    if (store::deref(entity).is_nil())
      throw std::invalid_argument("€dict: cannot hold NIL entity; for name: " + prop);
    dict_[prop] = entity;
  }
  bool dissoc(std::string const& prop) {
    return dict_.erase(prop) == 0 ? false : true;
  }
  void for_each(std::function<void(d_map::value_type const&) > fn) {
    for (CREF a : dict_)
      fn(a);
  }
  //removes all
public:
  void flush() {
    std::map<std::string, store::handle> dict;
    std::swap(dict, dict_);
  }

public: //entity override:
  virtual dict& as_dict() {
    return *this;
  }
};

class entity_int : public virtual entity {
  int value_;
public:
  entity_int() : value_(0) { }
  entity_int(int value) : value_(value) { } //implicit conversion: ok

  int value() const {
    return value_;
  }
  
  operator int() const {
    return value();
  }
public: //operators 
  entity_int& operator=(int val) {
    value_ = val;
    return *this;
  }
  entity_int& operator+=(int val) {
    value_ += val;
    return *this;
  }
  entity_int& operator-=(int val) {
    value_ -= val;
    return *this;
  }
  entity_int operator++(int val) {
    int orig = value_;
    value_ += val ? val : 1; //just ++: val == 0
    return entity_int(orig);
  }
  entity_int& operator++() {
    ++value_;
    return *this;
  }
  entity_int operator--(int val) {
    int orig = value_;
    value_ -= val ? val : 1; //just --: val == 0
    return entity_int(orig);
  }
  entity_int& operator--() {
    --value_;
    return *this;
  }

public: //entity override:
  virtual eint& as_int() {
    return *this;
  }
};

class text : public virtual entity {
  std::string text_;
public:
  text(const std::string& text) : text_(text) { }
  const std::string& value() const {
    return text_;
  }
  operator const std::string&()const {
    return value();
  }
protected:
  text() : text_() { }

public: //entity override:
  virtual text& as_text() {
    return *this;
  }
};

//can return text representation of game state (player)
class view : public virtual entity {
public:
  bool valid{true}; //is this not a view of an invalid action? (open nonexistent door, ...)
  view() { }
  view(bool valid) : valid(valid) { }

public: //"interface"
  ///transforms game state into string to show to user/player.
  ///expected to get overridden
  virtual std::string print(entity& subject, entity& object) const = 0;
  ///implicit subject: $player
  std::string print(entity& object) const {
    //return print(store::deref("$player"), object);
    return print(store::deref(), object);
  };
  ///no object; so both implicit: $player
  std::string print() const {
    //REF p = store::deref("$player");
    REF p = store::deref();
    return print(p, p);
  };
public: //entity override
  virtual view& as_view() {
    return *this;
  }
};

//just view that returns associated text
class textview : public text, public view {
public:
  textview(const std::string& txt) : text(txt) { }
  textview(const std::string& txt, bool valid) : text(txt), view(valid) { }
  virtual std::string print(entity& subject, entity& object) const {
    return text::value();
  }
};

//can change state;
//abstract: there will be actions like: change property of dict to ...; pick, drop, break, open... - events

class action : public virtual entity {
public:
  using arg_coll = std::vector<store::handle>;
  using ret_t = entity::handle;
  //is ret meaningful? ... : shared_ptr != NULL

  //returns view in ret / or data for events...
  //main invoke to override
  virtual ret_t invoke(entity& player, entity const& cause, arg_coll const& args) const = 0;
  //  {
  //    throw std::logic_error("action:invoke: not overridden");
  //  }
  ret_t invoke(entity const& cause, arg_coll const& args) const {
    return invoke(store::deref("$player"), cause, args);
  }
  ret_t invoke(entity const& cause, store::handle arg1)const {
    return invoke(store::deref("$player"), cause, { arg1 });
  }
  ret_t invoke(entity const& cause) const {
    return invoke(store::deref("$player"), cause, { });
  }
  virtual ret_t invoke_event(entity& player, arg_coll const& args) const {
    return invoke(player, store::deref("?event"), args);
  }
  ret_t invoke_event(entity& player) const {
    return invoke_event(player, arg_coll{});
  }
  ret_t invoke_event(entity& player, store::handle arg)const {
    return invoke_event(player, arg_coll{arg});
  }
  ret_t invoke_event(entity& player, store::handle arg1, store::handle arg2) const {
    return invoke_event(player,{arg1, arg2});
  }
public:
  static const ret_t nil_ret;

public: //entity override
  virtual action& as_action() {
    return *this;
  }
};

class bag : public virtual entity,
public std::set<store::handle> {

public: //entity override
  virtual bag& as_bag() {
    return *this;
  }
};

//for events; expected to contain 1 action that returns non-nil
//not meant for composing actions

class actionbag : public action, public bag {
  //returns last non-nil ret_t or nil
  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args) const {
    action::ret_t ret;
    for (REF e : (*this))
      if (REF a = e->as_action())
        if (auto rt = a.invoke(player, cause, args))
          ret = rt;
    return ret;
  }
  virtual action::ret_t invoke_event(entity& player, arg_coll const& args) const {
    action::ret_t ret;
    for (REF e : (*this))
      if (REF a = e->as_action())
        if (auto rt = a.invoke_event(player, args))
          ret = rt;
    return ret;
  }
};

//dereferences query upon dereferencing itself
//sadly, can't compare handles for equality but adds flexibility
class elink : public virtual entity {
  std::string query;
  entity& get() {
    return store::deref(query);
  }
public:
  elink(std::string const& query) : query(query) { }
  virtual action& as_action() {return get().as_action();}
  virtual bag& as_bag() {return get().as_bag();}
  virtual dict& as_dict() {return get().as_dict();}
  virtual eint& as_int() {return get().as_int();}
  virtual text& as_text() {return get().as_text();}
  virtual view& as_view() {return get().as_view();
  }
};

//allows creating temporary handles to entity references
//only works as long as reference holds
class eref : public virtual entity {
  std::string query;
  entity& e;
public:
  eref(entity& e) : e(e) { }
  eref(entity* e) : e(*e) { }
  virtual action& as_action() {return e.as_action();}
  virtual bag& as_bag() {return e.as_bag();}
  virtual dict& as_dict() {return e.as_dict();}
  virtual eint& as_int() {return e.as_int();}
  virtual text& as_text() {return e.as_text();}
  virtual view& as_view() {return e.as_view();}
};


#include <limits>
class nil_entity :
public dict,
public entity_int,
public text,
public view,
public action,
public bag {
  static nil_entity NILVAL;
  nil_entity() : entity_int(std::numeric_limits<int>::min()) { }
public:
  static nil_entity& get() {
    return NILVAL;
  }
  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args) const {
    return action::nil_ret;
  }
  virtual std::string print(entity& subject, entity& object) const {
    return "";
  }

  
public:
  virtual bool is_nil() const {
    return true;
  }
};


#endif	/* ENTITY_DEFS_H */

