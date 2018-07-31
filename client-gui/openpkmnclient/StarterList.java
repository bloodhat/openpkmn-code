/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.util.List;
import java.util.ArrayList;
import java.util.Collection;
import javax.swing.JComboBox;
import openpkmnclient.StarterSelectedListener.StarterSelectedEvent;

/**
 *
 * @author matt
 */
public class StarterList extends JComboBox implements TeamAssembledListener,
    BattleModel.BattleModelChangedEventListener {

    private Collection<StarterSelectedListener> listeners;
    public StarterList() {
        super(new StarterListModel());
        listeners = new ArrayList<StarterSelectedListener>();
    }

    public void load(List<PkmnInfoItem> list) {
        ((StarterListModel) getModel()).load(list);
    }

    public int getSelectedId() {
        int index = this.getSelectedIndex();
        if (index != -1) {
            return ((StarterListModel) getModel()).getIdAt(index);
        } else {
            return -1;
        }
    }

    public void addStarterSelectedListener(StarterSelectedListener l) {
        listeners.add(l);
    }

    public void removeStarterSelectedListener(StarterSelectedListener l) {
        listeners.remove(l);
    }

    @Override
    public void setSelectedIndex(int index) {
        super.setSelectedIndex(index);
        fireStarterSelected(new StarterSelectedEvent(this, 
                ((StarterListModel)getModel()).getIdAt(index),
                ((StarterListModel)getModel()).getNameAt(index)));
    }

    public void fireStarterSelected(StarterSelectedEvent e) {
        for(StarterSelectedListener l : listeners) {
            l.starterSelected(e);
        }
    }

    @Override
    public void teamAssembled(TeamAssembledEvent e) {
        if(e.getTeam() != null) {
            ((StarterListModel) getModel()).load(e.getTeam());
            this.setEnabled(true);
        }
        else {
            this.setSelectedIndex(-1);
            this.setEnabled(false);
        }
    }

    public Collection<StarterSelectedListener> getListeners() {
        return listeners;
    }

    public void setListeners(final Collection<StarterSelectedListener>
            listeners) {
        this.listeners = listeners;
    }

    @Override
    public void battleModelChanged(BattleModelChangedEvent e) {
        switch(e.getStatus()) {
            case OpponentLoss:
            case OwnLoss:
            case Tie:
                this.setEnabled(true);
                break;
        }
    }

}
