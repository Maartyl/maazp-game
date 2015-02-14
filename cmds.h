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

//simple application

class action_cmd {
  std::unique_ptr<action> a_;
  store::handle cause_h_;
public:
  action_cmd(store::handle cause, std::unique_ptr<action> a) : a_(std::move(a)), cause_h_(std::move(cause)) { }
  action_cmd(std::unique_ptr<action> a) : action_cmd(store::handle_of("?player"), std::move(a)) { }
  void operator()(std::string const& line) {
    auto p = parser::words(line);
    
    std::vector<store::handle> hv;
    hv.reserve(p.size());
    for (CREF w : p) hv.push_back(store::handle_of(w));

    if (CREF reth = a_->invoke(store::deref(cause_h_), hv))
      if (REF ret = store::deref(reth).as_view()) {
      prn(ret.print());
      if (ret.valid) {
        REF p = store::deref("$player");
        p["*action"].trigger(p);
        p["area"]["*action"].trigger(p);
      }
    } else {
      throw std::logic_error("action_cmd: Outmost action didn't return view.");
    }
  }
};

#endif	/* CMDS_H */

