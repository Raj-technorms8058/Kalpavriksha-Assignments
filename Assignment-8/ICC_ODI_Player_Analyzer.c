C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

#include "Players_data.h"

#define MAX_TEAMS 10
#define MAX_PLAYERS_PER_TEAM 50
#define MAX_NAME_LEN 51

typedef enum
{
    ROLE_BATSMAN = 0,
    ROLE_BOWLER = 1,
    ROLE_ALL_ROUNDER = 2,
    ROLE_INVALID = -1
} Role;

typedef struct
{
    int playerId;
    char name[MAX_NAME_LEN];
    char teamName[MAX_NAME_LEN];
    Role role;
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
} PlayerData;

typedef struct Node
{
    PlayerData data;
    struct Node* next;
    struct Node* roleNext;
} Node;

typedef struct
{
    int teamId;
    char name[MAX_NAME_LEN];
    int totalPlayers;
    float averageBattingStrikeRate;
    Node* head;
    Node* roleHead[3];
} Team;

Team T[MAX_TEAMS];

void clearInputBuffer()
{
    int ch = getchar();
    while (ch != '\n' && ch != EOF)
    {
        ch = getchar();
    }
}

int readLine(const char* msg, char* buf, size_t size)
{
    printf("%s", msg);
    if (fgets(buf, size, stdin) == NULL) return -1;
    if (strchr(buf, '\n') == NULL)
    {
        clearInputBuffer();
        printf("Input too long!\n");
        return 0;
    }
    return 1;
}

int getInt(const char* msg, int* out)
{
    char buffer[100], *end;
    long value;
    while (1)
    {
        int status = readLine(msg, buffer, sizeof(buffer));
        if (status == -1) return 0;
        if (status == 0) continue;

        char* start = buffer;
        while (isspace(*start)) start++;
        if (*start == '\0')
        {
            printf("Empty input!\n");
            continue;
        }
        value = strtol(start, &end, 10);
        if (start == end)
        {
            printf("Invalid integer!\n");
            continue;
        }
        while (isspace(*end)) end++;
        if (*end != '\0')
        {
            printf("Extra characters not allowed!\n");
            continue;
        }
        if (value < INT_MIN || value > INT_MAX)
        {
            printf("Integer out of range!\n");
            continue;
        }
        *out = (int)value;
        return 1;
    }
}

int getIntInRange(const char* msg, int min, int max, int* out)
{
    while (1)
    {
        if (!getInt(msg, out)) return 0;
        if (*out < min || *out > max)
        {
            printf("Value must be between %d and %d!\n", min, max);
            continue;
        }
        return 1;
    }
}

int getFloat(const char* msg, float* out)
{
    char buffer[100], *end;
    float value;
    while (1)
    {
        int status = readLine(msg, buffer, sizeof(buffer));
        if (status == -1) return 0;
        if (status == 0) continue;

        char* start = buffer;
        while (isspace(*start)) start++;
        if (*start == '\0')
        {
            printf("Empty input!\n");
            continue;
        }
        value = strtof(start, &end);
        if (start == end)
        {
            printf("Invalid number!\n");
            continue;
        }
        while (isspace(*end)) end++;
        if (*end != '\0')
        {
            printf("Extra characters not allowed!\n");
            continue;
        }
        if (!isfinite(value))
        {
            printf("Invalid floating-point value!\n");
            continue;
        }
        if (value < 0.0f)
        {
            printf("Value cannot be negative!\n");
            continue;
        }
        *out = value;
        return 1;
    }
}

int getName(const char* msg, char* dest)
{
    char buffer[100];
    while (1)
    {
        int status = readLine(msg, buffer, sizeof(buffer));
        if (status == -1) return 0;
        if (status == 0) continue;

        char* start = buffer;
        while (isspace(*start)) start++;
        size_t len = strlen(start);
        while (len > 0 && isspace(start[len - 1]))
        {
            start[len - 1] = '\0';
            len--;
        }
        if (len < 1 || len > 50)
        {
            printf("Invalid name! Length must be 1 to 50.\n");
            continue;
        }
        int valid = 1;
        for (size_t i = 0; i < len; i++)
        {
            if (!isalpha(start[i]) && start[i] != ' ')
            {
                valid = 0;
                break;
            }
        }
        if (!valid)
        {
            printf("Invalid name! Only letters and spaces allowed.\n");
            continue;
        }
        strcpy(dest, start);
        return 1;
    }
}

Role parseRoleString(const char* roleStr)
{
    if (strcmp(roleStr, "Batsman") == 0) return ROLE_BATSMAN;
    if (strcmp(roleStr, "Bowler") == 0) return ROLE_BOWLER;
    if (strcmp(roleStr, "All-rounder") == 0) return ROLE_ALL_ROUNDER;
    return ROLE_INVALID;
}

const char* getRoleName(Role role)
{
    if (role == ROLE_BATSMAN) return "Batsman";
    if (role == ROLE_BOWLER) return "Bowler";
    if (role == ROLE_ALL_ROUNDER) return "All-rounder";
    return "Unknown";
}

float calculatePI(PlayerData p)
{
    if (p.role == ROLE_BATSMAN) return (p.battingAverage * p.strikeRate) / 100.0f;
    if (p.role == ROLE_BOWLER) return (p.wickets * 2.0f) + (100.0f - p.economyRate);
    return ((p.battingAverage * p.strikeRate) / 100.0f) + (p.wickets * 2.0f);
}

void updateAverageStrikeRate(Team* t)
{
    float sum = 0.0f;
    int count = 0;
    Node* curr = t->head;
    while (curr != NULL)
    {
        if (curr->data.role == ROLE_BATSMAN || curr->data.role == ROLE_ALL_ROUNDER)
        {
            sum += curr->data.strikeRate;
            count++;
        }
        curr = curr->next;
    }
    t->averageBattingStrikeRate = (count == 0) ? 0.0f : (sum / count);
}

int searchTeam(int teamId)
{
    int low = 0, high = MAX_TEAMS - 1;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        if (T[mid].teamId == teamId) return mid;
        if (T[mid].teamId < teamId) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

int playerExists(int playerId)
{
    for (int i = 0; i < MAX_TEAMS; i++)
    {
        Node* curr = T[i].head;
        while (curr != NULL)
        {
            if (curr->data.playerId == playerId) return 1;
            curr = curr->next;
        }
    }
    return 0;
}

void insertSortedRole(Team* t, Node* newNode)
{
    if (newNode->data.role == ROLE_INVALID) return;
    int r = (int)newNode->data.role;
    if (t->roleHead[r] == NULL || t->roleHead[r]->data.performanceIndex < newNode->data.performanceIndex)
    {
        newNode->roleNext = t->roleHead[r];
        t->roleHead[r] = newNode;
        return;
    }
    Node* curr = t->roleHead[r];
    while (curr->roleNext != NULL && curr->roleNext->data.performanceIndex >= newNode->data.performanceIndex)
    {
        curr = curr->roleNext;
    }
    newNode->roleNext = curr->roleNext;
    curr->roleNext = newNode;
}

int insertPlayer(Team* t, PlayerData p)
{
    if (p.role == ROLE_INVALID) return 0;
    p.performanceIndex = calculatePI(p);
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL)
    {
        printf("Memory allocation failed!\n");
        return 0;
    }
    newNode->data = p;
    newNode->next = NULL;
    newNode->roleNext = NULL;
    if (t->head == NULL)
    {
        t->head = newNode;
    }
    else
    {
        Node* temp = t->head;
        while (temp->next != NULL) temp = temp->next;
        temp->next = newNode;
    }
    t->totalPlayers++;
    insertSortedRole(t, newNode);
    updateAverageStrikeRate(t);
    return 1;
}

void initializeData()
{
    for (int i = 0; i < MAX_TEAMS; i++)
    {
        T[i].teamId = i + 1;
        strcpy(T[i].name, teams[i]);
        T[i].totalPlayers = 0;
        T[i].averageBattingStrikeRate = 0.0f;
        T[i].head = NULL;
        for (int r = 0; r < 3; r++) T[i].roleHead[r] = NULL;
    }
    for (int i = 0; i < playerCount; i++)
    {
        PlayerData p;
        p.playerId = players[i].id;
        strcpy(p.name, players[i].name);
        strcpy(p.teamName, players[i].team);
        p.role = parseRoleString(players[i].role);
        p.totalRuns = players[i].totalRuns;
        p.battingAverage = players[i].battingAverage;
        p.strikeRate = players[i].strikeRate;
        p.wickets = players[i].wickets;
        p.economyRate = players[i].economyRate;
        p.performanceIndex = 0.0f;
        for (int j = 0; j < MAX_TEAMS; j++)
        {
            if (strcmp(p.teamName, T[j].name) == 0)
            {
                insertPlayer(&T[j], p);
                break;
            }
        }
    }
}

void printHeader(int showTeam)
{
    printf("============================================================\n");
    if (showTeam)
    {
        printf("ID\tName\t\t\tTeam\t\tRole\t\tRuns\tAvg\tSR\tWkts\tER\tPerf. Index\n");
    }
    else
    {
        printf("ID\tName\t\t\tRole\t\tRuns\tAvg\tSR\tWkts\tER\tPerf. Index\n");
    }
    printf("============================================================\n");
}

void printPlayer(PlayerData p, int showTeam)
{
    printf("%d\t%s\t\t", p.playerId, p.name);
    if (showTeam) printf("%s\t\t", p.teamName);
    printf("%s\t\t%d\t%.1f\t%.1f\t%d\t%.1f\t%.2f\n",
           getRoleName(p.role), p.totalRuns, p.battingAverage, p.strikeRate,
           p.wickets, p.economyRate, p.performanceIndex);
}

void heapifyDown(Node* heap[], int size, int idx)
{
    int largest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    if (left < size && heap[left]->data.performanceIndex > heap[largest]->data.performanceIndex) largest = left;
    if (right < size && heap[right]->data.performanceIndex > heap[largest]->data.performanceIndex) largest = right;
    if (largest != idx)
    {
        Node* temp = heap[idx];
        heap[idx] = heap[largest];
        heap[largest] = temp;
        heapifyDown(heap, size, largest);
    }
}

void heapPush(Node* heap[], int* size, Node* item)
{
    int i = (*size)++;
    heap[i] = item;
    while (i > 0)
    {
        int parent = (i - 1) / 2;
        if (heap[parent]->data.performanceIndex < heap[i]->data.performanceIndex)
        {
            Node* temp = heap[parent];
            heap[parent] = heap[i];
            heap[i] = temp;
            i = parent;
        }
        else break;
    }
}

Node* heapPop(Node* heap[], int* size)
{
    Node* top = heap[0];
    heap[0] = heap[--(*size)];
    heapifyDown(heap, *size, 0);
    return top;
}

int addPlayer()
{
    int teamId;
    if (!getInt("Enter Team ID to add player: ", &teamId)) return 0;
    int idx = searchTeam(teamId);
    if (idx == -1)
    {
        printf("Team not found!\n");
        return 1;
    }
    if (T[idx].totalPlayers >= MAX_PLAYERS_PER_TEAM)
    {
        printf("Team is full!\n");
        return 1;
    }
    printf("Enter Player Details:\n");
    PlayerData p;
    if (!getIntInRange("Player ID: ", 1, 1000, &p.playerId)) return 0;
    if (playerExists(p.playerId))
    {
        printf("Duplicate Player ID!\n");
        return 1;
    }
    if (!getName("Name: ", p.name)) return 0;
    strcpy(p.teamName, T[idx].name);

    int roleNum;
    if (!getIntInRange("Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 3, &roleNum)) return 0;
    p.role = (Role)(roleNum - 1);

    if (!getIntInRange("Total Runs: ", 0, INT_MAX, &p.totalRuns)) return 0;
    if (!getFloat("Batting Average: ", &p.battingAverage)) return 0;
    if (!getFloat("Strike Rate: ", &p.strikeRate)) return 0;
    if (!getIntInRange("Wickets: ", 0, INT_MAX, &p.wickets)) return 0;
    if (!getFloat("Economy Rate: ", &p.economyRate)) return 0;

    if (insertPlayer(&T[idx], p)) printf("Player added successfully to Team %s!\n", T[idx].name);
    return 1;
}

int displayTeam()
{
    int teamId;
    if (!getInt("Enter Team ID: ", &teamId)) return 0;
    int idx = searchTeam(teamId);
    if (idx == -1)
    {
        printf("Team not found!\n");
        return 1;
    }
    printf("\nPlayers of Team %s:\n", T[idx].name);
    printHeader(0);
    Node* curr = T[idx].head;
    while (curr != NULL)
    {
        printPlayer(curr->data, 0);
        curr = curr->next;
    }
    printf("============================================================\n");
    printf("Total Players: %d\n", T[idx].totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", T[idx].averageBattingStrikeRate);
    return 1;
}

void displayTeamsByStrikeRate()
{
    Team* sorted[MAX_TEAMS];
    for (int i = 0; i < MAX_TEAMS; i++) sorted[i] = &T[i];
    for (int i = 0; i < MAX_TEAMS - 1; i++)
    {
        int maxIdx = i;
        for (int j = i + 1; j < MAX_TEAMS; j++)
        {
            if (sorted[j]->averageBattingStrikeRate > sorted[maxIdx]->averageBattingStrikeRate) maxIdx = j;
        }
        Team* temp = sorted[i];
        sorted[i] = sorted[maxIdx];
        sorted[maxIdx] = temp;
    }
    printf("\nTeams Sorted by Average Batting Strike Rate\n");
    printf("==================================================\n");
    printf("ID\tTeam Name\t\tAvg Bat SR\tTotal Players\n");
    printf("==================================================\n");
    for (int i = 0; i < MAX_TEAMS; i++)
    {
        printf("%d\t%s\t\t%.1f\t\t%d\n",
               sorted[i]->teamId, sorted[i]->name,
               sorted[i]->averageBattingStrikeRate, sorted[i]->totalPlayers);
    }
    printf("==================================================\n");
}

int displayTopKPlayers()
{
    int teamId;
    if (!getInt("Enter Team ID: ", &teamId)) return 0;
    int idx = searchTeam(teamId);
    if (idx == -1)
    {
        printf("Team not found!\n");
        return 1;
    }
    int roleNum, k;
    if (!getIntInRange("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 3, &roleNum)) return 0;
    if (!getIntInRange("Enter number of players: ", 1, INT_MAX, &k)) return 0;

    printf("\nTop %d Players of Team %s:\n", k, T[idx].name);
    printHeader(0);
    Node* curr = T[idx].roleHead[roleNum - 1];
    int count = 0;
    while (curr != NULL && count < k)
    {
        printPlayer(curr->data, 0);
        curr = curr->roleNext;
        count++;
    }
    printf("============================================================\n");
    return 1;
}

int displayAllPlayersByRole()
{
    int roleNum;
    if (!getIntInRange("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 3, &roleNum)) return 0;
    int r = roleNum - 1;
    Node* heap[MAX_TEAMS];
    int heapSize = 0;
    for (int i = 0; i < MAX_TEAMS; i++)
    {
        if (T[i].roleHead[r] != NULL) heapPush(heap, &heapSize, T[i].roleHead[r]);
    }
    printf("\nAll Players of Selected Role:\n");
    printHeader(1);
    while (heapSize > 0)
    {
        Node* top = heapPop(heap, &heapSize);
        printPlayer(top->data, 1);
        if (top->roleNext != NULL) heapPush(heap, &heapSize, top->roleNext);
    }
    printf("============================================================\n");
    return 1;
}

void freeAllMemory()
{
    for (int i = 0; i < MAX_TEAMS; i++)
    {
        Node* curr = T[i].head;
        while (curr != NULL)
        {
            Node* nextNode = curr->next;
            free(curr);
            curr = nextNode;
        }
        T[i].head = NULL;
        for (int r = 0; r < 3; r++) T[i].roleHead[r] = NULL;
    }
}

void printMenu()
{
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

int main()
{
    initializeData();
    int choice = 0;
    while (choice != 6)
    {
        printMenu();
        if (!getInt("Enter your choice: ", &choice))
        {
            printf("\nInput stream closed. Exiting...\n");
            break;
        }
        switch (choice)
        {
            case 1:
                if (!addPlayer()) choice = 6;
                break;
            case 2:
                if (!displayTeam()) choice = 6;
                break;
            case 3:
                displayTeamsByStrikeRate();
                break;
            case 4:
                if (!displayTopKPlayers()) choice = 6;
                break;
            case 5:
                if (!displayAllPlayersByRole()) choice = 6;
                break;
            case 6:
                printf("Exiting Program...\n");
                break;
            default:
                printf("Invalid Choice!\n");
                break;
        }
    }
    freeAllMemory();
    return 0;
}