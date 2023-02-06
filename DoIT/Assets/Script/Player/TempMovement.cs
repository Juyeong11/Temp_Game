using System.Collections;
using System.Collections.Generic;
using UnityEngine;



public class TempMovement : MonoBehaviour
{
    public PlayerStater PStater;
    public float myCharacterMaxVelocity;
    public float myCharacterHorizontalVelocity;
    public float myCharacterJumpPower;
    public int maxJumpCount;
    private int nowJumpCount;
    public Rigidbody2D _rigidbody2D;
    public Footting selfFoot;
    // Start is called before the first frame update
    void Start()
    {
        nowJumpCount = maxJumpCount;
        selfFoot.footted += (Collider2D) => SelfFoottedEnter(Collider2D);
    }

    // Update is called once per frame
    void Update()
    {
        int keydown = 0;
        if (Input.GetKeyDown(KeyCode.LeftArrow))
        {
            keydown++;
            Debug.Log(1);
            PStater.changeState(playerState.left);
        }
        if (Input.GetKeyDown(KeyCode.RightArrow))
        {
            keydown++;
            PStater.changeState(playerState.right);
        }
        if (Input.GetKeyDown(KeyCode.UpArrow))
        {
            keydown++;
            PStater.changeState(playerState.up);
        }
        if (Input.GetKeyDown(KeyCode.DownArrow))
        {
            keydown++;
            PStater.changeState(playerState.down);
        }

        if (Input.GetKey(KeyCode.LeftArrow))
        {
            keydown++;
            if (_rigidbody2D.velocity.x > -myCharacterMaxVelocity)
            {
                _rigidbody2D.AddForce(new Vector2(-myCharacterHorizontalVelocity * Time.deltaTime,0f));
            }
        }
        if (Input.GetKey(KeyCode.RightArrow))
        {
            keydown++;
            if (_rigidbody2D.velocity.x < myCharacterMaxVelocity)
            {
                _rigidbody2D.AddForce(new Vector2(myCharacterHorizontalVelocity * Time.deltaTime,0f));
            }
        }
        if (Input.GetKey(KeyCode.UpArrow))
        {
            keydown++;
        }
        if (Input.GetKey(KeyCode.DownArrow))
        {
            keydown++;
        }
        
        if (keydown == 0)
        {
            PStater.changeState(playerState.idle);
        }

        if (Input.GetKeyDown(KeyCode.Space)) // TODO : 누르는 시간따라 다르게 수정 필요 (Up에서 횟수 차감, 누르고 있으면 계속 AddForce)
        {
            if (nowJumpCount > 0)
            {
                _rigidbody2D.AddForce(new Vector2(0f,myCharacterJumpPower));
                nowJumpCount--;
            }
            
        }

    }
    private void SelfFoottedEnter(Collider2D collision)
    {
        //Debug.Log(collision.transform.tag);
        nowJumpCount = maxJumpCount;
    }

    // private void OnCollisionStay2D(Collision2D collision)
    // {
    //     Debug.Log("충돌 중!");
    // }

    // private void OnCollisionExit2D(Collision2D collision)
    // {
    //     Debug.Log("충돌 끝!");
    // }
}
