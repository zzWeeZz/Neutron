#pragma once
#include <cstdint>
#include "NetCommon.h"
namespace Neutron
{
	template<typename T>
	struct MessageInfo
	{
		T id{};
		uint32_t size = 0;
	};

	template<typename T>
	struct Message
	{
		MessageInfo<T> info{};
		std::vector<uint8_t> bytes;

		[[nodiscard]] size_t GetSize() const
		{
			return bytes.size();
		}

		friend std::ostream& operator <<(std::ostream& os, const Message<T>& msg)
		{
			os << "Message ID: " << static_cast<std::ios_base & (*)(std::ios_base&)>(msg.info.id) << "Size of message: " << msg.info.size;
			return os;
		}

		template<typename dataType>
		friend Message<T>& operator <<(Message<T>& msg, const dataType& data)
		{
			static_assert(std::is_standard_layout_v<dataType>, "Data can not be added due to it being to complex. Perhaps it has static members or pointers?");

			size_t size = msg.bytes.size();


			msg.bytes.resize(msg.bytes.size() + sizeof(dataType));

			memcpy(msg.bytes.data() + size, &data, sizeof(dataType));

			msg.info.size = msg.GetSize();
			return msg;
		}


		template<typename dataType>
		friend Message<T>& operator >>(Message<T>& msg, dataType& data)
		{
			static_assert(std::is_standard_layout_v<dataType>, "Data can not be given due to it being to complex. Perhaps it has static members or pointers?");

			size_t size = msg.GetSize() - sizeof(dataType);

			memcpy(&data, msg.bytes.data() + size, sizeof(dataType));

			msg.bytes.resize(size);
			msg.info.size = msg.GetSize();

			return msg;
		}
	};

	template<typename T>
	class Connection;

	template<typename T>
	struct OwnedMessage
	{
		std::shared_ptr<Connection<T>> remote = nullptr;
		Message<T> msg;

		friend std::ostream& operator <<(std::ostream& os, const OwnedMessage<T>& msg)
		{
			os << msg.msg;
			return os;
		}
	};
}
