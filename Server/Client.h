#pragma once

enum COMP_OP { OP_RECV, OP_SEND, OP_ACCEPT };


class EXP_OVER {
public:
	WSAOVERLAPPED	_wsa_over;
	COMP_OP			_comp_op;
	WSABUF			_wsa_buf;
	unsigned char	_net_buf[BUFSIZE];
public:
	EXP_OVER(COMP_OP comp_op, char num_bytes, void* mess) : _comp_op(comp_op)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = reinterpret_cast<char*>(_net_buf);
		_wsa_buf.len = num_bytes;
		memcpy(_net_buf, mess, num_bytes);
	}

	EXP_OVER(COMP_OP comp_op) : _comp_op(comp_op) {}

	EXP_OVER()
	{
		_comp_op = OP_RECV;
	}

	~EXP_OVER()
	{
	}
};

enum STATE { ST_FREE, ST_ACCEPT, ST_INGAME };

const int RANGE = 5;// test�� ���� �Ÿ�
class Client
{
public:
	char name[MAX_NAME_SIZE];
	int	   id;
	short  x, y;

	std::mutex state_lock;
	//volatile����� �Ѵ�.
	volatile STATE state;

	std::unordered_set<int> viewlist;
	std::mutex vl;

	EXP_OVER recv_over;
	SOCKET socket; // ���� �ϱ� ������ data race -> state�� ��ȣ

	int		prev_recv_size;
public:

	Client() :state(ST_FREE)
	{
		x = 0;
		y = 0;
		prev_recv_size = 0;
	}

	~Client()
	{
		closesocket(socket);
	}




	void do_recv();
	void do_send(int num_bytes, void* mess);

};

