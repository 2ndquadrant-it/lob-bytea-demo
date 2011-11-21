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

#include <stdio.h>
#include <stdlib.h>
#include "libpq-fe.h"
#include "libpq/libpq-fs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "utils.h"
#include "errcode.h"

#define BUFSIZE 1024

#define NO_ACTION		0
#define LOB_IMPORT		1
#define LOB_EXPORT		2
#define LOB_REMOVE		3
#define OBJ_LIST		4
#define BYTEA_IMPORT	5
#define BYTEA_EXPORT	6
#define BYTEA_REMOVE	7

static void do_lob_import(void);
static void do_lob_export(void);
static void do_lob_remove(void);
static void do_obj_list(void);
static void do_bytea_import(void);
static void do_bytea_export(void);
static void do_bytea_remove(void);

static void	usage();
static void check_int(char *);
static void init_conn();
static void update_catalog(int action);

static const char	*progname;
static char			*mode = NULL;
static char			*cmd = NULL;
static int			action = NO_ACTION;
static char			in_filename[MAXLEN];
static char			out_filename[MAXLEN];
static char			dsn[MAXLEN];
static int			in_id;
PGconn				*conn;
char				sqlquery[QUERY_STR_LEN];
PGresult			*res;

int main(int argc, char **argv)
{
	progname = argv[0];

	if( argc<3 )
	{
		usage();
        exit(ERR_BAD_CONFIG);
	}

	strncpy (dsn,argv[optind++],MAXLEN);

    if (optind < argc)
    {
		mode = argv[optind++];
        if (strcasecmp(mode, "LOB") != 0 &&
			strcasecmp(mode, "BYTEA") != 0 &&
			strcasecmp(mode, "LIST") != 0 )
        {
            usage();
            exit(ERR_BAD_CONFIG);
        }
    }

    if (optind < argc)
    {
        cmd = argv[optind++];
        /* check posibilities for all commands */
        if (strcasecmp(mode, "LOB") == 0)
        {
            if (strcasecmp(cmd, "IMPORT") == 0)
                action = LOB_IMPORT;
			else if (strcasecmp(cmd, "EXPORT") == 0)
				action = LOB_EXPORT;
			else if (strcasecmp(cmd, "REMOVE") == 0)
				action = LOB_REMOVE;
        } else if(strcasecmp(mode, "BYTEA") == 0) {
			if (strcasecmp(cmd, "IMPORT") == 0)
				action = BYTEA_IMPORT;
			else if (strcasecmp(cmd, "EXPORT") == 0)
				action = BYTEA_EXPORT;
			else if (strcasecmp(cmd, "REMOVE") == 0)
				action = BYTEA_REMOVE;
        }
    }

	if(strcasecmp(mode, "LIST") == 0)
		action = OBJ_LIST;

    if(action == NO_ACTION)
    {
        usage();
		exit(ERR_BAD_CONFIG);
    }

    if ((action == LOB_IMPORT)
		|| (action == BYTEA_IMPORT))
    {
        if (optind < argc)
        {
            strncpy(in_filename, argv[optind++], MAXLEN);
        }else{
			fprintf(stderr, "ERROR: You must specify the filename to import.\n");
            exit(ERR_BAD_CONFIG);
		}
    }

    if ((action == LOB_EXPORT)
		|| (action == BYTEA_EXPORT))
	{
        if (optind < argc)
        {
			check_int(argv[optind++]);
        }else{
			fprintf(stderr, "ERROR: You must specify the oid to export.\n");
            exit(ERR_BAD_CONFIG);
		}
        if (optind < argc)
        {
            strcpy(out_filename, argv[optind++]);
        }else{
			fprintf(stderr, "ERROR: You must specify the filename of the file exported.\n");
            exit(ERR_BAD_CONFIG);
		}
    }

    if ((action == LOB_REMOVE)
		|| (action == BYTEA_REMOVE))
    {
        if (optind < argc)
        {
			check_int(argv[optind++]);
        }else{
			fprintf(stderr, "ERROR: You must specify the id to remove.\n");
            exit(ERR_BAD_CONFIG);
		}
    }

    switch (optind < argc)
    {
		case 0:
			break;
		default:
			fprintf(stderr, "ERROR: too many command-line arguments \n");
			usage();
			exit(ERR_BAD_CONFIG);
    }

	init_conn();

    switch (action)
    {
		case OBJ_LIST:
			do_obj_list();
			break;
		case LOB_IMPORT:
			do_lob_import();
			break;
		case LOB_EXPORT:
			do_lob_export();
			break;
		case LOB_REMOVE:
			do_lob_remove();
			break;
		case BYTEA_IMPORT:
			do_bytea_import();
			break;
		case BYTEA_EXPORT:
			do_bytea_export();
			break;
		case BYTEA_REMOVE:
			do_bytea_remove();
			break;
		default:
			usage();
			exit(ERR_BAD_CONFIG);
    }

    PQfinish(conn);
	return 0;
}

void usage()
{
	fprintf(stderr, "Usage: \n");
	fprintf(stderr, " %s DSN lob {import|export|remove} \n", progname);
	fprintf(stderr, " %s DSN bytea {import|export|remove} \n", progname);
	fprintf(stderr, " %s DSN list \n\n", progname);
	fprintf(stderr, "Command help: \n");
	fprintf(stderr, " DSN lob import <filename> \n");
	fprintf(stderr, "   This command imports <filename> as LOB, and returns its oid\n\n");
	fprintf(stderr, " DSN lob export <oid> <filename> \n");
	fprintf(stderr, "   This command exports LOB <oid> as <filename>.\n");
	fprintf(stderr, "   You can view OIDs actually in the catalog with list command\n\n");
	fprintf(stderr, " DSN lob remove <oid> \n");
	fprintf(stderr, "   This command removes LOB <id> from Postgres.\n   You can view IDs actually in the catalog with list command\n\n");
	fprintf(stderr, " DSN bytea import <oid> \n");
	fprintf(stderr, "   This command imports <filename> as a BYTEA.\n\n");
	fprintf(stderr, " DSN bytea export <oid> \n");
	fprintf(stderr, "   This command exports bytea <oid> as <filename>.\n   You can view OIDs actually in the catalog with list command\n\n");
	fprintf(stderr, " DSN bytea remove <oid> \n");
	fprintf(stderr, "   This command removes BYTEA <oid>.\n   You can view OIDs actually in the catalog with list command\n\n");
	fprintf(stderr, " DSN list \n");
	fprintf(stderr, "   This command shows the actual catalogue.\n");
	fprintf(stderr, "\n");
	exit(1);
}

void check_int(char *in_param)
{
	in_id = atoi(in_param);
	if(in_id==0)
	{
		fprintf(stderr, "[FAIL] Could not convert '%s' to integer\n", in_param);
		exit(1);
	}
}

static void do_obj_list(void)
{
	int i;
	sqlquery_snprintf(sqlquery,
		"SELECT id,mode,filename FROM audio_catalog " );

	res = PQexec(conn, sqlquery);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "[ERROR] Query: %s\n%s\n", sqlquery, PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        exit(ERR_BAD_CONFIG);
    }
	if( PQntuples(res)==0 )
		printf("No data in audio_catalog.\n");
	else
	{
		printf(" ID     | Type  | Filename \n--------+-------+----------\n");
		for (i=0; i<PQntuples(res); i++)
		{
			printf(" %-6s ", PQgetvalue(res, i, 0));
			printf("| %-6s", PQgetvalue(res, i, 1));
			printf("| %s\n", PQgetvalue(res, i, 2));
		}
		PQclear(res);
		printf("--------+-------+-----------\n");
	}

}

static void do_lob_import()
{
    Oid	lobjOid;

	res = PQexec(conn, "BEGIN");
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        exit(ERR_BAD_CONFIG);
    }
	PQclear(res);

    lobjOid = lo_import(conn, in_filename);
	if(lobjOid == InvalidOid){
		fprintf(stderr, "[FAIL] %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        exit(ERR_BAD_CONFIG);
	}
	in_id = lobjOid;
	update_catalog(LOB_IMPORT);
	printf("[OK] LOB imported correctly, OID=%d\n", lobjOid);

	res = PQexec(conn, "END");
    PQclear(res);
}

static void do_lob_export()
{
	res = PQexec(conn, "BEGIN");
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        exit(ERR_BAD_CONFIG);
    }
	PQclear(res);

	/* We need the id of LOB to export */
	sqlquery_snprintf(sqlquery, "SELECT objid FROM audio_catalog "
		"WHERE id = %d ", in_id );
	res = PQexec(conn, sqlquery);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
        fprintf(stderr, "ERROR: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        exit(ERR_BAD_CONFIG);
	}
	check_int(PQgetvalue(res, 0, 0));

	if(lo_export(conn, in_id, out_filename)==-1)
	{
		fprintf(stderr, "[FAIL] %s\n", PQerrorMessage(conn));
		PQfinish(conn);
		exit(ERR_BAD_CONFIG);
	}
	printf("[OK] LOB exported correctly. File is %s\n",
		out_filename);

	res = PQexec(conn, "END");
    PQclear(res);
}

static void do_lob_remove()
{
	res = PQexec(conn, "BEGIN");
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        exit(ERR_BAD_CONFIG);
    }
	PQclear(res);

	/* We need the id of LOB to unlink */
	sqlquery_snprintf(sqlquery, "SELECT objid FROM audio_catalog "
		"WHERE id = %d ", in_id );
	res = PQexec(conn, sqlquery);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
        fprintf(stderr, "[ERROR] %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        exit(ERR_BAD_CONFIG);
	}
	if (PQntuples(res)==0)
	{
		fprintf(stderr, "[ERROR] Cannot retrieve the LOB with id %d\n", in_id);
		exit(1);
	}

	check_int(PQgetvalue(res, 0, 0));

	if(lo_unlink(conn, in_id)==-1)
	{
		fprintf(stderr, "[ERROR] %s\n", PQerrorMessage(conn));
        PQfinish(conn);
		exit(1);
	}
	update_catalog(LOB_REMOVE);
	printf("[OK] LOB removed correctly. Oid was %d\n", in_id);

	res = PQexec(conn, "END");
    PQclear(res);
}

static void do_bytea_import()
{
	FILE *f;
	unsigned char *escaped_string;
	unsigned char *buf;
	long size;
	size_t result;
	int i;

	f = fopen(in_filename,"rb");
	if(f==NULL)
	{
		fprintf(stderr,"ERROR: could not open file %s\n", in_filename);
        PQfinish(conn);
		exit(1);
	}

	fseek(f, 0 , SEEK_END);
	size = ftell (f);
	rewind(f);

	buf = (char*) malloc (sizeof(char) * (size+1));
	fread(buf, size, 1, f) ;

	Oid in_oids[]={17,25,25};
	const char* params[]={buf,in_filename,"BYTEA"};
	const int params_length[]={size,strlen(in_filename),6};
	const int params_format[]={1,0,0};
	res = PQexecParams(conn,
		"INSERT INTO audio_catalog(therealdata, filename, mode) VALUES ($1, $2, $3)",
		sizeof(params)/sizeof(params[0]),
        in_oids, params, params_length,
        params_format, 1);
	if (res && PQresultStatus(res)==PGRES_COMMAND_OK)
	{
		/* success */
		printf("Bytea importato con successo!\n\n");
	}else{
		fprintf(stderr, "ERROR: Cannot insert object into catalog: %s\n",
		PQerrorMessage(conn));
		PQfinish(conn);
		exit(ERR_BAD_CONFIG);
	}

	fclose(f);
	free(buf);
	PQclear(res);
}

static void do_bytea_export()
{
	size_t *size;
	FILE *f;

	sqlquery_snprintf(sqlquery, "SELECT therealdata FROM audio_catalog "
		"WHERE id = %d AND mode LIKE 'BYTEA'", in_id );
	res = PQexec(conn, sqlquery);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
        fprintf(stderr, "ERROR: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        exit(ERR_BAD_CONFIG);
	}

	const unsigned char *v = PQgetvalue(res, 0, 0);
	unsigned char *buf = PQunescapeBytea( v,size );

	f = fopen(out_filename,"wb");
	if(f==NULL)
	{
		fprintf(stderr,"ERROR: could not open file %s\n", in_filename);
        PQfinish(conn);
		exit(1);
	}

	fwrite( buf,*(size),1,f );
	printf("[OK] File exported.\n");

	PQfreemem(buf);
    PQclear(res);
}

static void do_bytea_remove()
{
	sqlquery_snprintf(sqlquery, "DELETE FROM audio_catalog "
		"WHERE id = %d AND mode LIKE 'BYTEA'", in_id );
	if(!PQexec(conn, sqlquery))
	{
        fprintf(stderr, "ERROR: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        exit(ERR_BAD_CONFIG);
	}

	printf("[OK] Bytea removed correctly.\n");
}

static void init_conn()
{
	/*
     * set up the connection
     */
    conn = PQconnectdb(dsn);

    /* check to see that the backend connection was successfully made */
    if (PQstatus(conn) == CONNECTION_BAD)
    {
        fprintf(stderr, "Connection to database '%s' failed.\n", dsn);
        fprintf(stderr, "%s", PQerrorMessage(conn));
		PQfinish(conn);
		exit(ERR_BAD_CONFIG);
    }

	/* Create the catalog table */
	sqlquery_snprintf(sqlquery, "CREATE TABLE audio_catalog ( "
								" id SERIAL,     "
								" filename TEXT, "
								" mode TEXT, "
								" therealdata BYTEA, "
								" objid oid )" );
	if(!PQexec(conn, sqlquery))
	{
		fprintf(stderr, "[ERROR] Cannot create the table audio_catalog: %s\n",
			PQerrorMessage(conn));
		PQfinish(conn);
		exit(ERR_BAD_CONFIG);
	}
}

static void update_catalog(int action)
{
	switch(action)
	{
		case LOB_IMPORT:
			sqlquery_snprintf(sqlquery,
				"INSERT INTO audio_catalog (filename, mode, objid)"
				" VALUES ('%s', 'LOB', %d)",
				in_filename, in_id);
			if(!PQexec(conn, sqlquery))
			{
				fprintf(stderr, "ERROR: Cannot insert object into catalog: %s\n",
					PQerrorMessage(conn));
				PQfinish(conn);
				exit(ERR_BAD_CONFIG);
			}
			break;

		case LOB_REMOVE:
			sqlquery_snprintf(sqlquery,
				"DELETE FROM audio_catalog WHERE objid = %d",
				in_id);
			if(!PQexec(conn, sqlquery))
			{
				fprintf(stderr, "ERROR: Cannot delete object from catalog: %s\n",
					PQerrorMessage(conn));
				PQfinish(conn);
				exit(ERR_BAD_CONFIG);
			}
			break;
		default:
			break;
	}
}

/* vim: tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab
*/
