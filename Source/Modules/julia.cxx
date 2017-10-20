#include "swigmod.h"
#include "ctype.h"

static const char *usage = (char *) "\
Julia Options (available with -julia)\n\
     -package <name>    - Package name. Default is the module name.\n\
";

class JULIA : public Language {

protected:
  String *f_begin;
  String *f_runtime;
  String *f_header;
  String *f_wrappers;
  String *f_init;
  String *f_interface;

  void dumpCode(Node *n);
  void printArguments(Wrapper *wrapper, Node *n);
  void printFunctionAction(Wrapper *wrapper, Node *n, String *fname, SwigType *rettype);
  void printVariableAction(Wrapper *wrapper, Node *n, String *fname, SwigType *rettype, bool is_set);

public:
  JULIA();
  ~JULIA();
  virtual void main(int argc, char *argv[]);
  virtual int top(Node *n);
  virtual int functionWrapper(Node *n);

private:
  String *package_name;
  String *module_name;
  bool debug_mode;
};

JULIA::JULIA() :
  package_name(NULL),
  module_name(NULL),
  debug_mode(false) {
}

JULIA::~JULIA() {
  if (!package_name) {
    Delete(package_name);
  }
  if (!module_name) {
    Delete(module_name);
  }
}

void JULIA::main(int argc, char *argv[]) {

  // Options.
  for (int i = 1; i < argc; i++) {
    if (argv[i]) {
      if (strcmp(argv[i], "-package") == 0) {
        if (i+1 < argc && argv[i+1]) {
          Swig_mark_arg(i);
          package_name = NewString(argv[++i]);
          Swig_mark_arg(i);
        } else {
          Swig_arg_error();
        }
      } else if (strcmp(argv[i], "-debug") == 0) {
        debug_mode = true;
        Swig_mark_arg(i);
      } else if (strcmp(argv[i], "-help") == 0) {
        fputs(usage, stdout);
      }
    }
  }

  // Configuration and preprocessing.
  SWIG_library_directory("julia");
  Preprocessor_define("SWIGJULIA 1", 0);
  SWIG_config_file("julia.swg");
  SWIG_typemap_lang("julia");
  allow_overloading();
}

int JULIA::top(Node *n) {
  String *module = Getattr(n, "name");
  if (!package_name) {
    package_name = Copy(module);
  }
  module_name = Copy(package_name);
  char c = *Char(module_name);
  if (islower(c)) {
    char l[2] = {c, '\0'};
    char u[2] = {(char) toupper(c), '\0'};
    Replace(module_name, l, u, DOH_REPLACE_FIRST);
  } else if (!isalpha(c)) {
    char l[2] = {c, '\0'};
    char u[3] = {'X', c, '\0'};
    Replace(module_name, l, u, DOH_REPLACE_FIRST);
  }

  f_begin = NewString("");
  f_runtime = NewString("");
  f_init = NewString("");
  f_header = NewString("");
  f_wrappers = NewString("");
  f_interface = NewString("");

  Swig_register_filebyname("begin", f_begin);
  Swig_register_filebyname("runtime", f_runtime);
  Swig_register_filebyname("init", f_init);
  Swig_register_filebyname("header", f_header);
  Swig_register_filebyname("wrapper", f_wrappers);
  Swig_register_filebyname("interface", f_interface);

  Swig_banner(f_begin);

  Printf(f_interface, "module %s\n", module_name);
  Printf(f_interface, "const _swiglib = \"./%s.so\"\n", package_name);
 
  Printf(f_wrappers, "#ifdef __cplusplus\n");
  Printf(f_wrappers, "extern \"C\" {\n");
  Printf(f_wrappers, "#endif\n\n");

  Language::top(n);

  Printf(f_wrappers, "#ifdef __cplusplus\n");
  Printf(f_wrappers, "}\n");
  Printf(f_wrappers, "#endif\n");

  Printf(f_interface, "end\n", package_name);

  dumpCode(n);

  Delete(f_interface);
  Delete(f_wrappers);
  Delete(f_header);
  Delete(f_init);
  Delete(f_runtime);
  Delete(f_begin);

  return SWIG_OK;
}

void JULIA::dumpCode(Node *n) {
  // Julia interface file.
  String *interface_filename = NewString("");
  Printf(interface_filename, "%s%s.jl", SWIG_output_directory(), package_name);
  File *interface = NewFile(interface_filename, "w", SWIG_output_files());
  if (!interface) {
    FileErrorDisplay(interface_filename);
    SWIG_exit(EXIT_FAILURE);
  }

 Dump(f_interface, interface);

  Delete(interface);
  Delete(interface_filename);

  // C++ wrapper file.
  String *outfile = Getattr(n, "outfile");
  File *runtime = NewFile(outfile, "w", SWIG_output_files());
  if (!runtime) {
    FileErrorDisplay(outfile);
    SWIG_exit(EXIT_FAILURE);
  }
  Dump(f_begin, runtime);
  Dump(f_runtime, runtime);
  Dump(f_header, runtime);
  Dump(f_wrappers, runtime);
  Wrapper_pretty_print(f_init, runtime);

  Delete(runtime);
  Delete(outfile);
}

void JULIA::printArguments(Wrapper *wrapper, Node *n) {
  ParmList *parms = Getattr(n, "parms");
  Swig_typemap_attach_parms("juliatype", parms, wrapper);
  Parm *p = parms;
  bool is_first = true;
  while (p) {
    SwigType *juliatype = Getattr(p, "tmap:juliatype");
    SwigType *lname = Copy(Getattr(p, "lname"));
    if (!lname) {
      lname = Copy(Getattr(p, "name"));
      Append(lname, "_");
    }
    Printv(wrapper->def, is_first ? "" : ", ", lname, "::", juliatype, NIL);
    p = nextSibling(p);
    is_first = false;
    Delete(lname);
  }
}

void JULIA::printFunctionAction(Wrapper *wrapper, Node *n, String *fname, SwigType *rettype) {
  Printf(wrapper->code, "ccall((:%s, _swiglib), %s, (", fname, rettype);
  ParmList *parms = Getattr(n, "parms");
  Parm *p = parms;
  bool is_first = true;
  while (p) {
    SwigType *juliatype = Getattr(p, "tmap:juliatype");
    Printv(wrapper->code, is_first ? "" : ", ", juliatype, NIL);
    p = nextSibling(p);
    is_first = false;
  }
  Printf(wrapper->code, "), ");
  p = parms;
  is_first = true;
  while (p) {
    Printv(wrapper->code, is_first ? "" : ", ", Getattr(p, "lname"), NIL);
    p = nextSibling(p);
    is_first = false;
  }
  Printf(wrapper->code, ")::%s\n", rettype);
}

void JULIA::printVariableAction(Wrapper *wrapper, Node *n, String *fname, SwigType *rettype, bool is_set) {
  if (is_set) {
    rettype = Getattr(Getattr(n, "parms"), "tmap:juliatype");
  }
  Printf(wrapper->code, "ptr = cglobal((:%s, _swiglib), %s)\n", fname, rettype);
  if (is_set) {
    String *argname = Copy(Getattr(Getattr(n, "parms"), "name"));
    Append(argname, "_");
    Printf(wrapper->code, "unsafe_store!(ptr, %s)\n", argname);
    Delete(argname);
  }
  Printf(wrapper->code, "unsafe_load(ptr)::%s\n", rettype);
}

int JULIA::functionWrapper(Node *n) {
  String *fname = Getattr(n, "name");
  String *iname = Getattr(n, "sym:name");
  SwigType *rettype = Swig_typemap_lookup("juliatype", n, "", NULL);
  if (debug_mode) {
    Printf(stdout, 
	   "<functionWrapper> %s %s %s\n", fname, iname, rettype);
  }
  // Define a wrapper for Julia code.
  // There is no need for C wrappers now.
  Wrapper *wrapper = NewWrapper();
  String *kind = Getattr(n, "kind");
  bool is_function = Strcmp(kind, "function") == 0;
  bool is_set = false;
  if (!is_function) {
    is_set = Strcmp(Char(iname) + Len(iname) - 4, "_set") == 0;
    assert(is_set || Strcmp(Char(iname) + Len(iname) - 4, "_get") == 0);
  }

  Printf(wrapper->def, "# Start of %s\n", iname);
  Printv(wrapper->def, "function ", fname, is_set ? "!" : "", "(", NIL);
  printArguments(wrapper, n);
  Printf(wrapper->def, ")\n");
  if (is_function) {
    printFunctionAction(wrapper, n, fname, rettype);
  } else {
    printVariableAction(wrapper, n, fname, rettype, is_set);
  }

  Printf(wrapper->code, "end\n");
  
  Wrapper_print(wrapper, f_interface);

  Delete(rettype);
  DelWrapper(wrapper);

  return SWIG_OK;
}

extern "C" Language *
swig_julia(void) {
  return new JULIA();
}
