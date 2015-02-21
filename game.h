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
  store::handle main_loop(std::istream& is_lines, parser const& p) {
    try {
      p.process(is_lines);
      return action::nil_ret;
    } catch (end_game& e) {
      return e.what();
    }
  }
  store::handle load(std::istream& is, const bool also_flush = true) {
    if (also_flush) store::flush();
    parser p({//loading commands
      {"def", with_id(defs_simple::def_dict)},
      {"defint", with_id(defs_simple::def_int)},
      {"deftext", with_id(defs_simple::def_text)},
      {"assoc", with_id(defs_simple::assoc)}
    });
    return main_loop(is, p);
  }
  store::handle load(std::string const& file, const bool also_flush = true) {
    std::ifstream ifs(file);
    return load(ifs, also_flush);
  }
  store::handle play(std::istream& cmds) {
    //cmds: player commands (stdin)
    parser::cmd_map cmd_m{
      {"go", player_fns::go}
    };
    if (REF user_defined = store::deref("$player_commands", true).as_dict())
      user_defined.for_each([&](dict::kv_pair C& kvp) {
        if (store::deref(kvp.second).as_action())
          cmd_m.emplace(kvp.first, action_cmd(kvp.second));
      });
    return main_loop(cmds, parser(std::move(cmd_m)));
  }


private:

};

#endif	/* GAME_H */

