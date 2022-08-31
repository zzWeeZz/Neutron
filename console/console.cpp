#include "Neutron.h"
#include "Neutron/NetClient.h"

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};


class CustomServer : public Neutron::ServerBase<CustomMsgTypes>
{
public:
	CustomServer(uint16_t port) : ServerBase<CustomMsgTypes>(port)
	{

	}
protected:
	bool OnClientConnect(std::shared_ptr<Neutron::Connection<CustomMsgTypes>> client) override
	{
		return true;
	}
	void OnClientDisconnect(std::shared_ptr<Neutron::Connection<CustomMsgTypes>> client) override
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";

	}
	void OnMessage(std::shared_ptr<Neutron::Connection<CustomMsgTypes>> client, const Neutron::Message<CustomMsgTypes>& msg) override
	{
		switch (msg.info.id)
		{
		case CustomMsgTypes::ServerPing:
			std::cout << "[" << client->GetID() << "] Server Ping\n";
			MessageClient(client, msg);
			break;
		case CustomMsgTypes::MessageAll:
			std::cout << "[" << client->GetID() << "] Server Ping\n";
			MessageAllClients(msg);
			break;
		}
	}
};
class CustomClient : public Neutron::ClientBase<CustomMsgTypes>
{
public:
	void PingServer()
	{
		Neutron::Message<CustomMsgTypes> msg;
		msg.info.id = CustomMsgTypes::ServerPing;

		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		msg << timeNow;
		Send(msg);
	}
	void MessageAll()
	{
		Neutron::Message<CustomMsgTypes> msg;
		msg.info.id = CustomMsgTypes::MessageAll;

		std::string input;
		input = "HEllo other clients this is a test to see if i can talk to you";
		msg.bytes.resize(input.size());
		memcpy(msg.bytes.data(), input.data(), input.size());
		Send(msg);
	}
};

#ifdef SERVER_DEBUG
// server Main
int main()
{
	CustomServer server(60000);
	server.Start();
	while (true)
	{
		server.Update();
	}
	return 0;
}
#endif
#ifdef CLIENT_DEBUG
int main()
{
	CustomClient c;
	c.Connect("127.0.0.1", 60000);
	bool key[3] = { false, false, false };
	bool oldKey[3] = { false, false, false };
	bool quit = false;
	while (!quit)
	{
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
		}

		if (key[0] && !oldKey[0]) c.PingServer();
		if (key[1] && !oldKey[1]) c.MessageAll();
		if (key[2] && !oldKey[2]) c.Disconnect();
		

		for (size_t i = 0; i < 3; ++i)
		{
			oldKey[i] = key[i];
		}
		if (c.IsConnected())
		{
			if (!c.IncomingMessages().Empty())
			{
				auto msg = c.IncomingMessages().PopFront().msg;

				switch (msg.info.id)
				{
				case CustomMsgTypes::ServerPing:
				{
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					const auto value = std::chrono::duration<double>(timeNow - timeThen).count();
					std::cout << "Ping: " << value << "\n";
					break;
				}
				case CustomMsgTypes::MessageAll:
				{
					std::string from;
					memcpy((void*)from.data(), msg.bytes.data() , from.size());
					std::cout << from << std::endl;
					break;
				}
				}
			}
		}
	}
	return 0;
}
#endif

//#include <iostream>
//#ifdef _WIN32
//#define D_WIN32_WINNT = 0x0601
//#endif
//#define ASIO_STANDALONE
//#include "asio.hpp"
//#include <asio/ts/buffer.hpp>
//#include <asio/ts/internet.hpp>
//
//std::vector<char> vBuffer(20u * 1024);
//
//void GrabSomeData(asio::ip::tcp::socket& socket)
//{
//	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()), [&](std::error_code ec, size_t length)
//		{
//			if (!ec)
//			{
//				std::cout << "\n\nRead " << length << " bytes\n\n";
//				for (int i = 0; i < length; ++i)
//				{
//					std::cout << vBuffer[i];
//				}
//
//				GrabSomeData(socket);
//			}
//		});
//}
//
//int main()
//{
//	asio::error_code ec;
//
//	asio::io_context context;
//	asio::io_context::work idleWork(context);
//	std::thread threadContext = std::thread([&]() { context.run(); });
//	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);
//
//	asio::ip::tcp::socket socket(context);
//
//	socket.connect(endpoint, ec);
//	if (!ec)
//	{
//		std::cout << "Connected!\n";
//	}
//	else
//	{
//		std::cout << "Failed Because: " << ec.message() << std::endl;
//	}
//
//	if (socket.is_open())
//	{
//		GrabSomeData(socket);
//		std::string request = "GET /index.html HTTP/1.1\r\n" "Host: example.com\r\n" "Connection: close\r\n\r\n";
//		socket.write_some(asio::buffer(request.data(), request.size()), ec);
//
//
//		while (true)
//		{
//
//		}
//
//	}
//
//
//	return 0;
//}