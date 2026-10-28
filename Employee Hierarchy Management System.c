
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<limits.h>

#define NAME_SIZE 100
#define DESIGNATION_SIZE 50

enum OperationResult{
    OPERATION_ERROR=-1,
    OPERATION_MEMORY_ERROR=-2,
    OPERATION_FAILED=0,
    OPERATION_SUCCESS=1,
    INVALID_ID,
    DUPLICATE_ID,
    MANAGER_NOT_FOUND,
    EMPLOYEE_NOT_FOUND,
    TOP_LEVEL_EMPLOYEE
};

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

            if(!isalpha((unsigned char)c)&&!isspace((unsigned char)c)){
                found=-1;
                break;
            }
            if(!isspace((unsigned char)c))found=1;
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

enum OperationResult readEmployeeInput(int *id,char *name,char *designation,int *managerId,struct Employee *root){
    printf("Enter employee ID: ");
    if(!getInteger(id))return OPERATION_ERROR;

    if(!getText(name,NAME_SIZE,"Enter employee name: "))
        return OPERATION_ERROR;

    if(!getText(designation,DESIGNATION_SIZE,"Enter designation: "))
        return OPERATION_ERROR;

    *managerId=0;

    if(root!=NULL){
        printf("Enter reporting manager ID: ");
        if(!getInteger(managerId))return OPERATION_ERROR;
    }

    return OPERATION_SUCCESS;
}

enum OperationResult insertEmployee(struct Employee **root,int id,const char *name,const char *designation,int managerId){
    struct Employee *newEmployee,*manager=NULL,*last;

    if(id<=0)return INVALID_ID;
    if(findEmployee(*root,id)!=NULL)return DUPLICATE_ID;

    if(*root!=NULL){
        manager=findEmployee(*root,managerId);
        if(manager==NULL)return MANAGER_NOT_FOUND;
    }
    else if(managerId!=0){
        return MANAGER_NOT_FOUND;
    }

    newEmployee=calloc(1,sizeof(struct Employee));
    if(newEmployee==NULL)return OPERATION_MEMORY_ERROR;

    newEmployee->employeeId=id;
    strcpy(newEmployee->employeeName,name);
    strcpy(newEmployee->designation,designation);
    newEmployee->reportingManager=manager;

    if(*root==NULL){
        *root=newEmployee;
        return OPERATION_SUCCESS;
    }

    if(manager->firstReportee==NULL){
        manager->firstReportee=newEmployee;
    }
    else{
        last=manager->firstReportee;
        while(last->nextReportee!=NULL)
            last=last->nextReportee;

        last->nextReportee=newEmployee;
    }

    return OPERATION_SUCCESS;
}

void printAddResult(enum OperationResult result,int managerId){
    switch(result){
        case OPERATION_SUCCESS:
            if(managerId==0)
                printf("Top-level employee added successfully!\n");
            else
                printf("Employee added successfully!\n");
            break;

        case INVALID_ID:
            printf("Employee ID must be greater than zero.\n");
            break;

        case DUPLICATE_ID:
            printf("Employee ID already exists!\n");
            break;

        case MANAGER_NOT_FOUND:
            printf("Reporting manager not found!\n");
            break;

        case OPERATION_MEMORY_ERROR:
            printf("Memory allocation failed!\n");
            break;

        default:
            printf("Unable to add employee.\n");
    }
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

enum OperationResult removeEmployee(struct Employee **root,int id){
    struct Employee *target=findEmployee(*root,id);

    if(target==NULL)return EMPLOYEE_NOT_FOUND;
    if(target==*root)return TOP_LEVEL_EMPLOYEE;

    struct Employee *manager=target->reportingManager;
    struct Employee *reportees=target->firstReportee;
    struct Employee *destination,*last,*destinationLast;
    int minReports=INT_MAX;

    struct Employee *best=findBestSibling(manager->firstReportee,target,NULL,&minReports);

    target->firstReportee=NULL;
    unlinkEmployee(&manager->firstReportee,target);

    if(reportees!=NULL){
        destination=(best!=NULL)?best:manager;
        last=findLastReportee(reportees);

        updateManager(reportees,destination);

        if(destination->firstReportee==NULL){
            destination->firstReportee=reportees;
        }
        else{
            destinationLast=findLastReportee(destination->firstReportee);
            destinationLast->nextReportee=reportees;
        }

        last->nextReportee=NULL;
    }

    free(target);
    return OPERATION_SUCCESS;
}

void printRemoveResult(enum OperationResult result){
    switch(result){
        case OPERATION_SUCCESS:
            printf("Employee removed successfully.\n");
            break;

        case EMPLOYEE_NOT_FOUND:
            printf("Employee does not exist!\n");
            break;

        case TOP_LEVEL_EMPLOYEE:
            printf("Top-level employee cannot be removed!\n");
            break;

        default:
            printf("Unable to remove employee.\n");
    }
}

int countEmployees(struct Employee *employee){
    if(employee==NULL)return 0;
    return 1+countEmployees(employee->firstReportee)+countEmployees(employee->nextReportee);
}

void displayHierarchy(struct Employee *employee,int level,int siblings){
    if(employee==NULL)return;

    for(int i=0;i<level*4;i++)
        printf(" ");

    printf("%d - %s - %s\n",employee->employeeId,employee->employeeName,employee->designation);

    displayHierarchy(employee->firstReportee,level+1,1);

    if(siblings)
        displayHierarchy(employee->nextReportee,level,1);
}

void displayCompleteHierarchy(struct Employee *root){
    if(root==NULL){
        printf("Organization hierarchy is empty.\n");
        return;
    }

    printf("\n===== ORGANIZATION HIERARCHY =====\n");
    displayHierarchy(root,0,0);
    printf("\nTotal Employees: %d\n",countEmployees(root));
}

void displaySelectedHierarchy(struct Employee *employee){
    printf("\n===== EMPLOYEE HIERARCHY =====\n");
    displayHierarchy(employee,0,0);

    printf("\nTotal Employees Under %d: %d\n",
           employee->employeeId,
           countEmployees(employee->firstReportee));
}

void freeHierarchy(struct Employee *employee){
    if(employee==NULL)return;

    freeHierarchy(employee->firstReportee);
    freeHierarchy(employee->nextReportee);
    free(employee);
}

int main(void){
    struct Employee *root=NULL;
    int choice,id,managerId;
    char name[NAME_SIZE],designation[DESIGNATION_SIZE];
    enum OperationResult result;

    while(1){
        printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
        printf("1. Add Employee\n");
        printf("2. Remove Employee\n");
        printf("3. Display Employee Hierarchy\n");
        printf("4. Display Complete Hierarchy\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        if(!getInteger(&choice)){
            freeHierarchy(root);
            return 0;
        }

        switch(choice){
            case 1:
                result=readEmployeeInput(&id,name,designation,&managerId,root);

                if(result==OPERATION_ERROR){
                    freeHierarchy(root);
                    return 0;
                }

                result=insertEmployee(&root,id,name,designation,managerId);
                printAddResult(result,managerId);

                if(result==OPERATION_MEMORY_ERROR){
                    freeHierarchy(root);
                    return 1;
                }
                break;

            case 2:
                printf("Enter Employee ID to remove: ");
                if(!getInteger(&id)){
                    freeHierarchy(root);
                    return 0;
                }

                result=removeEmployee(&root,id);
                printRemoveResult(result);
                break;

            case 3:{
                printf("Enter Employee ID: ");
                if(!getInteger(&id)){
                    freeHierarchy(root);
                    return 0;
                }

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
                freeHierarchy(root);
                return 0;

            default:
                printf("Invalid choice! Enter 1 to 5.\n");
        }
    }
}
