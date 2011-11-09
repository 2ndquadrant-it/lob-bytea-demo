/**
 *  Copyright (C) 2011  2ndQuadrant Italia
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */
package it.secondquadrant.lotest.interfaces;

import java.math.BigInteger;

/**
 * Inteface for the Service classes
 * 
 * @author Giulio Calacoci <giulio.calacoci@2ndQuadrant.it>
 */
public interface ServiceInterface {

    public BigInteger importFile(String dsn, String filename) throws Exception;

    public void exportFile(String dsn, BigInteger oid, String filename);

    public void removeFile(String dsn, BigInteger oid) throws Exception;

    public void listFiles(String dsn);
}
