#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ctype.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 2048
#define MAX_CLIENTS 5

// Global State
SOCKET clients[MAX_CLIENTS];
char usernames[MAX_CLIENTS][50];
char document[BUFFER_SIZE] = "Welcome! Try: *bold*, _italic_, or #Header";
char locked_by[50] = "None";
CRITICAL_SECTION doc_lock;

// --- UTILS ---
void flush_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int word_count(char *text) {
    int count = 0, inWord = 0;
    for (int i = 0; text[i]; i++) {
        if (isspace(text[i])) inWord = 0;
        else if (!inWord) { inWord = 1; count++; }
    }
    return count;
}

// --- RICH TEXT RENDERER ---
void render(char *text) {
    for (int i = 0; text[i]; i++) {
        if (text[i] == '*') { // Bold
            printf("\033[1m"); i++;
            while (text[i] && text[i] != '*') putchar(text[i++]);
            printf("\033[0m");
        } else if (text[i] == '_') { // Italic
            printf("\033[3m"); i++;
            while (text[i] && text[i] != '_') putchar(text[i++]);
            printf("\033[0m");
        } else if (text[i] == '#') { // Cyan Header
            printf("\033[1;36m"); i++;
            while (text[i] && text[i] != '\n' && text[i] != '\0') putchar(text[i++]);
            printf("\033[0m");
        } else putchar(text[i]);
    }
    printf("\n");
}

// --- SERVER LOGIC ---
void broadcast(char *msg) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != 0) send(clients[i], msg, (int)strlen(msg), 0);
    }
}

DWORD WINAPI handle_client(LPVOID arg) {
    SOCKET sock = *(SOCKET*)arg;
    char buffer[BUFFER_SIZE], name[50];
    int idx = -1;

    if (recv(sock, name, 50, 0) <= 0) return 0;
    
    EnterCriticalSection(&doc_lock);
    for(int i=0; i<MAX_CLIENTS; i++) if(clients[i] == sock) { idx = i; strcpy(usernames[i], name); break; }
    LeaveCriticalSection(&doc_lock);

    while (1) {
        int len = recv(sock, buffer, BUFFER_SIZE, 0);
        if (len <= 0) break;
        buffer[len] = '\0';

        EnterCriticalSection(&doc_lock);
        if (strcmp(locked_by, "None") == 0 || strcmp(locked_by, name) == 0) {
            strcpy(locked_by, name);
            strcpy(document, buffer);
            
            char out[BUFFER_SIZE + 500];
            sprintf(out, "\n\033[H\033[J✨ COLLAB-C | User: %s\n------------------\n%s\n------------------\nWords: %d | Locked: %s\n", 
                    name, document, word_count(document), locked_by);
            broadcast(out);
        } else {
            char deny[] = "⚠ Document is currently locked by someone else!\n";
            send(sock, deny, (int)strlen(deny), 0);
        }
        LeaveCriticalSection(&doc_lock);
    }
    clients[idx] = 0;
    closesocket(sock);
    return 0;
}

void run_server() {
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
    InitializeCriticalSection(&doc_lock);
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { AF_INET, htons(PORT), INADDR_ANY };
    
    bind(s, (struct sockaddr*)&addr, sizeof(addr));
    listen(s, 3);
    printf("🚀 Server Live on Port %d\n", PORT);

    while(1) {
        SOCKET ns = accept(s, NULL, NULL);
        for(int i=0; i<MAX_CLIENTS; i++) {
            if(clients[i] == 0) {
                clients[i] = ns;
                SOCKET *p = malloc(sizeof(SOCKET)); *p = ns;
                CreateThread(NULL, 0, handle_client, p, 0, NULL);
                break;
            }
        }
    }
}

// --- CLIENT LOGIC ---
void run_client() {
    char user[50], pass[50], buffer[BUFFER_SIZE];
    
    printf("\n--- LOGIN ---\n");
    printf("Username: "); scanf("%49s", user);
    printf("Password: "); scanf("%49s", pass);
    flush_stdin(); // CRITICAL: Clear the buffer for the editor

    if (strcmp(pass, "123") != 0) { printf("Wrong pass!\n"); return; }

    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { AF_INET, htons(PORT) };
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) { printf("Offline!\n"); return; }
    send(s, user, (int)strlen(user), 0);

    // Receiver Thread for live updates
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recv, (void*)s, 0, NULL); 

    while(1) {
        printf("\nEdit (or type 'exit'): ");
        if (!fgets(buffer, BUFFER_SIZE, stdin)) break;
        buffer[strcspn(buffer, "\n")] = 0;
        if(strcmp(buffer, "exit") == 0) break;
        send(s, buffer, (int)strlen(buffer), 0);
        
        // Wait a tiny bit for broadcast then render
        Sleep(100); 
    }
}

int main() {
    int c;
    printf("1. Server\n2. Client\n> ");
    if(scanf("%d", &c) != 1) return 0;
    if(c == 1) run_server(); else run_client();
    return 0;
}