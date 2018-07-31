/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author matt
 */
class ChatMessagePacket {
    public final static int CHAT_MESSAGE_PACKET_TYPE = 25;
    String text;
    byte[] recipientMask;

    public ChatMessagePacket(String text, byte[] recipientMask) {
        this.text = text;
        this.recipientMask = recipientMask;
    }

    public boolean send(DataOutputStream stream, EncryptionScheme eScheme,
			String userName) {

	ByteArrayOutputStream payload = new ByteArrayOutputStream();

	PacketUtil.write4ByteInt(payload, CHAT_MESSAGE_PACKET_TYPE);
        byte[] textBytes = null;
        try {
            textBytes = text.getBytes("ASCII");
        } catch (UnsupportedEncodingException ex) {
            return false;
        }
        try {
            payload.write(recipientMask);
            payload.write(textBytes);
            payload.write(new byte[4096 - textBytes.length]);
        } catch (IOException ex) {
            return false;
        }

	return PacketUtil.sendStream(stream, CHAT_MESSAGE_PACKET_TYPE,
				     userName, eScheme, payload);
    }
}
