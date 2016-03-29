///
///
///
///
///
///

#ifndef ____Equation__
#define ____Equation__

template < typename InputType, typename OutputType >
class Equation {
public:
    virtual OutputType eval(InputType val) = 0;
    virtual OutputType operator()(InputType val) {return eval(val);}
};

#endif
