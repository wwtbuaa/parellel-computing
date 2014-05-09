#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>

//宏定义矩阵A和B的大小
#define M 2
#define N 3
#define P 4

//传入线程的参数，因参数过多，所以采用结构体的方式
struct threadArg{
	int tid;
	float (*B)[P];
	float *A_row;
	float *C_row;
	int numthreads;
};


int main(int argc,char **argv)
{
	int myid,numprocs,numthreads,numsend,sender;
	int i,j;
	pthread_t *tids;
	MPI_Status status;
	struct threadArg *targs;
	float *A_row,*C_row;
	double start,end;
	float A[M][N],B[N][P],C[M][P];
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);//获得进程ID
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);//获得总的进程数
	if(!myid){
		printf("matrix A:\n");//输出矩阵A
		for(i=0;i<M;i++){//初始化矩阵A
			for(j=0;j<N;j++){
				A[i][j]=i*j+1;
				printf("%f ",A[i][j]);
			}
			printf("\n");
		}
		printf("matrix B:\n");//输出矩阵B
		for(i=0;i<N;i++){//初始化矩阵B
			for(j=0;j<P;j++){
				B[i][j]=i*j+1;
				printf("%f ",B[i][j]);
			}
			printf("\n");
		}
	}
	MPI_Bcast(B[0],N*P,MPI_FLOAT,0,MPI_COMM_WORLD);//广播矩阵B	
	if(!myid){
		printf("broadcase end\n");
		j=(numprocs-1)<M?(numprocs-1):M;//j取进程数和A的行数中的较小数
		for(i=1;i<=j;i++)//依次给进程分配A中的一行
			MPI_Send(A[i-1],N,MPI_FLOAT,i,99,MPI_COMM_WORLD);
		numsend=j;//已发送的行数
		printf("numsend %d\n",numsend); 
		for(i=1;i<=M;i++){
			sender=(i-1)%(numprocs-1)+1;
			MPI_Recv(C[i-1],P,MPI_FLOAT,sender,100,MPI_COMM_WORLD,&status);//从进程号为sender的进程回收C[i-1]的结果
			printf("receive %d\n",i);
			if(numsend<M){//如果A没分配完，向返回计算结果的从进程中再分配A中的一行
				MPI_Send(A[numsend],N,MPI_FLOAT,sender,99,MPI_COMM_WORLD);
				numsend++;
			}
			else{//若A已经分配完毕，则发送一个MPI_TAG为0的消息
				MPI_Send(&j,0,MPI_INT,sender,0,MPI_COMM_WORLD);
			}
		}
		printf("matrix C:\n");//输出矩阵C
		for(i=0;i<M;i++){
			for(j=0;j<P;j++)
				printf("%f ",C[i][j]);
			printf("\n");
		}
	}
	else{
		numthreads=get_nprocs();
		tids=(pthread_t*)malloc(numthreads*sizeof(pthread_t));//用于存放进程ID
		A_row=(float*)malloc(N*sizeof(float));//用于存放A
		C_row=(float*)malloc(P*sizeof(float));//用于存放结果C
		while(1){
			MPI_Recv(A_row,N,MPI_FLOAT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);//接受A
			if(status.MPI_TAG==0){//如果是TAG是0则退出
				break;
			}
			#pragma omp parallel
			{	
				printf("thread number %d\n",omp_get_thread_num());
				#pragma omp for				
				for(i=0;i<P;i++){
					C_row[i]=0.0;
					for(j=0;j<N;j++){
						C_row[i]+=A_row[j]*B[j][i];
					}
				}
			}
			MPI_Send(C_row,P,MPI_FLOAT,0,100,MPI_COMM_WORLD);//发送计算结果
		}
	}
	MPI_Finalize();
	return 0;
}
