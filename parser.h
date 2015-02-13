/* 
 * File:   parser.h
 * Author: maartyl
 *
 * Created on February 7, 2015, 10:29 PM
 */

#ifndef PARSER_H
#define	PARSER_H

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <istream>
#include <stdexcept>

#include "utils.h"


/**
 * I decided to call this a parser, but it will really 'evaluate' all lines of the given istream.
 * (the 'evaluation' is not part of this class though)
 * Wrapper around idea: line starts with a command name. Rest of line is argument of that command. -> apply all
 * .multi 'command' allows joining lines and includes \n in texts.
 *
 * Works with both: user input and def-files. Only will be initialized with different commands.
 * (possibly?) -> Yes, player can use .multi . Only it will be probably useles...
 *
 * yes, processing is stateful and changes whatever somewhere... (known from cmds; otherwise would do nothing...)
 *
 * // _m method suffix means: mutates argument
 * // originally there were more, but they *are* ugly.

 */
class parser {
public: //typedefs
  typedef std::function<void(std::string const&/*rest of line: ~args*/) > cmdfn;
  typedef std::unordered_map<std::string, cmdfn> cmd_map;
  static constexpr const char* delims = " ,\t\n\r";
public:
  parser(cmd_map commands) : cmds_(commands) { }

public: //process istream
  void process(std::istream & is) {
    std::string line;
    size_t line_num = 0;
    while (std::getline(is, line)) {
      ++line_num;
      remove_comment_m(line);

      auto lp = cmd_and_rest(line);
      while (lp.first == ".multi") { //not expected to happen many times, but could...
        auto p = compose_multiline(lp.second, is, true); //{#lines, joined}
        line_num += p.first; //num of read lines
        lp = cmd_and_rest(p.second);
      }

      if (lp.first.size() == 0) //empty line / only comment
        continue;

      try {
        process_line(lp.first, lp.second);
      } catch (std::logic_error& e) {
        errprn("[ln:" << line_num << "]input logic error: " << e.what());
      } catch (std::runtime_error& e) {
        errprn("[ln:" << line_num << "]input runtime error: " << e.what());
      }
    }
  }

private: //handle multiline strings
  ///raeds until .endm
  ///-> {number of lines read, lines concatenated}
  ///include_comments: if true, doesn't remove comments; keeps lines intact
  std::pair<size_t, std::string> compose_multiline(std::string const& start, std::istream & is, const bool include_comments = true) {
    //keeps 'comments' (# and following text) [except for start: already processed - good]
    std::string line;
    std::stringstream rows; 
    rows << start;
    size_t relative_line_num = 0; //how many lines has this method read
    while (std::getline(is, line)) 
      if (".endm" == trim(remove_comment(line))) 
        return {++relative_line_num, std::move(rows.str())};
      else 
        rows << (relative_line_num++ ? '\n' : ' ') << (include_comments ? line : remove_comment(line));
    
    throw std::runtime_error("parser: EOF while reading .multi; expected .endm");
  }
private: // process line
  void process_line(std::string const& cmd, std::string const& args) {
    auto cmdit = cmds_.find(cmd);
    if (cmdit == std::end(cmds_))
      throw std::logic_error("no such command: " + cmd);

    cmdit->second(args); //actually invoke command
 }




public: //line helpers
  //like first and rest; also left-trims rest
  static std::pair<std::string, std::string> cmd_and_rest(std::string const& line_) {
    //auto p = first_and_rest(line_);
    return first_and_rest(line_);
  }
  //left-trims rest too... too weird without it, but possibly...
  //originally for parsing deftext; but well, cannot start with whitespace...
  static std::pair<std::string, std::string> first_and_rest(std::string const& line_) {
    auto&& line = triml(line_);
    auto ws0 = line.find_first_of(delims);
    if (ws0 == std::string::npos) return {
      line, ""
    };
    return {
      line.substr(0, ws0), triml(line.substr(ws0))
    };
  }
  static std::string remove_comment(std::string const& str) {
    return str.substr(0, str.find('#'));
  }
  static void remove_comment_m(std::string& str) {
    //find->npos is treated correctly
    str.substr(0, str.find('#')).swap(str);
  }
  static std::string triml(std::string const& str) {
    auto startpos = str.find_first_not_of(delims);
    if (startpos != std::string::npos)
      return str.substr(startpos);
    else return ""; //nothing else found: trimmed to nothing
  }
  static std::string trimr(std::string const& str) {
    auto endpos = str.find_last_not_of(delims);
    if (endpos != std::string::npos)
      return str.substr(0, endpos + 1);
    else return ""; //nothing else found: trimmed to nothing
  }
  static std::string trim(std::string const& str) {
    return trimr(triml(str));
  }
  static str_vec words(std::string const& str) {
    //very slow, very ugly... but for now, I guess fine...

    std::vector<std::string> v;
    for (auto p = first_and_rest(str);
         p.first.size() != 0;
         p = first_and_rest(std::move(p.second))) {
      v.push_back(std::move(p.first));
    }
    return v;
  }
private:
  cmd_map cmds_;
};

#endif	/* PARSER_H */

