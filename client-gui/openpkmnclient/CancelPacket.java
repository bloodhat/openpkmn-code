/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;

/**
 *
 * @author matt
 */
public class CancelPacket {
public static final int CANCEL_PACKET_TYPE = 24;

    Header header;
    public CancelPacket() {

    }

    public boolean send(DataOutputStream stream,
			EncryptionScheme eScheme,
			String userName) {

	ByteArrayOutputStream payload = new ByteArrayOutputStream();

	PacketUtil.write4ByteInt(payload, CANCEL_PACKET_TYPE);

	return PacketUtil.sendStream(stream, CANCEL_PACKET_TYPE,
				     userName, eScheme, payload);
    }
}
