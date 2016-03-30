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
    const std::string routing_key = argc > 2 ? argv[1] : "anonymous.info";
    const std::string msg =
            argc > 2 ? join(&argv[2], &argv[argc], " ") : "Hello World!";

    auto evbase = event_base_new();
    LibEventHandlerMyError handler(evbase);
    AMQP::TcpConnection connection(&handler, 
            AMQP::Address("localhost", 5672,
                          AMQP::Login("guest", "guest"), "/"));
    AMQP::TcpChannel channel(&connection);
    channel.onError([&evbase](const char* message)
        {
            std::cout << "Channel error: " << message << std::endl;
            event_base_loopbreak(evbase);
        });
    channel.declareExchange("topic_logs", AMQP::topic)
        .onError([&](const char* msg)
        {
            std::cout << "ERROR: " << msg << std::endl;
        })
        .onSuccess
        (
            [&]()
            {
                channel.publish("topic_logs", routing_key, msg);
                std::cout << "Sent " << routing_key << ": '"
                          << msg << "'" << std::endl;
                event_base_loopbreak(evbase);
            }
        );
    event_base_dispatch(evbase);
    event_base_free(evbase);
    return 0;
}

