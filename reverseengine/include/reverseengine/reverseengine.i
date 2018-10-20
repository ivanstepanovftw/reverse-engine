%module reverseengine_swig

// Add necessary symbols to generated header
%{
#include <reverseengine/shape.hh>
%}

// Process symbols in header
%include "shape.hh"
