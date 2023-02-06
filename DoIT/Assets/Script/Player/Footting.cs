using System.Collections;
using System.Collections.Generic;
using System;
using UnityEngine;

public class Footting : MonoBehaviour
{
    public Action<Collider2D> footted;

    private void OnTriggerEnter2D(Collider2D other)
    {
        footted(other);
    }

    private void OnTriggerStay2D(Collider2D other)
    {
        footted(other);
    }
}
