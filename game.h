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
  void load(std::istream& is, const bool also_flush = true) {
    if (also_flush) store::flush();
    parser p({//loading commands
      {"def", with_id(defs_simple::def_dict)},
      {"defint", with_id(defs_simple::def_int)},
      {"deftext", with_id(defs_simple::def_text)},
      {"assoc", with_id(defs_simple::assoc)}
    });
    p.process(is);
  }
  void load(std::string const& file, const bool also_flush = true) {
    std::ifstream ifs(file);
    std::istream& is = ifs;
    load(is, also_flush);
  }


private:

};

#endif	/* GAME_H */

