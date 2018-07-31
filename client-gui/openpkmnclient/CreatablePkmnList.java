/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.EnumSet;
import javax.swing.JList;
import java.util.Random;
import openpkmnclient.CreatablePkmnChangedListener.CreatablePkmnChangedEvent;

/**
 *
 * @author matt
 */
public class CreatablePkmnList extends JList implements Serializable {
    Collection<CreatablePkmnChangedListener> listeners;
    Random randomGen;
    public CreatablePkmnList() {
        super(new CreatablePkmnListModel());
        randomGen = new Random();
        listeners = new ArrayList<CreatablePkmnChangedListener>();
    }

    public int getRowOfID(int id) {
        return ((CreatablePkmnListModel) getModel()).getRowOfID(id);
    }

    public int getRowID(int rowNum) {
        return ((CreatablePkmnListModel) getModel()).getRowID(rowNum);
    }

    public int getSelectedPkmnId() {
        int index = getSelectedIndex();
        int id = -1;
        if(index != -1) {
            id = ((CreatablePkmnListModel)this.getModel()).getRowID(index);
        }
        return id;
    }

    public void makeRandomSelection() {
        setSelectedIndex((int)(randomGen.nextInt(getModel().getSize())));
    }

    public void updateFullyEvolved(boolean fullyEvolved) {
        ((CreatablePkmnListModel) getModel()).updateFullyEvolved(fullyEvolved);
    }

    public void updateRules(EnumSet<Rule> rules) {
        ((CreatablePkmnListModel) getModel()).updateRules(rules);
    }

   public void addCreatablePkmnChangedListener(CreatablePkmnChangedListener l) {
        listeners.add(l);
    }

    public void removeCreatablePkmnChangedListener(CreatablePkmnChangedListener l) {
        listeners.remove(l);
    }

    private void fireCreatablePkmnUpdated(int newId) {
        /* Guard against informing listeners during construction */
        if(listeners != null) {
            for (CreatablePkmnChangedListener l : listeners) {
                l.creatablePkmnChanged(new
                        CreatablePkmnChangedEvent(this, newId));
            }
        }
    }

    @Override
    protected void fireSelectionValueChanged(int firstIndex, int lastIndex,
                                                 boolean isAdjusting) {
        super.fireSelectionValueChanged(firstIndex, lastIndex, isAdjusting);
        this.fireCreatablePkmnUpdated(this.getSelectedPkmnId());
    }

    // Getters and setters allow this to be a Java Bean
    public Collection<CreatablePkmnChangedListener> getListeners() {
        return listeners;
    }

    public void setListeners(final Collection<CreatablePkmnChangedListener>
            listeners) {
        this.listeners = listeners;
    }
}
