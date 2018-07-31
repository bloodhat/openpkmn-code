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
public class RequestAvailableTeamsPacket {
    public static int
	REQUEST_AVAILABLE_TEAMS_PACKET_TYPE = 16;
    
    Header header;
    public RequestAvailableTeamsPacket() { }

    boolean send(DataOutputStream stream,
		 EncryptionScheme eScheme,
		 String userName) {
	ByteArrayOutputStream payload = new ByteArrayOutputStream();
	
	PacketUtil.write4ByteInt(payload,
				 REQUEST_AVAILABLE_TEAMS_PACKET_TYPE);

	return PacketUtil.sendStream(stream,
				     REQUEST_AVAILABLE_TEAMS_PACKET_TYPE,
				     userName, eScheme, payload);
    }
    
}
