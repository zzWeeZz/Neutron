#pragma once
#include "NetMessage.h"
#include "NetCommon.h"
#include "NeuQue.h"

namespace Neutron
{
	template<typename T>
	class Connection : public std::enable_shared_from_this<Connection<T>>
	{
	public:
		enum class owner
		{
			server,
			client
		};

		Connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, NeuQue<OwnedMessage<T>>& incomingMessages)
			: m_AsioContext(asioContext), m_Socket(std::move(socket)), m_MessagesIn(incomingMessages), m_OwnerType(parent) {}

		// ONLY CLIENT SHOULD CALL THIS
		void Connect(asio::ip::tcp::resolver::results_type& endpoints)
		{
			if (m_OwnerType == owner::client)
			{
				asio::async_connect(m_Socket, endpoints, [this](std::error_code ec, asio::ip::tcp::endpoint endPoint)
					{
						if (!ec)
						{
							ReadInfo();
						}
					});
			}
		}
		bool Disconnect()
		{
			if (IsConnected())
			{
				asio::post(m_AsioContext, [this] { m_Socket.close(); });
			}
			return true;
		}

		bool IsConnected() const
		{
			return m_Socket.is_open();
		}

		void AddToIncomingMessageQueue()
		{
			if (m_OwnerType == owner::server)
			{
				m_MessagesIn.PushBack({ this->shared_from_this(), m_CurrentReceivedMessage });
			}
			else
			{
				m_MessagesIn.PushBack({ nullptr, m_CurrentReceivedMessage });
			}

			ReadInfo();
		}

		void Send(const Message<T>& msg)
		{
			asio::post(m_AsioContext, [this, msg]
				{
					const bool WritingMessage = !m_MessagesOut.Empty();
					m_MessagesOut.PushBack(msg);
					if (!WritingMessage)
					{
						WriteInfo();
					}
				});
		}

		[[nodiscard]] uint32_t GetID() const
		{
			return m_Id;
		}

		void ConnectToClient(uint32_t id = 0)
		{
			if (m_OwnerType == owner::server)
			{
				if (m_Socket.is_open())
				{
					m_Id = id;
					ReadInfo();
				}
			}

		}

		virtual ~Connection() {}

	protected:

		void ReadInfo()
		{
			asio::async_read(m_Socket, asio::buffer(&m_CurrentReceivedMessage.info, sizeof(MessageInfo<T>)),
				[this](std::error_code ec, size_t length)
				{
					if (!ec)
					{
						if (length > 0)
						{
							m_CurrentReceivedMessage.bytes.resize(length);
							ReadBytes();
						}
						else
						{
							AddToIncomingMessageQueue();
						}
					}
					else
					{
						std::cout << "[" << m_Id << "] Read Info Failed.\n";
						m_Socket.close();
					}
				});

		}
		void ReadBytes()
		{
			asio::async_read(m_Socket, asio::buffer(m_CurrentReceivedMessage.bytes.data(), m_CurrentReceivedMessage.bytes.size()),
				[this](std::error_code ec, size_t length)
				{
					if (!ec)
					{
						AddToIncomingMessageQueue();
					}
					else
					{
						std::cout << "[" << m_Id << "] Read Bytes Failed.\n";
						m_Socket.close();
					}
				});
		}
		void WriteInfo()
		{
			asio::async_write(m_Socket, asio::buffer(&m_MessagesOut.Front().info, sizeof(MessageInfo<T>)),
				[this](std::error_code ec, size_t length)
				{
					if (!ec)
					{
						if (m_MessagesOut.Front().bytes.size() > 0)
						{
							WriteBytes();
						}
						else
						{
							m_MessagesOut.PopFront();

							if (!m_MessagesOut.Empty())
							{
								WriteInfo();
							}
						}
					}
					else
					{
						std::cout << "[" << m_Id << "] Write Info Failed.\n";
						m_Socket.close();
					}
				});
		}
		void WriteBytes()
		{
			asio::async_write(m_Socket, asio::buffer(m_MessagesOut.Front().bytes.data(), m_MessagesOut.Front().bytes.size()),
				[this](std::error_code ec, size_t length)
				{
					if (!ec)
					{
						m_MessagesOut.PopFront();

						if (!m_MessagesOut.Empty())
						{
							WriteInfo();
						}
					}
					else
					{
						std::cout << "[" << m_Id << "] Write Bytes Failed.\n";
						m_Socket.close();
					}
				});
		}

		asio::ip::tcp::socket m_Socket;
		asio::io_context& m_AsioContext;

		NeuQue<Message<T>> m_MessagesOut;
		NeuQue<OwnedMessage<T>>& m_MessagesIn;
		Message<T> m_CurrentReceivedMessage;
		owner m_OwnerType = owner::server;
		uint32_t m_Id = 0;
	};
}
