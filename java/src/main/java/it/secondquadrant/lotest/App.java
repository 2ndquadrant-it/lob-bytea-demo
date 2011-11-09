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
package it.secondquadrant.lotest;

import it.secondquadrant.lotest.interfaces.ServiceInterface;
import it.secondquadrant.lotest.service.ByteaService;
import it.secondquadrant.lotest.service.LobService;
import java.math.BigInteger;

/**
 * the main class
 *
 */
public class App {

    public static void main(String[] args) {
        try {
            if (args.length < 2) {
                usage();
                System.exit(0);
            }
            String dsn;
            if (args.length == 0) {
                dsn = "127.0.0.1:5432/lobtest,lob,lob";
            } else {
                dsn = args[0];
            }
            ServiceInterface s = null;
            //instantiate the right kind of service
            if (args[1].equals("lob") || args[1].equals("list")) {
                s = new LobService();
            } else if (args[1].equals("bytea")) {
                s = new ByteaService();
            }
            if (args.length == 2 && args[1].equals("list")) {
                s.listFiles(dsn);
                System.exit(0);
            }

            if (args[2].equals("import")) {

                s.importFile(dsn, args[3]);
            }
            if (args[2].equals("export")) {
                s.exportFile(dsn, new BigInteger(args[3]), args[4]);
            }
            if (args[2].equals("remove")) {

                s.removeFile(dsn, new BigInteger(args[3]));
            }
        } catch (Exception ex) {
            usage();
        }

    }
    /**
     * Method for the usage guide printing
     */
    public static void usage() {
        System.out.println("Usage: \n");
        System.out.println("  DSN lob {import|export|remove}  \n");
        System.out.println("  DSN bytea {import|export|remove} \n");
        System.out.println("  DSN list \n\n");
        System.out.println("DSN Format: \n\n");
        System.out.println("  serverIp:port/database,username,password \n\n");
        System.out.println("Command help: \n");
        System.out.println(" DSN lob import <filename> \n");
        System.out.println("   This command imports <filename> as LOB, and returns its oid\n\n");
        System.out.println(" DSN lob export <oid> <filename> \n");
        System.out.println("   This command exports LOB <oid> as <filename>.\n");
        System.out.println("   You can view OIDs actually in the catalog with list command\n\n");
        System.out.println(" DSN lob remove <oid> \n");
        System.out.println("   This command removes LOB <oid> from Postgres.\n   You can view OIDs actually in the catalog with list command\n\n");
        System.out.println(" DSN bytea import <oid> \n");
        System.out.println("   This command imports <filename> as a BYTEA.\n\n");
        System.out.println(" DSN bytea export <oid> \n");
        System.out.println("   This command exports bytea <oid> as <filename>.\n   You can view OIDs actually in the catalog with list command\n\n");
        System.out.println(" DSN bytea remove <oid> \n");
        System.out.println("   This command removes BYTEA <oid>.\n   You can view OIDs actually in the catalog with list command\n\n");
        System.out.println(" DSN list \n");
        System.out.println("   This command shows the actual catalogue.\n");
        System.out.println("\n");
    }
}
