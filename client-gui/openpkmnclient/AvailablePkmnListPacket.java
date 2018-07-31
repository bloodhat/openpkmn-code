/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;
import java.util.ArrayList;
import java.net.*;
import java.io.*;

/**
 *
 * @author matt
 */
public class AvailablePkmnListPacket {
    public final int AVAILABLE_PKMN_LIST_PACKET_TYPE = 14;

    ArrayList<AvailablePkmn> pkmn;
    public AvailablePkmnListPacket() {

    }

    public boolean receive(DataInputStream stream,
			   EncryptionScheme eScheme,
	         	   String userName) {
	
	System.err.println("entering receive code");
	ByteArrayInputStream payloadStream = null;
	System.err.println("about to receive stream");
	if((payloadStream =
	     PacketUtil.receiveStream(AVAILABLE_PKMN_LIST_PACKET_TYPE,
				      userName, eScheme, stream)) == null) {
	    return false;
	}
	System.err.println("stream received");
	int numPkmn = PacketUtil.get4ByteInt(payloadStream);
	System.err.println("got " + numPkmn + " pkmn");
	pkmn = new ArrayList();
	
	for(int i = 0; i < numPkmn; i++) {
	    int index =
		PacketUtil.get4ByteInt(payloadStream);
	    int pkmnId =
		PacketUtil.get4ByteInt(payloadStream);
	    int rules =
		PacketUtil.get4ByteInt(payloadStream);
	    int numMoves =
		PacketUtil.get4ByteInt(payloadStream);
	    ArrayList moves = new ArrayList();
	    for(int j = 0; j < numMoves; j++) {
		int move = PacketUtil.get4ByteInt(payloadStream);
		moves.add(new Integer(move));
	    }
	    pkmn.add(new AvailablePkmn(pkmnId, moves, rules, index));
	}
		
	return true;
	    
    }

    public ArrayList<AvailablePkmn> getPkmn() {
	return pkmn;
    }

}
