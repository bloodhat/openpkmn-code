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
public class PacketUtil {

    public static int get4ByteInt(ByteArrayInputStream payload) {
        int retVal = 0;
        for (int i = 0; i < 4; i++) {
            int readByte = payload.read();
            retVal += (readByte << (8 * (3 - i)));
        }
        return retVal;
    }

    public static int get2ByteInt(ByteArrayInputStream payload) {
        int retVal = 0;
        for (int i = 0; i < 2; i++) {
            int readByte = payload.read();
            retVal += (readByte << (8 * (1 - i)));
        }
        return retVal;
    }

    public static void write4ByteInt(ByteArrayOutputStream payload,
            int outputData) {
        payload.write((outputData & 0xff000000) >> 24);
        payload.write((outputData & 0x00ff0000) >> 16);
        payload.write((outputData & 0x0000ff00) >> 8);
        payload.write((outputData & 0x000000ff) >> 0);
    }

    public static void write2ByteInt(ByteArrayOutputStream payload,
            int outputData) {
        payload.write((outputData & 0x0000ff00) >> 8);
        payload.write((outputData & 0x000000ff) >> 0);
    }

    public static boolean sendStream(DataOutputStream stream,
            int packetType, String userName,
            EncryptionScheme eScheme,
            ByteArrayOutputStream payload) {
        byte[] encryptedPayload = eScheme.encrypt(payload.toByteArray());

        Header header = new Header(packetType, userName,
                eScheme.getEncryptionType(),
                encryptedPayload.length);

        header.send(stream);

        try {
            stream.write(encryptedPayload, 0, encryptedPayload.length);
        } catch (IOException e) {
            System.err.println(e + "Logout.stream");
            return false;
        }

        return true;
    }

    public static ByteArrayInputStream receiveHeadlessStream(
            int packetType, int payloadLen, String userName,
            EncryptionScheme eScheme, DataInputStream stream) {
        System.err.println("about to receive payload");
        byte[] payload = new byte[payloadLen];
        try {
            for (int i = 0; i < payloadLen; i++) {
                payload[i] = stream.readByte();
            }
        } catch (IOException e) {
            System.err.println(e);
            return null;
        }

        System.err.println("payload received");

        System.err.println("about to decrypt");
        byte[] decryptedPayloadBytes = eScheme.decrypt(payload);
        System.err.println("decrypted");


        ByteArrayInputStream decryptedPayloadStream;
        decryptedPayloadStream =
                new ByteArrayInputStream(decryptedPayloadBytes);

        int claimedPacketType =
                PacketUtil.get4ByteInt(decryptedPayloadStream);
        System.err.println("got internal packet type");

        if (packetType == claimedPacketType) {
            System.err.println("verified!");
            return decryptedPayloadStream;
        } else {
            System.err.println("failed to get proper packet type: expecting " + packetType + " got " + claimedPacketType);
            return null;
        }
    }

    public static ByteArrayInputStream receiveStream(int packetType,
            String userName, EncryptionScheme eScheme, DataInputStream stream) {
        Header header = new Header();
        System.err.println("about to receive header");
        if (header.receive(stream)) {
            System.err.println("received header");
            return receiveHeadlessStream(header.getPacketType(),
                    header.getPacketLen(), userName, eScheme, stream);
        }
        return null;
    }
}
