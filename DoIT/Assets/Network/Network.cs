using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net.Sockets;
using System.Net;
using System;

using System.Runtime.InteropServices;

public class Network
{
    //public GameObject gm;
    //GM g;
    static public Queue<byte[]> MessQueue = new Queue<byte[]>();

    const int BUFSIZE = 256;

    Socket ClientSocket;
    // Start is called before the first frame update
    byte[] receiveBytes = new byte[BUFSIZE];

    void receiveComplet(System.IAsyncResult ar)
    {

        // 메세지가 오면 왔다고 말을 해줘야 하는데 어떤 방법으로 해줘야 하지?
        // 메세지 큐에 담아두고 엔진 루프에서 꺼내주자
        Socket c_Socket = (Socket)ar.AsyncState;
        int strLength = c_Socket.EndReceive(ar);

        int index = 0;

        while (index < strLength)
        {
            //잘라서 받아야겠는걸??
            int size = receiveBytes[index];
            //Debug.Log(size);
            byte[] temp = new byte[size];
            Buffer.BlockCopy(receiveBytes, index, temp, 0, size);
            index += size;


            MessQueue.Enqueue(temp);

        }
        //Debug.Log(BitConverter.ToString(receiveBytes));

        ClientSocket.BeginReceive(receiveBytes, 0, BUFSIZE, SocketFlags.None, new System.AsyncCallback(receiveComplet), ClientSocket);
    }
    void sendComplet(System.IAsyncResult ar)
    {
        Socket c_s = (Socket)ar.AsyncState;
        int strLength = c_s.EndSend(ar);
    }
    public void CreateAndConnect()
    {
        //Client
        ClientSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.IP);

        //서버의 ip와 포트 번호
        ClientSocket.Connect(new IPEndPoint(IPAddress.Loopback, 4000));

        //수신 과정
        ClientSocket.BeginReceive(receiveBytes, 0, BUFSIZE, SocketFlags.None, new System.AsyncCallback(receiveComplet), ClientSocket);


        Protocol.cs_packet_login pk = new Protocol.cs_packet_login();
        pk.size = (byte)Marshal.SizeOf(typeof(Protocol.cs_packet_login));
        pk.type = Protocol.CONSTANTS.CS_PACKET_LOGIN;

        ClientSocket.BeginSend(pk.GetBytes(), 0, pk.size, SocketFlags.None, new System.AsyncCallback(sendComplet), ClientSocket);// 이게 overlapped 콜백등록하는 함수인가?

        //Array.Clear(receiveBytes, 0x0, receiveBytes.Length);

    }
    public void CloseSocket()
    {
        ClientSocket.Close();
    }
    // Update is called once per frame
    public void SendMovePacket(byte dir)
    {
        //System.Threading.Thread.Sleep(10);

        Protocol.cs_packet_move pk = new Protocol.cs_packet_move();
        pk.size = (byte)Marshal.SizeOf(typeof(Protocol.cs_packet_move));
        //byte[] n = System.Text.Encoding.Default.GetBytes("Test");
        // Buffer.BlockCopy(pk.name, 0, n, 0, n.Length);
        pk.type = Protocol.CONSTANTS.CS_PACKET_MOVE;

        pk.direction = dir;

        ClientSocket.BeginSend(pk.GetBytes(), 0, pk.size, SocketFlags.None, new System.AsyncCallback(sendComplet), ClientSocket);// 이게 overlapped 콜백등록하는 함수인가?



        if (Input.GetKeyDown(KeyCode.Space)) Application.Quit();
    }
}