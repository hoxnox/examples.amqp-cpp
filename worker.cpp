#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <amqpcpp.h>
#include "conn_handler.h"

int main(void)
{
    ConnHandler handler;
    AMQP::TcpConnection connection(handler,
            AMQP::Address("localhost", 5672,
                          AMQP::Login("guest", "guest"), "/"));
    AMQP::TcpChannel channel(&connection);

    channel.setQos(1);
    channel.declareQueue("task_queue", AMQP::durable);
    channel.consume("task_queue")
        .onReceived
        (
            [&channel](const AMQP::Message msg,
                       uint64_t tag,
                       bool redelivered)
            {
                const auto body = msg.message();
                std::cout << "Received: " << body << std::endl;
                size_t count = std::count(body.cbegin(), body.cend(), '.');
                std::this_thread::sleep_for(std::chrono::seconds(count));
                std::cout << "Done" << std::endl;
                channel.ack(tag);
            }
        );
    handler.Start();
    std::cout << "Closing connection." << std::endl;
    connection.close();
    return 0;
}

