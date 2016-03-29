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
    const std::string msg =
            argc > 1 ? join(&argv[1], &argv[argc], " ") : "Hello World!";

    auto evbase = event_base_new();
    LibEventHandlerMyError handler(evbase);
    AMQP::TcpConnection connection(&handler, 
            AMQP::Address("localhost", 5672,
                          AMQP::Login("guest", "guest"), "/"));
    AMQP::TcpChannel channel(&connection);
    channel.declareQueue("task_queue", AMQP::passive | AMQP::durable)
        .onSuccess
        (
            [msg, &channel, &evbase](const std::string &name,
                                     uint32_t messagecount,
                                     uint32_t consumercount)
            {
                AMQP::Envelope env(msg);
                env.setDeliveryMode(2);
                channel.publish("", "task_queue", env);
                std::cout << "Sent '" << msg << "'" << std::endl;
                event_base_loopbreak(evbase);
            }
        );
    event_base_dispatch(evbase);
    event_base_free(evbase);
    return 0;
}


