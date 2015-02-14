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

class player_move : public action {
  std::string direction; //%up, %south, ...
public:
  player_move(const std::string& direction) : direction(direction) { }
  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args)const {
    //cause is unimportant ... - different ret for events?
    //args ... could mean direction, but ... different command? / special direction? ("", use arg") ...
    if (REF tran = player["area"][direction].as_dict()) { //transition
      if (store::deref(tran["*passing"].trigger(player))["?stop"]) {
        // cannot pass: ... should use something from that stop
        //return "can't pass" view

      }
      if (store::deref(player["area"]["*leaving"].trigger(player))["?stop"]) {
        //can't move: not allowed to leave ... should use something from that stop for view
        //return "cant leave" view
      }
      player.set("area", tran.at("%to"));
      player["area"]["*left"].trigger(player);
      tran["*passed"].trigger(player);
      //return good view
      
    }
    //return: there is nothing in that direction
  }
};

//... kind of weird command
class player_go : public action {
public:
  virtual action::ret_t invoke(entity& player, const entity& cause, const arg_coll& args) const {
    //assert: 1 arg ... ? for now...
    if (args.size() != 1)
      return { }; //some "wrong command arguments view" or something...

    if (CREF direction = store::deref(args[0]).as_text()) {
      auto pm = player_move(direction.value());
      CREF pmr = pm;
      return pmr.invoke(player, cause, args);
    }
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

