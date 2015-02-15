/* 
 * File:   end_game.h
 * Author: maartyl
 *
 * Created on February 14, 2015, 4:30 AM
 */

#ifndef END_GAME_H
#define	END_GAME_H

#include "entity.h"


//class to be thrown to exit main game loop
// cannot be caught by anything else as it doesn't inherit anything else
class end_game {
  entity::handle data_;

public:
  end_game() : data_() { }
  end_game(entity::handle h) : data_(h) { }

  entity::handle what() {return data_;}
  static void cmd(std::string const& args) {throw end_game();}
};

#endif	/* END_GAME_H */

