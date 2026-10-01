#include<stdio.h> 
#include<ctype.h> 
#define MAXI 10000

int num[MAXI], numtop=-1; 
char op[MAXI], optop=-1; 
 
int calculate(int a,int b,char opr){ 
    if(opr=='+') return a+b; 
    if(opr=='-') return a-b; 
    if(opr=='*') return a*b; 
    if(opr=='/') return a/b; 
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
 
int main(){ 
    char exp[MAXI]; 
    int i=0, val,a,b,ans; 
    int expect_num=1; 
    char opr; 
    printf("Enter the expression:"); 
    fgets(exp,MAXI,stdin); 
     
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
            if(numtop>=MAXI-1){
                printf("Error: Number stack overflow.");
                return 0;
            }
            numtop++; 
            num[numtop]=val; 
            expect_num=0; 
        } 

        else if(isOp(exp[i])){ 
            if(expect_num==1){ 
                printf("Error: Invalid expression.\n"); 
                return 0; 
            } 
            while(optop!=-1 && priority_DMAS(op[optop])>=priority_DMAS(exp[i])){ 
                b=num[numtop]; 
                numtop--; 
                a=num[numtop]; 
                numtop--; 
                opr=op[optop]; 
                optop--; 

                if(opr=='/' && b==0){
                    printf("Error: Division by zero.\n");
                    return 0;
                }
                ans=calculate(a,b,opr); 
                if(numtop>=MAXI-1){
                    printf("Error: Number stack overflow.");
                    return 0;
                }
                numtop++; 
                num[numtop]=ans; 
            } 
            if(optop>=MAXI-1){
                printf("Error: Operator stack overflow.");
                return 0;
            }
            optop++; 
            op[optop]=exp[i]; 
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

    while(optop!=-1){ 
        b=num[numtop]; 
        numtop--; 
        a=num[numtop]; 
        numtop--; 
        opr=op[optop]; 
        optop--; 
        if(opr=='/' && b==0){
            printf("Error: Division by zero.\n");
            return 0;
        }
        ans=calculate(a,b,opr); 
        if(numtop>=MAXI-1){
            printf("Error: Number stack overflow.");
            return 0;
        }
        numtop++; 
        num[numtop]=ans; 
    } 
    printf("Result=%d\n",num[numtop]); 
    return 0; 
}