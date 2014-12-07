#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include "udp.h"
#include "structdefs.h"

int sd;
struct sockaddr_in saddr;

int sel()
{
	fd_set r;
	FD_ZERO(&r);
	FD_SET(sd, &r);
	
	struct timeval t;
	t.tv_sec = 5;
	t.tv_usec = 0;
	
	return select(sd+1, &r, NULL, NULL, &t);
}

int MFS_Init(char *hostname, int port)
{
	sd = UDP_Open(0);
	if(sd < 0)
		return -1;

	int rc = UDP_FillSockAddr(&saddr, hostname, port);
	if(rc != 0)
		return -1;
	return 0;
}

int MFS_Lookup(int pinum, char *name)
{
	message req;
	response res;
	
	strcpy(req.cmd,"MFS_Lookup");
	req.inum = pinum;
	if(strlen(name) > 59)
		return -1;
	strcpy(req.name, name);

	char *reqBuff = (char *)malloc(sizeof(message)*sizeof(char));
	memcpy(reqBuff, &req, sizeof(message));
	
	int rc;
	int selRc=0;

	while(selRc == 0)
	{
		rc = UDP_Write(sd, &saddr, reqBuff, sizeof(message));
		selRc = sel();
	}

	char *resBuff = (char *)malloc(sizeof(response)*sizeof(char));
	rc = UDP_Read(sd, &saddr, resBuff, sizeof(response));
	memcpy(&res, resBuff, sizeof(response));
	
	free(reqBuff);
	free(resBuff);	

	return res.rc;
}


int MFS_Stat(int inum, MFS_Stat_t *m)
{
	if(m == NULL)
		return -1;

	message req;
	response res;
	
	strcpy(req.cmd,"MFS_Stat");
	req.inum = inum;

	char *reqBuff = (char *)malloc(sizeof(message)*sizeof(char));
	memcpy(reqBuff, &req, sizeof(message));
	
	int rc;
	int selRc=0;

	while(selRc == 0)
	{
		rc = UDP_Write(sd, &saddr, reqBuff, sizeof(message));
		selRc = sel();
	}

	char *resBuff = (char *)malloc(sizeof(response)*sizeof(char));
	rc = UDP_Read(sd, &saddr, resBuff, sizeof(response));
	memcpy(&res, resBuff, sizeof(response));

	memcpy(m, &res.stat, sizeof(MFS_Stat_t));
	
	free(reqBuff);
	free(resBuff);	

	return res.rc;
}


int MFS_Write(int inum, char *block, int blocknum)
{
	message req;
	response res;
	
	strcpy(req.cmd,"MFS_Write");
	req.inum = inum;
	req.blocknum = blocknum;
	memcpy(req.block, block, 4096);

	char *reqBuff = (char *)malloc(sizeof(message)*sizeof(char));
	memcpy(reqBuff, &req, sizeof(message));
	
	int rc;
	int selRc=0;

	while(selRc == 0)
	{
		rc = UDP_Write(sd, &saddr, reqBuff, sizeof(message));
		selRc = sel();
	}

	char *resBuff = (char *)malloc(sizeof(response)*sizeof(char));
	rc = UDP_Read(sd, &saddr, resBuff, sizeof(response));
	memcpy(&res, resBuff, sizeof(response));
	
	free(reqBuff);
	free(resBuff);	

	return res.rc;
}

int MFS_Read(int inum, char *block, int blocknum)
{
	if(block == NULL)
		return -1;

        message req;
        response res;

        strcpy(req.cmd,"MFS_Read");
        req.inum = inum;
        req.blocknum = blocknum;

        char *reqBuff = (char *)malloc(sizeof(message)*sizeof(char));
        memcpy(reqBuff, &req, sizeof(message));

        int rc;
        int selRc=0;

        while(selRc == 0)
        {
                rc = UDP_Write(sd, &saddr, reqBuff, sizeof(message));
                selRc = sel();
        }

        char *resBuff = (char *)malloc(sizeof(response)*sizeof(char));
        rc = UDP_Read(sd, &saddr, resBuff, sizeof(response));
        memcpy(&res, resBuff, sizeof(response));

	memcpy(block, res.block, 4096);	

        free(reqBuff);
        free(resBuff);

        return res.rc;
}

int MFS_Creat(int inum, int type, char *name)
{
	if(name == NULL)
		return -1;
	if(strlen(name)>59)
		return -1;

        message req;
        response res;

        strcpy(req.cmd,"MFS_Create");
        req.inum = inum;
        req.type = type;
	strcpy(req.name, name);

        char *reqBuff = (char *)malloc(sizeof(message)*sizeof(char));
        memcpy(reqBuff, &req, sizeof(message));

        int rc;
        int selRc=0;

        while(selRc == 0)
        {
                rc = UDP_Write(sd, &saddr, reqBuff, sizeof(message));
                selRc = sel();
        }

        char *resBuff = (char *)malloc(sizeof(response)*sizeof(char));
        rc = UDP_Read(sd, &saddr, resBuff, sizeof(response));
        memcpy(&res, resBuff, sizeof(response));

        free(reqBuff);
        free(resBuff);

        return res.rc;
}

int MFS_Unlink(int inum, char *name)
{
        if(name == NULL)
                return -1;
        if(strlen(name)>59)
                return -1;

        message req;
        response res;

        strcpy(req.cmd,"MFS_Unlink");
        req.inum = inum;
        strcpy(req.name, name);

        char *reqBuff = (char *)malloc(sizeof(message)*sizeof(char));
        memcpy(reqBuff, &req, sizeof(message));

        int rc;
        int selRc=0;

        while(selRc == 0)
        {
                rc = UDP_Write(sd, &saddr, reqBuff, sizeof(message));
                selRc = sel();
        }

        char *resBuff = (char *)malloc(sizeof(response)*sizeof(char));
        rc = UDP_Read(sd, &saddr, resBuff, sizeof(response));
        memcpy(&res, resBuff, sizeof(response));

        free(reqBuff);
        free(resBuff);

        return res.rc;
}

int MFS_Shutdown()
{
        message req;
        response res;

        strcpy(req.cmd,"MFS_Shutdown");

        char *reqBuff = (char *)malloc(sizeof(message)*sizeof(char));
        memcpy(reqBuff, &req, sizeof(message));

        int rc;
        int selRc=0;

        while(selRc == 0)
        {
                rc = UDP_Write(sd, &saddr, reqBuff, sizeof(message));
                selRc = sel();
        }

        char *resBuff = (char *)malloc(sizeof(response)*sizeof(char));
        rc = UDP_Read(sd, &saddr, resBuff, sizeof(response));
        memcpy(&res, resBuff, sizeof(response));

        free(reqBuff);
        free(resBuff);

        return res.rc;
}




int main(int argc, char *argv[])
{
	int rc = MFS_Init("mumble-37.cs.wisc.edu", 12239);
	
	if(rc == -1)
	{
		printf("Unable to bind to 12239\n");
		return -1;
	}
	int i=1;
	while(rc != -1)
	{
		if(i==4107)
			break;
		char fname[20];
		sprintf(fname,"%d.txt",i);
		i++;
		rc = MFS_Creat(0,1,fname);
		if(rc == 0)
			printf("Successfully created %s\n",fname);
		else
			printf("Creation failed on %s\n",fname);
		rc = MFS_Unlink(0,fname);
		if(rc == 0)
			printf("Successfully deleted %s\n\n",fname);
		else
			printf("Deletion failed on %s\n",fname);
	}
	MFS_Shutdown();
}


/*
int main(int argc, char *argv[])
{
	message req;
	char *buf = (char *)malloc(sizeof(message)*sizeof(char));
	strcpy(req.cmd, "MFS_Lookup");
	memcpy(buf,&req,sizeof(message));
	
	int rc = MFS_Init("localhost", 12238);

	rc = MFS_Lookup(0,"dir1");
	printf("MFS_Lookup = %d\n",rc);

	UDP_Close(sd);
}*/


