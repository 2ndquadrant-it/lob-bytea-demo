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

import it.secondquadrant.lotest.interfaces.ServiceInterface;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.math.BigInteger;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Service class for the Bytea Management
 * @author Giulio Calacoci <giulio.calacoci@2ndQuadrant.it>
 */
public class ByteaService extends BaseService implements ServiceInterface {

    public BigInteger importFile(String dsn, String filename) throws Exception {
        FileInputStream fis = null;
        Connection conn = openConnection(dsn);
        try {
            File file = new File(filename);
            fis = new FileInputStream(file);
            PreparedStatement ps = conn.prepareStatement("INSERT INTO audio_catalog(filename, mode, therealdata) VALUES ( ?, ?, ?)");
            ps.setString(1, file.getName());
            ps.setString(2, "BYTEA");
            ps.setBinaryStream(3, fis, (int) file.length());
            Integer oid = ps.executeUpdate();
            ps.close();
            fis.close();
            return BigInteger.valueOf(oid);
        } catch (Exception ex) {
            Logger.getLogger(ByteaService.class.getName()).log(Level.SEVERE, null, ex);
        }

        throw new Exception("Insert Bytea field failed");
    }

    public void exportFile(String dsn, BigInteger oid, String filename) {
        try {
            Connection conn = openConnection(dsn);
            PreparedStatement ps = conn.prepareStatement("SELECT therealdata FROM audio_catalog WHERE id = ?");
            ps.setLong(1, oid.longValue());
            ResultSet rs = ps.executeQuery();
            InputStream bs = null;
            while (rs.next()) {
                bs = rs.getBinaryStream(1);
            }
            FileOutputStream fos = new FileOutputStream(filename);
            byte buf[] = new byte[2048];
            int s;
            while ((s = bs.read(buf, 0, 2048)) > 0) {
                fos.write(buf, 0, s);
            }
            fos.close();
            bs.close();
            rs.close();
            ps.close();
        } catch (Exception ex) {
            Logger.getLogger(ByteaService.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    public void removeFile(String dsn, BigInteger oid) throws Exception {
        Connection conn = openConnection(dsn);
        try {
            PreparedStatement ps = conn.prepareStatement("DELETE FROM audio_catalog WHERE id= ?");
            ps.setLong(1, oid.longValue());
            ps.executeUpdate();
            ps.close();
        } catch (Exception ex) {
            Logger.getLogger(ByteaService.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    public void listFiles(String dsn) {
        try {
            this.listObjects(dsn);
        } catch (Exception e) {
        }
    }
}
