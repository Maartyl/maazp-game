#include "entity_defs.h"

const action::ret_t action::nil_ret{};
nil_entity nil_entity::NILVAL = nil_entity(); //initialize nil
/**/
//entity& dict::operator[](const std::string& name) { //only a getter
//  if (auto h = at(name))
//    return store::deref(h);
//  else return nil_entity::get();
//}
//store::handle dict::at(const std::string& name) {
//  auto it = dict_.find(name);
//  if (it != std::end(dict_))
//    return (it->second);
//
//  return{};
//  //throw std::invalid_argument("€dict: not found: " + name);
//}