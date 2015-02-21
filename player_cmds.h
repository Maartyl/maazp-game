/* 
 * File:   player_cmds.h
 * Author: maartyl
 *
 * Created on February 8, 2015, 3:48 AM
 */

#ifndef PLAYER_CMDS_H
#define	PLAYER_CMDS_H

#include <stdexcept>

#include "cmds.h"
#include "actions.h"


namespace player_fns {
  void inline consume_action_ret(const action::ret_t reth) {
    if (REF ret = store::deref(reth).as_view()) {
      prn(""); //prn(">---"); //TODO: parameterize / load from text entity / ...
      prn(ret.print());
      if (ret.valid) {
        REF p = store::deref("$player");
        p["*action"].trigger(p);
        p["area"]["*action"].trigger(p);
      }
      return;
    }
    throw std::logic_error("action_cmd: Outmost action didn't return view.");
  }
  void inline consume_invoker(action const& a) {
    consume_action_ret(a.invoke(store::deref("?player")));
  }

  //basic cmds for parser_map
  void inline go(std::string const& args) {
    //south, north, east, west, up, down, or: %arg
    auto p = parser::first_and_rest(args);
    if (p.first == "" || p.second != "")
      throw std::invalid_argument("go: expects exactly 1 argument: direction; got: " + args);
    std::string d = parser::to_lower(std::move(p.first));
    if (d == "south" || d == "s") d = "%south";
    else if (d == "north" || d == "n") d = "%north";
    else if (d == "east" || d == "e") d = "%east";
    else if (d == "west" || d == "w") d = "%west";
    else if (d == "up" || d == "u") d = "%up";
    else if (d == "down" || d == "d") d = "%down";
    else d = "%" + d;
    //else probably won't find direction and throw correct ex.
    consume_invoker(player_move(d));
  } 
}

#endif	/* PLAYER_CMDS_H */

