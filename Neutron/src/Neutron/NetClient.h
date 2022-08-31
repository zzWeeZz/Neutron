#pragma once
#include "NetCommon.h"
#include "NeuQue.h"
#include "NetMessage.h"
#include "NetConnection.h"

namespace Neutron
{
	template<typename T>
	class ClientBase
	{

	public:

		ClientBase() : m_Socket(m_Context)
		{
		}

		virtual ~ClientBase()
		{
			Disconnect();
		}
		bool Connect(const std::string& hostIp, const uint16_t port)
		{
			try
			{
				asio::ip::tcp::resolver resolver(m_Context);
				asio::ip::tcp::resolver::results_type m_EndPoints = resolver.resolve(hostIp, std::to_string(port));

				m_Connection = std::make_unique<Connection<T>>(Connection<T>::owner::client, m_Context, asio::ip::tcp::socket(m_Context), m_MessagesIn);
				m_Connection->Connect(m_EndPoints);


				m_ContextThread = std::thread([this]
					{
						m_Context.run();
					});
				return true;
			}
			catch (std::exception& e)
			{
				std::cout << e.what();
				return false;
			}

			return true;
		}

		void Disconnect()
		{
			if (IsConnected())
			{
				m_Connection->Disconnect();
			}

			m_Context.stop();
			if (m_ContextThread.joinable())
			{
				m_ContextThread.join();
			}
			m_Connection.release();
		}

		bool IsConnected()
		{
			if (m_Connection)
			{
				return m_Connection->IsConnected();
			}
			return false;
		}

		NeuQue<OwnedMessage<T>>& IncomingMessages()
		{
			return m_MessagesIn;
		}


		void Send(const Message<T>& msg)
		{
			if (IsConnected())
				m_Connection->Send(msg);
		}

	protected:
		asio::io_context m_Context;
		std::thread m_ContextThread;
		asio::ip::tcp::socket m_Socket;

		std::unique_ptr<Connection<T>> m_Connection;

	private:
		NeuQue<OwnedMessage<T>> m_MessagesIn;
	};
}