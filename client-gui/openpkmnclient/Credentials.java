/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.math.BigInteger;

/**
 *
 * @author matt
 */
public class Credentials {
        String userName;
    String server;
    byte[] hashBytes;
    int port;
    public Credentials(String userName, String hashBytes, String server,
            int port) {
	this.userName = userName;
        while(hashBytes.length() < 32) {
            hashBytes = "0" + hashBytes;
        }
        this.hashBytes = new byte[16];
        for(int i = 0; i < 16; i++) {
            String s = hashBytes.substring(i * 2, (i * 2) + 2);
            int b = (Integer.parseInt(s, 16) & (int)0xFF);
            this.hashBytes[i] = (byte)b;
        }
	this.server = server;
	this.port = port;
    }

    public String getUserName() {
	return userName;
    }

    public byte[] getHashBytes() {
	return hashBytes;
    }

    public String getServer() {
	return server;
    }

    public int getPort() {
	return port;
    }
}
