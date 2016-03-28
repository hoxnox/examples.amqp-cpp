#include <iostream>
#include <event2/event.h>
#include <amqpcpp.h>
#include <amqpcpp/libevent.h>

#ifndef _
#define _(X) X
#endif

class ConnHandler : public AMQP::LibEventHandler
{
public:
    ConnHandler(struct event_base* ev) : AMQP::LibEventHandler(ev) {}
    virtual void onError(AMQP::Connection *connection, const char *message)
    {
        std::cout << message << std::endl;
    }
};

int
main(void)
{
    auto evbase = event_base_new();
    ConnHandler handler(evbase);

    AMQP::TcpConnection connection(&handler, AMQP::Address("amqp://localhost/"));
    AMQP::TcpChannel channel(&connection);
    channel.declareQueue("hello", AMQP::autodelete)
        .onSuccess
        (
            [&connection](const std::string &name,
                          uint32_t messagecount,
                          uint32_t consumercount)
            {
                std::cout << "Created queue: " << name << std::endl;
            }
        );
    channel.consume("hello", AMQP::noack)
        .onReceived
        (
            [](const AMQP::Message &msg, uint64_t tag, bool redelivered)
            {
                std::cout << "Received: " << msg.message() << std::endl;
            }
        );
    std::cout << "Waiting for messages. To exit press CTRL-C." << std::endl;

    event_base_dispatch(evbase);
    event_base_free(evbase);

    return 0;
}


