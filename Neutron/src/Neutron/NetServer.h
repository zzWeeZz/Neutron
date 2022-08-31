#pragma once
#include "NetClient.h"
#include "NetCommon.h"
#include "NetConnection.h"
#include "NetMessage.h"

namespace Neutron
{
	template<typename T>
	class ServerBase
	{
	public:
		ServerBase(uint16_t port) : m_AsioAcceptor(m_AsioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) { }

		bool Start()
		{
			try
			{
				WaitForClientConnection();
				m_ContextThread = std::thread([this]() {m_AsioContext.run(); });
				std::cout << "[SERVER] Start Successful!\n";
			}
			catch (std::exception& e)
			{
				std::cerr << "[SERVER] Exception: " << e.what() << std::endl;
				return false;
			}
			return true;
		}
		void Stop()
		{
			m_AsioContext.stop();
			if (m_ContextThread.joinable()) m_ContextThread.join();
			std::cout << "[SERVER] Stopped!\n";
		}

		void WaitForClientConnection()
		{
			m_AsioAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket)
				{
					if (!ec)
					{
						std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << std::endl;
						std::shared_ptr<Connection<T>> newConn = std::make_shared<Connection<T>>(Connection<T>::owner::server, m_AsioContext, std::move(socket), m_MessagesIn);

						if (OnClientConnect(newConn))
						{
							m_DeqConnections.push_back(std::move(newConn));
							m_DeqConnections.back()->ConnectToClient(m_IdCounter++);
							std::cout << "[" << m_DeqConnections.back()->GetID() << "] Connection Approved\n";
						}
						else
						{
							std::cout << "[SERVER] Connection Denied\n";
						}
					}
					else
					{
						std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
					}

					WaitForClientConnection();
				});
		}

		void MessageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg)
		{
			if (client && client->IsConnected())
			{
				client->Send(msg);
			}
			else
			{
				OnClientDisconnect(client);
				client.reset();
				m_DeqConnections.erase(std::remove(m_DeqConnections.begin(), m_DeqConnections.end(), client), m_DeqConnections.end());
			}
		}

		void MessageAllClients(const Message<T>& msg, std::shared_ptr<Connection<T>> ignoreConnection = nullptr)
		{
			bool invalidClientExist = false;
			for (auto& client : m_DeqConnections)
			{
				if (client && client->IsConnected())
				{
					if (client != ignoreConnection)
						client->Send(msg);
				}
				else
				{
					OnClientDisconnect(client);
					client.reset();
					invalidClientExist = true;
				}
			}
			if (invalidClientExist)
				m_DeqConnections.erase(std::remove(m_DeqConnections.begin(), m_DeqConnections.end(), nullptr), m_DeqConnections.end());
		}

		void Update(size_t MaxMessages = -1)
		{
			m_MessagesIn.Wait();
			size_t MessageCount = 0;
			while (MessageCount < MaxMessages && !m_MessagesIn.Empty())
			{
				auto msg = m_MessagesIn.PopFront();
				OnMessage(msg.remote, msg.msg);
				MessageCount++;
			}
		}

		virtual ~ServerBase() { Stop(); }
	protected:
		virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client)
		{
			return false;
		}

		virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client)
		{

		}

		virtual void OnMessage(std::shared_ptr<Connection<T>> client, const Message<T>& msg)
		{
			
		}

		NeuQue<OwnedMessage<T>> m_MessagesIn;

		std::deque<std::shared_ptr<Connection<T>>> m_DeqConnections;

		asio::io_context m_AsioContext;
		std::thread m_ContextThread;

		asio::ip::tcp::acceptor m_AsioAcceptor;
		uint32_t m_IdCounter = 10000;
	};
}