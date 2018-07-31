/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;
import java.util.ArrayList;
import java.io.*;
/**
 *
 * @author matt
 */
public class EventLog {
    public static final int EVENT_LOG_PACKET_TYPE = 9;
    ArrayList<Integer> events;
    int packetType;
    public boolean receive(DataInputStream stream,
			   EncryptionScheme eScheme,
			   String userName) {
	System.err.println("Constructing event log");
	int event = -1;
	events = new ArrayList();
	
	ByteArrayInputStream payloadStream = null;
	System.err.println("about to receive stream");
	if((payloadStream =
	    PacketUtil.receiveStream(EVENT_LOG_PACKET_TYPE,
				     userName, eScheme, stream)) == null) {
	    return false;
	}
	System.err.println("stream received");

		    
	System.err.println("packet type is " + packetType);
	while(true) {
	    event = PacketUtil.get2ByteInt(payloadStream);
	    System.err.println("our unaltered event is " +
			       event);
	    if(event < 0) {
		event += ((Short.MAX_VALUE + 1) * 2);
	    }
	    
	    System.err.println("our altered event is " +
			       event);

	    if(event == Lexicon.END_EVENTS) {
		break;
	    }
	    else {
		events.add(new Integer(event));
	    }
	    
	}
	return true;
    }


    public String getText(String ownName,
			  String opponentName,
			  String ownPkmnName,
			  String opponentPkmnName) {
	String text = "";
	
	for(int i = 0; i < events.size(); i++) {
	    /* get the next 2 events */
	    text += processEvent(events.get(i).intValue(),
				 ownName, opponentName,
				 ownPkmnName,opponentPkmnName) + "\n";
	    }
	
	return text;
    }

    private String processEvent(int event,
				String ownName, String opponentName,
				String ownPkmnName,
				String opponentPkmnName) {
	boolean ownMessage = ((event & 0x0100) == 0x0100);
	int messageNumber = (event & 0xfe00) >> 9;
	int details = event & 0x00ff;
	/*
	System.out.println("message number " + messageNumber +
			   " user is subject " + ownMessage +
			   " details " + details);*/
	
	String s = Lexicon.eventMessages[messageNumber];
	
	String subject, object, subPkmn, objPkmn;
	if(ownMessage) {
	    subject = ownName;
	    object = opponentName;
	    subPkmn = ownPkmnName;
	    objPkmn = opponentPkmnName;
	}
	else {
	    subject = opponentName;
	    object = ownName;
	    subPkmn = opponentName + "'s " + opponentPkmnName;
	    objPkmn = ownPkmnName;
	}

	int i;
	
	/* replace all '@' inserts with their values */
	while((i = s.indexOf('@')) != -1) {
	    switch(s.charAt(i + 1)) {
	    case 'u':
		s = s.substring(0, i) + subject +
		    s.substring(i + 2);
		break;
	    case 'p':
		s = s.substring(0, i) + subPkmn +
		    s.substring(i + 2);
		break;
	    case 'o':
		s = s.substring(0, i) + objPkmn +
		    s.substring(i + 2);
		break;
	    case 'm':
		s = s.substring(0, i) + Lexicon.moveNames[details] +
		    s.substring(i + 2);
		break;
	    case 'i':
		s = s.substring(0, i) + details +
		    s.substring(i + 2);
		break;
	    default:
		System.err.println("unrecoginzed case " + s.charAt(i + 1) +
				   " in " + s);
		break;
	    }
	}
	return s;
    }
		   
}
