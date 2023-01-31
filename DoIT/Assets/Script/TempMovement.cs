using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TempMovement : MonoBehaviour
{
    public Rigidbody2D _rigidbody2D;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        if (Input.GetKey(KeyCode.LeftArrow))
        {
            _rigidbody2D.AddForce(new Vector2(-300.0f * Time.deltaTime,0f));
        }
        if (Input.GetKey(KeyCode.RightArrow))
        {
            _rigidbody2D.AddForce(new Vector2(300.0f * Time.deltaTime,0f));
        }
    }
}
