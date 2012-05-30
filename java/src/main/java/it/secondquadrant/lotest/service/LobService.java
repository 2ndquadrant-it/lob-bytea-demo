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
import java.math.BigInteger;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.postgresql.largeobject.LargeObject;
import org.postgresql.largeobject.LargeObjectManager;

/**
 * Service class for the LOB Management
 * @author Giulio Calacoci <giulio.calacoci@2ndQuadrant.it>
 */
public class LobService extends BaseService implements ServiceInterface {

    public BigInteger importFile(String dsn, String filename) throws Exception {
        Connection conn = this.openConnection(dsn);
        try {
            conn.setAutoCommit(false);
            LargeObjectManager lobj = ((org.postgresql.PGConnection) conn).getLargeObjectAPI();
            Long oid = lobj.createLO(LargeObjectManager.READ | LargeObjectManager.WRITE);

            LargeObject obj = lobj.open(oid, LargeObjectManager.WRITE);
            System.out.println("import: " + filename);
            File file = new File(filename);
            FileInputStream fis = new FileInputStream(file);

            byte buf[] = new byte[2048];
            int s;
            while ((s = fis.read(buf, 0, 2048)) > 0) {
                obj.write(buf, 0, s);
            }

            obj.close();

            PreparedStatement ps = conn.prepareStatement("INSERT INTO audio_catalog(filename, mode, objid) VALUES ( ?, ?, ?)");
            ps.setString(1, file.getName());
            ps.setString(2, "LOB");
            ps.setLong(3, oid);
            ps.executeUpdate();
            ps.close();
            fis.close();

            conn.commit();
            return BigInteger.valueOf(oid);
        } catch (Exception ex) {
            Logger.getLogger(LobService.class.getName()).log(Level.SEVERE, null, ex);
        }

        throw new Exception("Insert LOB field failed");
    }

    public void exportFile(String dsn, BigInteger oid, String filename) {
        try {
            Connection conn = this.openConnection(dsn);
            conn.setAutoCommit(false);

            LargeObjectManager lobj = ((org.postgresql.PGConnection) conn).getLargeObjectAPI();

            PreparedStatement ps = conn.prepareStatement("SELECT objid FROM audio_catalog WHERE id = ?");
            ps.setLong(1, oid.longValue());
            ResultSet rs = ps.executeQuery();
            Long exportoid = null;
            while (rs.next()) {
                exportoid = rs.getLong(1);
            }
            LargeObject obj = lobj.open(exportoid, LargeObjectManager.READ);

            FileOutputStream fos = new FileOutputStream(filename);
            byte buf[] = new byte[2048];
            int s;
            while ((s = obj.read(buf, 0, 2048)) > 0) {
                fos.write(buf, 0, s);
            }
            fos.close();
            obj.close();
            rs.close();
            ps.close();

            conn.commit();
        } catch (Exception ex) {
            Logger.getLogger(LobService.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    public void removeFile(String dsn, BigInteger oid) throws Exception {
        try {
            Connection conn = this.openConnection(dsn);
            conn.setAutoCommit(false);
            PreparedStatement ps = conn.prepareStatement("SELECT objid FROM audio_catalog WHERE id = ?");
            ps.setLong(1, oid.longValue());
            ResultSet rs = ps.executeQuery();
            while (rs.next()) {
                Long exportoid = rs.getLong("objid");
                LargeObjectManager lobj = ((org.postgresql.PGConnection) conn).getLargeObjectAPI();
                lobj.delete(exportoid);
            }
            PreparedStatement ps2 = conn.prepareStatement("DELETE FROM audio_catalog WHERE id = ?");
            ps2.setLong(1, oid.longValue());
            ps2.executeUpdate();
            ps2.close();
            conn.commit();
        } catch (Exception e) {
            e.printStackTrace();
            throw new Exception("Remove LOB field failed");
        }
    }

    public void listFiles(String dsn) {
        try {
            this.listObjects(dsn);
        } catch (Exception e) {
        }
    }
}
