#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MINVAL 2
#define MAXVAL 10

int getValidInput(int *n)
{
    int status,ch,valid;

    while(1)
    {
        printf("Enter matrix size (%d-%d): ",MINVAL,MAXVAL);
        status=scanf("%d",n);

        if(status==EOF)
            return 0;

        valid=1;
        ch=getchar();

        while(ch!='\n'&&ch!=EOF)
        {
            if(ch!=' '&&ch!='\t'&&ch!='\r')
                valid=0;

            ch=getchar();
        }

        if(status==1&&valid&&*n>=MINVAL&&*n<=MAXVAL)
            return 1;

        printf("Invalid input! Please enter an integer between %d and %d.\n",MINVAL,MAXVAL);
    }
}

int allocateMatrix(int ***mat,int **data,int n)
{
    int i;

    *mat=malloc((size_t)n*n*sizeof(int *));

    if(*mat==NULL)
        return 0;

    *data=malloc((size_t)n*n*sizeof(int));

    if(*data==NULL)
    {
        free(*mat);
        *mat=NULL;
        return 0;
    }

    for(i=0;i<n*n;i++)
        *(*mat+i)=*data+i;

    return 1;
}

void generateMatrix(int **mat,int n)
{
    int i;

    for(i=0;i<n*n;i++)
        **(mat+i)=rand()%256;
}

void printMatrix(int **mat,int n)
{
    int i,j;

    for(i=0;i<n;i++)
    {
        for(j=0;j<n;j++)
            printf("%4d ",**(mat+i*n+j));

        printf("\n");
    }
}

void swapSlots(int **a,int **b)
{
    int *temp;

    temp=*a;
    *a=*b;
    *b=temp;
}

void rotateMatrix(int **mat,int n)
{
    int i,j;

    for(i=0;i<n;i++)
    {
        for(j=i+1;j<n;j++)
            swapSlots(mat+i*n+j,mat+j*n+i);
    }

    for(i=0;i<n;i++)
    {
        for(j=0;j<n/2;j++)
            swapSlots(mat+i*n+j,mat+i*n+(n-1-j));
    }
}

int calculateAverage(int **mat,int n,int i,int j,int *prev,int *curr)
{
    int r,c,sum=0,count=0;

    for(r=i-1;r<=i+1;r++)
    {
        if(r<0||r>=n)
            continue;

        for(c=j-1;c<=j+1;c++)
        {
            if(c<0||c>=n)
                continue;

            if(r==i-1)
                sum+=*(prev+c);
            else if(r==i)
                sum+=*(curr+c);
            else
                sum+=**(mat+r*n+c);

            count++;
        }
    }

    return sum/count;
}

int smoothMatrix(int **mat,int n)
{
    int i,j;
    int *prev=NULL,*curr=NULL,*temp;

    prev=malloc((size_t)n*sizeof(int));
    curr=malloc((size_t)n*sizeof(int));

    if(prev==NULL||curr==NULL)
    {
        free(prev);
        free(curr);
        return 0;
    }

    for(i=0;i<n;i++)
    {
        for(j=0;j<n;j++)
            *(curr+j)=**(mat+i*n+j);

        for(j=0;j<n;j++)
            **(mat+i*n+j)=calculateAverage(mat,n,i,j,prev,curr);

        temp=prev;
        prev=curr;
        curr=temp;
    }

    free(prev);
    free(curr);

    return 1;
}

int main(void)
{
    int n;
    int **mat=NULL;
    int *data=NULL;

    if(!getValidInput(&n))
        return 0;

    srand((unsigned int)time(NULL));

    if(!allocateMatrix(&mat,&data,n))
    {
        printf("Memory allocation failed.\n");
        return 1;
    }

    generateMatrix(mat,n);

    printf("\nOriginal Randomly Generated Matrix:\n");
    printMatrix(mat,n);

    rotateMatrix(mat,n);

    printf("\nMatrix after 90 Degree Clockwise Rotation:\n");
    printMatrix(mat,n);

    if(!smoothMatrix(mat,n))
    {
        printf("\nMemory allocation failed for smoothing buffers.\n");
        free(mat);
        free(data);
        return 1;
    }

    printf("\nMatrix after Applying 3x3 Smoothing Filter:\n");
    printMatrix(mat,n);

    free(mat);
    free(data);

    return 0;
}