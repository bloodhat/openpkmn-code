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
public class BattleNowPacket {

    public final static int BATTLE_NOW_PACKET_TYPE = 29;
    int teamID;
    int starter;

    public BattleNowPacket(int t, int s) {
        teamID = t;
        starter = s;
    }

    public boolean send(DataOutputStream stream, EncryptionScheme eScheme,
            String userName) {
        ByteArrayOutputStream payload = new ByteArrayOutputStream();

        PacketUtil.write4ByteInt(payload, BATTLE_NOW_PACKET_TYPE);
        PacketUtil.write4ByteInt(payload, teamID);
        PacketUtil.write4ByteInt(payload, starter);

        return PacketUtil.sendStream(stream, BATTLE_NOW_PACKET_TYPE,
                userName, eScheme, payload);
    }
}
