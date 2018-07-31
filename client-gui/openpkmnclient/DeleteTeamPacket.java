/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;

/**
 *
 * @author matt
 */
class DeleteTeamPacket {
    public final static int DELETE_TEAM_PACKET_TYPE = 28;
    Header header;
    int id;

    public DeleteTeamPacket(int id) {
        this.id = id;
    }

    public boolean send(DataOutputStream stream, EncryptionScheme eScheme,
                        String userName) {

        ByteArrayOutputStream payload = new ByteArrayOutputStream();

        PacketUtil.write4ByteInt(payload, DELETE_TEAM_PACKET_TYPE);
        PacketUtil.write4ByteInt(payload, id);

        return PacketUtil.sendStream(stream, DELETE_TEAM_PACKET_TYPE,
                userName, eScheme, payload);
    }

}
