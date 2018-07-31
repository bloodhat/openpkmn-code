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
class DeletePkmnPacket {
    public final static int DELETE_PKMN_PACKET_TYPE = 27;
    Header header;
    int id;

    public DeletePkmnPacket(int id) {
        this.id = id;
    }

    public boolean send(DataOutputStream stream, EncryptionScheme eScheme,
                        String userName) {

        ByteArrayOutputStream payload = new ByteArrayOutputStream();

        PacketUtil.write4ByteInt(payload, DELETE_PKMN_PACKET_TYPE);
        PacketUtil.write4ByteInt(payload, id);

        return PacketUtil.sendStream(stream, DELETE_PKMN_PACKET_TYPE,
                userName, eScheme, payload);
    }

}
