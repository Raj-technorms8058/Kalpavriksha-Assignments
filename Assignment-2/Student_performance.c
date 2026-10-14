#include<stdio.h>

#define MAX_STUDENTS 100
#define MAX_NAME 100
#define SUBJECTS 3

#define GRADE_A_MIN 85
#define GRADE_B_MIN 70
#define GRADE_C_MIN 50
#define GRADE_D_MIN 35

struct student{
    int rollNum;
    char name[MAX_NAME];
    int marks[SUBJECTS];
};

int isValidRollNum(int roll){
    if(roll<=0) return 0;
    return 1;
}

//The scanset %99[A-Za-z ] allows letters and spaces, but it also accepts names containing only spaces.
int isValidName(const char name[]){ //To check that no empty name passes bcoz we are taking names with spaces 
    for(int i=0;name[i]!='\0';i++){
        if(name[i]!=' '){
            return 1; 
        }
    }
    return 0; 
}

int isValidMarks(const int marks[]){
    for(int i=0;i<SUBJECTS;i++){
        if(marks[i]<0||marks[i]>100){
            return 0;
        }
    }
    return 1;
}

int calculateTotal(const struct student *s){
    int total=0;
    for(int i=0;i<SUBJECTS;i++){
        total+=s->marks[i];
    }
    return total;
}

float calculateAverage(int total){
    float avg=(float)total/SUBJECTS;
    return avg;
}

char getGrade(float avg){
    if(avg>=GRADE_A_MIN) return 'A';
    else if(avg>=GRADE_B_MIN) return 'B';
    else if(avg>=GRADE_C_MIN) return 'C';
    else if(avg>=GRADE_D_MIN) return 'D';
    else return 'F';
}

void printPerformance(char grade){
    int stars;
    if(grade=='A') stars=5;
    else if(grade=='B') stars=4;
    else if(grade=='C') stars=3;
    else if(grade=='D') stars=2;
    else return;

    printf("Performance:");
    for(int i=0;i<stars;i++){
        printf("*");
    }
    printf("\n");
}

void printRollNumbers(const struct student std[],int n,int idx){
    if(idx==n) return;
    printf("%d",std[idx].rollNum);
    if(idx<n-1){
        printf(" ");
    }
    printRollNumbers(std,n,idx+1);
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

void printStudent(const struct student *s,int total,float avg,char grade){
    printf("\nRoll: %d\n",s->rollNum);
    printf("Name: %s\n",s->name);
    printf("Total: %d\n",total);
    printf("Average: %.2f\n",avg);
    printf("Grade: %c\n",grade);
}

int main(){
    int n;
    struct student std[MAX_STUDENTS];
    if(scanf("%d",&n)!=1||n<1||n>MAX_STUDENTS){
        printf("Error: Invalid no. of students.\n");
        return 1;
    }

    for(int i=0;i<n;i++){
        if(readStudent(&std[i])==0){
            printf("Error: Invalid input.\n");
            return 1;
        }
    }
    for(int i=0;i<n;i++){
        int total=calculateTotal(&std[i]);
        float avg=calculateAverage(total);
        char grade=getGrade(avg);
        printStudent(&std[i],total,avg,grade);
        printPerformance(grade);
    }
    printf("\nList of Roll Numbers (via recursion): ");
    printRollNumbers(std,n,0);
    printf("\n");
    return 0;
}