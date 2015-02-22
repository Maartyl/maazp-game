/* 
 * File:   store_context_frame.cpp
 * Author: maartyl
 * 
 * Created on February 22, 2015, 6:06 AM
 */

#include "store_context_frame.h"
#include "entity_defs.h"
store_context_frame::store_context_frame(dict_map&& d, const bool lax) : store_context_frame(store::transient<dict>(std::move(d), lax)) { }
store_context_frame::store_context_frame(store::handle d, const bool lax) : dict_(d), lax(lax) {
  if (!lax && !store::deref(d).as_dict()) //just test
    throw std::invalid_argument("context_frame: requires dict handle");
  next_lvl = store::VAL.context_;
  store::VAL.context_ = this;
}
store_context_frame::~store_context_frame() {
  store::VAL.context_ = next_lvl;
}
store::handle store_context_frame::find(store::id id) {
  if (REF d = store::deref(dict_, lax).as_dict()) {
    if (auto h = store::dict_get(d, id))
      return h;
  } else if (!lax) throw std::invalid_argument("context_frame.find: requires dict handle");
  if (next_lvl)
    return next_lvl->find(id);
  return store::handle_of();
}

