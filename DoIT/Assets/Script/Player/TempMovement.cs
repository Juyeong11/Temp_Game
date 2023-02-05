using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TempMovement : MonoBehaviour
{
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
        if (Input.GetKey(KeyCode.LeftArrow))
        {
            if (_rigidbody2D.velocity.x > -myCharacterMaxVelocity)
            {
                _rigidbody2D.AddForce(new Vector2(-myCharacterHorizontalVelocity * Time.deltaTime,0f));
            }
        }
        if (Input.GetKey(KeyCode.RightArrow))
        {
            if (_rigidbody2D.velocity.x < myCharacterMaxVelocity)
            {
                _rigidbody2D.AddForce(new Vector2(myCharacterHorizontalVelocity * Time.deltaTime,0f));
            }
        }
        if (Input.GetKeyDown(KeyCode.Space)) // TODO : ������ �ð����� �ٸ��� ���� �ʿ� (Up���� Ƚ�� ����, ������ ������ ��� AddForce)
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
        Debug.Log(collision.transform.tag);
        nowJumpCount = maxJumpCount;
    }

    // private void OnCollisionStay2D(Collision2D collision)
    // {
    //     Debug.Log("�浹 ��!");
    // }

    // private void OnCollisionExit2D(Collision2D collision)
    // {
    //     Debug.Log("�浹 ��!");
    // }
}
