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
public class ChallengeNotificationPacket {
    public final static int CHALLENGE_NOTIFICATION_PACKET_TYPE = 22;
    int challengerID;

    public boolean receive(DataInputStream stream,
            EncryptionScheme eScheme,
            String userName) {
        System.err.println("entering receive code");
        ByteArrayInputStream payloadStream = null;
        System.err.println("about to receive stream");
        if ((payloadStream =
                PacketUtil.receiveStream(CHALLENGE_NOTIFICATION_PACKET_TYPE,
                userName, eScheme, stream)) == null) {
            System.err.println("could not receive");
            return false;
        }
        System.err.println("stream received");
        challengerID = PacketUtil.get4ByteInt(payloadStream);
        System.err.println("read response code");
        return true;
    }

    public int getChallengerID() {
        return challengerID;
    }
}