/* 
 * File:   view_utils.h
 * Author: maartyl
 *
 * Created on March 21, 2015, 8:17 PM
 */

#ifndef VIEW_UTILS_H
#define	VIEW_UTILS_H

#include "entity_defs.h"

class capture_bind : public view {
  store::handle subject_;
  store::handle object_;
  store::handle view_;
public:
  capture_bind(store::handle subject,
               store::handle object,
               store::handle view) : subject_(subject), object_(object), view_(view) {
    if (REF v = store::deref(view_).as_view()) 
      valid = v.valid;
    else throw std::invalid_argument("capture_bind: passed invalid view");
  }
  virtual std::string print(entity& subject, entity& object) const {
    REF sub = subject_ ? store::deref(subject_) : subject;
    REF obj = object_ ? store::deref(object_) : object;
    if (REF v = store::deref(view_).as_view())
      return v.print(sub, obj);
    throw std::invalid_argument("capture_bind: passed invalid view");
  }
  virtual text& as_text() {
    if (subject_ && object_) {
      //nil args: doesn't matter: correct binded will be used
      //will only last until end of current command: mustn't be saved for later through some link...
      // (that should be obvious for everything)
      return store::deref(store::transient<text>(print(store::deref(), store::deref()))).as_text();
    }
    return view::as_text();
  }

  //--actual binding
  static store::handle bind_view(store::handle subject,
                                 store::handle object,
                                 store::handle view) {
    return store::transient<capture_bind>(subject, object, view);
  }
  static store::handle bind_view(store::handle object, store::handle view) {
    return bind_view(store::handle_of(), object, view);
  }
};
//!! these are just first-level functions; because of order problems...
inline store::handle bind_view(store::handle subject,
                               store::handle object,
                               store::handle view) {
  return capture_bind::bind_view(subject, object, view);
}
inline store::handle bind_view(store::handle object, store::handle view) {
  return capture_bind::bind_view(object, view);
}

#endif	/* VIEW_UTILS_H */

