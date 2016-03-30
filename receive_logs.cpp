#include <iostream>
#include <amqpcpp.h>
#include "conn_handler.h"

int
main(void)
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

    auto printLog = [](const AMQP::Message &message,
                       uint64_t deliveryTag,
                       bool redelivered)
    {
        std::cout << "Log: " << message.message() << std::endl;
    };

    channel.declareExchange("logs", AMQP::fanout)
        .onSuccess
        (
            [&]()
            {
                std::cout << "Exchange created." << std::endl;
                channel.declareQueue(AMQP::exclusive)
                .onSuccess
                (
                    [&channel, printLog](const std::string &name,
                                         uint32_t messagecount,
                                         uint32_t consumercount)
                    {
                        std::cout << "Queue created." << std::endl;
                        channel.bindQueue("logs", name,"");
                        channel.consume(name, AMQP::noack).onReceived(printLog);
                    }
                );
            }
        );
    handler.Start();
    std::cout << "Closing connection." << std::endl;
    connection.close();
    return 0;
}



