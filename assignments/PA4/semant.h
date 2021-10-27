#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include <set>
#include <map>
#include <unordered_map>
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  #define ClassTableMap std::map 
  int semant_errors;
  void install_basic_classes();
  ostream& error_stream;
  std::map<Symbol,Class_> class_table;
  std::map<Symbol,Symbol> inherit_graph;
  //std::map<Class_,Class_> inherit_class_graph;

public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);

  void add_to_class_table(Class_ c);
  bool is_valid_inheritance();
  bool has_class(Symbol name);
  Class_ get_class(Symbol name);
  bool is_subclass(Symbol child,Symbol father,Env);
  Symbol get_parent_with_method(Symbol child,Symbol name,bool self_included=false);
  bool check_method_inheritance(Symbol s1,Symbol s2,Symbol method_name); 
  
  std::vector<Symbol> get_parents(Symbol s);
  Symbol lub(Symbol s1,Symbol s2,Env env);
  Formals get_formals(Symbol classname,Symbol methodname);
  Symbol get_return_type(Symbol classname,Symbol methodname);
  bool check(Formals formals, const  std::vector<Symbol> &par_types,Env env);
};

#define debugStream \
  if(!semant_debug) {} else std::cerr

#endif

