#include<stdio.h>

int main(int argc,char **argv)
{
	#pragma omp parallel
	{
		int temp;
		temp=omp_get_thread_num();
		if(temp==0)
			printf("haha!\n");
		else
			printf("houhou!\n");
	}
}
