#include "entity.h"
#include "entity_defs.h"
#include "store.h"

#define ENILIMPL(TYPE, NAME) TYPE & entity::as_##NAME() {return nil_entity::get();}
ENILIMPL(dict, dict)
ENILIMPL(eint, int)
ENILIMPL(text, text)
ENILIMPL(view, view)
ENILIMPL(action, action)
ENILIMPL(bag, bag)
#undef ENILIMPL


entity& entity::operator[](const std::string& name) {
  if (REF d = as_dict())
    if (auto h = d.at(name))
      return store::deref(h);
  else return d; //nil
}
bool entity::set(std::string const& key, store::handle h) {
  if (REF d = as_dict()) {
    d.assoc(key, h);
    return true;
  }
  return false;
}
bool entity::set(int val) {
  if (REF i = as_int()) {
    i = val;
    return true;
  }
  return false;
}
entity::handle entity::trigger(entity& player) {
  return as_action().invoke_event(player);
}
