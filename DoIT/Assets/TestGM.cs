using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;



public class TestGM : MonoBehaviour
{
    GameObject[] Players;

    public GameObject PlayerPrefeb;
    public GameObject OtherPlayerPrefeb;
    Network Net = new Network();
    // Start is called before the first frame update
    void Start()
    {
        Net.CreateAndConnect();

        Players = new GameObject[Protocol.CONSTANTS.MAX_CLIENT];
    }
    private void OnApplicationQuit()
    {
        Net.CloseSocket();
    }
    public void PutPlayerObject(int type, int id, short x, short y)
    {
        Players[id] = Instantiate(PlayerPrefeb);
        Debug.Log("오브젝트 넣어! " + id);
        Players[id].GetComponent<ST_move>().SetPos(x, y);
    }
    public void PutObject(int type, int id, short x, short y)
    {
        Players[id] = Instantiate(OtherPlayerPrefeb);
        Debug.Log(id);
        Players[id].GetComponent<ST_move>().SetPos(x, y);
    }
    public void MoveObject(int type, int id, short x, short y)
    {
        Debug.Log("Move  " + id);
        Players[id].GetComponent<ST_move>().SetPos(x, y);
    }
    public void ReMoveObject(int id)
    {
        Destroy(Players[id]);
    }

    public void Update()
    {
        if (Network.MessQueue.Count > 0)
        {
            byte[] data = Network.MessQueue.Dequeue();

            byte type = data[1];

            switch (type)
            {
                case Protocol.CONSTANTS.SC_PACKET_LOGIN_OK:
                    {
                        Protocol.sc_packet_login_ok p = Protocol.sc_packet_login_ok.SetByteToVar(data);

                        PutPlayerObject(p.type, p.id, p.x, p.y);
                    }
                    break;
                case Protocol.CONSTANTS.SC_PACKET_MOVE:
                    {
                        Protocol.sc_packet_move p = Protocol.sc_packet_move.SetByteToVar(data);

                        MoveObject(p.type, p.id, p.x, p.y);

                    }
                    break;
                case Protocol.CONSTANTS.SC_PACKET_PUT_OBJECT:
                    {
                        Protocol.sc_packet_put_object p = Protocol.sc_packet_put_object.SetByteToVar(data);

                        PutObject(p.type, p.id, p.x, p.y);

                    }
                    break;
                case Protocol.CONSTANTS.SC_PACKET_REMOVE_OBJECT:
                    {
                        Protocol.sc_packet_remove_object p = Protocol.sc_packet_remove_object.SetByteToVar(data);

                        ReMoveObject(p.id);

                    }
                    break;
                default:
                    Debug.Log("이상한 타입이네");
                    break;
            }
        }
        //System.Threading.Thread.Sleep(10);
        if (Input.GetKeyDown(KeyCode.W))
        {
            Net.SendMovePacket(1);

        }
        else if (Input.GetKeyDown(KeyCode.S))
        {
            Net.SendMovePacket(0);
        }
        else if (Input.GetKeyDown(KeyCode.A))
        {
            Net.SendMovePacket(2);
        }
        else if (Input.GetKeyDown(KeyCode.D))
        {
            Net.SendMovePacket(3);
        }
    }
}
