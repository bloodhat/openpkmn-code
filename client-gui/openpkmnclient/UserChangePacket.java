/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;

/**
 *
 * @author matt
 */
class UserChangePacket {

    public static final int USER_CHANGE_PACKET_TYPE = 31;
    Header header;
    AvailableUser user;

    public UserChangePacket() {
    }

    public boolean receiveHeadless(int payloadLen, DataInputStream stream,
            EncryptionScheme eScheme, String userName) {
        ByteArrayInputStream payloadStream = null;
        System.err.println("about to receive stream");
        if ((payloadStream =
                PacketUtil.receiveHeadlessStream(USER_CHANGE_PACKET_TYPE,
                payloadLen, userName, eScheme, stream)) == null) {
            return false;
        }

        return process(payloadStream);
    }

    public boolean receive(DataInputStream stream, EncryptionScheme eScheme,
            String userName) {
        ByteArrayInputStream payloadStream = null;
        System.err.println("about to receive stream");
        if ((payloadStream =
                PacketUtil.receiveStream(USER_CHANGE_PACKET_TYPE,
                userName, eScheme, stream)) == null) {
            return false;
        }

        return process(payloadStream);
    }

    public boolean process(ByteArrayInputStream payloadStream) {
        String userName;
        int id = PacketUtil.get4ByteInt(payloadStream);
        byte b[] = new byte[32];
        /* read 32 bytes of string */
        for (int j = 0; j < 32; j++) {
            b[j] = (byte) (payloadStream.read());
        }
        userName = (new String(b)).trim();
        int ruleset =
                PacketUtil.get4ByteInt(payloadStream);
        int status =
                PacketUtil.get4ByteInt(payloadStream);
        int teamRules =
                PacketUtil.get4ByteInt(payloadStream);
        boolean isNew = (PacketUtil.get4ByteInt(payloadStream) == 1);
        user = new AvailableUser(id, userName, ruleset, status, teamRules,
                isNew);
        return true;
    }

    public AvailableUser getUser() {
        return user;
    }
}
