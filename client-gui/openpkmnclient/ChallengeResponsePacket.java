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
public class ChallengeResponsePacket {
    public final static int CHALLENGE_RESPONSE_PACKET_TYPE = 23;
    int accepted;

    public ChallengeResponsePacket(boolean a) {
	accepted = a ? 1 : 0;
    }

    ChallengeResponsePacket() {
        
    }

    public boolean send(DataOutputStream stream, EncryptionScheme eScheme,
			String userName) {
	
	ByteArrayOutputStream payload = new ByteArrayOutputStream();
	
	PacketUtil.write4ByteInt(payload, CHALLENGE_RESPONSE_PACKET_TYPE);
	PacketUtil.write4ByteInt(payload, accepted);
	
	return PacketUtil.sendStream(stream, CHALLENGE_RESPONSE_PACKET_TYPE,
				     userName, eScheme, payload);
    }
    
    public boolean receive(DataInputStream stream,
			   EncryptionScheme eScheme,
			   String userName) {
	System.err.println("entering receive code");
	ByteArrayInputStream payloadStream = null;
	System.err.println("about to receive stream");
	if((payloadStream =
	     PacketUtil.receiveStream(CHALLENGE_RESPONSE_PACKET_TYPE,
				      userName, eScheme, stream)) == null) {
            System.err.println("could not receive");
	    return false;
	}
	System.err.println("stream received");
	accepted = PacketUtil.get4ByteInt(payloadStream);
	System.err.println("read response code");
	return true;
    }
    
    public boolean getAcceptance() {
        return accepted == 1;
    }

}