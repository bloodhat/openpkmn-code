/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import javax.swing.table.AbstractTableModel;
import openpkmnclient.MainModel.Status;

/**
 *
 * @author matt
 */
public class ChatUsersTableModel extends AbstractTableModel {

        private String[] columnNames = { "Name", "Ignored", "Recipient"};
        private Class[] columnClasses = { String.class, Boolean.class,
            Boolean.class};
        private HashMap<String, ArrayList> data;

        public ChatUsersTableModel() {
            data = new HashMap<String, ArrayList>();
            for (int i = 0; i < columnNames.length; i++) {
                data.put(columnNames[i], new ArrayList());
            }
            data.put("ID", new ArrayList());
            data.put("Status", new ArrayList());
            data.put("Rules", new ArrayList());
            data.put("Team Rules", new ArrayList());
        }

        @Override
        public int getColumnCount() {
            return columnNames.length;
        }

        @Override
        public int getRowCount() {
            return data.get("Name").size();
        }

        @Override
        public String getColumnName(int col) {
            return columnNames[col];
        }

        @Override
        public Object getValueAt(int row, int col) {
            List column = data.get(columnNames[col]);
            if(column != null && row < column.size()) {
                Object value = column.get(row);
                return value;
            }
            return null;
        }

        @Override
        public Class getColumnClass(int c) {
            return columnClasses[c];
        }

        @Override
        public boolean isCellEditable(int row, int col) {
            return (col == 1) || (col == 2);
        }

        public int getRowId(int row) {
            return (Integer) data.get("ID").get(row);
        }

        public int getRowById(int id) {
            ArrayList<Integer> ids = data.get("ID");
            for (int i = 0; i < ids.size(); i++) {
                if (id == ids.get(i)) {
                    return i;
                }
            }
            return -1;
        }

        public String getNameById(int id) {
            int row = getRowById(id);
            if (row != -1) {
                return (String) data.get("Name").get(row);
            }
            return null;
        }

        public String getRulesById(int id) {
            int row = getRowById(id);
            if (row != -1) {
                return (String) data.get("Rules").get(row);
            }
            return null;
        }

    @Override
    public void setValueAt(Object value, int row, int col) {
        data.get(columnNames[col]).set(row, value);
        fireTableCellUpdated(row, col);
    }

        public String getTeamRulesById(int id) {
            int row = getRowById(id);
            if (row != -1) {
                return (String) data.get("Team Rules").get(row);
            }
            return null;
        }

        public Status getStatusById(int id) {
            int row = getRowById(id);
            if (row != -1) {

                return (Status)data.get("Status").get(row);
            }
            return null;
        }

        public void load(List<AvailableUser> users) {
            data = new HashMap<String, ArrayList>();
            for (int i = 0; i < columnNames.length; i++) {
                data.put(columnNames[i], new ArrayList());
            }
            data.put("ID", new ArrayList());
            data.put("Status", new ArrayList());
            data.put("Rules", new ArrayList());
            data.put("Team Rules", new ArrayList());
            for (AvailableUser user : users) {
                data.get("Name").add(user.getUserName());
                data.get("Rules").add(Rule.getCodeFromValue(user.getRuleset()));
                Status status = Status.values()[user.getStatus()];
                data.get("Status").add(status);
                String s = Rule.getCodeFromValue(user.getTeamRules());
                data.get("Team Rules").add(s);
                data.get("ID").add(user.getID());
                data.get("Ignored").add(Boolean.FALSE);
                data.get("Recipient").add(Boolean.FALSE);
            }
            this.fireTableDataChanged();
        }

    List<Integer> getRecipientIds() {
        List<Integer> ids = new ArrayList<Integer>();
        List<Boolean> isRecipientList = (List<Boolean>)data.get("Recipient");
        List<Integer> idList = (List<Integer>)data.get("ID");
        for(int i= 0; i < isRecipientList.size(); i++) {
            if(isRecipientList.get(i)) {
                ids.add(idList.get(i));
            }
        }
        return ids;
    }

    boolean isIgnored(String userName) {
        List<String> names = data.get("Name");
        int index = names.indexOf(userName);
        if(index != -1) {
            List<Boolean> ignoredList = (List<Boolean>)data.get("Ignored");
            return ignoredList.get(index);
        } else {
            return false;
        }
    }

        public void updateUser(AvailableUser user) {
            int updateIndex = data.get("ID").indexOf(user.getID());
             if(updateIndex == -1) {
                    data.get("Name").add(user.getUserName());
                data.get("Rules").add(Rule.getCodeFromValue(user.getRuleset()));
                Status status = Status.values()[user.getStatus()];
                data.get("Status").add(status);
                String s = Rule.getCodeFromValue(user.getTeamRules());
                data.get("Team Rules").add(s);
                data.get("ID").add(user.getID());
                data.get("Ignored").add(Boolean.FALSE);
                data.get("Recipient").add(Boolean.FALSE);
                }
             else {
                Status status = Status.values()[user.getStatus()];
                if(!status.equals(Status.LOGGED_OUT)) {
                    data.get("Name").set(updateIndex, user.getUserName());
                    data.get("Rules").set(updateIndex,
                        Rule.getCodeFromValue(user.getRuleset()));
                
                    data.get("Status").set(updateIndex, status);
                    String s = Rule.getCodeFromValue(user.getTeamRules());
                    data.get("Team Rules").set(updateIndex, s);
                } else {
                    for(String s : data.keySet()) {
                        data.get(s).remove(updateIndex);
                    }
                }
            }
            this.fireTableDataChanged();
        }
    }
