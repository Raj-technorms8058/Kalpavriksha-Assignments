#include<stdio.h>
#include<string.h>
#define FILENAME "users.txt"
#define TEMPFILE "temp.txt"

struct User{
    int id;
    char name[100];
    int age;
};

void createFile(){
    FILE *f=fopen(FILENAME,"a");
    if(f==NULL){
        printf("Error in creating file.\n");
        return;
    }
    fclose(f);
}

int readUser(FILE *f,struct User *u){
    return fscanf(f,"%d|%99[^|]|%d\n",&u->id,u->name,&u->age)==3;
}

void writeUser(FILE *f,const struct User *u){
    fprintf(f,"%d|%s|%d\n",u->id,u->name,u->age);
}

int idExists(int id){
    FILE *f=fopen(FILENAME,"r");
    struct User u;
    if(f==NULL) return 0;
    while(readUser(f,&u)){
        if(u.id==id){
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void createUser(struct User u){
    if(idExists(u.id)){
        printf("This ID already exists.\n");
        return;
    }
    FILE *f=fopen(FILENAME,"a");
    if(f==NULL){
        printf("Error in opening file.\n");
        return;
    }
    writeUser(f,&u);
    fclose(f);
    printf("User addition successful.\n");
}

int rewriteFile(int id,int operation,const struct User *newData){
    FILE *f=fopen(FILENAME,"r");
    FILE *temp;
    struct User u;
    int found=0;
    if(f==NULL){
        printf("Error in opening file.\n");
        return 0;
    }
    temp=fopen(TEMPFILE,"w");
    if(temp==NULL){
        printf("Error in opening temporary file.\n");
        fclose(f);
        return 0;
    }

    while(readUser(f,&u)){
        if(u.id==id){
            found=1;
            if(operation==1) writeUser(temp,newData);
        }
        else{
            writeUser(temp,&u);
        }
    }
    fclose(f);
    fclose(temp);
    if(remove(FILENAME)!=0){
        printf("Error in deleting the old file.\n");
        remove(TEMPFILE);
        return 0;
    }

    if(rename(TEMPFILE,FILENAME)!=0){
        printf("Error in renaming temporary file.\n");
        return 0;
    }
    return found;
}

void updateUser(int id,struct User newData){
    if(rewriteFile(id,1,&newData)){
        printf("User updation successful.\n");
    }
    else{
        printf("User not found.\n");
    }
}

void read_user(){
    FILE *f=fopen(FILENAME,"r");
    struct User u;
    if(f==NULL){
        printf("Error Opening File.\n");
        return;
    }
    printf("User Records:\n");
    while(readUser(f,&u)){
        printf("ID=%d | Name=%s | Age=%d\n",u.id,u.name,u.age);
    }
    fclose(f);
}

void deleteUser(int id){
    if(rewriteFile(id,2,NULL)) printf("User deletion successful.\n");
    else printf("User not found.\n");
}

int main(){
    int k;
    int id;
    struct User u;
    createFile();

    while(1){
        printf("Enter your Choice number to perform CRUD Operation:\n");
        printf("(1)Create | (2)Read | (3)Update | (4)Delete | (5)Exit\n");
        printf("Enter your Choice:");
        scanf("%d",&k);
        getchar();

        switch(k){
            case 1:{
                printf("Enter ID:");
                scanf("%d",&u.id);
                getchar();
                printf("Enter Name:");
                fgets(u.name,sizeof(u.name),stdin);
                u.name[strcspn(u.name,"\n")]='\0';
                printf("Enter Age:");
                scanf("%d",&u.age);
                getchar();
                createUser(u);
                break;
            }

            case 2:{
                read_user();
                break;
            }

            case 3:{
                printf("Enter ID:");
                scanf("%d",&id);
                getchar();
                printf("Enter new Name:");
                fgets(u.name,sizeof(u.name),stdin);
                u.name[strcspn(u.name,"\n")]='\0';
                printf("Enter new Age:");
                scanf("%d",&u.age);
                getchar();
                updateUser(id,u);
                break;
            }

            case 4:{
                printf("Enter ID:");
                scanf("%d",&id);
                getchar();
                deleteUser(id);
                break;
            }

            case 5:{
                return 0;
            }

            default:{
                printf("Wrong Choice.\n");
            }
        }
    }
}