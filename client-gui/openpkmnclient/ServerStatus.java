/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;
import java.net.*;
import java.io.*;
import java.util.List;
import java.util.ArrayList;
/**
 *
 * @author matt
 */
public class ServerStatus {
    int packetType;
    int majorStatus;
    OpponentStatus opponentStatus;
    OpponentPkmn opponentPkmn;
    List<BattlePkmnRosterItem> ownPkmn;
    List<Move> ownMoves;
    int active;
    int num_pkmn;
    int num_moves;

    public static final int SERVER_STATUS_PACKET_TYPE = 8;
    
    public boolean receive(DataInputStream stream,
			   EncryptionScheme eScheme,
			   String userName) {
	
	ByteArrayInputStream payloadStream = null;

	System.err.println("about to receive stream");
	if((payloadStream =
	     PacketUtil.receiveStream(SERVER_STATUS_PACKET_TYPE,
				      userName, eScheme, stream)) == null) {
	    return false;
	}
	System.err.println("stream received");
	
	majorStatus = payloadStream.read();
	System.err.println("got status " +
			   majorStatus);
	
	opponentStatus = new OpponentStatus();
	if(!opponentStatus.receive(payloadStream)) {
	    return false;
	}
	
	opponentPkmn = new OpponentPkmn();
	if(!opponentPkmn.receive(payloadStream)) {
	    return false;
	}
	
	active = PacketUtil.get4ByteInt(payloadStream);
	num_pkmn = PacketUtil.get4ByteInt(payloadStream);
	num_moves = PacketUtil.get4ByteInt(payloadStream);
	
	System.err.println("active pkmn is number " + active);
	
	ownPkmn = new ArrayList();
	for(int i = 0; i < num_pkmn; i++) {
	    BattlePkmnRosterItem listItem = new BattlePkmnRosterItem();
	    if(!listItem.receive(payloadStream)) {
		return false;
	    }
	    
	    ownPkmn.add(listItem);
	    
	}
	ownMoves = new ArrayList(); 
	for(int i = 0; i < num_moves; i++) {
	    Move move = new Move();
	    if(!move.receive(payloadStream)) {
		return false;
            }
	    ownMoves.add(move);
	}
	
	
	
	return true;
    }

    public int getMajorStatus() {
	return majorStatus;
    }

    public int getActivePkmn() {
	return active;
    }

    public List<BattlePkmnRosterItem> getOwnPkmn() {
	return ownPkmn;
    }

    public List<Move> getOwnMoves() {
	return ownMoves;
    }

    public OpponentStatus getOpponentStatus() {
	return opponentStatus;
    }

    public OpponentPkmn getOpponentPkmn() {
	return opponentPkmn;
    }

}
