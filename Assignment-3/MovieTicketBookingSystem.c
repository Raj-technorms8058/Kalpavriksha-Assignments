
#include<stdio.h>
#include<string.h>
#include<ctype.h>


#define STANDARD_PRICE 150
#define PREMIUM_PRICE 250
#define RECLINER_PRICE 400
#define MIN_TICKETS 1
#define MAX_TICKETS 20
#define MIN_DISC_TICKETS 5
#define DISCOUNT_RATE 0.10
#define NAME_SIZE 100

int isValidName(const char customerName[]){
    int i=0;
    if(customerName[0]=='\0') return 0; //Rejects an empty customer name, the question requires a single name without spaces
    while(customerName[i]!='\0'){
        if(!isalpha(customerName[i])) return 0;
        i++;
    }
    return 1;
}

int isValidTicketCount(int ticketCount){
    return ticketCount>=MIN_TICKETS && ticketCount<=MAX_TICKETS;
}

int readBookingDetails(char customerName[],int *ticketCount,int *categoryChoice){
    if(fgets(customerName,NAME_SIZE,stdin)==NULL) return 0;
    customerName[strcspn(customerName,"\n")]='\0';

    if(scanf("%d",ticketCount)!=1) return 0;
    if(scanf("%d",categoryChoice)!=1) return 0;
    return 1;
}

int getPrice(int categoryChoice){
    switch(categoryChoice){
        case 1: return STANDARD_PRICE;
        case 2: return PREMIUM_PRICE;
        case 3: return RECLINER_PRICE;
        default: return 0;
    }
}

const char *getCategoryName(int categoryChoice){
    switch(categoryChoice){
        case 1: return "Standard";
        case 2: return "Premium";
        case 3: return "Recliner";
        default: return NULL;
    }
}

double calculateSubTotal(int ticketCount,int pricePerTicket){
    return (double)ticketCount*pricePerTicket;
}

double calculateDiscount(int ticketCount,double subTotal){
    if(ticketCount>=MIN_DISC_TICKETS) return subTotal*DISCOUNT_RATE;
    else return 0.0;
}

double calculateFinalBill(double subTotal,double discount){
    return subTotal-discount;
}

void printBill(const char customerName[],const char category[],int ticketCount,double subTotal,double discount,double finalBill){
    printf("Customer: %s\n",customerName);
    printf("Ticket Category: %s\n",category);
    printf("Tickets: %d\n",ticketCount);
    printf("Subtotal: %.2f\n",subTotal);
    printf("Discount: %.2f\n",discount);
    printf("Final Bill: %.2f\n",finalBill);
}

int main(){
    char customerName[NAME_SIZE];
    const char *category;

    int ticketCount,categoryChoice,pricePerTicket;
    double subTotal,discount,finalBill;
    
    if(!readBookingDetails(customerName,&ticketCount,&categoryChoice)){
        printf("Error: Invalid input.\n");
        return 1;
    }
    if(!isValidName(customerName)){
        printf("Error: Invalid customer name.\n");
        return 1;
    }
    if(!isValidTicketCount(ticketCount)){
        printf("Error: Invalid ticket count.\n");
        return 1;
    }

    category=getCategoryName(categoryChoice);
    if(category==NULL){
        printf("Error: Invalid ticket category.\n");
        return 1;
    }
    pricePerTicket=getPrice(categoryChoice);

    subTotal=calculateSubTotal(ticketCount,pricePerTicket);
    discount=calculateDiscount(ticketCount,subTotal);
    finalBill=calculateFinalBill(subTotal,discount);

    printBill(customerName,category,ticketCount,subTotal,discount,finalBill);
    return 0;
}