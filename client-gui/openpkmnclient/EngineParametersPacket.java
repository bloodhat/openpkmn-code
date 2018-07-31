/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;

/**
 *
 * @author matt
 */
class EngineParametersPacket {

    public EngineParametersPacket() {
    }
    public static final int ENGINE_PACKET_TYPE = 7;
    int port;
    String opponentName;

    public EngineParametersPacket(int port, String oName) {
        this.port = port;
        this.opponentName = oName;
    }

    public boolean receive(DataInputStream stream, EncryptionScheme eScheme,
            String userName) {

        ByteArrayInputStream payloadStream = null;

        System.err.println("about to receive stream");
        if ((payloadStream = PacketUtil.receiveStream(ENGINE_PACKET_TYPE,
                userName, eScheme, stream)) == null) {
            return false;
        }
        System.err.println("stream received");

        port = PacketUtil.get4ByteInt(payloadStream);
        byte[] b = new byte[32];

        for (int i = 0; i < 32; i++) {
            b[i] = (byte) payloadStream.read();
        }
        opponentName = (new String(b)).trim();

        return true;
    }

    public int getPort() {
        return port;
    }

    public String getOpponentName() {
        return opponentName;
    }
}
