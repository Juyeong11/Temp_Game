using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ST_move : MonoBehaviour
{
    Vector3 PlayerPos = new Vector3(MinPos, MinPos, 0f);


    const float Displacement = 1 - 0.01f;

    const float MaxPos = 3.5f * Displacement;
    const float MinPos = -3.5f * Displacement;
    // Start is called before the first frame update


    // Update is called once per frame
    public void SetPos(short x, short y)
    {
        PlayerPos.x = x - 3.5f;
        PlayerPos.y = y - 3.5f;
        PlayerPos.x = Mathf.Clamp(PlayerPos.x, MinPos, MaxPos);
        PlayerPos.y = Mathf.Clamp(PlayerPos.y, MinPos, MaxPos);
        gameObject.transform.position = PlayerPos;
    }
}