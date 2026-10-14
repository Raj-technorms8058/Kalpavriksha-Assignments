
#include<stdio.h>
#include<string.h>

#define STANDARD_PRICE 150
#define PREMIUM_PRICE 250
#define RECLINER_PRICE 400
#define MIN_TICKETS 1
#define MAX_TICKETS 20
#define MIN_DISC_TICKETS 5
#define DISCOUNT_RATE 0.10
#define NAME_SIZE 100
#define CATEGORY_SIZE 20

int isValidName(const char customerName[]){
    int i=0;
    if(customerName[0]=='\0') return 0;
    while(customerName[i]!='\0'){
        if(!((customerName[i]>='A'&&customerName[i]<='Z') || (customerName[i]>='a'&&customerName[i]<='z'))){
            return 0;
        }
        i++;
    }
    return 1;
}

int isValidTicketCount(int ticketCount){
    return ticketCount>=MIN_TICKETS && ticketCount<=MAX_TICKETS;
}

int readBookingDetails(char customerName[],int *ticketCount,int *categoryChoice){
    if(scanf("%99s",customerName)!=1) return 0;
    if(scanf("%d",ticketCount)!=1) return 0;
    if(scanf("%d",categoryChoice)!=1) return 0;
    return 1;
}

int selectTicketCategory(int categoryChoice,char category[],int *pricePerTicket){  // can also write "char *category" bcoz C treats array parameters as pointers.
    switch(categoryChoice){
        case 1:
            *pricePerTicket=STANDARD_PRICE;
            strcpy(category,"Standard");
            return 1;
        case 2:
            *pricePerTicket=PREMIUM_PRICE;
            strcpy(category,"Premium");
            return 1;
        case 3:
            *pricePerTicket=RECLINER_PRICE;
            strcpy(category,"Recliner");
            return 1;
        default:
            return 0;
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

//const used to ensure function dont modify the customer name and category, it only reads and prints it.
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
    char category[CATEGORY_SIZE];
    int ticketCount,categoryChoice,pricePerTicket;
    double subTotal,discount,finalBill;
    
    if(!readBookingDetails(customerName,&ticketCount,&categoryChoice)){
        printf("Error:Invalid input.\n");
        return 1;
    }
    if(!isValidName(customerName)){
        printf("Error:Invalid customer name.\n");
        return 1;
    }
    if(!isValidTicketCount(ticketCount)){
        printf("Error:Invalid ticket count.\n");
        return 1;
    }
    if(!selectTicketCategory(categoryChoice,category,&pricePerTicket)){
        printf("Error:Invalid ticket category.\n");
        return 1;
    }

    subTotal=calculateSubTotal(ticketCount,pricePerTicket);
    discount=calculateDiscount(ticketCount,subTotal);
    finalBill=calculateFinalBill(subTotal,discount);

    printBill(customerName,category,ticketCount,subTotal,discount,finalBill);
    return 0;
}