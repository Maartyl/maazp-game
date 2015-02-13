/* 
 * File:   utils.h
 * Author: maartyl
 *
 * Created on February 6, 2015, 12:59 AM
 */

#ifndef UTILS_H
#define	UTILS_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>

#define REF auto&
#define CREF const REF
#define C const

typedef std::vector<std::string> str_vec;


//
inline std::string operator "" _s(const char* c, size_t len) {
  return std::string(c);
}

#define prn(cnt) std::cout << cnt << std::endl
#define errprn(cnt) std::cerr << cnt << std::endl

//
template<class T, class... U>
std::unique_ptr<T> make_unique(U&&... u) {
  return std::unique_ptr<T>(new T(std::forward<U>(u)...));
}

#endif	/* UTILS_H */

