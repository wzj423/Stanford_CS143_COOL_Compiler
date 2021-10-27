#include <assert.h>
#include <stdio.h>

#include <map>
#include <set>
#include <stack>
#include <vector>
#include <utility>

#include "emit.h"
#include "cool-tree.h"
#include "symtab.h"

enum Basicness     {Basic, NotBasic};
#define TRUE 1
#define FALSE 0

class CgenClassTable;
typedef CgenClassTable *CgenClassTableP;

class CgenNode;
typedef CgenNode *CgenNodeP;

class CgenClassTable : public SymbolTable<Symbol,CgenNode> {
private:
   List<CgenNode> *nds;
   ostream& str;
   int stringclasstag;
   int intclasstag;
   int boolclasstag;


// The following methods emit code for
// constants and global declarations.

   void code_global_data();
   void code_global_text();
   void code_bools(int);
   void code_select_gc();
   void code_constants();

// The following creates an inheritance graph from
// a list of classes.  The graph is implemented as
// a tree of `CgenNode', and class names are placed
// in the base class symbol table.

   void install_basic_classes();
   void install_class(CgenNodeP nd);
   void install_classes(Classes cs);
   void install_class_tags();//only called by install_classes()
   void build_inheritance_tree();
   void build_full_methods_attrs();//only called by build_inheritance_tree()
   void set_relations(CgenNodeP nd);

   void code_class_name_table();
   void code_class_parent_name_table();
   void code_class_obj_table();
   void code_dispatch_tables();
   void code_obj_prototypes();

   void code_initializers();
   void code_methods();

public:

   std::map<Symbol,CgenNodeP> class_name_map;//class name table
   std::vector<CgenNodeP> classes_list;// a list of all classes involved so that their tags can be defined.
   CgenClassTable(Classes, ostream& str,CgenClassTableP& );
   void code();
   CgenNodeP root();
};


class CgenNode : public class__class {
private: 
   CgenNodeP parentnd;                        // Parent of class
   List<CgenNode> *children;                  // Children of class
   Basicness basic_status;                    // `Basic' if class is basic
                                              // `NotBasic' otherwise
   int tag;// class tag in the assembly file to be generated.

public:
   CgenNode(Class_ c,
            Basicness bstatus,
            CgenClassTableP class_table);

   void add_child(CgenNodeP child);
   List<CgenNode> *get_children() { return children; }
   void set_parentnd(CgenNodeP p);
   CgenNodeP get_parentnd() { return parentnd; }
   int basic() { return (basic_status == Basic); }

   void set_tag(int t) {tag=t;}
   int get_tag() const {return tag; }

   std::vector<std::pair<CgenNodeP,attr_class*> > full_attrs;
   std::vector<std::pair<CgenNodeP,method_class*> > full_methods;
   //void set_full_features(std::vector<attr_class*> &ancestor_attrs,std::vector<method_class*> &ancestor_methods);
   void set_full_features(CgenNodeP parent);
   const std::vector<std::pair<CgenNodeP,attr_class*> > & get_full_attrs() {return full_attrs; }
   const std::vector<std::pair<CgenNodeP,method_class*> > & get_full_methods() {return full_methods; }

   int find_method_id(Symbol name) {
     for(int i=full_methods.size()-1;i>=0;--i) {
        if(full_methods[i].second->name==name) return i;
     }
     return -1;
   }
};

class BoolConst 
{
 private: 
  int val;
 public:
  BoolConst(int);
  void code_def(ostream&, int boolclasstag);
  void code_ref(ostream&) const;
};

/** Environment class: record the enviromment and storage information involved in code generation
 *       -current class
 *       -current method and its formals
 *       -current class attributes
*/
class Environment
{
public:
   /* data */
   CgenNodeP cur_class;//the class involved in the current scope
   method_class* cur_method;
   std::vector<std::pair<CgenNodeP,attr_class*>> cur_attrs;
   std::vector<Formal> method_formals;
   std::vector<Symbol> local_var_stack;
public:
   Environment(/* args */)=default;
   ~Environment(){};

   void clear_attrs() {cur_attrs.clear();}
   void clear_method_formals() {method_formals.clear();}
   void clear_stack() {local_var_stack.clear();}

   void clear() {
      clear_attrs();
      clear_method_formals();
      clear_stack();
      cur_class=nullptr;
      cur_method=nullptr;
   }

   CgenNodeP get_cur_class() {return cur_class;}

   void set_cur_class(CgenNodeP new_current_class)
   {
      clear_attrs();
      clear_method_formals();
      cur_class = new_current_class;
      cur_attrs = std::vector<std::pair<CgenNodeP, attr_class *>>(cur_class->get_full_attrs());
   }

   void set_cur_method(method_class* new_current_method) {
      clear_method_formals();
      cur_method=new_current_method;
      for(int i=cur_method->formals->first();cur_method->formals->more(i);i=cur_method->formals->next(i)) {
         method_formals.push_back(cur_method->formals->nth(i));
      }
   }

   auto get_cur_attrs() {return cur_attrs;}
   void set_cur_attrs(std::vector<std::pair<CgenNodeP,attr_class*>> new_attrs) {cur_attrs=new_attrs;}
   auto get_cur_method_formals() {return method_formals;}
   void set_cur_method_formals(std::vector<Formal> new_formals) {method_formals=new_formals;}
   //auto get_cur_method_formals() {return method_formals;}
   //void set_cur_attrs(std::vector<Formal> new_formals) {method_formals=new_formals;}  

   void push_stack(Symbol name) {
      local_var_stack.push_back(name);
   }    
   void pop_stack() {
      local_var_stack.pop_back();
   }
   int find_symbol_on_stack(Symbol name) {
      for(int i=local_var_stack.size()-1;i>=0;--i) {
         if(local_var_stack[i]==name) {
            return local_var_stack.size()-1-i;
         }
      }
      return -1;
   }

   //first argument pushed on the stack first
   int find_formal_symbol(Symbol name) {
      for(int i=method_formals.size()-1;i>=0;--i) {
         auto p=dynamic_cast<formal_class*>(method_formals[i]);
         if(p&&name==p->name) {
            return method_formals.size()-i;
         }
      }
      return -1;
   }

   int find_class_attr_symbol(Symbol name) {
      for(int i=0;i<(int)cur_attrs.size();++i) {
         if(name==cur_attrs[i].second->name) {
            return i;
         }
      }
      return -1;
   }

}env;

/********************************************************
 Introduction on local variable management for my cgen

 Cool programs introduces new variables upon three situations:
      -class attributes
      -method arguments
      -let statements
   besides, anonymous temporary variables may also be introduced
   on arithmetic and various other statements.
      - (*) anony_temp_obj

   class attributes has fixed locations, thus easy to manage.

   For method arguments, they are pushed upon the stack before
a method is called. So we can simply locate their positions on
stack through $SP register in the form of $SP+X. The only thing 
that worth noting is that their memory locations are in the 
range of previous stack frames.

   The most complicated and subtle part is the management of
local variables introduced by let statements. My design decision
is to allocated fixed amount of memory upon method calling( when
the stack frame is created.) The reason comes from the following 
facts:
   - local variables are bond to scopes, and method is the second 
      biggest scope(only smaller than class scopes.)
   - one let statement just introduce one local variables, even if
      the let statement is inside a while loop. Thus, every node 
      either introduces (and allocates) exactly one local variable 
      or introduces zero.
   - for each AST nodes, the number of local variables allocated
      previously is fixed and can be determined, since a tree node
      has exactly one way up to the root.

Therefore, we can calculate the max amount of memory a method would
use to store local variables with the following rules:
Define L(x) for the amount of memory needed for local variables in 
AST subtree x: 
   if-else statement: L(self)=L(pred)+max(L(if),L(else))
   while statement: L(self)= L(pred)+L(body)
   let statement: L(self)= L(body)+1
   other statement: L(self) = sum of L(son of x)

and with a simple tree-tranversal, we can assign every local-var a
fixed word in memory :-)
*********************************************************/