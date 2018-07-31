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
public class ClientResponse {
public final static int CLIENT_RESPONSE_PACKET_TYPE = 10;
    int command;
    int option;
    public ClientResponse(int command, int option) {
	this.command = command;
	this.option = option;
    }

    public boolean send(DataOutputStream stream,
			EncryptionScheme eScheme,
			String userName) {
	ByteArrayOutputStream payload = new ByteArrayOutputStream();
	
	PacketUtil.write4ByteInt(payload, CLIENT_RESPONSE_PACKET_TYPE);
	PacketUtil.write2ByteInt(payload, command);
	PacketUtil.write2ByteInt(payload, option);
	
	return PacketUtil.sendStream(stream, CLIENT_RESPONSE_PACKET_TYPE,
				     userName, eScheme, payload);
	
    }
}
