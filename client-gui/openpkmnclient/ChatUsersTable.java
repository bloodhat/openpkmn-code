/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.io.Serializable;
import java.util.List;
import javax.swing.JTable;

/**
 *
 * @author matt
 */
public class ChatUsersTable extends JTable implements Serializable {

    public ChatUsersTable() {
        super(new ChatUsersTableModel());
    }

    public void load(List<AvailableUser> users) {
        ((ChatUsersTableModel) getModel()).load(users);
    }


    public String getNameById(int id) {
        return ((ChatUsersTableModel) getModel()).getNameById(id);
    }

    public String getRulesById(int id) {
        return ((ChatUsersTableModel) getModel()).getRulesById(id);
    }

    public String getTeamRulesById(int id) {
        return ((ChatUsersTableModel) getModel()).getTeamRulesById(id);
    }

    void updateUser(AvailableUser u) {
        ((ChatUsersTableModel) getModel()).updateUser(u);
    }

    boolean isIgnored(String userName) {
         return ((ChatUsersTableModel) getModel()).isIgnored(userName);
    }

    List<Integer> getRecipientIds() {
        return ((ChatUsersTableModel) getModel()).getRecipientIds();
    }
}
