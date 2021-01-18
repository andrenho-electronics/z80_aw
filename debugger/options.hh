#ifndef DEBUGGER_OPTIONS_HH
#define DEBUGGER_OPTIONS_HH

class Options {
public:
    Options(int argc, char* argv[]);
    
    bool show_demo_window() const { return show_demo_window_; }

private:
    bool show_demo_window_ = false;
    
    void show_help(const char* program);
};

#endif
