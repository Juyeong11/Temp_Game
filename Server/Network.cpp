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
		//? �� ���� �ʿ�����?
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

		//�ٸ� Ŭ���̾�Ʈ���� ���ο� Ŭ���̾�Ʈ�� ������ �˸�
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

			// ���� ���� Ŭ���̾�Ʈ�� ������ �ִٸ� �� ����Ʈ�� �ְ� put packet�� ������.
			other.vl.lock();
			other.viewlist.insert(client_id);
			other.vl.unlock();

			send_put_object(other.id, client_id);
		}

		//���� ������ Ŭ���̾�Ʈ���� ���� Ŭ���̾�Ʈ���� ��Ȳ�� �˷���
		for (auto& other : clients) {
			if (other.id == client_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other.state) {
				other.state_lock.unlock();
				continue;
			}
			other.state_lock.unlock();

			// ������ �ִ� Ŭ���̾�Ʈ�� ������ �ִٸ� �� ����Ʈ�� �ְ� put packet�� ������.
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
		
		// �̵��� Ŭ���̾�Ʈ�� ���� nearlist ����
		// �� unordered_set�̿��� �ұ�?
		// �󸶳� �߰����� �𸣰�, �����ʹ� id�̱� ������ �ߺ������� ������ִ�. id�� ���о��ϴ� ��찡 �ֳ�?
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

		//lock�ð��� ���̱� ���� �ڷḦ �����ؼ� ���
		cl.vl.lock();
		std::unordered_set<int> my_vl{ cl.viewlist };
		cl.vl.unlock();


		// ���������ν� �þ߿� ���� �÷��̾� Ȯ�� �� �߰�
		for (int other : nearlist) { 
			// cl�� �丮��Ʈ�� ������
			if (0 == my_vl.count(other)) {
				// cl�� �丮��Ʈ�� �߰��ϰ�
				cl.vl.lock();
				cl.viewlist.insert(other);
				cl.vl.unlock();
				// �������� �׸���� ��Ŷ�� ������.
				send_put_object(cl.id, other);


				// ������ ���̸� ��뿡�Ե� ���δٴ� ���̴�
				// ��� �丮��Ʈ�� Ȯ���Ѵ�.
				clients[other].vl.lock();

				// ��� �丮��Ʈ�� ������
				if (0 == clients[other].viewlist.count(cl.id)) {
					// �丮��Ʈ�� �߰��ϰ� cl�� �׸���� ����
					clients[other].viewlist.insert(cl.id);
					clients[other].vl.unlock();
					send_put_object(other, cl.id);// cl other�̿��� ����
				}
				// ��� �丮��Ʈ�� ������ �̵� ��Ŷ ����
				else {
					clients[other].vl.unlock();
					send_move_object(other, cl.id);
				}

			}
			//��� �þ߿� �����ϴ� �÷��̾� ó��
			else {
				clients[other].vl.lock();
				//���濡 �丮��Ʈ�� ���� �ִ��� Ȯ��
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


		// ���������ν� �þ߿��� ���� �÷��̾� Ȯ�� �� ����
		for (int other : my_vl) {
			// nearlist�� ������
			if (0 == nearlist.count(other)) {
				// �����׼� �����
				cl.vl.lock();
				cl.viewlist.erase(other);
				cl.vl.unlock();
				send_remove_object(cl.id, other);

				// ���浵 ���� �����.
				clients[other].vl.lock();
				//�ִٸ� ����
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
		std::cout << "�̻��� ��Ŷ ����\n";
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
			//�ϳ��� ���Ͽ� ���� Recvȣ���� ������ �ϳ� -> EXP_OVER(����, WSAOVERLAPPED) ���� ����
			//��Ŷ�� �߰��� �߷��� ä�� ������ �� �ִ�. -> ���ۿ� ���ξ��ٰ� ������ �� �����Ϳ� ���� -> ������ ���� ũ�⸦ ����� �� ��ġ���� �ޱ� ��������
			//��Ŷ�� ���� �� �ѹ��� ������ �� �ִ�.	 -> ù ��°�� �������̴� �߶� ó������
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
				std::cout << "�۽Ź��� ���� ��\n";
				std::cout << "Ŭ���̾�Ʈ ���� ����\n";
				disconnect_client(client_id);
			}
			delete exp_over;
		}
		break;
		case OP_ACCEPT:
		{
			SOCKET c_socket = *(reinterpret_cast<SOCKET*>(exp_over->_net_buf)); // Ȯ�� overlapped����ü�� �־� �ξ��� ��Ĺ�� ������
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

			// exp_over ��Ȱ��
			ZeroMemory(&exp_over->_wsa_over, sizeof(exp_over->_wsa_over));
			c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			//char* ���۸� socket*�� �ٲ� ������ ����ų �� �ֵ��� �Ѵ�. ���ϵ� �������ε�?
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