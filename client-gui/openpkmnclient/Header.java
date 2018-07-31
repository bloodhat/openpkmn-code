/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.io.*;

/**
 *
 * @author matt
 */
public class Header {

    public static final int NUM_BYTES = 40;
    int packetType;
    String userName;
    int encryptionType;
    int packetLen;

    public Header() {
    }

    public Header(int pt, String un, int et, int pl) {
        packetType = pt;
        userName = un;
        encryptionType = et;
        packetLen = pl;
    }

    public boolean receive(DataInputStream stream) {
        try {
            packetType = stream.readInt();
        } catch (IOException e) {
            System.err.println("IOException Header.packetType " + e);
            return false;
        }
        byte b[] = new byte[32];
        /* read 32 bytes of string */
        try {
            for (int i = 0; i < 32; i++) {
                b[i] = stream.readByte();
            }
            userName = (new String(b)).trim();
        } catch (IOException e) {
            System.err.println("IOException Header.userName " + e);
            return false;
        }
        try {
            encryptionType = stream.readInt();
        } catch (IOException e) {
            System.err.println("IOException Header.encryptionType " + e);
            return false;
        }
        try {
            packetLen = stream.readInt();
        } catch (IOException e) {
            System.err.println(e);
            return false;
        }
        return true;

    }

    public boolean send(DataOutputStream stream) {
        try {
            stream.writeInt(packetType);
        } catch (IOException e) {
            System.err.println(e + " packet type");
            return false;
        }
        try {
            byte[] b = userName.getBytes("ASCII");
            for (int i = 0; i < b.length; i++) {
                stream.writeByte(b[i]);
            }
        } catch (IOException e) {
            System.err.println(e + " user name");
            return false;
        }
        try {
            for (int i = 0; i < (32 - userName.length()); i++) {
                stream.writeByte(0);
            }
        } catch (IOException e) {
            System.err.println(e + " user padding");
            return false;
        }
        try {
            stream.writeInt(encryptionType);
        } catch (IOException e) {
            System.err.println(e + " encryption type");
            return false;
        }
        try {
            stream.writeInt(packetLen);
        } catch (IOException e) {
            System.err.println(e + " packet len");
            return false;
        }
        return true;
    }

    public int getPacketLen() {
        return packetLen;
    }

    int getPacketType() {
        return packetType;
    }
}
