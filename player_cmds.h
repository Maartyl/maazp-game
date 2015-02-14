/* 
 * File:   player_cmds.h
 * Author: maartyl
 *
 * Created on February 8, 2015, 3:48 AM
 */

#ifndef PLAYER_CMDS_H
#define	PLAYER_CMDS_H

#include "cmds.h"



namespace player_fns {
  void inline consume_action(const action::ret_t reth) {
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
}

#endif	/* PLAYER_CMDS_H */

