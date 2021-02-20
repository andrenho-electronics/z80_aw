#include "realhardware.hh"

RealHardware::RealHardware(std::string const& serial_port)
    : Z80AW(), serial_(serial_port.c_str())
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
}

RealHardware::~RealHardware()
{
    google::protobuf::ShutdownProtobufLibrary();
}

uint16_t
RealHardware::free_mem() const
{
    Request req;
    req.set_type(Request::FREE_MEM);
    serial_.request(req);
    return 0;
}
