#include <string.h>
#define ERRRETVAL               -1000000
#define ERR_INCOMM_CLIENT_OPEN	-1000001
#define ERR_INCOMM_CLIENT_WRITE	-1000002
#define ERR_INCOMM_CLIENT_READ	-1000003
#define ERR_INCOMM_CLIENT_CLOSE	-1000004

static int tde_gate_err=0;

int tde_gate_cmd_begin(int icall, ptde_shm pthe_shm)
{
	pthe_shm->cmd_id=icall;
	pthe_shm->cmd_status=1;

	return 0;
}





int tde_gate_cmd_wait(int icall, ptde_shm pthe_shm, double* pdata, int nsamp)
{
/*	open socket */
	if (incomm_client_open(2039)==-1) {
/*		set error status */
		printf("incomm_client_open failed.\n");
		tde_gate_err=1;
		pthe_shm->retval=(double) ERR_INCOMM_CLIENT_OPEN;
		return -1;
	}
	
	if (incomm_client_write((void*) pthe_shm, sizeof(tde_shm))==-1) {
		printf("incomm_client_write failed.\n");
/*		reset shm */
		memset(pthe_shm,0,sizeof(tde_shm));
/*		set error status */
		tde_gate_err=1;
		pthe_shm->retval=(double) ERR_INCOMM_CLIENT_WRITE;
		return -1;		
	}
	
	if (incomm_client_read((void*) pthe_shm, sizeof(tde_shm))==-1) {
		printf("incomm_client_read failed.\n");
/*		reset shm */
		memset(pthe_shm,0,sizeof(tde_shm));
/*		set error status */
		tde_gate_err=1;
		pthe_shm->retval=(double) ERR_INCOMM_CLIENT_READ;
		return -1;		
	}

	if (icall==CALL_TDE_GET_DATA) {
/*		read data */
		if (incomm_client_read((void*) pdata, nsamp*sizeof(double))==-1) {
			printf("incomm_client_read failed.\n");
/*			reset data */
			memset(pdata,0,nsamp*sizeof(double));
/*			set error status */			
		        tde_gate_err=1;
			pthe_shm->retval=(double) ERR_INCOMM_CLIENT_READ;
			return -1;
		}
	}

/*	close socket */
	if (incomm_client_close()==-1) {
		printf("incomm_client_close failed.\n");
		tde_gate_err=1;
		pthe_shm->retval=(double) ERR_INCOMM_CLIENT_CLOSE;
		return -1;		
	}

	return 0;
}
