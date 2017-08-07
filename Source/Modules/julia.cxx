#include "swigmod.h"

static const char *usage = (char *) "\
Julia Options (available with -julia)\n\
     -pkg <name>    - Generate a Julia package and set its name as <name>.\n\
";

class JULIA : public Language {

protected:
  File *f_begin;
  File *f_runtime;
  File *f_header;
  File *f_wrappers;
  File *f_init;

public:

  JULIA() : package_name(NULL) { }
  ~JULIA() {
    if (package_name != NULL) {
      Delete(package_name);
    }
  }
  virtual void main(int argc, char *argv[]);
  virtual int top(Node *n);
  virtual int functionWrapper(Node *n);

private:
  String *package_name;

};

void JULIA::main(int argc, char *argv[]) {

  // Options.
  for (int i = 1; i < argc; i++) {
    if (argv[i]) {
      if (strcmp(argv[i], "-pkg") == 0) {
        if (i+1 < argc && argv[i+1]) {
          package_name = NewString(argv[i+1]);
          Swig_mark_arg(i);
          Swig_mark_arg(i+1);
        } else {
          Swig_arg_error();
        }
      } else if (strcmp(argv[i], "-help") == 0) {
        fputs(usage, stdout);
      }
    }
  }

  // Configuration and preprocessing
  SWIG_library_directory("julia");
  Preprocessor_define("SWIGJULIA 1", 0);
  SWIG_config_file("julia.swg");
  SWIG_typemap_lang("julia");
  allow_overloading();
}

int JULIA::top(Node *n) {
  String *module = Getattr(n, "name");
  String *outfile = Getattr(n, "outfile");

  f_begin = NewFile(outfile, "w", SWIG_output_files());
  if (!f_begin) {
    FileErrorDisplay(outfile);
    SWIG_exit(EXIT_FAILURE);
  }
  f_runtime = NewString("");
  f_init = NewString("");
  f_header= NewString("");
  f_wrappers= NewString("");

  Swig_register_filebyname("begin", f_begin);
  Swig_register_filebyname("header", f_header);
  Swig_register_filebyname("wrapper", f_wrappers);
  Swig_register_filebyname("runtime", f_runtime);
  Swig_register_filebyname("init", f_init);

  Swig_banner(f_begin);

  Language::top(n);

  Dump(f_runtime, f_begin);
  Dump(f_header, f_begin);
  Dump(f_wrappers, f_begin);
  Wrapper_pretty_print(f_init, f_begin);

  Delete(f_runtime);
  Delete(f_header);
  Delete(f_wrappers);
  Delete(f_init);
  Delete(f_begin);

  return SWIG_OK;
}

int JULIA::functionWrapper(Node *n) {
}

extern "C" Language *
swig_julia(void) {
  return new JULIA();
}
