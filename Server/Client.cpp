#include "pch.h"
#include "Network.h"
#include "Client.h"


void Client::do_recv()
{
	DWORD recv_flag = 0;
	ZeroMemory(&recv_over._wsa_over, sizeof(recv_over._wsa_over));
	recv_over._wsa_buf.buf = reinterpret_cast<char*>(recv_over._net_buf + prev_recv_size);
	recv_over._wsa_buf.len = sizeof(recv_over._net_buf) - prev_recv_size;
	int ret = WSARecv(socket, &recv_over._wsa_buf, 1, 0, &recv_flag, &recv_over._wsa_over, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
			error_display(error_num);
	}
}

void Client::do_send(int num_bytes, void* mess)
{
	EXP_OVER* ex_over = new EXP_OVER(OP_SEND, num_bytes, mess);
	WSASend(socket, &ex_over->_wsa_buf, 1, 0, 0, &ex_over->_wsa_over, NULL);
}