#ifndef EMULATOR_HARDWARE_HH
#define EMULATOR_HARDWARE_HH

#include <cstdint>
#include <memory>
#include <unordered_set>
#include <vector>
#include <functional>

#define CHECKSUM_ADDR 0x7ffe

struct Registers {
    uint16_t AF, BC, DE, HL, AFx, BCx, DEx, HLx, IX, IY, PC, SP;
    uint8_t R, I;
    bool HALT;
};

class Hardware {
public:
    virtual ~Hardware() = default;
    virtual void                 set_memory(uint16_t addr, uint8_t data) = 0;
    virtual uint8_t              get_memory(uint16_t addr) const = 0;
    virtual std::vector<uint8_t> get_memory(uint16_t addr, uint16_t sz) const = 0;
    
    virtual void update_registers() = 0;

    virtual void reset() = 0;

    bool next_is_subroutine() const;

    virtual void add_breakpoint(uint16_t addr);
    virtual void remove_breakpoint(uint16_t addr);
    void add_breakpoint_next();
    bool is_breakpoint(uint16_t addr) const;

    virtual void step() = 0;
    
    void add_to_upload_staging(std::vector<uint8_t> const& data, uint16_t addr);
    bool matching_upload_checksum() const;
    virtual void upload(std::function<void(double)> on_progress=[](double){}) = 0;
    
    Registers const& registers() const { return registers_; }
    
    virtual void register_keypress(uint8_t key) = 0;
    
    void print_char(uint8_t chr);
    void set_on_print_char(struct Terminal* terminal, std::function<void(struct Terminal&, uint8_t)> const& on_print_char);
    
    virtual void start_running() = 0;
    virtual void stop_running() = 0;
    virtual void evaluate_events() = 0;

protected:
    Hardware() = default;
    
    std::unordered_set<uint16_t> breakpoints_;
    
    struct UploadStagingArea {
        std::vector<uint8_t> data;
        uint16_t addr;
    };
    std::vector<UploadStagingArea> upload_staging_areas_;
    uint16_t                       upload_staging_checksum_ = 0;
    Registers                      registers_ {};
    
    struct Terminal* terminal_ = nullptr;
    std::function<void(struct Terminal&, uint8_t)> on_print_char_ = nullptr;
};

extern std::unique_ptr<Hardware> hardware;

#endif //EMULATOR_HARDWARE_HH
