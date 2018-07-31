/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.io.*;

public class LogoutPacket {
    public static final int LOGOUT_PACKET_TYPE = 11;
    
    Header header;
    public LogoutPacket() {
	
    }

    public boolean send(DataOutputStream stream,
			EncryptionScheme eScheme,
			String userName) {

	ByteArrayOutputStream payload = new ByteArrayOutputStream();
	
	PacketUtil.write4ByteInt(payload, LOGOUT_PACKET_TYPE);

	return PacketUtil.sendStream(stream, LOGOUT_PACKET_TYPE,
				     userName, eScheme, payload);
    }
}
