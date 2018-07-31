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
public class OpponentStatus {
int faintedCount;
    int statusCount;
    int normalCount;

    public boolean receive(ByteArrayInputStream payloadStream) {
	faintedCount = payloadStream.read();
	statusCount = payloadStream.read();
	normalCount = payloadStream.read();
	return true;
    }
    public int getFainted() {
	return faintedCount;
    }
    public int getStatus() {
	return statusCount;
    }
    public int getNormal() {
	return normalCount;
    }
}
