#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<limits.h>

#define NAME_SIZE 100
#define DESIGNATION_SIZE 50

struct Employee{
    int employeeId;
    char employeeName[NAME_SIZE];
    char designation[DESIGNATION_SIZE];
    struct Employee *reportingManager,*firstReportee,*nextReportee;
};

void clearInputBuffer(void){
    int ch;
    while((ch=getchar())!='\n'&&ch!=EOF);
}

int getInteger(int *value){
    char input[NAME_SIZE],extra;
    while(1){
        if(fgets(input,sizeof(input),stdin)==NULL)return 0;
        if(strchr(input,'\n')==NULL) clearInputBuffer();

        if(sscanf(input," %d %c",value,&extra)==1)
            return 1;

        printf("Invalid integer! Try again: ");
    }
}

int getText(char *text,size_t size,const char *prompt){
    char input[NAME_SIZE];
    int found;

    while(1){
        printf("%s",prompt);
        if(fgets(input,sizeof(input),stdin)==NULL)return 0;

        if(strchr(input,'\n')==NULL){
            clearInputBuffer();
            printf("Input too long. Try again.\n");
            continue;
        }
        input[strcspn(input,"\n")]='\0';

        if(strlen(input)>=size){
            printf("Input too long. Try again.\n");
            continue;
        }

        found=0;
        for(int i=0;input[i]!='\0';i++){
            char c=input[i];

            if(!isalpha(c)&&!isspace(c)){
                found=-1;
                break;
            }
            if(!isspace(c))found=1;
        }

        if(found==-1){
            printf("Enter only letters and spaces.\n");
            continue;
        }
        if(found==0){
            printf("Input cannot be empty.\n");
            continue;
        }
        strcpy(text,input);
        return 1;
    }
}

struct Employee *findEmployee(struct Employee *employee,int id){
    if(employee==NULL)return NULL;
    if(employee->employeeId==id)return employee;

    struct Employee *found=findEmployee(employee->firstReportee,id);

    if(found!=NULL)return found;
    return findEmployee(employee->nextReportee,id);
}

int addEmployee(struct Employee **root){
    int id,managerId;
    struct Employee *newEmployee,*manager;
    printf("Enter employee ID: ");

    if(!getInteger(&id))return -1;
    if(id<=0){
        printf("Employee ID must be greater than zero.\n");
        return 0;
    }
    if(findEmployee(*root,id)!=NULL){
        printf("Employee ID already exists!\n");
        return 0;
    }

    newEmployee=calloc(1,sizeof(struct Employee));
    if(newEmployee==NULL){
        printf("Memory allocation failed!\n");
        return -1;
    }

    newEmployee->employeeId=id;
    if(!getText(newEmployee->employeeName,
                sizeof(newEmployee->employeeName),
                "Enter employee name: ")){
        free(newEmployee);
        return -1;
    }

    if(!getText(newEmployee->designation,
                sizeof(newEmployee->designation),
                "Enter designation: ")){
        free(newEmployee);
        return -1;
    }

    if(*root==NULL){
        *root=newEmployee;
        printf("Top-level employee added successfully!\n");
        return 1;
    }
    printf("Enter reporting manager ID: ");

    if(!getInteger(&managerId)){
        free(newEmployee);
        return -1;
    }

    manager=findEmployee(*root,managerId);
    if(manager==NULL){
        printf("Reporting manager not found!\n");
        free(newEmployee);
        return 0;
    }

    newEmployee->reportingManager=manager;
    newEmployee->nextReportee=manager->firstReportee;
    manager->firstReportee=newEmployee;
    printf("Employee added successfully!\n");
    return 1;
}


int countDirectReportees(struct Employee *employee){
    if(employee==NULL)return 0;
    return 1+countDirectReportees(employee->nextReportee);
}


struct Employee *findBestSibling(struct Employee *employee,struct Employee *target,struct Employee *best,int *minReports){
    if(employee==NULL)return best;
    if(employee!=target){
        int count=countDirectReportees(employee->firstReportee);
        if(count<*minReports){
            *minReports=count;
            best=employee;
        }
    }
    return findBestSibling(employee->nextReportee,target,best,minReports);
}


struct Employee *findLastReportee(struct Employee *employee){
    if(employee==NULL||employee->nextReportee==NULL)return employee;
    return findLastReportee(employee->nextReportee);
}


void updateManager(struct Employee *employee,struct Employee *manager){
    if(employee==NULL)return;
    employee->reportingManager=manager;
    updateManager(employee->nextReportee,manager);
}


void unlinkEmployee(struct Employee **list,struct Employee *target){
    if(*list==NULL)return;
    if(*list==target){
        *list=target->nextReportee;
        target->nextReportee=NULL;
        return;
    }
    unlinkEmployee(&(*list)->nextReportee,target);
}


int removeEmployee(struct Employee **root,int id){
    struct Employee *target=findEmployee(*root,id);
    if(target==NULL){
        printf("Employee does not exist!\n");
        return 0;
    }
    if(target==*root){
        printf("Top-level employee cannot be removed!\n");
        return 0;
    }

    struct Employee *manager=target->reportingManager;
    struct Employee *reportees=target->firstReportee;
    int minReports=INT_MAX;

    struct Employee *best=findBestSibling(manager->firstReportee,target,NULL,&minReports);

    target->firstReportee=NULL;
    unlinkEmployee(&manager->firstReportee,target);

    if(reportees!=NULL){
        struct Employee *destination=(best!=NULL)?best:manager;
        struct Employee *last=findLastReportee(reportees);

        updateManager(reportees,destination);
        last->nextReportee=destination->firstReportee;
        destination->firstReportee=reportees;
    }
    free(target);
    printf("Employee removed successfully.\n");
    return 1;
}


int countEmployees(struct Employee *employee){
    if(employee==NULL)return 0;
    return 1 + countEmployees(employee->firstReportee) + countEmployees(employee->nextReportee);
}

void displayHierarchy(struct Employee *employee,int siblings){
    if(employee==NULL)return;

    printf("%d - %s - %s\n",employee->employeeId,employee->employeeName,employee->designation);

    displayHierarchy(employee->firstReportee,1);
    if(siblings){
        displayHierarchy(employee->nextReportee,1);
    }
}

void displayCompleteHierarchy(struct Employee *root){
    if(root==NULL){
        printf("Organization hierarchy is empty.\n");
        return;
    }

    printf("\n===== ORGANIZATION HIERARCHY =====\n");
    displayHierarchy(root,0);
    printf("\nTotal Employees: %d\n",countEmployees(root));
}

void displaySelectedHierarchy(struct Employee *employee){
    printf("\n===== EMPLOYEE HIERARCHY =====\n");
    displayHierarchy(employee,0);

    printf("\nTotal Employees Under %d: %d\n",
           employee->employeeId,
           countEmployees(employee->firstReportee));
}

int main(void){
    struct Employee *root=NULL;
    int choice,id,result;

    while(1){
        printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
        printf("1. Add Employee\n");
        printf("2. Remove Employee\n");
        printf("3. Display Employee Hierarchy\n");
        printf("4. Display Complete Hierarchy\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        if(!getInteger(&choice))return 0;

        switch(choice){
            case 1:
                result=addEmployee(&root);
                if(result==-1)return 1;
                break;

            case 2:
                printf("Enter Employee ID to remove: ");
                if(!getInteger(&id))return 0;
                removeEmployee(&root,id);
                break;

            case 3:{
                printf("Enter Employee ID: ");
                if(!getInteger(&id))return 0;
                struct Employee *employee=findEmployee(root,id);
                if(employee==NULL)
                    printf("Employee not found!\n");
                else
                    displaySelectedHierarchy(employee);

                break;
            }

            case 4:
                displayCompleteHierarchy(root);
                break;

            case 5:
                return 0;

            default:
                printf("Invalid choice! Enter 1 to 5.\n");
        }
    }
}