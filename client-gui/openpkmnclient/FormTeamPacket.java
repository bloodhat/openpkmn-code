/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.io.*;
import java.util.Collection;

/**
 *
 * @author matt
 */
class FormTeamPacket {

    public static final int FORM_TEAM_PACKET_TYPE = 17;
    Header header;
    int numMembers;
    Collection<Integer> members;

    public FormTeamPacket(Collection<Integer> m) {
        members = m;
        numMembers = m.size();
    }

    public boolean send(DataOutputStream stream,
                        EncryptionScheme eScheme,
                        String userName) {

        ByteArrayOutputStream payload = new ByteArrayOutputStream();

        PacketUtil.write4ByteInt(payload, FORM_TEAM_PACKET_TYPE);
        PacketUtil.write4ByteInt(payload, numMembers);
        for (Integer i : members) {
            PacketUtil.write4ByteInt(payload, i);
        }

        return PacketUtil.sendStream(stream, FORM_TEAM_PACKET_TYPE,
                userName, eScheme, payload);
    }
}
