#pragma once
#include "Asynchron.hpp"
#include "Room.hpp"

class Session
{
public:
	constexpr Session(unsigned place)
		: mySwitch()
		, myPlace(place), mySocket(NULL), myID(0), myRoom(nullptr)
		, myReceiver(nullptr), myRecvBuffer()
	{}

	virtual ~Session()
	{}

	inline void Acquire() volatile
	{
		while (mySwitch.test_and_set(std::memory_order_acquire));
	}

	inline bool TryAcquire() volatile
	{
		return !mySwitch.test_and_set(std::memory_order_acquire);
	}

	inline void Release() volatile
	{
		mySwitch.clear(std::memory_order_release);
	}

	inline int BeginSend(Asynchron *asynchron)
	{
		return asynchron->Send(mySocket, nullptr, 0);
	}

	inline int BeginRecv()
	{
		myReceiver = make_shared<Asynchron>(srv::Operations::RECV);
		myReceiver->SetBuffer(myRecvBuffer, 0); // Page 락을 줄이기 위해 맨 처음에 0으로 받음

		return myReceiver->Recv(mySocket, nullptr, 0);
	}

	inline int Send(Asynchron *asynchron, char *const buffer, unsigned size, unsigned offset = 0)
	{
		auto &wbuffer = asynchron->myBuffer;
		wbuffer.buf = buffer + offset;
		wbuffer.len = static_cast<unsigned long>(size - offset);

		return asynchron->Send(mySocket, nullptr, 0);
	}

	template<unsigned original_size>
	inline int Send(Asynchron *asynchron, const char(&buffer)[original_size], unsigned offset = 0)
	{
		auto &wbuffer = asynchron->myBuffer;
		wbuffer.buf = buffer + offset;
		wbuffer.len = static_cast<unsigned long>(original_size - offset);

		return asynchron->Send(mySocket, nullptr, 0);
	}

	inline int Recv(unsigned size, unsigned offset = 0)
	{
		auto &wbuffer = myReceiver->myBuffer;
		wbuffer.buf = (myRecvBuffer)+offset;
		wbuffer.len = static_cast<unsigned long>(size - offset);

		return myReceiver->Recv(mySocket, nullptr, 0);
	}

	inline void AssignState(const srv::SessionStates state)
	{
		myState.store(state, std::memory_order_acq_rel);
	}

	inline void AssignSocket(const SOCKET &sock)
	{
		mySocket.store(sock, std::memory_order_acq_rel);
	}

	inline void AssignSocket(SOCKET &&sock)
	{
		mySocket.store(std::forward<SOCKET>(sock), std::memory_order_acq_rel);
	}

	inline void AssignID(const unsigned long long id)
	{
		myID.store(id, std::memory_order_acq_rel);
	}

	inline void AssignRoom(const shared_ptr<Room> &room)
	{
		myRoom.store(room, std::memory_order_acq_rel);
	}

	inline void AssignRoom(shared_ptr<Room> &&room)
	{
		myRoom.store(std::forward<shared_ptr<Room>>(room), std::memory_order_acq_rel);
	}

	inline void SetState(const srv::SessionStates state)
	{
		myState.store(state, std::memory_order_relaxed);
	}

	inline void SetSocket(const SOCKET &sock)
	{
		mySocket.store(sock, std::memory_order_relaxed);
	}

	inline void SetSocket(SOCKET &&sock)
	{
		mySocket.store(std::forward<SOCKET>(sock), std::memory_order_relaxed);
	}

	inline void SetID(const unsigned long long id)
	{
		myID.store(id, std::memory_order_relaxed);
	}

	inline void SetRoom(const shared_ptr<Room> &room)
	{
		myRoom.store(room, std::memory_order_relaxed);
	}

	inline void SetRoom(shared_ptr<Room> &&room)
	{
		myRoom.store(std::forward<shared_ptr<Room>>(room), std::memory_order_relaxed);
	}

	inline constexpr virtual bool IsUser()
	{
		return false;
	}

	inline constexpr virtual bool IsNotUser()
	{
		return true;
	}

	const unsigned int myPlace;

	atomic_flag mySwitch;
	atomic<srv::SessionStates> myState;
	atomic<SOCKET> mySocket;
	atomic<unsigned long long> myID;
	atomic<shared_ptr<Room>> myRoom;

	shared_ptr<Asynchron> myReceiver;
	char myRecvBuffer[BUFSIZ];
};
