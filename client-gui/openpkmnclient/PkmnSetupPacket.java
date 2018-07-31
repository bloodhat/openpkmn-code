/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;
import java.net.*;
import java.io.*;
import java.util.Collection;
import java.util.List;

/**
 *
 * @author matt
 */
public class PkmnSetupPacket {
       public static final int PKMN_SETUP_PACKET_TYPE = 3;
    
    String nickname;
    int number;
    int level;
    int maxHP;
    
    int attack;
    int defense;
    int speed;
    int special;

    Collection<Move> moves;
    int numMoves;
    
    
    public PkmnSetupPacket(String nickname, int number, int level,
		int[] stats, Collection<Move> moves) {
	this.nickname = nickname;
	this.number = number;
	this.level = level;

	numMoves = moves.size();
	System.out.println(numMoves + " moves to pack");
	this.moves = moves;
	
	this.maxHP = stats[0];
	this.attack = stats[1];
	this.defense = stats[2];
	this.speed = stats[3];
	this.special = stats[4];

    }

    public boolean send(DataOutputStream stream, EncryptionScheme eScheme,
			String userName) {

	ByteArrayOutputStream payload = new ByteArrayOutputStream();
	
	PacketUtil.write4ByteInt(payload, PKMN_SETUP_PACKET_TYPE);
	byte[] b;
	try {
	     b = nickname.getBytes("ASCII");
	} catch (UnsupportedEncodingException e) {
	    System.err.println(e + "ASCII not supported on this system??");
	    return false;
	}
	for(int i = 0; i < b.length; i++) {
	    payload.write(b[i]);
	}
	for(int i = 0; i < 32 - (nickname.length()); i++) {
	    payload.write((byte)0);
	}
	
	payload.write(number);
	payload.write(level);
	PacketUtil.write2ByteInt(payload, maxHP);
	PacketUtil.write2ByteInt(payload, attack);
	PacketUtil.write2ByteInt(payload, defense);
	PacketUtil.write2ByteInt(payload, speed);
	PacketUtil.write2ByteInt(payload, special);
	PacketUtil.write4ByteInt(payload, numMoves);
	
	
	for(Move move : moves) {
	    PacketUtil.write4ByteInt(payload, move.getNumber());
	    PacketUtil.write4ByteInt(payload, move.getPP());
	}

	return PacketUtil.sendStream(stream, PKMN_SETUP_PACKET_TYPE,
				     userName, eScheme, payload);
    }
}
