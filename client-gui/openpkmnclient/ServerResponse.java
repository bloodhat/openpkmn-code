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
public class ServerResponse {
    public static final int ACK_CODE = 1;
    public static final int ERR_CODE = 2;
    public static final int SERVER_RESPONSE_PACKET_TYPE = 2;

    int responseCode;

    public boolean receive(DataInputStream stream,
			   EncryptionScheme eScheme,
			   String userName) {
	System.err.println("entering receive code");
	ByteArrayInputStream payloadStream = null;
	System.err.println("about to receive stream");
	if((payloadStream =
	     PacketUtil.receiveStream(SERVER_RESPONSE_PACKET_TYPE,
				      userName, eScheme, stream)) == null) {
            System.err.println("could not receive");
	    return false;
	}
	System.err.println("stream received");
	responseCode = PacketUtil.get4ByteInt(payloadStream);
	System.err.println("read response code");
	return true;
    }

    public int getResponseCode() {
	return responseCode;
    }

}
