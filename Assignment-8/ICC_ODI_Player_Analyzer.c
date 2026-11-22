#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "Players_data.h"

#define MAX_TEAMS 10
#define MAX_PLAYERS_PER_TEAM 50
#define MAX_NAME_LEN 51

typedef struct{
    int playerId;
    char name[MAX_NAME_LEN];
    char teamName[MAX_NAME_LEN];
    char role[20];
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
} PlayerData;

typedef struct Node{
    PlayerData data;
    struct Node* next;
    struct Node* roleNext;
} Node;

typedef struct{
    int teamId;
    char name[MAX_NAME_LEN];
    int totalPlayers;
    float averageBattingStrikeRate;
    Node* head;
    Node* roleHead[3];
} Team;

Team T[MAX_TEAMS];

void clearInputBuffer(){
    int ch = getchar();
    while(ch != '\n' && ch != EOF){
        ch = getchar();
    }
}

int getInt(const char* msg){
    char buffer[100], *end;
    long value;
    while(1){
        printf("%s", msg);
        if(fgets(buffer, sizeof(buffer), stdin) == NULL){
            continue;
        }
        if(strchr(buffer, '\n') == NULL){
            clearInputBuffer();
            printf("Input too long!\n");
            continue;
        }
        char* start = buffer;
        while(isspace(*start)){
            start++;
        }
        if(*start == '\0'){
            printf("Empty input!\n");
            continue;
        }
        value = strtol(start, &end, 10);
        if(start == end){
            printf("Invalid integer!\n");
            continue;
        }
        while(isspace(*end)){
            end++;
        }
        if(*end != '\0'){
            printf("Extra characters not allowed!\n");
            continue;
        }
        if(value < INT_MIN || value > INT_MAX){
            printf("Integer out of range!\n");
            continue;
        }
        return (int)value;
    }
}

float getFloat(const char* msg){
    char buffer[100], *end;
    float value;
    while(1){
        printf("%s", msg);
        if(fgets(buffer, sizeof(buffer), stdin) == NULL){
            continue;
        }
        if(strchr(buffer, '\n') == NULL){
            clearInputBuffer();
            printf("Input too long!\n");
            continue;
        }
        char* start = buffer;
        while(isspace(*start)){
            start++;
        }
        if(*start == '\0'){
            printf("Empty input!\n");
            continue;
        }
        value = strtof(start, &end);
        if(start == end){
            printf("Invalid number!\n");
            continue;
        }
        while(isspace(*end)){
            end++;
        }
        if(*end != '\0'){
            printf("Extra characters not allowed!\n");
            continue;
        }
        if(value < 0.0f){
            printf("Value cannot be negative!\n");
            continue;
        }
        return value;
    }
}

void getName(const char* msg, char* dest){
    char buffer[100];
    while(1){
        printf("%s", msg);
        if(fgets(buffer, sizeof(buffer), stdin) == NULL){
            continue;
        }
        if(strchr(buffer, '\n') == NULL){
            clearInputBuffer();
            printf("Name too long! Max 50 characters.\n");
            continue;
        }
        char* start = buffer;
        while(isspace(*start)){
            start++;
        }
        size_t len = strlen(start);
        while(len > 0 && isspace(start[len - 1])){
            start[len - 1] = '\0';
            len--;
        }
        if(len < 1 || len > 50){
            printf("Invalid name! Length must be 1 to 50.\n");
            continue;
        }
        int valid = 1;
        for(size_t i = 0; i < len; i++){
            if(!isalpha(start[i]) && start[i] != ' '){
                valid = 0;
                break;
            }
        }
        if(!valid){
            printf("Invalid name! Only letters and spaces allowed.\n");
            continue;
        }
        strcpy(dest, start);
        return;
    }
}

int getRoleIndex(const char* role){
    if(strcmp(role, "Batsman") == 0){
        return 0;
    }
    if(strcmp(role, "Bowler") == 0){
        return 1;
    }
    if(strcmp(role, "All-rounder") == 0){
        return 2;
    }
    return -1;
}

float calculatePI(PlayerData p){
    if(strcmp(p.role, "Batsman") == 0){
        return (p.battingAverage * p.strikeRate) / 100.0f;
    }
    if(strcmp(p.role, "Bowler") == 0){
        return (p.wickets * 2.0f) + (100.0f - p.economyRate);
    }
    return ((p.battingAverage * p.strikeRate) / 100.0f) + (p.wickets * 2.0f);
}

void updateAverageStrikeRate(Team* t){
    float sum = 0.0f;
    int count = 0;
    Node* curr = t->head;
    while(curr != NULL){
        if(strcmp(curr->data.role, "Batsman") == 0 || strcmp(curr->data.role, "All-rounder") == 0){
            sum += curr->data.strikeRate;
            count++;
        }
        curr = curr->next;
    }
    t->averageBattingStrikeRate = (count == 0) ? 0.0f : (sum / count);
}

int searchTeam(int teamId){
    int low = 0, high = MAX_TEAMS - 1;
    while(low <= high){
        int mid = (low + high) / 2;
        if(T[mid].teamId == teamId){
            return mid;
        }
        if(T[mid].teamId < teamId){
            low = mid + 1;
        }
        else{
            high = mid - 1;
        }
    }
    return -1;
}

int playerExists(int playerId){
    for(int i = 0; i < MAX_TEAMS; i++){
        Node* curr = T[i].head;
        while(curr != NULL){
            if(curr->data.playerId == playerId){
                return 1;
            }
            curr = curr->next;
        }
    }
    return 0;
}

void insertSortedRole(Team* t, Node* newNode){
    int r = getRoleIndex(newNode->data.role);
    if(r == -1){
        return;
    }
    if(t->roleHead[r] == NULL || t->roleHead[r]->data.performanceIndex < newNode->data.performanceIndex){
        newNode->roleNext = t->roleHead[r];
        t->roleHead[r] = newNode;
        return;
    }
    Node* curr = t->roleHead[r];
    while(curr->roleNext != NULL && curr->roleNext->data.performanceIndex >= newNode->data.performanceIndex){
        curr = curr->roleNext;
    }
    newNode->roleNext = curr->roleNext;
    curr->roleNext = newNode;
}

void insertPlayer(Team* t, PlayerData p){
    p.performanceIndex = calculatePI(p);
    Node* newNode = (Node*)malloc(sizeof(Node));
    if(newNode == NULL){
        printf("Memory allocation failed!\n");
        return;
    }
    newNode->data = p;
    newNode->next = NULL;
    newNode->roleNext = NULL;
    if(t->head == NULL){
        t->head = newNode;
    }
    else{
        Node* temp = t->head;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = newNode;
    }
    t->totalPlayers++;
    insertSortedRole(t, newNode);
    updateAverageStrikeRate(t);
}

void initializeData(){
    for(int i = 0; i < MAX_TEAMS; i++){
        T[i].teamId = i + 1;
        strcpy(T[i].name, teams[i]);
        T[i].totalPlayers = 0;
        T[i].averageBattingStrikeRate = 0.0f;
        T[i].head = NULL;
        for(int r = 0; r < 3; r++){
            T[i].roleHead[r] = NULL;
        }
    }
    for(int i = 0; i < playerCount; i++){
        PlayerData p;
        p.playerId = players[i].id;
        strcpy(p.name, players[i].name);
        strcpy(p.teamName, players[i].team);
        strcpy(p.role, players[i].role);
        p.totalRuns = players[i].totalRuns;
        p.battingAverage = players[i].battingAverage;
        p.strikeRate = players[i].strikeRate;
        p.wickets = players[i].wickets;
        p.economyRate = players[i].economyRate;
        p.performanceIndex = 0.0f;
        for(int j = 0; j < MAX_TEAMS; j++){
            if(strcmp(p.teamName, T[j].name) == 0){
                insertPlayer(&T[j], p);
                break;
            }
        }
    }
}

void printHeader(int showTeam){
    printf("============================================================\n");
    if(showTeam){
        printf("ID\tName\t\t\tTeam\t\tRole\t\tRuns\tAvg\tSR\tWkts\tER\tPerf. Index\n");
    }
    else{
        printf("ID\tName\t\t\tRole\t\tRuns\tAvg\tSR\tWkts\tER\tPerf. Index\n");
    }
    printf("============================================================\n");
}

void printPlayer(PlayerData p, int showTeam){
    printf("%d\t%s\t\t", p.playerId, p.name);
    if(showTeam){
        printf("%s\t\t", p.teamName);
    }
    printf("%s\t\t%d\t%.1f\t%.1f\t%d\t%.1f\t%.2f\n",
           p.role, p.totalRuns, p.battingAverage, p.strikeRate,
           p.wickets, p.economyRate, p.performanceIndex);
}

void heapifyDown(Node* heap[], int size, int idx){
    int largest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    if(left < size && heap[left]->data.performanceIndex > heap[largest]->data.performanceIndex){
        largest = left;
    }
    if(right < size && heap[right]->data.performanceIndex > heap[largest]->data.performanceIndex){
        largest = right;
    }
    if(largest != idx){
        Node* temp = heap[idx];
        heap[idx] = heap[largest];
        heap[largest] = temp;
        heapifyDown(heap, size, largest);
    }
}

void heapPush(Node* heap[], int* size, Node* item){
    int i = (*size)++;
    heap[i] = item;
    while(i > 0){
        int parent = (i - 1) / 2;
        if(heap[parent]->data.performanceIndex < heap[i]->data.performanceIndex){
            Node* temp = heap[parent];
            heap[parent] = heap[i];
            heap[i] = temp;
            i = parent;
        }
        else{
            break;
        }
    }
}

Node* heapPop(Node* heap[], int* size){
    Node* top = heap[0];
    heap[0] = heap[--(*size)];
    heapifyDown(heap, *size, 0);
    return top;
}

void addPlayer(){
    int teamId = getInt("Enter Team ID to add player: ");
    int idx = searchTeam(teamId);
    if(idx == -1){
        printf("Team not found!\n");
        return;
    }
    if(T[idx].totalPlayers >= MAX_PLAYERS_PER_TEAM){
        printf("Team is full!\n");
        return;
    }
    printf("Enter Player Details:\n");
    PlayerData p;
    p.playerId = getInt("Player ID: ");
    if(p.playerId < 1 || p.playerId > 1000 || playerExists(p.playerId)){
        printf("Invalid or Duplicate Player ID!\n");
        return;
    }
    getName("Name: ", p.name);
    strcpy(p.teamName, T[idx].name);
    int roleNum = getInt("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if(roleNum == 1){
        strcpy(p.role, "Batsman");
    }
    else if(roleNum == 2){
        strcpy(p.role, "Bowler");
    }
    else if(roleNum == 3){
        strcpy(p.role, "All-rounder");
    }
    else{
        printf("Invalid Role!\n");
        return;
    }
    p.totalRuns = getInt("Total Runs: ");
    if(p.totalRuns < 0){
        printf("Total Runs cannot be negative!\n");
        return;
    }
    p.battingAverage = getFloat("Batting Average: ");
    p.strikeRate = getFloat("Strike Rate: ");
    p.wickets = getInt("Wickets: ");
    if(p.wickets < 0){
        printf("Wickets cannot be negative!\n");
        return;
    }
    p.economyRate = getFloat("Economy Rate: ");
    insertPlayer(&T[idx], p);
    printf("Player added successfully to Team %s!\n", T[idx].name);
}

void displayTeam(){
    int teamId = getInt("Enter Team ID: ");
    int idx = searchTeam(teamId);
    if(idx == -1){
        printf("Team not found!\n");
        return;
    }
    printf("\nPlayers of Team %s:\n", T[idx].name);
    printHeader(0);
    Node* curr = T[idx].head;
    while(curr != NULL){
        printPlayer(curr->data, 0);
        curr = curr->next;
    }
    printf("============================================================\n");
    printf("Total Players: %d\n", T[idx].totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", T[idx].averageBattingStrikeRate);
}

void displayTeamsByStrikeRate(){
    Team* sorted[MAX_TEAMS];
    for(int i = 0; i < MAX_TEAMS; i++){
        sorted[i] = &T[i];
    }
    for(int i = 0; i < MAX_TEAMS - 1; i++){
        int maxIdx = i;
        for(int j = i + 1; j < MAX_TEAMS; j++){
            if(sorted[j]->averageBattingStrikeRate > sorted[maxIdx]->averageBattingStrikeRate){
                maxIdx = j;
            }
        }
        Team* temp = sorted[i];
        sorted[i] = sorted[maxIdx];
        sorted[maxIdx] = temp;
    }
    printf("\nTeams Sorted by Average Batting Strike Rate\n");
    printf("==================================================\n");
    printf("ID\tTeam Name\t\tAvg Bat SR\tTotal Players\n");
    printf("==================================================\n");
    for(int i = 0; i < MAX_TEAMS; i++){
        printf("%d\t%s\t\t%.1f\t\t%d\n",
               sorted[i]->teamId, sorted[i]->name,
               sorted[i]->averageBattingStrikeRate, sorted[i]->totalPlayers);
    }
    printf("==================================================\n");
}

void displayTopKPlayers(){
    int teamId = getInt("Enter Team ID: ");
    int idx = searchTeam(teamId);
    if(idx == -1){
        printf("Team not found!\n");
        return;
    }
    int roleNum = getInt("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if(roleNum < 1 || roleNum > 3){
        printf("Invalid Role!\n");
        return;
    }
    int k = getInt("Enter number of players: ");
    if(k <= 0){
        printf("Invalid K!\n");
        return;
    }
    printf("\nTop %d Players of Team %s:\n", k, T[idx].name);
    printHeader(0);
    Node* curr = T[idx].roleHead[roleNum - 1];
    int count = 0;
    while(curr != NULL && count < k){
        printPlayer(curr->data, 0);
        curr = curr->roleNext;
        count++;
    }
    printf("============================================================\n");
}

void displayAllPlayersByRole(){
    int roleNum = getInt("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if(roleNum < 1 || roleNum > 3){
        printf("Invalid Role!\n");
        return;
    }
    int r = roleNum - 1;
    Node* heap[MAX_TEAMS];
    int heapSize = 0;
    for(int i = 0; i < MAX_TEAMS; i++){
        if(T[i].roleHead[r] != NULL){
            heapPush(heap, &heapSize, T[i].roleHead[r]);
        }
    }
    printf("\nAll Players of Selected Role:\n");
    printHeader(1);
    while(heapSize > 0){
        Node* top = heapPop(heap, &heapSize);
        printPlayer(top->data, 1);
        if(top->roleNext != NULL){
            heapPush(heap, &heapSize, top->roleNext);
        }
    }
    printf("============================================================\n");
}

void freeAllMemory(){
    for(int i = 0; i < MAX_TEAMS; i++){
        Node* curr = T[i].head;
        while(curr != NULL){
            Node* nextNode = curr->next;
            free(curr);
            curr = nextNode;
        }
        T[i].head = NULL;
    }
}

void printMenu(){
    printf("\n==================================================\n");
    printf("ICC ODI Player Performance Analyzer\n");
    printf("==================================================\n");
    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Specific Team by Role\n");
    printf("5. Display all Players of specific role Across All Teams by performance index\n");
    printf("6. Exit\n");
    printf("==================================================\n");
}

int main(){
    initializeData();
    int choice = 0;
    while(choice != 6){
        printMenu();
        choice = getInt("Enter your choice: ");
        switch(choice){
            case 1:{
                addPlayer();
                break;
            }
            case 2:{
                displayTeam();
                break;
            }
            case 3:{
                displayTeamsByStrikeRate();
                break;
            }
            case 4:{
                displayTopKPlayers();
                break;
            }
            case 5:{
                displayAllPlayersByRole();
                break;
            }
            case 6:{
                printf("Exiting Program...\n");
                break;
            }
            default:{
                printf("Invalid Choice!\n");
                break;
            }
        }
    }
    freeAllMemory();
    return 0;
}