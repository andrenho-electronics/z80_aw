#include "z80aw.hh"

Z80AW::Z80AW(const char* port)
    : serial_(port)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
}

Z80AW::~Z80AW()
{
    google::protobuf::ShutdownProtobufLibrary();
}

uint16_t
Z80AW::free_mem() const
{
    Request req;
    req.set_type(Request::FREE_MEM);
    serial_.request(req);
}
