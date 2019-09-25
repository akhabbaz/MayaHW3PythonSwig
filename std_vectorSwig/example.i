/* File : example.i */
%module example
%{
#include "example.h"
%}
%include stl.i
/* instantiate the required template specializations */
namespace std {
    %template(IntVector)    vector<int>;
    %template(DoubleVector) vector<double>;
    %template(VecVector)    vector<vector<double>>;
}
/* Let's just grab the original header file here */
%apply std::vector<double>& INOUT{ std::vector<double>& };
%apply std::vector<std::vector<double>>& INOUT { std::vector<std::vector<double>>&};
%include "example.h"
