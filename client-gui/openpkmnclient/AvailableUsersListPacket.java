/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.util.ArrayList;
import java.io.*;

/**
 *
 * @author matt
 */
public class AvailableUsersListPacket {

    public final int AVAILABLE_USERS_LIST_PACKET_TYPE = 20;
    Header header;
    ArrayList<AvailableUser> users;

    public AvailableUsersListPacket() {

    }

    public boolean receive(DataInputStream stream,
            EncryptionScheme eScheme,
            String userName) {
        ByteArrayInputStream payloadStream = null;
        System.err.println("about to receive stream");
        if ((payloadStream =
                PacketUtil.receiveStream(AVAILABLE_USERS_LIST_PACKET_TYPE,
                userName, eScheme, stream)) == null) {
            return false;
        }

        System.err.println("stream received");
        int numUsers = PacketUtil.get4ByteInt(payloadStream);
        System.err.println("got " + numUsers + " users");
        users = new ArrayList();

        for (int i = 0; i < numUsers; i++) {
            int id =
                    PacketUtil.get4ByteInt(payloadStream);
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
            users.add(new AvailableUser(id, userName, ruleset, status,
                    teamRules, true));
        }
        return true;
    }

    public ArrayList<AvailableUser> getUsers() {
        return users;
    }
}
