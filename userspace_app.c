#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<sys/mman.h>
#include<string.h>


//#define WR_VALUE _IOW('a','a',int *)
//#define RD_VALUE _IOR('a','b',int *)
#define MAX_SIZE 1024

#define SET_BAUDRATE _IOW('a','c',int*)
#define GET_BAUDRATE _IOR('a','d',int *)


char write_buf[MAX_SIZE];
char read_buf[MAX_SIZE];

void main()
{
	char *mmap_addr;
	int option;
	int value,fd,set_baudrate,get_baudrate;
	printf("opening device ....\n");

	fd = open("/dev/my_device",O_RDWR);
	if(fd < 0)
	{
		printf("unable to open device file...\n") ;
		exit(1);
	}

	while(1)
	{
		printf("******## Please enter the opetion ##*****\n");
		printf("1.write \n");
		printf("2.read \n");
		printf("3.set baudrate \n");
		printf("4.get baudrate \n");
		printf("5.mmap operation \n");
		printf("6.exit \n");

		//printf("please enter the value");

		scanf("%d",&option);

		switch(option)
		{
			case 1:
				printf("enter the data to write \n");
				scanf(" %[^\n]s",write_buf);
				write(fd,write_buf,strlen(write_buf)+1);
				printf("writing data done \n");
				break;
			case 2:

				printf("data reading \n");
				read(fd,read_buf,MAX_SIZE);
				printf("%s\n",read_buf);
				break;
			case 3:
				printf("please enter the device supported baudrate\n");
				scanf("%d",&set_baudrate);
				printf("setting baudrate..\n");
				ioctl(fd,SET_BAUDRATE,&set_baudrate);
				break;
			case 4:
				printf("getting baudrate... ");
				ioctl(fd,GET_BAUDRATE,&get_baudrate);
				printf("%d\n",get_baudrate);
				break;
			case 5:
				printf("mmap operation \n");
				mmap_addr = mmap(NULL,1024,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
				if(mmap_addr == MAP_FAILED)
				{
					printf("mapping failed \n");
					exit(1);
				}
				printf("maping address = %p\n",mmap_addr);
				scanf(" %[^\n]s",mmap_addr);
				munmap(mmap_addr,1024);
				break;
			case 6:
				printf("closing the opened files \n");
				close(fd);
	exit(1);
				break;

			default:
				printf("enter valid option\n");
				break;
		}
	}



}
