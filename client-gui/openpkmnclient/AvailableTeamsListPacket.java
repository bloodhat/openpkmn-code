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
public class AvailableTeamsListPacket {
    public final int AVAILABLE_TEAMS_LIST_PACKET_TYPE = 18;

    Header header;
    ArrayList<AvailableTeam> teams;
    public AvailableTeamsListPacket() {

    }

    public boolean receive(DataInputStream stream,
			   EncryptionScheme eScheme,
	         	   String userName) {
	
	
	System.err.println("entering receive code");
	ByteArrayInputStream payloadStream = null;
	System.err.println("about to receive stream");
	if((payloadStream =
	    PacketUtil.receiveStream(AVAILABLE_TEAMS_LIST_PACKET_TYPE,
				      userName, eScheme, stream)) == null) {
	    return false;
	}
	System.err.println("stream received");


	int numTeams = PacketUtil.get4ByteInt(payloadStream);
	System.err.println("got " + numTeams + " teams");
	teams = new ArrayList();
	
	for(int i = 0; i < numTeams; i++) {
	    int index =
		PacketUtil.get4ByteInt(payloadStream);
	    int rules =
		PacketUtil.get4ByteInt(payloadStream);
	    int numMembers =
		PacketUtil.get4ByteInt(payloadStream);
	    ArrayList members = new ArrayList();
	    for(int j = 0; j < numMembers; j++) {
		int member =
		    PacketUtil.get4ByteInt(payloadStream);
		members.add(new Integer(member));
	    }
	    teams.add(new AvailableTeam(members, rules, index));
	}
		
	return true;
	    
    }

    public ArrayList<AvailableTeam> getTeams() {
	return teams;
    }
}
