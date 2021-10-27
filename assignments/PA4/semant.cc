#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stack>
#include <set>
#include <map>
#include <unordered_set>
#include <vector>
#include "semant.h"
#include "utilities.h"

extern int semant_debug;
extern char *curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtim#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stack>
#include <set>
#include <unordered_set>
#include <vector>
#include "semant.h"
#include "utilities.h"

extern int semant_debug;
extern char *curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg = idtable.add_string("arg");
    arg2 = idtable.add_string("arg2");
    Bool = idtable.add_string("Bool");
    concat = idtable.add_string("concat");
    cool_abort = idtable.add_string("abort");
    copy = idtable.add_string("copy");
    Int = idtable.add_string("Int");
    in_int = idtable.add_string("in_int");
    in_string = idtable.add_string("in_string");
    IO = idtable.add_string("IO");
    length = idtable.add_string("length");
    Main = idtable.add_string("Main");
    main_meth = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any
    //   user-defined class.
    No_class = idtable.add_string("_no_class");
    No_type = idtable.add_string("_no_type");
    Object = idtable.add_string("Object");
    out_int = idtable.add_string("out_int");
    out_string = idtable.add_string("out_string");
    prim_slot = idtable.add_string("_prim_slot");
    self = idtable.add_string("self");
    SELF_TYPE = idtable.add_string("SELF_TYPE");
    Str = idtable.add_string("String");
    str_field = idtable.add_string("_str_field");
    substr = idtable.add_string("substr");
    type_name = idtable.add_string("type_name");
    val = idtable.add_string("_val");
}

ClassTable::ClassTable(Classes classes) : semant_errors(0), error_stream(cerr)
{

    /* Fill this in */
    install_basic_classes();
    for (int i = classes->first(); classes->more(i); i = classes->next(i))
    {
        add_to_class_table(classes->nth(i));
    }
}

void ClassTable::install_basic_classes()
{

    // The tree package uses these globals to annotate the classes built below.
    // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");

    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.

    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    //
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
        class_(Object,
               No_class,
               append_Features(
                   append_Features(
                       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
                       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
                   single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
               filename);

    //
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class =
        class_(IO,
               Object,
               append_Features(
                   append_Features(
                       append_Features(
                           single_Features(method(out_string, single_Formals(formal(arg, Str)),
                                                  SELF_TYPE, no_expr())),
                           single_Features(method(out_int, single_Formals(formal(arg, Int)),
                                                  SELF_TYPE, no_expr()))),
                       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
                   single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
               filename);

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer.
    //
    Class_ Int_class =
        class_(Int,
               Object,
               single_Features(attr(val, prim_slot, no_expr())),
               filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
        class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())), filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //
    Class_ Str_class =
        class_(Str,
               Object,
               append_Features(
                   append_Features(
                       append_Features(
                           append_Features(
                               single_Features(attr(val, Int, no_expr())),
                               single_Features(attr(str_field, prim_slot, no_expr()))),
                           single_Features(method(length, nil_Formals(), Int, no_expr()))),
                       single_Features(method(concat,
                                              single_Formals(formal(arg, Str)),
                                              Str,
                                              no_expr()))),
                   single_Features(method(substr,
                                          append_Formals(single_Formals(formal(arg, Int)),
                                                         single_Formals(formal(arg2, Int))),
                                          Str,
                                          no_expr()))),
               filename);
    // The codes above only constructs built-in basic classes, the maintainance of those information is left to us.
    // The following codes are to maintain those basic classes with student-defined data structures.
    add_to_class_table(Object_class);
    add_to_class_table(IO_class);
    add_to_class_table(Int_class);
    add_to_class_table(Bool_class);
    add_to_class_table(Str_class);
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream &ClassTable::semant_error(Class_ c)
{
    return semant_error(c->get_filename(), c);
}

ostream &ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream &ClassTable::semant_error()
{
    semant_errors++;
    return error_stream;
}

// start
// add class to classtable
void ClassTable::add_to_class_table(Class_ c)
{
    Symbol name = c->get_name();
    Symbol parent = c->get_parent(); // The name of the parent
    Features features = c->get_features();
    Symbol filename = c->get_filename();
    // first we check whether the inheritance is legal, since Bool, Str, SELF_TYPE cannot be inherited.
    if ((parent == Bool) || (parent == Str) || (parent == SELF_TYPE))
    { // Note that Str is a name while Str_class is a Class_
        semant_error(c) << name << "Cannot inherit from " << parent << "!" << endl;
    }
    else if (name == SELF_TYPE)
    {
        // we cannot define SELF_TYPE either.
        semant_error(c) << name << "Cannot define SELF_TYPE!" << endl;
    }
    else if ((class_table.find(name) != class_table.end()) || inherit_graph.find(name) != inherit_graph.end())
    {
        semant_error(c) << name << "Cannot be defined multiple times!" << endl;
    }
    else
    {
        // legal case
        class_table[name] = c;
        inherit_graph[name] = parent;
    }
}
/**
 * To check whether the class inheritance relationship is corrent
 * 1. the graph is an acyclic tree.
 * 2, the graph should contain the Main class
 * 3. all of the parent class should be defined.
*/
bool ClassTable::is_valid_inheritance()
{
    bool has_cycle = false;
    bool has_main = false;
    bool has_parent_defined = true;
    for (auto it = inherit_graph.begin(); it != inherit_graph.end(); ++it)
    {
        Symbol child = it->first;
        Symbol parent = it->second;
        if (parent == SELF_TYPE || child == SELF_TYPE)
        {
            semant_error() << "Class definition cannot contain SELF_TYPE!" << std::endl;
            return false;
        }
        if (child == Main)
        {
            has_main = true;
        }
        //check cycle with bruteforce method:
        while (parent != No_class)
        {
            if (parent == child)
            {
                // go up the inheritance tree changing parent, once parent == child, there is a cycle;
                semant_error(class_table[child]) << " Has inheritance cycle!" << endl;
                has_cycle = true;
                return false;
            }
            if (!inherit_graph.count(parent))
            {
                semant_error(class_table[child]) << " Doesn't contain parent!" << endl;
                return false;
            }
            parent = inherit_graph[parent]; // go up the inheritance tree
        }
    }
    if (!has_main)
    {
        semant_error() << " Class Main is not defined." << endl;
        return false;
    }
    return true;
}

bool ClassTable::has_class(Symbol name)
{
    return class_table.count(name) >= 1;
}

Class_ ClassTable::get_class(Symbol name)
{
    return class_table[name];
}

bool ClassTable::is_subclass(Symbol child, Symbol father, Env env)
{
    debugStream << "Calling is_subclass, checking " << child << "<=" << father << endl;
    if (child == SELF_TYPE && father == SELF_TYPE)
    {
        return true;
    }
    if (father == SELF_TYPE)
    {
        return false;
    }
    if (child == SELF_TYPE && father != SELF_TYPE)
    {
        debugStream << "change child to " << env.cur_class->get_name() << endl;
        child = env.cur_class->get_name();
    }

    debugStream << "child=" << child << ",father=" << father << endl;
    if (father == Object)
    {
        return true;
    }
    while (child != father && child != Object)
    {
        child = inherit_graph[child];
    }
    if (child == father)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/* Because of inheritance, a class' method may be originally defined in one of its ancestors,
    return the lowest ancestor with the given method directly defined.
    if self_included is set to true, return
 */
Symbol ClassTable::get_parent_with_method(Symbol child, Symbol name, bool self_included)
{
    Symbol parent = self_included ? child : inherit_graph[child];
    while (parent != No_class)
    {
        Class_ parent_class = class_table[parent];
        if (parent_class->get_method(name))
        {
            return parent_class->get_name();
        }
        parent = inherit_graph[parent];
    }
    return Symbol(NULL);
}

bool ClassTable::check_method_inheritance(Symbol s1, Symbol s2, Symbol name)
{
    Class_ c1 = class_table[s1];
    Class_ c2 = class_table[s2];
    Feature f1 = c1->get_method(name);
    Feature f2 = c2->get_method(name);
    Formals fo1 = f1->get_formals();
    Formals fo2 = f2->get_formals();
    Symbol rt1 = f1->get_return_type();
    Symbol rt2 = f2->get_return_type();

    if (rt1 != rt2)
    {
        return false;
    }
    int i = fo1->first();
    int j = fo2->first();
    while ((fo1->more(i)) && (fo2->more(j)))
    {
        if (fo1->nth(i)->get_type() != fo2->nth(j)->get_type())
        {
            return false;
        }
        i = fo1->next(i);
        j = fo2->next(j);
    }

    if (fo1->more(i) || fo2->more(j))
    {
        return false;
    }
    return true;
}

std::vector<Symbol> ClassTable::get_parents(Symbol s)
{
    std::vector<Symbol> ret;
    while (s != No_class)
    {
        ret.push_back(s);
        s = inherit_graph[s];
    }
    return ret;
}

Symbol ClassTable::lub(Symbol s1, Symbol s2, Env env)
{
    if (s1 == SELF_TYPE and s2 == SELF_TYPE)
    {
        return SELF_TYPE;
    }
    if (s1 == SELF_TYPE)
        s1 = env.cur_class->get_name();
    if (s2 == SELF_TYPE)
        s2 = env.cur_class->get_name();
    Symbol res = Object;
    std::vector<Symbol> v1 = get_parents(s1);
    std::vector<Symbol> v2 = get_parents(s2);
    int n1 = v1.size();
    int n2 = v2.size();
    int i = n1 - 1;
    int j = n2 - 1;
    while ((i >= 0) && (j >= 0) && (v1[i] == v2[j]))
    {
        res = v1[i];
        --i;
        --j;
    }

    return res;
}

Formals ClassTable::get_formals(Symbol classname, Symbol methodname)
{
    Symbol real_classname = get_parent_with_method(classname, methodname, true);
    if (!real_classname)
    {
        return NULL;
    }
    Class_ real_class = class_table[real_classname];
    Formals fo = real_class->get_method(methodname)->get_formals();
    return fo;
}

Symbol ClassTable::get_return_type(Symbol classname, Symbol methodname)
{
    Symbol real_classname = get_parent_with_method(classname, methodname, true);
    if (!real_classname)
    {
        return NULL;
    }
    Class_ real_class = class_table[real_classname];
    Symbol ret_type = real_class->get_method(methodname)->get_return_type();
    return ret_type;
}

// 判断par_types中元素是否为formals中对应元素的子类
bool ClassTable::check(Formals formals, const std::vector<Symbol> &par_types, Env env)
{
    int i = formals->first();
    int j = 0;
    int n = par_types.size();
    while (formals->more(i) && (j < n))
    {
        if (!is_subclass(par_types[j], formals->nth(i)->get_type(), env))
        {
            return false;
        }
        j++;
        i = formals->next(i);
    }
    if ((j < n) || (formals->more(i)))
    {
        return false;
    }
    return true;
}

/**
 * Operators beyond the ClassTable class
*/
// class__class
void class__class::init(Env env)
{

    if (name != Object)
    {
        env.ct->get_class(parent)->init(env);
    }
    debugStream << "Start checking class " << name << endl;
    /* Init the class method & attr(s) */
    for (int i = features->first(); features->more(i); i = features->next(i))
    {
        Feature f = features->nth(i);
        debugStream << "\tAdding feature " << f->get_name() << " to env\n";
        f->add_to_env(env);
    }
    debugStream << "End checking class " << name << endl;
}

bool class__class::has_method(Symbol name)
{
    for (int i = features->first(); features->more(i); i = features->next(i))
    {
        if ((features->nth(i)->is_method()) && (features->nth(i)->get_name() == name))
        {
            return true;
        }
    }
    return false;
}

Feature class__class::get_method(Symbol name)
{
    for (int i = features->first(); features->more(i); i = features->next(i))
    {
        if ((features->nth(i)->is_method()) && (features->nth(i)->get_name() == name))
        {
            return features->nth(i);
        }
    }
    return NULL;
}

// method_class
/**
 * In COOL, every function belongs to a class, therefore 
 * there's no such thing as "global functions". Consequently,
 * the information of methods is contained in the information 
 * of classes. Thus, methods' information is maintained by env.ct
 * (classtable), not env.om, making method_class::add_to_env an
 * empty function.
 * 
*/
void method_class::add_to_env(Env env) {}

void attr_class::add_to_env(Env env)
{
    if (env.om->probe(name) == NULL)
    {
        env.om->addid(name, &type_decl);
    }
    else
    {
        env.ct->semant_error(env.cur_class) << "Cannot have multiple definitions of the same attributes! " << endl;
    }
}

/**
 * Here goes the type_check part (Mainly for children of Expression_class)
*/

Class_ class__class::type_check(Env env)
{
    debugStream << "Start type-checking class " << name << endl;
    for (int i = features->first(); features->more(i); i = features->next(i))
    {
        debugStream << "\tStart type-checking feature " << features->nth(i)->get_name() << endl;
        features->nth(i)->type_check(env);
        debugStream << "\tEnd type-checking feature " << features->nth(i)->get_name() << endl;
    }
    debugStream << "End type-checking class " << name << endl;
    return this;
}

/**
 * 1. 进入新的作用域
 * 2. 添加当前类到环境
 * 3. 遍历变量
 * 4. 判断方法是是否继承正确
 * 5. 判断实际返回类型
 *    1. 如果返回类型是SELF_TYPE, 实际返回类型也为SELF_TYPE
 *    2. 返回类型存在
 *    3. 实际返回类型是声明类型的子类
 **/
Feature method_class::type_check(Env env)
{
    //enter a new scope
    env.om->enterscope();
    // adding current class, binding its name to self
    // I guess this step can be done either when entering a new class or
    // entering each method of the class. Here I choose the latter approach,
    // I will try the other way out later.(TODO)
    Symbol cur_class_name = env.cur_class->get_name(); //
    env.om->addid(self, &cur_class_name);
    //typechecking all of its parameters
    debugStream << "Starting checking formals" << endl;
    for (int i = formals->first(); formals->more(i); i = formals->next(i))
    {
        formals->nth(i)->type_check(env);
    }
    debugStream << "Checking Inheritance relationships" << endl;
    // Checking the inheritance relationship of the function
    Symbol parent_class = env.ct->get_parent_with_method(cur_class_name, name, false);
    if (parent_class != NULL)
    {
        // there exists a member function inheritance relationship
        if (!env.ct->check_method_inheritance(cur_class_name, parent_class, name))
        {
            env.ct->semant_error(env.cur_class) << " Method " << name << " inherits illegaly"
                                                << " from " << parent_class << endl;
        }
    }
    debugStream << "Checking return relationships expr=" << typeid(*expr).name() << endl;
    Symbol true_return_type = expr->type_check(env)->type;
    debugStream << "#Checking true_return _type is " << true_return_type<< " <-> " << return_type << endl;

    if (false == env.ct->is_subclass(true_return_type, return_type, env))
    {
        env.ct->semant_error(env.cur_class->get_filename(), this) << "Inferred return type " << true_return_type << " of method foo does not conform to declared return type " << return_type << "." << endl;
    }
    debugStream << "Checking method successful!" << endl;
    //exit a given scope
    env.om->exitscope();
    return this;
}

// method_class is so-fucking complicate, I shall figure this out first :-(

/**
 * 1. Enter a new scope
 * 2. Add the current class into this new scope
 * 3. Calculate the initial value
 *      3.1. A class attr must not be self
 *      3.2. If the return type is SELF_TYPE, then the type is Current Ckass
 *      3.3. 
*/
Feature attr_class::type_check(Env env)
{
    debugStream << "Start type-checking attr " << name << endl;

    env.om->enterscope();

    Symbol cur_class_name = env.cur_class->get_name();
    env.om->addid(self, &cur_class_name);

    Symbol true_return_type = init->type_check(env)->type;

    if (name == self)
    {
        env.ct->semant_error(env.cur_class->get_filename(), this) << "Attr shall not be self!" << endl;
        return this;
    }

    debugStream << "True return type is " << true_return_type << endl;
    if (true_return_type != No_type)
    {
        if (!(env.ct->is_subclass(true_return_type, type_decl, env)))
        {
            env.ct->semant_error(env.cur_class) << "True attr type isn't subcalss of type_decl!" << endl;
        }
    }

    env.om->exitscope();
    debugStream << "End type-checking attr " << name << endl;
    return this;
}

Formal formal_class::type_check(Env env)
{
    if (env.om->probe(name))
    {
        env.ct->semant_error(env.cur_class) << "Name already exist!" << endl;
    }
    else if (type_decl == SELF_TYPE)
    {
        env.ct->semant_error(env.cur_class) << "Type shouldn't be SELF_TYPE!" << endl;
    }
    else
    {
        env.om->addid(name, &type_decl);
    }
    return this;
}

/**
 * branch
 * ID : TYPE => expr
 * name : type_decl => expr
 **/
Symbol branch_class::type_check(Env env)
{
    bool is_shadow_name = false;
    if (env.om->probe(name))
    {
        env.om->enterscope();
        env.om->addid(name, &this->type_decl);
        is_shadow_name = true;
        // according to an example in pa5(shadow-let-case),case names can shadow variable names.
    }
    env.om->addid(name, &type_decl);

    auto ret = expr->type_check(env)->type;
    if (is_shadow_name)
    {
        env.om->exitscope();
    }
    return ret;
}

Expression assign_class::type_check(Env env)
{
    debugStream << "Enter assign_class" << name << typeid(*expr).name() << endl;
    if (name == self)
    {
        env.ct->semant_error(env.cur_class) << "Assign error: cannot assign anything to self." << endl;
        type = Object;
        return this;
    }
    Symbol expect_type = *env.om->lookup(name);
    debugStream << expect_type << endl;
    Symbol true_type = expr->type_check(env)->type;
    debugStream << expect_type << true_type << endl;
    if (env.ct->is_subclass(true_type, expect_type, env))
    {
        type = true_type;
    }
    else
    {
        env.ct->semant_error(env.cur_class) << "Assign error: cannot assign " << true_type << " to " << expect_type << endl;
        type = Object;
    }
    debugStream << "Checked assign statement , type= " << type << endl;
    return this;
}

/**
 * static_dispatch
 * expr@TYPEID.OBJECTID(actual)
 * expr@type_name.name(actual)
    Expression expr;
    Symbol type_name;
    Symbol name;
    Expressions actual;
 **/
Expression static_dispatch_class::type_check(Env env)
{
    Symbol t0 = expr->type_check(env)->type;
    debugStream << t0 << "###" << type_name << endl;
    Symbol cur_class = t0;
    if (t0 == SELF_TYPE)
    {
        cur_class = env.cur_class->get_name();
    }
    if (!env.ct->is_subclass(t0, type_name, env))
    {
        debugStream << "Survived checking!" << endl;
        env.ct->semant_error(env.cur_class) << "Wrong class!" << endl;
        type = Object;
        return this;
    }
    std::vector<Symbol> para_types; // types of parameters
    for (int i = actual->first(); actual->more(i); i = actual->next(i))
    {
        Symbol t = actual->nth(i)->type_check(env)->type;
        para_types.push_back(t);
    }
    Formals formals = env.ct->get_formals(cur_class, name);
    Symbol return_type = env.ct->get_return_type(cur_class, name);
    if ((formals == NULL) || (return_type == NULL))
    {
        env.ct->semant_error(env.cur_class) << "Method define wrong!" << endl;
        type = Object;
        return this;
    }

    if (!env.ct->check(formals, para_types, env))
    {
        env.ct->semant_error(env.cur_class) << "Parameters' type definition is wrong!" << endl;
        type = Object;
        return this;
    }

    if (return_type == SELF_TYPE)
    { // Tn+1=T0 if Tn+1' =SELF_TYPE
        type = t0;
        /*if T0==SELF_TYPE, then the type of this sta_dispatch is SELF_TYPE*/
    }
    else
    {
        type = return_type;
    }
    return this;
}

/**
 * dispatch
 * expr.name(actual)
 * 1. eval expr, 判断是否存在
 * 2. eval actual, 获得参数类型
 * 3. 判断方法是否定义有误
 * 4. 判断actual的类型是否匹配
 *    Expression expr;
   Symbol name;
   Expressions actual;
 **/
Expression dispatch_class::type_check(Env env)
{
    Symbol t0 = expr->type_check(env)->type;
    Symbol cur_class = t0;
    if (t0 == SELF_TYPE)
    {
        cur_class = env.cur_class->get_name();
    }
    std::vector<Symbol> para_types; // types of parameters
    for (int i = actual->first(); actual->more(i); i = actual->next(i))
    {
        Symbol t = actual->nth(i)->type_check(env)->type;
        para_types.push_back(t);
    }
    Formals formals = env.ct->get_formals(cur_class, name);
    Symbol return_type = env.ct->get_return_type(cur_class, name);
    if ((formals == NULL) || (return_type == NULL))
    {
        env.ct->semant_error(env.cur_class) << "Method define wrong!" << endl;
        type = Object;
        return this;
    }

    if (!env.ct->check(formals, para_types, env))
    {
        env.ct->semant_error(env.cur_class) << "Parameters' type definition is wrong!" << endl;
        type = Object;
        return this;
    }

    if (return_type == SELF_TYPE)
    { // Tn+1=T0 if Tn+1' =SELF_TYPE
        type = t0;
    }
    else
    {
        type = return_type;
    }
    debugStream << "In dispatch_class return type=" << type << endl;
    return this;
}

/**
 * cond
 * IF expr THEN expr ELSE expr FI
 * if pred then then_exp else else_exp fi
 **/
Expression cond_class::type_check(Env env)
{
    Symbol t0 = pred->type_check(env)->type;
    Symbol t1 = then_exp->type_check(env)->type;
    Symbol t2 = else_exp->type_check(env)->type;
    if (t0 != Bool)
    {
        env.ct->semant_error(env.cur_class) << "Pre type should be bool!" << endl;
        type = Object;
    }
    else
    {
        type = env.ct->lub(t1, t2, env);
    }

    return this;
}

/**
 * loop
 * while expr loop expr pool
 * while pred loop body pool
 **/
Expression loop_class::type_check(Env env)
{
    Symbol pred_type = pred->type_check(env)->type;
    Symbol body_type = body->type_check(env)->type;

    type = Object;
    if (pred_type != Bool)
    {
        env.ct->semant_error(env.cur_class) << " Predicate type should be bool!" << endl;
    }
    return this;
}

/**
 * case
 * case expr of [[ID : TYPE => expr; ]]+ esac
 * case expr of cases esac
 * 
 *    Expression expr;
 *    Cases cases;//contains a list of branch_class(es)
 * 
 * Note that the variables declared on each branch of a case must have distinct types(cool-manual page-21) 
 **/
Expression typcase_class::type_check(Env env)
{
    Symbol expr_type = this->expr->type_check(env)->type;
    debugStream << "In typcase_class expr_type=" << expr_type << endl;
    if (expr->type == No_type)
    {
        env.ct->semant_error(env.cur_class) << " Expr cannot be No_type! " << endl;
        type = Object;
        return this;
    }
    std::map<Symbol, int> type_map;
    for (int i = cases->first(); cases->more(i); i = cases->next(i))
    {
        if (++type_map[cases->nth(i)->get_type()] > 1)
        {
            env.ct->semant_error(env.cur_class) << "Case type should be all distinct!" << endl;
            type = Object;
            return this;
        }
    }
    //lub
    int i = cases->first();
    Symbol s = cases->nth(i)->type_check(env);
    debugStream << "branch type = "
                << "\t$#$" << s << endl;
    for (; cases->more(i); i = cases->next(i))
    {
        env.om->enterscope();
        // O[T1/x1] consists of two parts:entering and exiting a new scope, type_checking and getting T1', the true_return type
        // Here the first job is done by typcase_class::type_check, the latter one is left to branch_class
        Symbol s1 = cases->nth(i)->type_check(env);
        debugStream << "branch type = " << s1 << "\t$#$" << s << endl;
        s = env.ct->lub(s, s1, env);
        env.om->exitscope();
    }
    type = s;
    return this;
}

/**
 * block class
 * [[expr;]]+
 * body
 **/
Expression block_class::type_check(Env env)
{
    Symbol s = nullptr;
    for (int i = body->first(); body->more(i); i = body->next(i))
    {
        s = body->nth(i)->type_check(env)->type;
    }
    type = s;
    return this;
}

/**
 * let
 * let ID : TYPE [ <- expr ] [[; ID : TYPE [ <- expr ]]]* in expr
 * let identifier : type_decl <- init in body
 **/
Expression let_class::type_check(Env env)
{
    debugStream << "Typechecking LET" << endl;
    if (this->identifier == self)
    {
        env.ct->semant_error(env.cur_class) << " Cannot bound expressions to self in a let statement." << endl;
        type = Object;
        return this;
    }
    Symbol declare_type = this->type_decl;
    Symbol true_type = init->type_check(env)->type;
    debugStream << true_type << "#" << declare_type << endl;
    if (true_type == No_type)
    {
        env.om->enterscope();

        env.om->addid(identifier, &declare_type);
        Symbol body_type = body->type_check(env)->type;
        type = body_type;

        env.om->exitscope();
    }
    else
    {
        if (!env.ct->is_subclass(true_type, declare_type, env))
        {
            env.ct->semant_error(env.cur_class) << " True return type is not a subclass of the explicitly declared type! " << endl;
            type = Object;
        }
        else
        {
            env.om->enterscope();

            env.om->addid(identifier, &declare_type);
            Symbol body_type = body->type_check(env)->type;
            type = body_type;
            env.om->exitscope();
        }
    }
    return this;
}

Expression plus_class::type_check(Env env)
{
    Symbol s1 = e1->type_check(env)->type;
    Symbol s2 = e2->type_check(env)->type;
    if ((s1 == Int) && (s2 == Int))
    {
        type = Int;
    }
    else
    {
        env.ct->semant_error(env.cur_class) << "Type should both be int!" << endl;
        type = Object;
    }

    return this;
}

Expression sub_class::type_check(Env env)
{
    Symbol s1 = e1->type_check(env)->type;
    Symbol s2 = e2->type_check(env)->type;
    if ((s1 == Int) && (s2 == Int))
    {
        type = Int;
    }
    else
    {
        env.ct->semant_error(env.cur_class) << "Type should both be int!" << endl;
        type = Object;
    }

    return this;
}

Expression mul_class::type_check(Env env)
{
    Symbol s1 = e1->type_check(env)->type;
    Symbol s2 = e2->type_check(env)->type;
    if ((s1 == Int) && (s2 == Int))
    {
        type = Int;
    }
    else
    {
        env.ct->semant_error(env.cur_class) << "Type should both be int!" << endl;
        type = Object;
    }

    return this;
}

Expression divide_class::type_check(Env env)
{
    Symbol s1 = e1->type_check(env)->type;
    Symbol s2 = e2->type_check(env)->type;
    if ((s1 == Int) && (s2 == Int))
    {
        type = Int;
    }
    else
    {
        env.ct->semant_error(env.cur_class) << "Type should both be int!" << endl;
        type = Object;
    }

    return this;
}

Expression neg_class::type_check(Env env)
{
    Symbol s1 = e1->type_check(env)->type;
    if (s1 == Int)
    {
        type = Int;
    }
    else
    {
        env.ct->semant_error(env.cur_class) << "Type should be int!"
                                            << " " << s1 << endl;
        type = Object;
    }

    return this;
}

Expression lt_class::type_check(Env env)
{
    Symbol s1 = e1->type_check(env)->type;
    Symbol s2 = e2->type_check(env)->type;
    if ((s1 == Int) && (s2 == Int))
    {
        type = Bool;
    }
    else
    {
        env.ct->semant_error(env.cur_class) << "Type should both be int!" << endl;
        type = Object;
    }

    return this;
}

Expression eq_class::type_check(Env env)
{
    Symbol s1 = e1->type_check(env)->type;
    Symbol s2 = e2->type_check(env)->type;
    if ((s1 == Int && s2 != Int) || (s1 != Int && s2 == Int) ||
        (s1 == Str && s2 != Str) || (s1 != Str && s2 == Str) ||
        (s1 == Bool && s2 != Bool) || (s1 != Bool && s2 == Bool))
    {
        env.ct->semant_error(env.cur_class) << "Wrong type!"
                                            << "Cannot compare " << s1 << " and " << s2 << endl;
        type = Object;
    }
    else
    {
        type = Bool;
    }

    return this;
}

Expression leq_class::type_check(Env env)
{
    Symbol s1 = e1->type_check(env)->type;
    Symbol s2 = e2->type_check(env)->type;
    if (((s1 == Int) && (s2 == Int)) ||
        ((s1 == Str) && (s2 == Str)) ||
        ((s1 == Bool) && (s2 == Bool)))
    {
        type = Bool;
    }
    else
    {
        env.ct->semant_error(env.cur_class) << "Wrong type!" << endl;
        type = Object;
    }

    return this;
}

Expression comp_class::type_check(Env env)
{
    Symbol s1 = e1->type_check(env)->type;
    if (s1 == Bool)
    {
        type = Bool;
    }
    else
    {
        env.ct->semant_error(env.cur_class) << "Type should be bool!" << endl;
        type = Object;
    }

    return this;
}

Expression int_const_class::type_check(Env env)
{
    type = Int;
    return this;
}
Expression bool_const_class::type_check(Env env)
{
    type = Bool;
    return this;
}
Expression string_const_class::type_check(Env env)
{
    type = Str;
    return this;
}

Expression new__class::type_check(Env env)
{
    Symbol s = type_name;
    if (s == SELF_TYPE)
    {
        type = s;
    }
    else if (env.ct->has_class(s))
    {
        type = s;
    }
    else
    {
        env.ct->semant_error(env.cur_class) << " Class " << type_name << " is not defined!" << endl;
        type = Object;
    }
    return this;
}

Expression isvoid_class::type_check(Env env)
{
    Symbol s = e1->type_check(env)->type;
    if (!env.ct->has_class(s))
    {
        env.ct->semant_error(env.cur_class) << "Doesn't have class!" << endl;
        type = Object;
    }
    type = Bool;
    return this;
}

Expression no_expr_class::type_check(Env env)
{
    type = No_type;
    return this;
}

Expression object_class::type_check(Env env)
{
    if (name == self)
    {
        type = SELF_TYPE;
    }
    else if (env.om->lookup(name))
    {
        type = *(env.om->lookup(name));
    }
    else
    {
        env.ct->semant_error(env.cur_class) << "Doesn't have class!" << endl;
        type = Object;
    }
    return this;
}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    ClassTable *classtable = new ClassTable(classes);
    if (classtable->errors())
    {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }
    /* some semantic analysis code may go here */
    bool is_inherit_valid = classtable->is_valid_inheritance();
    if (is_inherit_valid)
    {
        debugStream << "Start checking!" << endl;
        Env env = Env(classtable);
        for (int i = classes->first(); classes->more(i); i = classes->next(i))
        {
            /* Each class is an relatively indepedent environment */
            /* First we need tp enter the scope */
            env.om->enterscope();
            env.cur_class = classes->nth(i);
            classes->nth(i)->init(env);
            classes->nth(i)->type_check(env);
            /* In the last donot forget to leave the scope */
            env.om->exitscope();
        }
    }

    if (classtable->errors())
    {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }
}
