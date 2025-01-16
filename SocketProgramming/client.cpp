#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <signal.h>
#include <mutex>
#include <iostream>
#include <vector>
#define max_lenght 100
#define num_col 8

using namespace std;
/*
const int ROWS = 3; // �rnek: 3 sat�r
const int COLS = 4; // �rnek: 4 s�tun

// Veriyi ve parite bitlerini i�eren iki boyutlu matris
vector<vector<int>> matrix(ROWS, vector<int>(COLS + 1, 0));

// Veriyi matrise yerle�tirme
void fillMatrix(const string &data) {
    int dataIndex = 0;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = data[dataIndex++] - '0';
        }
    }
}

// Parite bitlerini hesaplama
void calculateParityBits() {
    for (int i = 0; i < ROWS; i++) {
        int rowParity = 0;
        for (int j = 0; j < COLS; j++) {
            rowParity ^= matrix[i][j];
        }
        matrix[i][COLS] = rowParity;
    }

    for (int j = 0; j < COLS; j++) {
        int colParity = 0;
        for (int i = 0; i < ROWS; i++) {
            colParity ^= matrix[i][j];
        }
        matrix[ROWS - 1][j] = colParity;
    }
}

// Hata kontrol� yapma
bool checkParityBits() {
    for (int i = 0; i < ROWS; i++) {
        int rowParity = 0;
        for (int j = 0; j < COLS; j++) {
            rowParity ^= matrix[i][j];
        }
        if (rowParity != matrix[i][COLS]) {
            return false; // Hata var
        }
    }

    for (int j = 0; j < COLS; j++) {
        int colParity = 0;
        for (int i = 0; i < ROWS; i++) {
            colParity ^= matrix[i][j];
        }
        if (colParity != matrix[ROWS - 1][j]) {
            return false; // Hata var
        }
    }

    return true; // Hata yok
}

// Matrisi ekrana yazd�rma
void printMatrix() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS + 1; j++) {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}
*/

// ��k�� bayra�� ve thread'ler i�in de�i�kenler
bool exit_flag = false;
thread t_send, t_recv;
int client_socket;
string def_col = "\033[0m"; // Varsay�lan renk
string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"}; // Renklerin listesi

// Ctrl+C sinyali i�in i�leyici
void catch_ctrl_c(int signal);

// Renk d�nd�ren fonksiyon
string color(int code);

// Mesaj g�nderen fonksiyon
void send_message(int client_socket);

// Mesaj alan fonksiyon
void recv_message(int client_socket);

int main()
{
    /* string inputData;

    // Veriyi kullan�c�dan al
    cout << "Enter binary data (length should be " << ROWS * COLS << "): ";
    cin >> inputData;

    // Veriyi matrise yerle�tirme
    fillMatrix(inputData);

    // Parite bitlerini hesaplama
    calculateParityBits();

    // Matrisi ekrana yazd�rma
    cout << "Matrix with parity bits:" << endl;
    printMatrix();

    // Hata kontrol� yapma
    if (checkParityBits()) {
        cout << "No error detected. Data is correct." << endl;
    } else {
        cout << "Error detected. Data may be corrupted." << endl;
    }
*/
    // Soket olu�turma
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket: ");
        exit(-1);
    }

    // Sunucu bilgileri
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(10000); // Sunucunun port numaras�
    client.sin_addr.s_addr = INADDR_ANY;
    //client.sin_addr.s_addr=inet_addr("127.0.0.1"); // Sunucunun IP adresini sa�lay�n
    bzero(&client.sin_zero, 0);

    // Sunucuya ba�lanma
    if ((connect(client_socket, (struct sockaddr *)&client, sizeof(struct sockaddr_in))) == -1)
    {
        perror("connect: ");
        exit(-1);
    }

    // Ctrl+C sinyali i�leyiciyi ayarlama
    signal(SIGINT, catch_ctrl_c);

    char name[max_lenght];
    cout << "Enter your name : ";
    cin.getline(name, max_lenght);
    send(client_socket, name, sizeof(name), 0);

    cout << colors[num_col - 1] << "\n\t  ====== Welcome to the chat-room ======   " << endl
         << def_col;

    // Mesaj g�nderme ve alma i�lemleri i�in thread'leri ba�latma
    thread t1(send_message, client_socket);
    thread t2(recv_message, client_socket);

    t_send = move(t1);
    t_recv = move(t2);

    // Thread'leri birle�tirme
    if (t_send.joinable())
        t_send.join();
    if (t_recv.joinable())
        t_recv.join();

    // Soketi kapatma
    close(client_socket);
    return 0;
}

// Ctrl+C sinyali i�leyici
void catch_ctrl_c(int signal)
{
    char str[max_lenght] = "#exit";
    send(client_socket, str, sizeof(str), 0);
    exit_flag = true;
    t_send.detach();
    t_recv.detach();
    close(client_socket);
    exit(signal);
}

// Renk d�nd�ren fonksiyon
string color(int code)
{
    return colors[code % num_col];
}

// Mesaj g�nderme fonksiyonu
void send_message(int client_socket)
{
    while (1)
    {
        cout << colors[1] << "You : " << def_col; // Kendi mesaj�n�z� yazmak i�in renk ayar�
        char str[max_lenght];
        cin.getline(str, max_lenght);
        send(client_socket, str, sizeof(str), 0);
        if (strcmp(str, "#exit") == 0)
        {
            exit_flag = true;
            t_recv.detach();
            return;
        }
    }
}

// Mesaj alma fonksiyonu
void recv_message(int client_socket)
{
    while (1)
    {
        if (exit_flag)
            return;
        char name[max_lenght], str[max_lenght];
        int color_code;
        int bytes_received = recv(client_socket, name, sizeof(name), 0);
        if (bytes_received <= 0)
            continue;
        recv(client_socket, &color_code, sizeof(color_code), 0);
        recv(client_socket, str, sizeof(str), 0);
        cout << "\033[K"; // Mesaj� yazmadan �nce sat�r� temizleme
        if (strcmp(name, "#NULL") != 0)
            cout << color(color_code) << name << " : " << def_col << str << endl; // Di�er kullan�c�lar�n mesajlar�n� ekrana yazd�rma
        else
            cout << color(color_code) << str << endl; // Sunucu mesajlar�n� ekrana yazd�rma
        cout << colors[1] << "You : " << def_col; // Kendi mesaj�n�z� yazmak i�in renk ayar�
        fflush(stdout);
    }
}
