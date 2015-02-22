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
  static map_iter iter_err(store::id const& id) {
    throw std::invalid_argument("€store: invalid id: " + id);
  }
  static map_iter iter_of(store::id const& id) {
    auto it = find(id);
    return it != end() ? it : iter_err(id);
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
  static entity& deref(store::id const& id, const bool nil_on_fail) {
    return deref(handle_direct(id, nil_on_fail));
  }
  static entity& deref(store::id const& id)/*throw on fail*/ {
    return deref(handle_direct(id));
  }
  static handle handle_of(); //nil handle
  static handle handle_of(store::id const& id)/*throw on fail*/ {
    return handle_of(id, false);
  }
  static handle handle_of(store::id const& id, const bool nil_on_fail/*false*/) {
    if (id[0] == '^') return transient<elink>(id); //handle to query: performs query upon dereferencing
    return handle_direct(id, nil_on_fail);
  }
  static handle handle_direct(store::id const& id)/*throw on fail*/ {
    return handle_direct(id, false);
  }
  static handle handle_direct(store::id const& id, const bool nil_on_fail/*false*/) {
    if (id[0] == '^') return query_handle(id, nil_on_fail); //actually perform query

    auto it = find(id);
    return it != end() ? it->second : (nil_on_fail ? handle_of() : iter_err(id)->second);
  }
  static store::id const& id_of(handle const& h) {
    auto it = VAL.iters_.find(h);
    if (it != std::end(VAL.iters_)) {
      return it->second->first;
    }
    throw std::invalid_argument("€store[id_of]: invalid handle");
  }
  static entity& query(std::string const& qry, entity& origin, const bool nil_on_fail = false) {
    return deref(query_handle(qry, origin, nil_on_fail));
  }
  static entity& query(std::string const& qry, const bool nil_on_fail = false) {
    return query(qry, deref(), nil_on_fail);
  }
  static handle query_handle(std::string const& qry, const bool nil_on_fail = false) {
    return query_handle(qry, deref(), nil_on_fail);
  }
  static handle query_handle(std::string const& qry, entity& origin, const bool nil_on_fail = false) {
    if (qry[0] != '^') {
      if (nil_on_fail) return handle_of();
      else throw std::invalid_argument("invalid query: " + qry);
    }
    if (origin && qry[1] != '.') {
      if (nil_on_fail) return handle_of();
      else throw std::invalid_argument("invalid relative query: " + qry);
    }
    //TODO: possibly full correctness check (valid chars)

    CREF q = parser::words(parser::triml(qry, "^"), "."); //query parts
    if (q.size() == 0) {
      if (nil_on_fail) return handle_of();
      else throw std::invalid_argument("invalid (empty) query: " + qry);
    }

    auto it = std::begin(q);
    //ptr to be able to change it
    handle e = (origin ? dict_get(origin, *it) : handle_of(*it, nil_on_fail)); //current entity ptr; hrom origin || handle of first
    while (e && (++it != std::end(q))) //for q, but first; stop for nil entity
      e = dict_get(e, *it); //update: ptr of reference from dict at [current property from q]

    if (!nil_on_fail && !e) //is nil: throw instead of returning ;; virtual second
      throw std::logic_error("query failed at: " + *it + " of: " + qry);

    return e;
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
    return insert(id, make_handle<TE>(std::forward<Args>(args)...)); //requires polymorphism
  }
  //alias
  static void add_alias(store::id const& alias, store::id const& existing_id) {
    auto it = find(existing_id); //searches aliases too
    if (it == end())
      throw std::invalid_argument("€store: cannot alias [" + alias + "] to nonexistent id: " + existing_id);

    if (find(alias) != end()) //use find to check among IDs too, not only aliases
      throw std::invalid_argument("€store: id already used: " + alias);

    VAL.aliases_.emplace(alias, it);
  }

public: //removal
  //some flush, that would delete everything: ok
  //SOLUTION: OK: delete only adds the operation to queue; queue processed in some safe moment (game loop end / ...)

  ///removes everything from store
  static void flush() {
    //works thanks to shared pointers
    // ... cyclic references in dicts? PROBLEM
    // sadly, I must traverse it and erase dicts myself...
    flush_dicts(VAL);
    delete_sweep(); //free queue ...

    std::unordered_map<store::id, handle> entities;
    std::unordered_map<handle, map_iter> iters;
    std::unordered_map<store::id, map_iter> aliases;
    std::swap(entities, VAL.entities_);
    std::swap(iters, VAL.iters_);
    std::swap(aliases, VAL.aliases_);
    init_();
  }
  static handle delete_mark(handle h) {
    VAL.delete_queue_.push_back(h);
    return h;
  }
  //handles problems with references and fast-dying* transients
  // - (* shared_ptr.count == 0; but an entity& is is still being used)
  //does NOT solve aliases
  // can delete something aliased from somewhere :: PROBLEM :: must be used carefully
  // - handles will work fine, but iterators can be invalidated
  // cannot delete aliases
  static void delete_sweep() {
    for (CREF h : VAL.delete_queue_) {
      CREF it = VAL.iters_.find(h);
      if (it != std::end(VAL.iters_)) {
        VAL.iters_.erase(h);
        VAL.entities_.erase(it->second);
      }
    }
    std::vector<handle> empty;
    std::swap(empty, VAL.delete_queue_);
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
    return delete_mark(make_handle<T>(std::forward<Args>(args)...));
  }
private: //misc
  template<typename T, typename... Args>
  static handle make_handle(Args... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
  }

  static handle dict_get(entity& d, store::id const& prop);
  static handle dict_get(handle d, store::id const& prop) {
    return dict_get(deref(d), prop);
  }
  

private:
  static void init_();
  static void flush_dicts(store& s);

private:
  std::unordered_map<store::id, handle> entities_{};
  std::unordered_map<handle, map_iter> iters_{}; //to get names from handles: to dump to file; and for actions; and for sweep
  std::unordered_map<store::id, map_iter> aliases_{};
  //size_t unique_id_num{1}; //for creating unique ids dynamically (...if ever needed ... ?)
  std::vector<handle> delete_queue_{}; //handles to delete on next sweep

  static store VAL; //singleton object
};
//entity& operator "" _sd(const char* str, std::size_t len);

#endif	/* STORE_H */

