/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.ConnectException;
import java.net.Socket;
import java.net.UnknownHostException;

/**
 *
 * @author matt
 */
public class OpenpkmnNetworkController {
    protected String userName;
    protected EncryptionScheme eScheme;
    protected String serverName;
    protected int port;

    public boolean setConnectionInfo(Credentials cred) {
        // read user name from credentials
        userName = cred.getUserName();
        // load the password into the encryption scheme
        eScheme = new EncryptionScheme(EncryptionScheme.AES,
                cred.getHashBytes());
        serverName = cred.getServer();
        port = cred.getPort();

        return true;
    }

    protected Socket connect(String serverName, int port) {
        Socket sock = null;
        try {
            sock = new Socket(serverName, port);

        } catch (UnknownHostException e) {
            System.err.println("Unknown host. " + e);

        } catch (ConnectException e) {
            System.err.println("Could not connect. " + e);

        } catch (IOException e) {
            System.err.println("IOException:login " + e);

        }
        return sock;
    }

    protected DataOutputStream getOutputStream(Socket s) {
        try {
            return new DataOutputStream(s.getOutputStream());
        } catch (IOException ex) {
            return null;
        }

    }

    protected DataInputStream getInputStream(Socket s) {
        try {
            return new DataInputStream(s.getInputStream());
        } catch (IOException ex) {
            return null;
        }
    }

}
