/* 
 * File:   store.cpp
 * Author: maartyl
 * 
 * Created on February 6, 2015, 2:14 AM
 */

#include "store.h"

store store::VAL = store();

#include "entity_defs.h"

//
entity& store::deref(store::id const& id, const bool nil_on_fail) {
  if (!nil_on_fail)
    return deref(handle_of(id));

  auto it = find(id);
  return it != end() ? deref(it->second) : nil_entity::get();
}
entity& store::deref(handle const& h) {
  return h ? *h : nil_entity::get();
}
void store::flush_dicts(store& s) {
  // ... cyclic references in dicts? PROBLEM
  // sadly, I must traverse it and erase dicts myself...
  for (REF e : s.entities_)
    if (REF d = e.second->as_dict())
      d.flush();
}
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

