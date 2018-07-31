/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;
import java.security.*;
import javax.crypto.*;
import javax.crypto.spec.*;
import java.math.BigInteger;

/**
 *
 * @author matt
 */
public class EncryptionScheme {

    public static final int NO_ENCRYPTION = 0;
    public static final int AES = 1;
    int encryptionType;
    Cipher ecipher, dcipher;
    Key key;
    
    public EncryptionScheme(int et, byte[] hashBytes) {
	encryptionType = et;

	System.err.println("hash is " + hashBytes.length + " bytes long");

	ecipher = null;
	try {
	    ecipher = Cipher.getInstance("AES");
	} catch(Exception e) {
	    System.err.println(e + " System does not support AES");
	}

	dcipher = null;
	try {
	    dcipher = Cipher.getInstance("AES");
	} catch(Exception e) {
	    System.err.println(e + " System does not support AES");
	}
	key = null;
	try {
	    key = new SecretKeySpec(hashBytes, "AES");
	} catch(Exception e) {
	    System.err.println(e + " System does not support AES");
	}
    }

    public int getEncryptionType() {
	return encryptionType;
    }

    byte[] encrypt(byte[] clear) {
	if(encryptionType == NO_ENCRYPTION) {
	    return clear;
	}
	else if(encryptionType == AES) {
	     try {
		ecipher.init(Cipher.ENCRYPT_MODE, key);
	    } catch(Exception e) {
		System.err.println(e + " illegal key");
	    }
	    
	    byte[] encrypted = null;
	    try {
		encrypted = ecipher.doFinal(clear);
	    } catch(Exception e) {
		System.err.println(e + " illegal block size");
	    }

	    return encrypted;
	}
	else {
	    return null;
	}
    }

    byte[] decrypt(byte[] encrypted) {
	if(encryptionType == NO_ENCRYPTION) {
	    return encrypted;
	}
	else if(encryptionType == AES) {
	    	
	    try {
		dcipher.init(Cipher.DECRYPT_MODE, key);
	    } catch(Exception e) {
		System.err.println(e + " illegal key");
	    }
	    
	    
	    System.err.println("encrypted buffer size: " + encrypted.length);
	    byte[] clear = null;
	    try {
		clear = dcipher.doFinal(encrypted);
	    } catch(Exception e) {
		System.err.println(e);
	    }

	    return clear;

	}
	else {
	    return null;
	}
    }
}
