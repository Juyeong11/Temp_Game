using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.U2D;

public enum playerState
{
    idle,
    left,
    right,
    up,
    down,
};

public class PlayerStater : MonoBehaviour
{
    public playerState State;
    public Animator anim;
    
    public void changeState(playerState s)
    {
        switch(s)
        {
            case playerState.idle:
            anim.SetInteger("StateNums",0);
            break;
            case playerState.left:
            anim.SetInteger("StateNums",1);
            break;
            case playerState.right:
            anim.SetInteger("StateNums",2);
            break;
            case playerState.up:
            anim.SetInteger("StateNums",3);
            break;
            case playerState.down:
            anim.SetInteger("StateNums",4);
            break;
        }
        State = s;
    }
}

