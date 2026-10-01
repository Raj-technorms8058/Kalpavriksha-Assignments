#include<stdio.h>
struct user{
    int id;
    char name[40];
    int age;
};

void create_user(){
    FILE *f=fopen("users.txt","a");
    FILE *temp=fopen("users.txt","r");
    struct user u,t;
    if(f==NULL||temp==NULL){
        printf("Error in Opening File.\n");
        if(f!=NULL) fclose(f);
        if(temp!=NULL) fclose(temp);
        return;
    }
    printf("Enter ID:");
    scanf("%d",&u.id);

    while(fscanf(temp,"%d %39s %d",&t.id,t.name,&t.age)!=EOF){
        if(t.id==u.id){
            printf("This ID already exists.\n");
            fclose(f);
            fclose(temp);
            return;
        }
    }
    fclose(temp);
    printf("Enter Name:");
    scanf("%39s",u.name);
    printf("Enter Age:");
    scanf("%d",&u.age);

    fprintf(f,"%d %s %d\n",u.id,u.name,u.age);
    printf("User addition successful.\n");
    fclose(f);
}

void update_user(){
    FILE *f=fopen("users.txt","r");
    FILE *new=fopen("new.txt","w");
    struct user u;
    int id;
    int found=0;

    if(f==NULL || new==NULL){
        printf("Error in Opening File.\n");
        if(f!=NULL) fclose(f);
        if(new!=NULL) fclose(new);
        return;
    }
    printf("Enter ID:");
    scanf("%d",&id);
    while(fscanf(f,"%d %39s %d",&u.id,u.name,&u.age)!=EOF){
        if(u.id==id){
            found=1;
            printf("Enter new Name:");
            scanf("%39s",u.name);
            printf("Enter new Age:");
            scanf("%d",&u.age);
        }
        fprintf(new,"%d %s %d\n",u.id,u.name,u.age);
    }
    fclose(f);
    fclose(new);
    
    if(remove("users.txt")!=0){
        printf("Error in deleting the old file.\n");
        return;
    }
    if(rename("new.txt","users.txt")!=0){
        printf("Error in renaming new file to old file.\n");
        return;
    }

    if(found){
        printf("User updation successful.\n");
    }
    else{
        printf("User not found.\n");
    }
}

void read_user(){
    FILE *f=fopen("users.txt","r");
    struct user u;
    if(f==NULL){
        printf("Error Opening File.\n");
        return;
    }
    printf("User Records:\n");
    while(fscanf(f,"%d %39s %d",&u.id,u.name,&u.age)!=EOF){
        printf("ID=%d | Name=%s | Age=%d\n",u.id,u.name,u.age);
    }
    fclose(f);
}

void delete_user(){
    FILE *f=fopen("users.txt","r");
    FILE *new=fopen("new.txt","w");
    struct user u;
    int id;
    int found=0;

    if(f==NULL || new==NULL){
        printf("Error in Opening File.\n");
        if(f!=NULL) fclose(f);
        if(new!=NULL) fclose(new);
        return;
    }
    printf("Enter ID:");
    scanf("%d",&id);

    while(fscanf(f,"%d %39s %d",&u.id,u.name,&u.age)!=EOF){
        if(u.id==id){
            found=1;
        }
        else{
            fprintf(new,"%d %s %d\n",u.id,u.name,u.age);
        }
    }
    fclose(f);
    fclose(new);
    if(remove("users.txt")!=0){
        printf("Error in deleting the old file.\n");
        return;
    }
    if(rename("new.txt","users.txt")!=0){
        printf("Error in renaming new file to old file.\n");
        return;
    }

    if(found){
        printf("User deletion successful.\n");
    }
    else{
        printf("User not found.\n");
    }
}

int main(){
    FILE *f=fopen("users.txt","a");
    if(f==NULL){
        printf("Error Creating File.\n");
        return 1;
    }
    fclose(f);
    int k;
    while(1){
        printf("Enter your Choice number to perform CRUD Operation:\n");
        printf("(1)Create | (2)Read | (3)Update | (4)Delete | (5)Exit\n");
        printf("Enter your Choice:");
        scanf("%d",&k);

        switch(k){
            case 1: create_user(); break;
            case 2: read_user(); break;
            case 3: update_user(); break;
            case 4: delete_user(); break;
            case 5: return 0;
            default: printf("Wrong Choice.\n");
        }
    }
    return 0;
}