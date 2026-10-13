
#include<stdio.h>

#define STANDARD_FARE 150
#define PREMIUM_FARE 250
#define RECLINER_FARE 400
#define MIN_BOOKING 1
#define MAX_BOOKING 20
#define BULK_BOOKING_LIMIT 5
#define BULK_DISCOUNT_RATE 0.10
#define NAME_SIZE 100
#define CATEGORY_SIZE 20

int isValidName(const char customerName[]){
    int i=0;
    if(customerName[0]=='\0') return 0;
    while(customerName[i]!='\0'){
        if(!((customerName[i]>='A'&&customerName[i]<='Z')||(customerName[i]>='a'&&customerName[i]<='z'))){
            return 0;
        }
        i++;
    }
    return 1;
}

int readBooking(char customerName[],int *seatsBooked,int *planChoice){
    if(scanf("%99s",customerName)!=1) return 0;

    if(!isValidName(customerName)) return 0;

    if(scanf("%d",seatsBooked)!=1) return 0;
    if(scanf("%d",planChoice)!=1) return 0;

    return 1;
}

int selectTicketPlan(int planChoice,char planName[],int *unitFare){
    switch(planChoice){
        case 1:
            *unitFare=STANDARD_FARE;
            snprintf(planName,CATEGORY_SIZE,"Standard");
            return 1;
        case 2:
            *unitFare=PREMIUM_FARE;
            snprintf(planName,CATEGORY_SIZE,"Premium");
            return 1;
        case 3:
            *unitFare=RECLINER_FARE;
            snprintf(planName,CATEGORY_SIZE,"Recliner");
            return 1;
        default:
            return 0;
    }
}

double calculateSubtotal(int seatsBooked,int unitFare){
    return (double)seatsBooked*unitFare;
}

double calculateDiscount(int seatsBooked,double grossAmount){
    if(seatsBooked>=BULK_BOOKING_LIMIT){
        return grossAmount*BULK_DISCOUNT_RATE;
    }
    else{
        return 0.0;
    }
}

void printBookingBill(const char customerName[],const char planName[],int seatsBooked,double grossAmount,double discountAmount){
    printf("Customer: %s\n",customerName);
    printf("Ticket Category: %s\n",planName);
    printf("Tickets: %d\n",seatsBooked);
    printf("Subtotal: %.2f\n",grossAmount);
    printf("Discount: %.2f\n",discountAmount);
    printf("Final Bill: %.2f\n",grossAmount-discountAmount);
}

int main(){
    char customerName[NAME_SIZE];
    char planName[CATEGORY_SIZE];
    int seatsBooked,planChoice,unitFare;
    double grossAmount,discountAmount;

    if(!readBooking(customerName,&seatsBooked,&planChoice)){
        printf("Error: Invalid customer name or input.\n");
        return 1;
    }

    if(seatsBooked<MIN_BOOKING||seatsBooked>MAX_BOOKING){
        printf("Error: Invalid ticket count.\n");
        return 1;
    }

    if(!selectTicketPlan(planChoice,planName,&unitFare)){
        printf("Error: Invalid ticket category.\n");
        return 1;
    }

    grossAmount=calculateSubtotal(seatsBooked,unitFare);
    discountAmount=calculateDiscount(seatsBooked,grossAmount);

    printBookingBill(customerName,planName,seatsBooked,grossAmount,discountAmount);

    return 0;
}
