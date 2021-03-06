#pragma once

class Asynchron : public WSAOVERLAPPED
{
public:
	constexpr Asynchron(const srv::Operations& service)
		: Asynchron(service, {})
	{}

	constexpr Asynchron(const srv::Operations& service, const WSABUF& wbuffer)
		: myOperation(service)
		, myBuffer(wbuffer)
		, isFirst(true)
	{}

	constexpr Asynchron(const srv::Operations& service, WSABUF&& wbuffer)
		: myOperation(service)
		, myBuffer(std::forward<WSABUF>(wbuffer))
		, isFirst(true)
	{}

	~Asynchron()
	{
		Release();
	}

	inline constexpr void SetBuffer(const WSABUF& wbuffer) noexcept
	{
		myBuffer = wbuffer;
	}

	inline constexpr void SetBuffer(WSABUF&& wbuffer) noexcept
	{
		myBuffer = std::forward<WSABUF>(wbuffer);
	}

	inline constexpr void SetBuffer(char* buffer, size_t length) noexcept
	{
		myBuffer.buf = buffer;
		myBuffer.len = static_cast<ULONG>(length);
	}

	inline int Send(SOCKET target, LPDWORD bytes, DWORD flags)
	{
		return WSASend(target, &myBuffer, 1, bytes, flags, this, nullptr);
	}

	inline int Recv(SOCKET target, LPDWORD bytes, DWORD flags)
	{
		return WSARecv(target, &myBuffer, 1, bytes, &flags, this, nullptr);
	}

	inline void Release() noexcept
	{
		if (myBuffer.buf)
		{
			delete[myBuffer.len] myBuffer.buf;

			myBuffer.buf = nullptr;
			myBuffer.len = 0;
		}
	}

	inline void Clear() noexcept
	{
		ZeroMemory(this, sizeof(WSAOVERLAPPED));
	}

	const srv::Operations myOperation;
	WSABUF myBuffer;
	bool isFirst;
};

namespace srv
{
	inline Asynchron* CreateAsynchron(const Operations& op)
	{
		return new Asynchron(op);
	}
}
