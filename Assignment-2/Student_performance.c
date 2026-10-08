#include<stdio.h>
#define MAXI_STD 100
#define MAXI_NAME 100
#define SUBS 3

struct student{
    int rollNum;
    char name[MAXI_NAME];
    int marks[SUBS];
};

int isValidRollNum(int roll){
    if(roll<=0) return 0;
    return 1;
}

int isValidName(char name[]){
    for(int i=0;name[i]!='\0';i++){
        if((name[i]<'A'||name[i]>'Z') && (name[i]<'a'||name[i]>'z') && name[i]!=' '){
            return 0;
        }
    }
    return 1;
}

int isValidMarks(int marks[]){
    for(int i=0;i<SUBS;i++){
        if(marks[i]<0||marks[i]>100){
            return 0;
        }
    }
    return 1;
}

int totaling(struct student s){
    int total=0;
    for(int i=0;i<SUBS;i++){
        total+=s.marks[i];
    }
    return total;
}

float averaging(int total){
    float avg=(float)total/SUBS;
    return avg;
}

char grading(float avg){
    if(avg>=85) return 'A';
    else if(avg>=70) return 'B';
    else if(avg>=50) return 'C';
    else if(avg>=35) return 'D';
    else return 'F';
}

void performance(char grade){
    int star;
    if(grade=='A') star=5;
    else if(grade=='B') star=4;
    else if(grade=='C') star=3;
    else if(grade=='D') star=2;
    else return;

    printf("Performance:");
    for(int i=0;i<star;i++){
        printf("*");
    }
    printf("\n");
}

void print_rollno(struct student std[],int n,int idx){
    if(idx==n) return; //base condition
    printf("%d",std[idx].rollNum);
    if(idx<n-1){
        printf(" ");
    }
    print_rollno(std,n,idx+1);//recurr
}

int readStudent(struct student *s){
    if(scanf("%d %99[A-Za-z ] %d %d %d",&s->rollNum,s->name,&s->marks[0],&s->marks[1],&s->marks[2])!=5){
        return 0;
    }
    if(isValidRollNum(s->rollNum)==0) return 0;
    if(isValidName(s->name)==0) return 0;
    if(isValidMarks(s->marks)==0) return 0;
    return 1;
}

void printStudent(struct student s,int total,float avg,char grade){
    printf("\nRoll: %d\n",s.rollNum);
    printf("Name: %s\n",s.name);
    printf("Total: %d\n",total);
    printf("Average: %.2f\n",avg);
    printf("Grade: %c\n",grade);
}

int main(){
    int n;
    struct student std[MAXI_STD];
    if(scanf("%d",&n)!=1||n<1||n>MAXI_STD){
        printf("Error: Invalid no. of students.\n");
        return 0;
    }

    for(int i=0;i<n;i++){
        if(readStudent(&std[i])==0){
            printf("Error: Invalid input.\n");
            return 0;
        }
    }
    for(int i=0;i<n;i++){
        int total=totaling(std[i]);
        float avg=averaging(total);
        char grade=grading(avg);
        printStudent(std[i],total,avg,grade);

        if(avg<35){
            continue;
        }
        performance(grade);
    }
    printf("\nList of Roll Numbers (via recursion): ");
    print_rollno(std,n,0);
    return 0;
}