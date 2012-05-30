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
package it.secondquadrant.lotest.service;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;

/**
 *
 * @author Giulio Calacoci <giulio.calacoci@2ndQuadrant.it>
 */
public class BaseService {
    /**
     * Open a connection with the database, using jdbc
     * @param dsn the connection string
     * @return a connection object
     * @throws Exception if fails to connect
     */
    protected Connection openConnection(String dsn) throws Exception {
        Connection con = null;
        try {
            Class.forName("org.postgresql.Driver");
            String[] cstring = dsn.split(",");
            String pwd = (cstring.length < 3) ? "" : cstring[2];
            con = DriverManager.getConnection("jdbc:postgresql://" + cstring[0], cstring[1], pwd);
            System.out.println("DSN Connection ok.");
        } catch (Exception e) {
            throw new Exception("Unable To connect");
        }
        try {
            PreparedStatement ps = con.prepareStatement("CREATE TABLE audio_catalog ( "
                    + " id SERIAL,     "
                    + " filename TEXT, "
                    + " mode TEXT, "
                    + " therealdata BYTEA, "
                    + " objid oid )");
            ps.execute();
            ps.close();
        } catch (Exception e){}
        return con;
    }
    /**
     * Lists the Large Objects stored on the database.
     * @param dsn the connection string
     * @throws Exception  if connection fails
     */
    protected void listObjects(String dsn) throws Exception {
        Connection conn = this.openConnection(dsn);
        PreparedStatement ps = conn.prepareStatement("SELECT id,mode,filename FROM audio_catalog");
        ResultSet rs = ps.executeQuery();

        System.out.println(" ID     | Type  | Filename \n--------+-------+----------\n");
        while (rs.next()) {
            System.out.printf(" %-6s ", rs.getString(1));
            System.out.printf("| %-6s", rs.getString(2));
            System.out.printf("| %s\n", rs.getString(3));
        }
        ps.close();
        System.out.println("--------+-------+-----------\n");
    }
}
