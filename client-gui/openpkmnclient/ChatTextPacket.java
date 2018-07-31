/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;

/**
 *
 * @author matt
 */
class ChatTextPacket {

    public static final int CHAT_TEXT_PACKET_TYPE = 26;
    private String sender;
    private String text;

    public boolean receive(DataInputStream stream,
            EncryptionScheme eScheme,
            String userName) {
        System.err.println("entering receive code");
        ByteArrayInputStream payloadStream = null;
        System.err.println("about to receive stream");
        if ((payloadStream =
                PacketUtil.receiveStream(CHAT_TEXT_PACKET_TYPE,
                userName, eScheme, stream)) == null) {
            System.err.println("could not receive");
            return false;
        }
        return process(payloadStream);
    }

    public String getText() {
        return text;
    }

    public String getUserName() {
        return sender;
    }

    boolean receiveHeadless(int payloadLen, DataInputStream stateInputStream,
            EncryptionScheme eScheme, String userName) {
        ByteArrayInputStream payloadStream = null;
        System.err.println("about to receive stream");
        if ((payloadStream =
                PacketUtil.receiveHeadlessStream(CHAT_TEXT_PACKET_TYPE,
                payloadLen, userName, eScheme, stateInputStream)) == null) {
            System.err.println("could not receive");
            return false;
        }
        return process(payloadStream);
    }

    boolean process(ByteArrayInputStream payloadStream) {
        byte userNameBytes[] = new byte[32];
        byte messageBytes[] = new byte[4096];

        try {
            payloadStream.read(userNameBytes);
            payloadStream.read(messageBytes);
        } catch (IOException ex) {
            System.err.println(ex.getLocalizedMessage());
            return false;
        }
        sender = new String(userNameBytes).trim();
        text = new String(messageBytes).trim();
        return true;
    }
}
