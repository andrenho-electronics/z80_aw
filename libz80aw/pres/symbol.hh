#ifndef LIBZ80AW_SYMBOL_HH
#define LIBZ80AW_SYMBOL_HH

#include <string>

struct Symbol {
    std::string symbol;
    std::string file;
    size_t      line;
    uint16_t    addr;
    
    Symbol(std::string const& symbol, std::string const& file, size_t const& line, uint16_t const& addr)
        : symbol(symbol), file(file), line(line), addr(addr) {}
    
    bool operator==(Symbol const& rhs) const
    {
        return symbol == rhs.symbol &&
               file == rhs.file &&
               line == rhs.line &&
               addr == rhs.addr;
    }
    
    bool operator!=(Symbol const& rhs) const
    {
        return !(rhs == *this);
    }
};

#endif