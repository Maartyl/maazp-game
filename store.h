/* 
 * File:   store.h
 * Author: maartyl
 *
 * Created on February 6, 2015, 2:14 AM
 */

#ifndef STORE_H
#define	STORE_H

#include <unordered_map>
#include <string>
#include <stdexcept>


#include "entity.h"
//#include "entity_defs.h" !!! auto declared here: breaks stuff

//entity store 'singleton' ...
//well, it's not really a singleton, but from the design of the application, I don't think it's too bad...
//no concurrency, only little bit of modifications...; no one is gonna run 2 games simultaneously...
//API logic: if store weren't global, each entity would have to reference it and thus, it wouldn't change the way it works...
class store {
public:
  typedef entity::handle handle; //only this should be used in APIs. ~Always dereference...
  typedef std::string id; //global id (used in settings file ...)

private:
  typedef std::unordered_map<store::id, handle>::iterator map_iter;
  store() { } //private .ctor: singleton
  //  static handle create_handle(entity* obj) {
  //    return obj;
  //  }
  static map_iter iter_of(store::id const& id) {
    auto it = find(id);
    if (it != end())
      return it;

    throw std::invalid_argument("€store: invalid id: " + id);
  }
  static map_iter find(store::id const& id) {
    auto it = find_not_alias(id);
    if (it != end()) 
      return it;
    
    auto ita = VAL.aliases_.find(id);
    if (ita != std::end(VAL.aliases_))
      return ita->second;

    return it; //end
  }
  static map_iter find_not_alias(store::id const& id) {
    return VAL.entities_.find(id);
  }
  static map_iter end() {
    return std::end(VAL.entities_);
  }

  
public: //access
  ///dereference
  static entity& deref(handle const& h);
  static entity& deref(store::id const& id, const bool nil_on_fail = false);
  //  static entity& deref(store::id const& id) { // maybe... to make it work as functional argument
  //    return deref(id, false);
  //  }
  static handle handle_of(store::id const& id) {
    return iter_of(id)->second;
  }
  //  static handle handle_of(entity & e) {
  //    return &e;
  //  }
  //  static store::id const& id_of(entity & e) {
  //    return id_of(handle_of(e));
  //  }
  static store::id const& id_of(handle const& h) {
    auto it = VAL.iters_.find(h);
    if (it != std::end(VAL.iters_)) {
      return it->second->first;
    }
    throw std::invalid_argument("€store[id_of]: invalid handle");
  }




public: //inserting 
  static store::handle insert(std::string const& id, handle entity) {
    if (entity->is_nil())
      throw std::invalid_argument("€store: cannot hold NIL entity for id: " + id);

    auto itp = VAL.entities_.emplace(id, entity);
    if (!itp.second)
      throw std::invalid_argument("€store: id already used: " + id);
    
    auto it = itp.first;
    VAL.iters_.emplace(it->second, it); //no need to check for present again: both inserted only here

    return it->second;
  }
  template<typename TE, typename... Args>
  static store::handle emplace(store::id const& id, Args&&... args) {
    return insert(id, std::make_shared<TE>(std::forward<Args>(args)...)); //new: requires polymorphism
  }
  //alias
  static void add_alias(store::id const& alias, store::id const& existing_id) {
    auto it = find(existing_id); //searches aliases too
    if (it == end())
      throw std::invalid_argument("€store: cannot alias [" + alias + "] to nonexistent id: " + existing_id);

    if (find(alias) != end()) //use find to check among IDs, not only aliases
      throw std::invalid_argument("€store: id already used: " + alias);

    VAL.aliases_.emplace(alias, it);
  }

public: //removal
  //some flush, that would delete everything: ok
  //deleting individual items... - someone could still hold reference -- shared pointer sort of solves... but... and still ugly...
  // but possibly... but slow: searching the whole store for references in dicts and only allow if none holds it
  // making handles shared pointers should make this easier

  ///removes everything from store
  static void flush() {
    //works thanks to shared pointers
    // ... cyclic references in dicts? PROBLEM
    // sadly, I must traverse it and erase dicts myself...
    flush_dicts(VAL);

    std::unordered_map<store::id, handle> entities;
    std::unordered_map<handle, map_iter> iters;
    std::unordered_map<store::id, map_iter> aliases;
    std::swap(entities, VAL.entities_);
    std::swap(iters, VAL.iters_);
    std::swap(aliases, VAL.aliases_);
    init_();
 }

public: //initialization
  static void init() {
    static bool initialized = false;
    if (initialized) return;
    init_();
    initialized = true;
  }
private:
  static void init_();
  static void flush_dicts(store& s);

private:
  std::unordered_map<store::id, handle> entities_{};
  std::unordered_map<handle, map_iter> iters_{}; //to get names from handles: to dump to file; and for actions
  std::unordered_map<store::id, map_iter> aliases_{};
  size_t unique_id_num{1}; //for creating unique ids dynamically

  static store VAL; //singleton object
};
//entity& operator "" _sd(const char* str, std::size_t len);

#endif	/* STORE_H */

