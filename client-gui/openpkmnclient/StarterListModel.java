/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.ArrayList;
import java.util.List;
import javax.swing.DefaultComboBoxModel;

/**
 *
 * @author matt
 */

    public class StarterListModel extends DefaultComboBoxModel  {

        private List<PkmnInfoItem> items;
        private ArrayList<String> names = new ArrayList<String>();

        public void load(List<PkmnInfoItem> list) {
            items = list;
            names = new ArrayList<String>();
            for (PkmnInfoItem item : list) {
                names.add(item.getSpecies() + " (" + item.getID() + ")");
            }
            this.setSelectedItem(null);
            this.fireContentsChanged(this, 0, list.size());
        }

        @Override
        public int getSize() {
            return names.size();
        }

        @Override
        public Object getElementAt(int index) {
            if(index >= 0) {
                return names.get(index);
            } else {
                return "";
            }
        }

        public int getIdAt(int index) {
            if(index >= 0) {
                return items.get(index).getID();
            } else {
                return -1;
            }
        }

        public String getNameAt(int index) {
            if(index >= 0) {
                return items.get(index).getSpecies();
            } else {
                return "";
            }
        }

    }
