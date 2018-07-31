/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.net.*;
import java.io.*;
/**
 *
 * @author matt
 */
public class LoginPacket {
    public static final int LOGIN_PACKET_TYPE = 1;    
    Header header;
    public LoginPacket() {
	
    }

    public boolean send(DataOutputStream stream,
			EncryptionScheme eScheme,
			String userName) {
	ByteArrayOutputStream payload = new ByteArrayOutputStream();
	
	PacketUtil.write4ByteInt(payload, LOGIN_PACKET_TYPE);

	return PacketUtil.sendStream(stream, LOGIN_PACKET_TYPE,
				     userName, eScheme, payload);
    }

}
