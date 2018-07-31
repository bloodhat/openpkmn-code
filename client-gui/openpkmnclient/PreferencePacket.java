/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;
import java.io.*;
import java.net.*;
import java.util.List;
/**
 *
 * @author matt
 */
public class PreferencePacket {
    public final static int PREFERENCE_PACKET_TYPE = 21;
    int ruleValue;

    public PreferencePacket(int r) {
	ruleValue = r;
    }

    public boolean send(DataOutputStream stream, EncryptionScheme eScheme,
		    String userName) {
	
	ByteArrayOutputStream payload = new ByteArrayOutputStream();
	
	PacketUtil.write4ByteInt(payload, PREFERENCE_PACKET_TYPE);
	PacketUtil.write4ByteInt(payload, ruleValue);

	return PacketUtil.sendStream(stream, PREFERENCE_PACKET_TYPE,
				     userName, eScheme, payload);
    }
}
