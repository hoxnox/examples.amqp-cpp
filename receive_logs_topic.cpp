#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <amqpcpp.h>
#include "conn_handler.h"

int
main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: <bind key>..." << std::endl;
        return 0;
    }
    ConnHandler handler;
    AMQP::TcpConnection connection(handler,
            AMQP::Address("localhost", 5672,
                          AMQP::Login("guest", "guest"), "/"));
    AMQP::TcpChannel channel(&connection);
    channel.onError([&handler](const char* message)
        {
            std::cout << "Channel error: " << message << std::endl;
            handler.Stop();
        });
    channel.declareExchange("topic_logs", AMQP::topic);
    channel.declareQueue("logs", AMQP::exclusive);
    std::for_each (argv + 1, argv + argc,
        [&](const char* bind_key)
        {
            std::cout << bind_key << std::endl;
            channel.bindQueue("topic_logs", "logs", bind_key);
            channel.consume("logs", AMQP::noack)
                .onReceived
                (
                    [](const AMQP::Message& m, uint64_t, bool)
                    {
                        std::cout << m.routingKey() << ": "
                                  << m.message() << std::endl;
                    }
                );
        }
    );
    handler.Start();
    std::cout << "Closing connection." << std::endl;
    connection.close();
    return 0;
}


