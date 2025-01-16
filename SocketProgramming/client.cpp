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
const int ROWS = 3; // Örnek: 3 satýr
const int COLS = 4; // Örnek: 4 sütun

// Veriyi ve parite bitlerini içeren iki boyutlu matris
vector<vector<int>> matrix(ROWS, vector<int>(COLS + 1, 0));

// Veriyi matrise yerleþtirme
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

// Hata kontrolü yapma
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

// Matrisi ekrana yazdýrma
void printMatrix() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS + 1; j++) {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}
*/

// Çýkýþ bayraðý ve thread'ler için deðiþkenler
bool exit_flag = false;
thread t_send, t_recv;
int client_socket;
string def_col = "\033[0m"; // Varsayýlan renk
string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"}; // Renklerin listesi

// Ctrl+C sinyali için iþleyici
void catch_ctrl_c(int signal);

// Renk döndüren fonksiyon
string color(int code);

// Mesaj gönderen fonksiyon
void send_message(int client_socket);

// Mesaj alan fonksiyon
void recv_message(int client_socket);

int main()
{
    /* string inputData;

    // Veriyi kullanýcýdan al
    cout << "Enter binary data (length should be " << ROWS * COLS << "): ";
    cin >> inputData;

    // Veriyi matrise yerleþtirme
    fillMatrix(inputData);

    // Parite bitlerini hesaplama
    calculateParityBits();

    // Matrisi ekrana yazdýrma
    cout << "Matrix with parity bits:" << endl;
    printMatrix();

    // Hata kontrolü yapma
    if (checkParityBits()) {
        cout << "No error detected. Data is correct." << endl;
    } else {
        cout << "Error detected. Data may be corrupted." << endl;
    }
*/
    // Soket oluþturma
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket: ");
        exit(-1);
    }

    // Sunucu bilgileri
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(10000); // Sunucunun port numarasý
    client.sin_addr.s_addr = INADDR_ANY;
    //client.sin_addr.s_addr=inet_addr("127.0.0.1"); // Sunucunun IP adresini saðlayýn
    bzero(&client.sin_zero, 0);

    // Sunucuya baðlanma
    if ((connect(client_socket, (struct sockaddr *)&client, sizeof(struct sockaddr_in))) == -1)
    {
        perror("connect: ");
        exit(-1);
    }

    // Ctrl+C sinyali iþleyiciyi ayarlama
    signal(SIGINT, catch_ctrl_c);

    char name[max_lenght];
    cout << "Enter your name : ";
    cin.getline(name, max_lenght);
    send(client_socket, name, sizeof(name), 0);

    cout << colors[num_col - 1] << "\n\t  ====== Welcome to the chat-room ======   " << endl
         << def_col;

    // Mesaj gönderme ve alma iþlemleri için thread'leri baþlatma
    thread t1(send_message, client_socket);
    thread t2(recv_message, client_socket);

    t_send = move(t1);
    t_recv = move(t2);

    // Thread'leri birleþtirme
    if (t_send.joinable())
        t_send.join();
    if (t_recv.joinable())
        t_recv.join();

    // Soketi kapatma
    close(client_socket);
    return 0;
}

// Ctrl+C sinyali iþleyici
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

// Renk döndüren fonksiyon
string color(int code)
{
    return colors[code % num_col];
}

// Mesaj gönderme fonksiyonu
void send_message(int client_socket)
{
    while (1)
    {
        cout << colors[1] << "You : " << def_col; // Kendi mesajýnýzý yazmak için renk ayarý
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
        cout << "\033[K"; // Mesajý yazmadan önce satýrý temizleme
        if (strcmp(name, "#NULL") != 0)
            cout << color(color_code) << name << " : " << def_col << str << endl; // Diðer kullanýcýlarýn mesajlarýný ekrana yazdýrma
        else
            cout << color(color_code) << str << endl; // Sunucu mesajlarýný ekrana yazdýrma
        cout << colors[1] << "You : " << def_col; // Kendi mesajýnýzý yazmak için renk ayarý
        fflush(stdout);
    }
}
