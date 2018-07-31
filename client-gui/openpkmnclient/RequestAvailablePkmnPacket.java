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
public class RequestAvailablePkmnPacket {
public static int
	REQUEST_AVAILABLE_PACKET_TYPE = 15;
    
    Header header;
    public RequestAvailablePkmnPacket() { }

    boolean send(DataOutputStream stream,
		 EncryptionScheme eScheme,
		 String userName) {
	ByteArrayOutputStream payload = new ByteArrayOutputStream();
	
	PacketUtil.write4ByteInt(payload,
				 REQUEST_AVAILABLE_PACKET_TYPE);

	return PacketUtil.sendStream(stream, REQUEST_AVAILABLE_PACKET_TYPE,
				     userName, eScheme, payload);
    }
    
}
