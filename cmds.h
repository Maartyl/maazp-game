/* 
 * File:   cmds.h
 * Author: maartyl
 *
 * Created on February 8, 2015, 3:47 AM
 */

#ifndef CMDS_H
#define	CMDS_H

#include <functional>
#include <stdexcept>
#include <vector>
#include <iterator>
//#include <memory>

#include "store.h"
#include "parser.h"

//#include "entity.h"
#include "entity_defs.h"

//common, generic commands for parser

//loading cmds: definitions, setting, text ...
//player cmds:  go left, grab that ... -- parsing of text and forwarding objs. to actions...
//invoke action -> view -> print
//helpers: with id ...

class with_id {
public:
  typedef std::function<void(store::id const& id, std::string const& rest) > with_id_fn;
  with_id(with_id_fn fn) : fn(fn) { }
  void operator()(std::string const& line) {
    auto p = parser::first_and_rest(line);
    if (p.first.size() == 0)
      throw std::logic_error("too few arguments: requires id");
    fn(p.first, p.second);
  }
private:
  with_id_fn fn;
};

//application of action

class action_cmd {
public:
  using consumer_t = std::function<void(const action::ret_t) >;
  typedef store::handle action_h;
private:
  action_h a_;
  store::handle cause_h_;
  consumer_t consumer;
public:
  action_cmd(consumer_t cr, store::handle cause, action_h a)
  : a_((a)), cause_h_(std::move(cause)), consumer(cr) {
    if (!store::deref(a).as_action())
      throw std::invalid_argument("action_cmd: not given action handle");
    }
  //cause: ?player
  action_cmd(consumer_t cr, action_h a) : action_cmd(cr, store::handle_of("?player"), (a)) { }
  //ignore result; cause: ?player
  action_cmd(action_h a) : action_cmd(default_consumer, (a)) { }
  void operator()(std::string const& line) {
    auto p = parser::words(line);
    
    std::vector<store::handle> hv;
    hv.reserve(p.size());
    for (CREF w : p) hv.push_back(store::handle_of(w));

    consumer(store::deref(a_).as_action().invoke(store::deref(cause_h_), hv));
  }
private:
  static void default_consumer(action::ret_t ignore) { }
};

#endif	/* CMDS_H */

