#include<stdio.h>
#include"mpi.h"

int main(int argc,char **argv)
{
	int myid,numprocs;
	double start,end;
	char message[10]="HELLO";
	MPI_Status status;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	if(myid==0){
		start=MPI_Wtime();
		printf("%d %f\n",myid,start);
		MPI_Send(message,strlen(message)+1,MPI_CHAR,1,99,MPI_COMM_WORLD);
		MPI_Recv(message,100,MPI_CHAR,numprocs-1,99,MPI_COMM_WORLD,&status);
		end=MPI_Wtime();
		printf("That tooks %f seconds\n",end-start);
	}
	else{
		MPI_Recv(message,100,MPI_CHAR,myid-1,99,MPI_COMM_WORLD,&status);
		printf("%d receive %s\n",myid,message);
		printf("%d %f\n",myid,MPI_Wtime());
		MPI_Send(message,strlen(message)+1,MPI_CHAR,(myid+1)%numprocs,99,MPI_COMM_WORLD);
	}
	MPI_Finalize();
	return 0;
}
