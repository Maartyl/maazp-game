/* 
 * File:   actions.h
 * Author: maartyl
 *
 * Created on February 10, 2015, 3:45 PM
 */

#ifndef ACTIONS_H
#define	ACTIONS_H

#include <string>

#include "store.h"
#include "entity_defs.h"

/* ---------- basic player actions ---------- */

class player_move : public action {
  std::string direction; //%up, %south, ...
public:
  player_move(const std::string& direction) : direction(direction) { }
  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args)const {
    //cause is unimportant ... - different ret for events?
    //args ... could mean direction, but ... different command? / special direction? ("", use arg") ...
    REF area = player["area"];
    if (REF tran = area[direction].as_dict()) { //transition
      if (REF ps = store::deref(tran["*passing"].trigger(player))) {
        if (ps["?stop"]) {
          // cannot pass: ... should use something from that stop
          //return "can't pass" view
          return ps.as_dict().at("&stop");
        }
      }
      if (REF ps = store::deref(area["*leaving"].trigger(player))) {
        if (ps["?stop"]) {
          //can't move: not allowed to leave ... should use something from that stop for view
          //return "cant leave" view
          return ps.as_dict().at("&stop");
        }
      }
      player.set("area", tran.at("%to")); //actually move
      area["*left"].trigger(player); //use original area
      tran["*passed"].trigger(player);
      return tran.at("&go");
      
    }
    return store::transient<textview>("HACK; nothing in direction: " + direction);
  }
};

////... kind of weird command; possibly not used / special ... (takes transient texts...)
//class player_go : public action {
//public:
//  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args) const {
//    //assert: 1 arg ... ? for now...
//    if (args.size() != 1)
//      return { }; //some "wrong command arguments view" or something...
//
//    if (CREF direction = store::deref(args[0]).as_text()) {
//      auto pm = player_move(direction.value());
//      CREF pmr = pm;
//      return pmr.invoke(player, cause, args);
//    }
//  }
//};

/* ---------- generic composable actions ---------- */

//compose actions; returns dict<[action id / query], [ret val]>
//performs actions in given order
class comp_a : public action {
  str_vec actions;
public:
  comp_a(str_vec const& action_ids) : actions(action_ids) { }
  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args) const {
    CREF rslt = store::transient<dict>();
    REF retd = store::deref(rslt);
    for (REF aw : actions)
      if (REF a = store::deref(aw).as_action())
        retd.set(aw, a.invoke(player, cause, args));
    return rslt;
  }
};
//compose actions; return ret of 1 chosen, main

class comp_main_a : comp_a {
  std::string main_action;
public:
  comp_main_a(std::string const& main_a, str_vec const& action_ids) : main_action(main_a), comp_a(action_ids) { }
  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args) const {
    comp_a::invoke(player, cause, args); //ignore result
    if (REF a = store::deref(main_action).as_action())
      return a.invoke(player, cause, args);
    return action::nil_ret;
  }

};

//assoc[iate] first arg to prop of ^query
class assoc_a : public action {
  std::string query; //what object to change (absolute query / id)
  std::string prop; //what property to change on query\dict
public:
  assoc_a(std::string const& query, std::string const& property) : query(query), prop(property) { }
  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args) const {
    //assert 1 arg ?
    //assert cannot be a player action
    //returns ... arg is known, - can get handle of query-obj?
    //nil if fail ... that would throw... which?
    if (REF d = store::deref(query).as_dict()) {
      d.assoc(prop, args[0]);
      return store::handle_of("$$"); //ok
    } else return action::nil_ret;
    
  }
};

//dissoc[iate] prop of query-dict
class dissoc_a : public action {
  std::string query; //what object to change (absolute query / id)
  std::string prop; //what property to change on query\dict
public:
  dissoc_a(std::string const& query, std::string const& property) : query(query), prop(property) { }
  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args) const {
    //assert 0 arg ?
    //assert cannot be a player action
    //returns ... arg is known, - can get handle of query-obj?
    //nil if fail ... that would throw... which?
    if (REF d = store::deref(query).as_dict()) {
      d.dissoc(prop);
      return store::handle_of("$$"); //ok
    } else return action::nil_ret;
  }
};

//conj[oin] item to bag
class bag_conj_a : public action {
  std::string query; //what object to change (absolute query / id)
public:
  bag_conj_a(std::string const& query) : query(query) { }
  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args) const {
    //assert 1 arg ?
    //assert cannot be a player action
    //returns ... arg is known, - can get handle of query-obj?
    //nil if fail ... that would throw... which?
    if (REF b = store::deref(query).as_bag()) {
      b.insert(args[0]);
      return store::handle_of("$$"); //ok
    } else return action::nil_ret;

  }
};

//disj[oin] item from bag
class bag_disj_a : public action {
  std::string query; //what object to change (absolute query / id)
public:
  bag_disj_a(std::string const& query) : query(query) { }
  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args) const {
    //assert 1 arg ?
    //assert cannot be a player action
    //returns ... arg is known, - can get handle of query-obj?
    //nil if fail ... that would throw... which?
    if (REF b = store::deref(query).as_bag()) {
      b.erase(args[0]);
      return store::handle_of("$$"); //ok
    } else return action::nil_ret;
  }
};


class test_set_ret : public action {
public:
  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args) const {
    return store::transient<textview>("hello from test action; transient");
    //return store::emplace<textview>("tvTEST", "hello from test action; not transient");
  }
};

#endif	/* ACTIONS_H */

