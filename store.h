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
#include "parser.h"
//#include "entity_defs.h" !!! auto declared here: breaks stuff

//entity store 'singleton' ...
//well, it's not really a singleton, but from the design of the application, I don't think it's too bad...
//no concurrency, only little bit of modifications...; no one is gonna run 2 games simultaneously...
//Normal object would be better but then it could not be referenced from anywhere... (Clojure var binding would be perfect, but...)
//if necessary: would get implemented in bindings manner
class store {
public:
  typedef entity::handle handle; //~only this should be used in APIs. ~Always dereference...
  typedef std::string id; //global id (used in settings file ...)

private:
  typedef std::unordered_map<store::id, handle>::iterator map_iter;
  store() { } //private .ctor: ~singleton
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
  static entity& deref(); //nil
  static entity& deref(handle const& h) {
    return h ? *h : deref();
  }
  static entity& deref(store::id const& id, const bool nil_on_fail = false) {
    if (id.size() == 0) // to be able to get get nil easily from anywhere :: deref("", true);
      if (nil_on_fail) return deref();
      else throw std::logic_error("deref: no id provided");

    if (id[0] == '^') return query(id, nil_on_fail);

    if (!nil_on_fail)
      return deref(handle_of(id));

    auto it = find(id);
    return it != end() ? deref(it->second) : deref();
  }
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
  static entity& query(std::string const& qry, entity& origin, const bool nil_on_fail = false) {
    if (qry[0] != '^')
      if (nil_on_fail) return deref();
      else throw std::logic_error("invalid query: " + qry);
    if (origin && qry[1] != '.')
      if (nil_on_fail) return deref();
      else throw std::logic_error("invalid relative query: " + qry);
    //TODO: possibly full correctness check

    CREF q = parser::words(parser::triml(qry, "^"), "."); //query parts
    auto it = std::begin(q);
    //ptr to be able to change it
    entity* e = &(origin ? origin : deref(*it, nil_on_fail)); //current entity ptr; origin || deref first
    if (origin) e = &(*e)[*it]; //in case of relative query: first argument is property too
    while (*e && (++it != std::end(q))) //for q, but first; stop for nil entity
      e = &(*e)[*it]; //update: ptr of reference from dict at [current property from q]

    if (!nil_on_fail && !*e) //is nil: throw instead of returning ;; virtual second
      throw std::logic_error("query failed at: " + *it + " of: " + qry);

    return *e;
  }
  static entity& query(std::string const& qry, const bool nil_on_fail = false) {
    return query(qry, deref(), nil_on_fail);
  }




public: //inserting 
  static store::handle insert(std::string const& id, handle entity) {
    if (entity->is_nil())
      throw std::invalid_argument("€store: cannot hold NIL entity; for id: " + id);

    auto itp = VAL.entities_.emplace(id, entity);
    if (!itp.second)
      throw std::invalid_argument("€store: id already used: " + id);
    
    auto it = itp.first;
    VAL.iters_.emplace(it->second, it); //no need to check for present again: both inserted only here

    //prn((entity == it->second));
    return it->second;
  }
  template<typename TE, typename... Args>
  static store::handle emplace(store::id const& id, Args&&... args) {
    return insert(id, transient<TE>(std::forward<Args>(args)...)); //requires polymorphism
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
  //deleting individual items... - someone could still hold reference -- shared pointer sort of solves... but references...
  // but possibly... but slow: searching the whole store for references in dicts and only allow if none holds it
  // - still problem with cyclic references ... ? (could flush deleted dicts...)
  // making handles shared pointers should make this easier
  //SOLUTION: delete only adds the operation to queue; queue processed in some safe moment (game loop end / ...)

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
    flush();
    initialized = true;

    /*why flush and not init_? if: flush called first: already initialized,
     *  then init still can be called 2nd time. */
  }

public: //misc
  //dangerous: can deallocate entity while still in use; only pointer is no longer referenced
  template<typename T, typename... Args>
  static handle transient(Args... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
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

