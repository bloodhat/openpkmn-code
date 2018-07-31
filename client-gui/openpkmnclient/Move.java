/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

/**
 *
 * @author matt
 */
import java.net.*;
import java.io.*;
public class Move {
    int number;
    int pp;
    int restriction;

    public Move() {
    }
  public boolean receive(ByteArrayInputStream payloadStream) {
	number = payloadStream.read();
	
	if(number < 0) {
	    number += 256;
	}
	System.err.println("received move is " + number + "\n");
	
	pp = payloadStream.read();
	
	restriction = payloadStream.read();

	payloadStream.read();
	
	return true;
    }
    public Move(int number, int ppUps) {
        this.number = number;
        if (ppUps >= 3 || ppUps < 0) {
            this.pp = Environment.moveMaxPP[number];
        } else if (Environment.moveMaxPP[number] == 61) {
            this.pp = (64 * (5 + ppUps)) / 8;
        } else {
            this.pp = (Environment.moveMaxPP[number] *
                    (5 + ppUps)) / 8;
        }


        this.restriction = 0;
    }

    public int getNumber() {
        return number;
    }

    public String getName() {
        return Lexicon.moveNames[number];
    }

    public int getPP() {
        return pp;
    }
}
