#ifndef LIBZ80AW_VIEW_HH
#define LIBZ80AW_VIEW_HH

#include "z80state.hh"

class View {
protected:
    explicit View(Z80State const& z80_state) : z80_state_(z80_state) {}
    
    Z80State z80_state_;
};

#endif
