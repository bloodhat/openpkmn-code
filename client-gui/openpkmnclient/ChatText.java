/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

/**
 *
 * @author matt
 */
class ChatText {
    private String userName;
    private String text;
    public ChatText(String userName, String text) {
        this.userName = userName;
        this.text = text;
    }

    public String getUserName() {
        return userName;
    }

    public String getText() {
        return text;
    }

    public String getFormattedMessage() {
        return "" + userName + ": " + text + "\n";
    }
}
