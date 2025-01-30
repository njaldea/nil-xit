#include <flatbuffers/buffer.h>
#include <message.fbs.h>

#include <iostream>

int main()
{
    nil::xit::fbs::MessageT message;
    message.type = nil::xit::fbs::MessageType_Client_File_Request;
    message.payload = {1, 2, 3, 4, 5};

    flatbuffers::FlatBufferBuilder builder;
    {
        auto payload = builder.CreateVector(message.payload);
        auto msg = nil::xit::fbs::CreateMessage(builder, message.type, payload);
        builder.Finish(msg);
    }
    // std::cout << builder.GetBufferPointer() << std::endl;
    std::cout << builder.GetSize() << std::endl;

    {
        const auto* msg = flatbuffers::GetRoot<nil::xit::fbs::Message>(builder.GetBufferPointer());
        std::cout << "type    : " << msg->type() << std::endl;
        std::cout << "payload : " << msg->payload()->size() << std::endl;
    }
    return 0;
}
