/* 
 * File:   store.cpp
 * Author: maartyl
 * 
 * Created on February 6, 2015, 2:14 AM
 */

#include "store.h"

store store::VAL = store();

#include "entity_defs.h"
#include "store_context_frame.h"

//
entity& store::deref() {
  return nil_entity::get();
}
store::handle store::handle_of() {
  return{};
}
store::handle store::dict_get(entity& dh, store::id const& prop) {
  if (REF d = dh.as_dict())
    return d.at(prop);
  else return handle_of();
}
store::handle store::find_in_context(store::id const& id) {
  if (REF c = VAL.context_)
    return c->find(id);
  return handle_of();
}
store::handle store::find_area_alias(store::id const& id) {
  return deref(find_not_alias("$player")->second)["area"]["aliases"].as_dict().at(id);
}

///-----

void store::flush_dicts(store& s) {
  // ... cyclic references in dicts? PROBLEM
  // sadly, I must traverse it and erase dicts myself...
  for (REF e : s.entities_)
    if (REF d = e.second->as_dict())
      d.flush();
}
//entity& store::query(std::string const& qry, const bool nil_on_fail/*false*/) 
//
void store::init_() {
  emplace<dict>("$player");
  emplace<dict>("$$"); //just 'present' dummy for ?tags (in dict properties))
  //$$ alternative: self. But dangerous: reference counting, memory leak... - could be simpler sometimes.
  //so long as 1 knows object won't live for long / will ...

  //causes
  emplace<dict>("?event");
  emplace<dict>("?player");
  emplace<dict>("?loading");
}
//entity& operator "" _sd(const char* str, std::size_t len) {
//  return store::deref(str);
//}

