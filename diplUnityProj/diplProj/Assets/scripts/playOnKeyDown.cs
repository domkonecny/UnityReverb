using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(AudioSource))]

public class playOnKeyDown : MonoBehaviour {

	AudioSource source;

	// Use this for initialization
	void Start () {
		source = this.GetComponent<AudioSource> ();
	}
	
	// Update is called once per frame
	void Update () {
	

		if (Input.GetKeyDown (KeyCode.Space))
			source.Play ();
	}
}
