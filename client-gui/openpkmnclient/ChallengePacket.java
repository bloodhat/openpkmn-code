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
public class ChallengePacket {
    public final static int CHALLENGE_PACKET_TYPE = 6;
    int teamID;
    int starter;
    int opponentID;

    public ChallengePacket(int opponentID, int t, int s) {
        this.teamID = t;
        this.starter = s;
        this.opponentID = opponentID;
    }

    public boolean send(DataOutputStream stream, EncryptionScheme eScheme,
			String userName) {
	
	ByteArrayOutputStream payload = new ByteArrayOutputStream();
	
	PacketUtil.write4ByteInt(payload, CHALLENGE_PACKET_TYPE);
	PacketUtil.write4ByteInt(payload, opponentID);
	PacketUtil.write4ByteInt(payload, teamID);
    PacketUtil.write4ByteInt(payload, starter);
	
	return PacketUtil.sendStream(stream, CHALLENGE_PACKET_TYPE,
				     userName, eScheme, payload);
    }

}
