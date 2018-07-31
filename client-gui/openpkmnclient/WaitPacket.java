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
public class WaitPacket {

    public final static int WAIT_PACKET_TYPE = 5;
    int teamID;
    int starter;

    public WaitPacket(int t, int s) {
        teamID = t;
        starter = s;
    }

    public boolean send(DataOutputStream stream, EncryptionScheme eScheme,
            String userName) {
        ByteArrayOutputStream payload = new ByteArrayOutputStream();

        PacketUtil.write4ByteInt(payload, WAIT_PACKET_TYPE);
        PacketUtil.write4ByteInt(payload, teamID);
        PacketUtil.write4ByteInt(payload, starter);

        return PacketUtil.sendStream(stream, WAIT_PACKET_TYPE,
                userName, eScheme, payload);
    }
}
