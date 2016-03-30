#include <iostream>
#include <algorithm>
#include <thread>
#include <sstream>
#include <chrono>
#include <amqpcpp.h>
#include "conn_handler.h"

int
main(int argc, char* argv[])
{
    std::stringstream ss;
    ss << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    std::string correlation = ss.str();
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
    channel.declareQueue(AMQP::exclusive)
        .onSuccess
        (
            [&channel, correlation](const std::string& name, int, int)
            {
                AMQP::Envelope env("30");
                env.setCorrelationID(correlation);
                env.setReplyTo(name);
                channel.publish("", "rpc_queue", env);
                std::cout << "Requesting fib(30)" << std::endl;
            }
        );
    channel.consume("", AMQP::noack)
        .onReceived
        (
            [correlation, &handler](const AMQP::Message& m, uint64_t, bool)
            {
                if (m.correlationID() != correlation)
                    return; // just skip it
                std::cout << "Got " << m.message() << std::endl;
                handler.Stop();
            }
        );
    handler.Start();
    std::cout << "Closing connection." << std::endl;
    connection.close();
    return 0;
}



