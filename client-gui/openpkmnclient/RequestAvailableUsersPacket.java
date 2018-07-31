/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.io.*;
import java.net.*;

/**
 *
 * @author matt
 */
public class RequestAvailableUsersPacket {

    public static int REQUEST_AVAILABLE_USERS_PACKET_TYPE = 19;

    public RequestAvailableUsersPacket() {
    }

    boolean send(DataOutputStream stream,
            EncryptionScheme eScheme,
            String userName) {
        ByteArrayOutputStream payload = new ByteArrayOutputStream();

        PacketUtil.write4ByteInt(payload,
                REQUEST_AVAILABLE_USERS_PACKET_TYPE);

        return PacketUtil.sendStream(stream,
                REQUEST_AVAILABLE_USERS_PACKET_TYPE,
                userName, eScheme, payload);
    }
}
