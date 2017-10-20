/* -----------------------------------------------------------------------------
 * julia.swg
 *
 * Main library file for the Julia language module. See the Julia chapter in the
 * SWIG manual for explanation on the typemaps, pragmas, etc. used.
 * ----------------------------------------------------------------------------- */

%typemap(juliatype) bool,               const bool &               "Bool"
%typemap(juliatype) char,               const char &               "Cchar"
%typemap(juliatype) signed char,        const signed char &        "Cchar"
%typemap(juliatype) unsigned char,      const unsigned char &      "Cuchar"
%typemap(juliatype) short,              const short &              "Cshort"
%typemap(juliatype) unsigned short,     const unsigned short &     "Cushort"
%typemap(juliatype) int,                const int &                "Cint"
%typemap(juliatype) unsigned int,       const unsigned int &       "Cuint"
%typemap(juliatype) long,               const long &               "Clong"
%typemap(juliatype) unsigned long,      const unsigned long &      "Culong"
%typemap(juliatype) long long,          const long long &          "Clonglong"
%typemap(juliatype) unsigned long long, const unsigned long long & "Culonglong"
%typemap(juliatype) float,              const float &              "Cfloat"
%typemap(juliatype) double,             const double &             "Cdouble"
