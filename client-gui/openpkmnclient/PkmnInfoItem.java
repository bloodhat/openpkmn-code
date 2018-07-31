/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

/**
 *
 * @author matt
 */
public class PkmnInfoItem {
    
    private String species;
    private int id;
    public PkmnInfoItem(String s, int i) {
        id = i;
        species = s;
    }

    int getID() {
        return id;
    }

    String getSpecies() {
       return species;
    }

}
