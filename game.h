/* 
 * File:   game.h
 * Author: maartyl
 *
 * Created on February 13, 2015, 6:35 PM
 */

#ifndef GAME_H
#define	GAME_H

#include <fstream>
#include <istream>

#include "store.h"
#include "loading_cmds.h"
#include "end_game.h"
#include "player_cmds.h"


//main game loop
//handles loading games ... interaction
//menus ...
class game {
public:
/*
   * what should it do:
   * menu
   * - open "menu game"
   * - player can choose what level / world / game to play
   * - then ends, loads the given file and starts that
   *
   * both can share REPL

   */
  
  store::handle load(std::istream& is, const bool also_flush = true) {
    if (also_flush) store::flush();
    return main_loop(is, parser(dynamic_cmds({
      {"def", with_id(defs_simple::def_dict)},
      {"defint", with_id(defs_simple::def_int)},
      {"deftext", with_id(defs_simple::def_text)},
      {"alias", with_id(defs_simple::alias)},
      {"aliases", with_id(defs_simple::aliases)},
      {"copy", with_id(defs_simple::copy)},
      {"assoc", with_id(defs_simple::assoc)}
    }, "$loading_commands")));
  }
  store::handle load(std::string const& file, const bool also_flush = true) {
    std::ifstream ifs(file);
    return load(ifs, also_flush);
  }
  store::handle play(std::istream& cmds) {
    return main_loop(cmds, parser(dynamic_cmds({
      {"go", player_fns::go}
    }, "$player_commands")));
  }


private:
  store::handle main_loop(std::istream& is_lines, parser const& p) {
    //~RE(P)L
    try {
      p.process(is_lines);
      return action::nil_ret;
    } catch (end_game& e) {
      return e.what();
    }
  }
  parser::cmd_map dynamic_cmds(parser::cmd_map && cmds, store::id const& bind) {
    if (REF user_defined = store::deref(bind, true).as_dict())
      user_defined.for_each([&](dict::kv_pair C & kvp) {
        if (store::deref(kvp.second).as_action())
          cmds.emplace(kvp.first, action_cmd(kvp.second));
      });
    return cmds;
 }

};

#endif	/* GAME_H */

