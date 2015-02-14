/* 
 * File:   main.cpp
 * Author: maartyl
 *
 * Created on February 5, 2015, 11:33 PM
 */

#include <cstdlib>
#include <iostream>
#include <string>

#include <sstream>

#include "utils.h"
#include "entity_defs.h"
#include "store.h"

#include "parser.h"
#include "cmds.h"
#include "loading_cmds.h"
#include "actions.h"

//#define prn(cnt) std::cout << cnt << std::endl
/*
 * 
 */
int main(int argc, char** argv) {
  store::init();

  REF n = nil_entity::get();
  //n.is_nil();

  auto h = store::emplace<entity_int>("#int1", 5);
  auto h2 = store::emplace<dict>("lol");
  auto h3 = store::emplace<text>("#say1", "this is some text to test this");

  auto h4 = store::emplace<bag>("bag1");
  store::deref("bag1").as_bag().insert(h);
  REF hr = *store::deref("bag1").as_bag().begin();
  if (REF i = hr->as_int())
    prn((int) i);

  store::deref(h).set(42);

  if (REF t = store::deref("#say1").as_text())
    std::cout << "t is " << t.value() << std::endl;
  else
    std::cout << "t is not" << std::endl;

  auto crp = parser::cmd_and_rest("a b");
  prn(crp.first << "::" << crp.second);

  parser p({
    {"yo", with_id([](store::id C& id, std::string C& args) {
        prn("hello! " << id << " and all of: " << args);
      })},
    {"yah", with_id([](store::id C& id, std::string C& args) {
        prn("smash! " << id << " and all of: " << args);
      })},
    {"def", with_id(defs_simple::def_dict)},
    {"defint", with_id(defs_simple::def_int)},
    {"deftext", with_id(defs_simple::def_text)},
    {"assoc", with_id(defs_simple::assoc)}
  });

  std::string ps = R"xxx(
yo mary john
yah martin test1 test2
.multi yo
id_is_here
        arg1 arg2
        arg3
  .endm #here ends .multi
yah miko

          def rampa
          defint roo 0xA5
          deftext say2,  this is some long text that should work fine
          def qww
          def q, a rampa, b roo
          assoc rampa, q q

 .multi deftext say3 #this is a multiline string declaration
          This a cool story about a mouse named lorry.
Stuff...
I don't KNOW!!!!
          Flowers.....
          .endm # ain't this cool?

.multi .multi yo
                                                          key
.endm # will this be included? probably not
    some
       more stuff
  .endm
          
          )xxx";

  std::stringstream ss(ps);
  p.process(ss);

  if (REF d = store::deref("rampa")["q"]["b"].as_int())
    prn("roo is " << d.value());
  else
    std::cout << "roo is not" << std::endl;

  if (REF d = store::query("^rampa.q.b").as_int())
    prn("roo is " << d.value());
  else
    std::cout << "roo is not" << std::endl;

  if (REF t = store::deref("say3").as_text())
    std::cout << "t is |" << t.value() << std::endl;
  else
    std::cout << "t is not" << std::endl;

  prn("action start");
  action_cmd ac(make_unique<test_set_ret>());
  ac("");
  prn("action end");
  
  return 0;
}

