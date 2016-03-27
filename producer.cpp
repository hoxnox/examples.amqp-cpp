#include <iostream>
#include <event2/event.h>
#include <amqpcpp.h>
#include <amqpcpp/libevent.h>

class ConnHandler : public AMQP::LibEventHandler
{
public:
    ConnHandler(struct event_base* ev) : AMQP::LibEventHandler(ev) {}
    virtual void onError(AMQP::Connection *connection, const char *message)
    {
        // @todo
        //  add your own implementation, for example by reporting the error
        //  to the user of your program, log the error, and destruct the
        //  connection object because it is no longer in a usable state
        std::cout << message << std::endl;
    }
};

int main(void)
{
    auto evbase = event_base_new();
    ConnHandler handler(evbase);

    AMQP::TcpConnection connection(&handler, AMQP::Address("amqp://localhost/"));
    AMQP::TcpChannel channel(&connection);
    channel.declareQueue(AMQP::exclusive).onSuccess(
        [&connection](const std::string &name,
                      uint32_t messagecount,
                      uint32_t consumercount)
        {
            std::cout << "declared queue " << name << std::endl;
            connection.close();
        }
    );

    event_base_dispatch(evbase);
    event_base_free(evbase);

    return 0;
}

