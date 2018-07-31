/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;
import java.net.*;
import java.io.*;

/**
 *
 * @author matt
 */
public class OpponentPkmn {
    int number;
    int level;
    int status;
    int hpBars;

    String nickname;
    public boolean receive(ByteArrayInputStream payloadStream) {
	number = payloadStream.read();
	if(number < 0) {
	    number += 256;
	}
	level = payloadStream.read();
	status = payloadStream.read();
	/* java treats bytes as signed
	   so we'll have to work around
	   this */
	hpBars = payloadStream.read();
	if(hpBars < 0) {
	    hpBars += 256;
	}

	byte b[] = new byte[32];
	/* read 32 bytes of string */
	for(int i = 0; i < 32; i++) {
	    b[i] = (byte)payloadStream.read();
	}
	nickname = (new String(b)).trim();
	return true;
    }

    public String getNickname() {
        return nickname;
    }
    public int getNumber() {
	return number;
    }
    public String getName() {
	return Lexicon.pkmnNames[number];
    }
    public int getLevel() {
	return level;
    }
    public int getStatus() {
	return status;
    }
    public double getlifePercent() {
	return ((double)(hpBars)) / 255.0;
    }
}
