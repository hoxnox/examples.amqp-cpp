#include <amqpcpp.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include "conn_handler.h"

template<class InIter> inline std::string
join(InIter begin, InIter end, std::string delim)
{
    std::stringstream ss;
    std::copy(begin, end,
            std::ostream_iterator<const char*>(ss, delim.c_str()));
    return ss.str();
}

int
main(int argc, const char* argv[])
{
    const std::string log_level = argc > 2 ? argv[1] : "info";
    const std::string msg =
            argc > 2 ? join(&argv[2], &argv[argc], " ") : "Hello World!";

    auto evbase = event_base_new();
    LibEventHandlerMyError handler(evbase);
    AMQP::TcpConnection connection(&handler, 
            AMQP::Address("localhost", 5672,
                          AMQP::Login("guest", "guest"), "/"));
    AMQP::TcpChannel channel(&connection);
    channel.declareExchange("direct_logs", AMQP::direct)
        .onSuccess
        (
            [&]()
            {
                channel.publish("direct_logs", log_level, msg);
                std::cout << "Sent " << log_level << ": '"
                          << msg << "'" << std::endl;
                event_base_loopbreak(evbase);
            }
        );
    event_base_dispatch(evbase);
    event_base_free(evbase);
    return 0;
}



