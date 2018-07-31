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
public class BattlePkmnRosterItem {

    int number;
    int level;
    int status;
    int currentHP;
    int maxHP;
    String nickname;

    public boolean receive(ByteArrayInputStream payloadStream) {
        number = payloadStream.read();
        if (number < 0) {
            number += 256;
        }
        level = payloadStream.read();

        status = payloadStream.read();
        payloadStream.read();
        currentHP = PacketUtil.get2ByteInt(payloadStream);
        maxHP = PacketUtil.get2ByteInt(payloadStream);

        byte b[] = new byte[32];
        /* read 32 bytes of string */
        for (int i = 0; i < 32; i++) {
            b[i] = (byte) payloadStream.read();
        }
        nickname = (new String(b)).trim();

        return true;
    }

    public String getNickname() {
        return nickname;
    }

    public int getNumber() {
        return number;
    }

    public String getName() {
        return Lexicon.pkmnNames[number];
    }

    public int getLevel() {
        return level;
    }

    public int getStatus() {
        return status;
    }

    public int getMaxHP() {
        return maxHP;
    }

    public int getCurrentHP() {
        return currentHP;
    }

    public double getLifePercent() {
        return (double) currentHP /
                (double) maxHP;
    }
}
