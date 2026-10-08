#include<stdio.h> 
#include<ctype.h> 
#define MAXI 10000

typedef struct{
    int data[MAXI];
    int top;
}intStack;

typedef struct{
    char data[MAXI];
    int top;
}charStack;

void initialise_intStack(intStack *s){
    s->top=-1;
}

void initialise_charStack(charStack *s){
    s->top=-1;
}
 
int pushInt(intStack *s,int val){
    if(s->top>=MAXI-1) return 0;
    s->top++;
    s->data[s->top]=val;
    return 1;
}
int popInt(intStack *s,int *val){
    if(s->top<0) return 0;
    *val=s->data[s->top];
    s->top--;
    return 1;
}

int pushChar(charStack *s,char val){
    if(s->top>=MAXI-1) return 0;
    s->top++;
    s->data[s->top]=val;
    return 1;
}
int popChar(charStack *s,char *val){
    if(s->top<0) return 0;
    *val=s->data[s->top];
    s->top--;
    return 1;
}

int calculate(int a,int b,char opr,int *result){
    if(opr=='+') *result=a+b;
    else if(opr=='-') *result=a-b;
    else if(opr=='*') *result=a*b;
    else if(opr=='/'){
        if(b==0) return 2;
        *result=a/b;
    }
    else return 1;
    return 0;
}

int isOp(char ch){ 
    if(ch=='+'||ch=='-'||ch=='*'||ch=='/') return 1; 
    return 0; 
} 
 
int priority_DMAS(char op){ 
    if(op=='*'||op=='/') return 2; 
    if(op=='+'||op=='-') return 1; 
    return 0; 
}  

int reduceOnce(intStack *numStack,charStack *opStack){
    int a,b,result,error;
    char opr;
    if(!popInt(numStack,&b)) return 1;
    if(!popInt(numStack,&a)) return 1;
    if(!popChar(opStack,&opr)) return 1;

    error=calculate(a,b,opr,&result);
    if(error!=0) return error;
    if(!pushInt(numStack,result)) return 1;
    return 0;
}

int main(){ 
    char exp[MAXI]; 
    int i=0,val; 
    int expect_num=1; 
    printf("Enter the expression:"); 
    fgets(exp,MAXI,stdin);
    intStack numStack;
    charStack opStack;

    initialise_intStack(&numStack);
    initialise_charStack(&opStack);
     
    while(exp[i]!='\0'){  
        if(isspace(exp[i])){ 
            i++;
            continue;
        } 

        if(isdigit(exp[i])){ 
            if(expect_num==0){ 
                printf("Error: Invalid expression.\n"); 
                return 0; 
            }

            val=0; 
            while(isdigit(exp[i])){ 
                val=val*10+(exp[i]-'0'); 
                i++; 
            } 
            if(!pushInt(&numStack,val)){
                printf("Error: Number stack overflow.\n");
                return 0;
            }
            expect_num=0; 
        } 

        else if(isOp(exp[i])){ 
            if(expect_num==1){ 
                printf("Error: Invalid expression.\n"); 
                return 0; 
            } 
            while(opStack.top!=-1 && priority_DMAS(opStack.data[opStack.top])>=priority_DMAS(exp[i])){ 
                int error=reduceOnce(&numStack,&opStack);

                if(error==1){
                    printf("Error: Invalid expression.\n");
                    return 0;
                } 
                if(error==2){ 
                    printf("Error: Division by zero.\n");
                    return 0;
                }
            } 

            if(!pushChar(&opStack,exp[i])){
                printf("Error: Operator stack overflow.\n");
                return 0;
            }
            expect_num=1; 
            i++; 
        } 
        
        else{ 
            printf("Error: Invalid expression.\n"); 
            return 0; 
        } 
    } 
    
    if(expect_num==1){ 
        printf("Error: Invalid expression.\n"); 
        return 0; 
    } 
    while(opStack.top!=-1){ 
        int error=reduceOnce(&numStack,&opStack);

        if(error==1){
            printf("Error: Invalid expression.\n");
            return 0;
        }
        if(error==2){
            printf("Error: Division by zero.\n");
            return 0;
        }
    } 
    printf("%d\n",numStack.data[numStack.top]);
    return 0; 
}