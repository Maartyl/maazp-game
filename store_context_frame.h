/* 
 * File:   store_context_frame.h
 * Author: maartyl
 *
 * Created on February 22, 2015, 6:06 AM
 */

#ifndef STORE_CONTEXT_FRAME_H
#define	STORE_CONTEXT_FRAME_H

#include "utils.h"
#include "store.h"
#include "entity.h"

class store_context_frame {
  friend store;

  //uses RAII for safe, correct stacking
  store::handle dict_;
  store_context_frame* next_lvl;
public:
  typedef std::map<std::string, entity::handle> dict_map;
  store_context_frame(dict_map&& d);
  store_context_frame(store::handle d);
  virtual ~store_context_frame();
public:
  store::handle find(store::id id);
};

#endif	/* STORE_CONTEXT_FRAME_H */

