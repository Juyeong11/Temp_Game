#include "pch.h"

#include "Network.h"

Network* Network::instance = nullptr;

Network* Network::GetInstance()
{
	return instance;
}

void error_display(int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, 0);
	std::wcout << lpMsgBuf << std::endl;
	//while (true);
	LocalFree(lpMsgBuf);
}

void Network::send_login_ok(int c_id)
{
	sc_packet_login_ok packet;
	packet.id = c_id;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_LOGIN_OK;
	packet.x = clients[c_id].x;
	packet.y = clients[c_id].y;
	clients[c_id].do_send(sizeof(packet), &packet);
}

void Network::send_move_object(int c_id, int mover)
{
	sc_packet_move packet;
	packet.id = mover;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_MOVE;
	packet.x = clients[mover].x;
	packet.y = clients[mover].y;
	clients[c_id].do_send(sizeof(packet), &packet);
}

void Network::send_put_object(int c_id, int target) {
	sc_packet_put_object packet;

	//strcpy_s(packet.name, clients[target].name);
	packet.id = target;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_PUT_OBJECT;
	packet.object_type = 0;
	packet.x = clients[target].x;
	packet.y = clients[target].y;
	clients[c_id].do_send(sizeof(packet), &packet);
}

void Network::send_remove_object(int c_id, int victim)
{
	sc_packet_remove_object packet;
	packet.id = victim;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_REMOVE_OBJECT;
	clients[c_id].do_send(sizeof(packet), &packet);
}

void Network::disconnect_client(int c_id)
{
	clients[c_id].state_lock.lock();
	clients[c_id].state = ST_FREE;
	closesocket(clients[c_id].socket);

	clients[c_id].state_lock.unlock();

	for (auto& cl : clients) {
		//? 왜 락이 필요하지?
		cl.state_lock.lock();//
		if (ST_INGAME != cl.state) {
			cl.state_lock.unlock();
			continue;
		};
		cl.state_lock.unlock();
		send_remove_object(cl.id, c_id);

	}
}

int Network::get_new_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		clients[i].state_lock.lock();
		if (ST_FREE == clients[i].state) {
			clients[i].state = ST_ACCEPT;
			clients[i].state_lock.unlock();
			return i;
		}
		clients[i].state_lock.unlock();
	}
	std::cout << "Maximum Number of Clients Overflow!!\n";
	return -1;
}

void Network::process_packet(int client_id, unsigned char* p)
{
	unsigned char packet_type = p[1];
	Client& cl = clients[client_id];

	switch (packet_type)
	{
	case CS_PACKET_LOGIN:
	{
		cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(p);
		strcpy_s(cl.name, packet->name);
		send_login_ok(client_id);

		cl.state_lock.lock();
		cl.state = ST_INGAME;
		cl.state_lock.unlock();

		//다른 클라이언트에게 새로운 클라이언트가 들어옴을 알림
		for (auto& other : clients) {
			if (other.id == client_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other.state) {
				other.state_lock.unlock();
				continue;
			}
			other.state_lock.unlock();

			if (false == is_near(other.id, client_id))
				continue;

			// 새로 들어온 클라이언트가 가까이 있다면 뷰 리스트에 넣고 put packet을 보낸다.
			other.vl.lock();
			other.viewlist.insert(client_id);
			other.vl.unlock();

			send_put_object(other.id, client_id);
		}

		//새로 접속한 클라이언트에게 현재 클라이언트들의 현황을 알려줌
		for (auto& other : clients) {
			if (other.id == client_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other.state) {
				other.state_lock.unlock();
				continue;
			}
			other.state_lock.unlock();

			// 기존에 있던 클라이언트가 가까이 있다면 뷰 리스트에 넣고 put packet을 보낸다.
			cl.vl.lock();
			cl.viewlist.insert(other.id);
			cl.vl.unlock();

			send_put_object(client_id, other.id);
		}
	}
	break;
	case CS_PACKET_MOVE:
	{
		cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(p);
		short& x = cl.x;
		short& y = cl.y;
		switch (packet->direction) {
		case 0: if (y > 0) y--; break;
		case 1: if (y < (WORLD_HEIGHT - 1)) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < (WORLD_WIDTH - 1)) x++; break;
		default:
			std::cout << "Invalid move in client " << client_id << std::endl;
			exit(-1);
		}
		
		// 이동한 클라이언트에 대한 nearlist 생성
		// 꼭 unordered_set이여야 할까?
		// 얼마나 추가될지 모르고, 데이터는 id이기 때문에 중복없음이 보장되있다. id로 구분안하는 경우가 있나?
		std::unordered_set<int> nearlist;
		for (auto& other : clients) {
			if (other.id == client_id)
				continue;
			if (ST_INGAME != other.state)
				continue;
			if (false == is_near(client_id, other.id))
				continue;

			nearlist.insert(other.id);
		}

		send_move_object(cl.id, cl.id);

		//lock시간을 줄이기 위해 자료를 복사해서 사용
		cl.vl.lock();
		std::unordered_set<int> my_vl{ cl.viewlist };
		cl.vl.unlock();


		// 움직임으로써 시야에 들어온 플레이어 확인 및 추가
		for (int other : nearlist) { 
			// cl의 뷰리스트에 없으면
			if (0 == my_vl.count(other)) {
				// cl의 뷰리스트에 추가하고
				cl.vl.lock();
				cl.viewlist.insert(other);
				cl.vl.unlock();
				// 보였으니 그리라고 패킷을 보낸다.
				send_put_object(cl.id, other);


				// 나한테 보이면 상대에게도 보인다는 뜻이니
				// 상대 뷰리스트도 확인한다.
				clients[other].vl.lock();

				// 상대 뷰리스트에 없으면
				if (0 == clients[other].viewlist.count(cl.id)) {
					// 뷰리스트에 추가하고 cl을 그리라고 전송
					clients[other].viewlist.insert(cl.id);
					clients[other].vl.unlock();
					send_put_object(other, cl.id);// cl other이였음 수정
				}
				// 상대 뷰리스트에 있으면 이동 패킷 전송
				else {
					clients[other].vl.unlock();
					send_move_object(other, cl.id);
				}

			}
			//계속 시야에 존재하는 플레이어 처리
			else {
				clients[other].vl.lock();
				//상대방에 뷰리스트에 내가 있는지 확인
				if (0 != clients[other].viewlist.count(cl.id))
				{
					clients[other].vl.unlock();

					send_move_object(other, cl.id);
				}
				else {
					clients[other].viewlist.insert(cl.id);
					clients[other].vl.unlock();

					send_put_object(other, cl.id);
				}
			}
		}


		// 움직임으로써 시야에서 빠진 플레이어 확인 및 제거
		for (int other : my_vl) {
			// nearlist에 없으면
			if (0 == nearlist.count(other)) {
				// 나한테서 지우고
				cl.vl.lock();
				cl.viewlist.erase(other);
				cl.vl.unlock();
				send_remove_object(cl.id, other);

				// 상대방도 나를 지운다.
				clients[other].vl.lock();
				//있다면 지움
				if (0 != clients[other].viewlist.count(cl.id)) {

					clients[other].viewlist.erase(cl.id);
					clients[other].vl.unlock();

					send_remove_object(other, cl.id);
				}
				else clients[other].vl.unlock();
			}

		}
	}
	break;
	default:
		std::cout << "이상한 패킷 수신\n";
		break;
	}
}

void Network::worker()
{
	while (true) {
		DWORD num_byte;
		LONG64 iocp_key;
		WSAOVERLAPPED* p_over;
		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);

		int client_id = static_cast<int>(iocp_key);
		EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(p_over);

		if (FALSE == ret) {

			//error_display(WSAGetLastError());
			std::cout << client_id << "user logout\n";
			disconnect_client(client_id);
			if (exp_over->_comp_op == OP_SEND)
				delete exp_over;
			continue;
		}

		switch (exp_over->_comp_op)
		{
		case OP_RECV:
		{
			//하나의 소켓에 대해 Recv호출은 언제나 하나 -> EXP_OVER(버퍼, WSAOVERLAPPED) 재사용 가능
			//패킷이 중간에 잘려진 채로 도착할 수 있다. -> 버퍼에 놔두었다가 다음에 온 데이터와 결합 -> 이전에 받은 크기를 기억해 그 위치부터 받기 시작하자
			//패킷이 여러 개 한번에 도착할 수 있다.	 -> 첫 번째가 사이즈이니 잘라서 처리하자
			Client& cl = clients[client_id];

			int remain_data = cl.prev_recv_size + num_byte;
			unsigned char* buf_point = exp_over->_net_buf;
			int packet_size = buf_point[0];

			while (packet_size <= remain_data) {
				process_packet(client_id, buf_point);
				remain_data -= packet_size;
				buf_point += packet_size;
				if (remain_data > 0)
					packet_size = buf_point[0];
			}

			cl.prev_recv_size = remain_data;

			if (remain_data) {
				memcpy_s(&exp_over->_net_buf, remain_data, buf_point, remain_data);
			}

			cl.do_recv();
		}
		break;
		case OP_SEND:
		{
			if (num_byte != exp_over->_wsa_buf.len) {
				std::cout << "송신버퍼 가득 참\n";
				std::cout << "클라이언트 연결 끊음\n";
				disconnect_client(client_id);
			}
			delete exp_over;
		}
		break;
		case OP_ACCEPT:
		{
			SOCKET c_socket = *(reinterpret_cast<SOCKET*>(exp_over->_net_buf)); // 확장 overlapped구조체에 넣어 두었던 소캣을 꺼낸다
			int new_id = get_new_id();
			Client& cl = clients[new_id];
			cl.x = 0;
			cl.y = 0;
			cl.id = new_id;
			cl.prev_recv_size = 0;
			cl.recv_over._comp_op = OP_RECV;
			//cl._state = ST_INGAME;
			cl.recv_over._wsa_buf.buf = reinterpret_cast<char*>(cl.recv_over._net_buf);
			cl.recv_over._wsa_buf.len = sizeof(cl.recv_over._net_buf);
			ZeroMemory(&cl.recv_over._wsa_over, sizeof(cl.recv_over._wsa_over));
			cl.socket = c_socket;

			CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_h_iocp, new_id, 0);

			cl.do_recv();

			// exp_over 재활용
			ZeroMemory(&exp_over->_wsa_over, sizeof(exp_over->_wsa_over));
			c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			//char* 버퍼를 socket*로 바꿔 소켓을 가르킬 수 있도록 한다. 소켓도 포인터인디?
			*(reinterpret_cast<SOCKET*>(exp_over->_net_buf)) = c_socket;

			AcceptEx(g_s_socket, c_socket, exp_over->_net_buf + sizeof(SOCKET), 0, sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16, NULL, &exp_over->_wsa_over);
		}
		break;
		default:
			break;
		}
	}
}