#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <amqpcpp.h>
#include "conn_handler.h"

int fib(int n)
{
    switch (n)
    {
    case 0:
        return 0;
    case 1:
        return 1;
    default:
        return fib(n - 1) + fib(n - 2);
    }
}

int
main(int argc, char* argv[])
{
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
    channel.setQos(1);
    channel.declareQueue("rpc_queue");
    channel.consume("")
        .onReceived
        (
            [&channel](const AMQP::Message& m, uint64_t tag, bool)
            {
                std::cout << "fib(" << m.message() << ")" << std::endl;

                AMQP::Envelope env(std::to_string(fib(std::stoi(m.message()))));
                env.setCorrelationID(m.correlationID());

                channel.publish("", m.replyTo(), env);
                channel.ack(tag);
            }
        );
    handler.Start();
    std::cout << "Closing connection." << std::endl;
    connection.close();
    return 0;
}



