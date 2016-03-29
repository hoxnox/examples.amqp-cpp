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
        std::cout << "Usage: log_level..." << std::endl
                  << "log level examples: info, warning, error" << std::endl;
        return 0;
    }
    ConnHandler handler;
    AMQP::TcpConnection connection(handler,
            AMQP::Address("localhost", 5672,
                          AMQP::Login("guest", "guest"), "/"));
    AMQP::TcpChannel channel(&connection);
    channel.declareExchange("direct_logs", AMQP::direct);
    channel.declareQueue("logs", AMQP::exclusive);
    std::for_each (argv + 1, argv + argc,
        [&](const char* log_level)
        {
            channel.bindQueue("direct_logs", "", log_level);
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

