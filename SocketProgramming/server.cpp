#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <cstring>  
#include <algorithm> 
#include <iostream>
#include <vector>
#define max_lenght 100
#define num_col 8

using namespace std;
/*const int ROWS = 3; // �rnek: 3 sat�r
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
struct Terminal
{
    int id;
    string name;
    int socket;
    thread th;
};

// Aktif istemci terminal listesi ve renkler i�in de�i�kenler
vector<Terminal> clients;
string def_col = "\033[0m"; // Varsay�lan rengi s�f�rla
string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};
int seed = 0; 
mutex mtx_cout, clients_mtx; // Konsol ��kt�s� ve istemci listesi i�in mutexler


string color(int code);
void set_name(int id, const char name[]);

// Konsola ��kt� yazarken thread g�venli�i sa�layan fonksiyon
void shared_print(const string &str, bool endLine = true);

// Mesaj� t�m istemcilere ileten fonksiyonlar
int broadcast_message(const string &message, int sender_id);
int broadcast_message(int num, int sender_id);

// Ba�lant�y� sonland�ran fonksiyonlar
void end_connection(int id);
void handle_client(int client_socket, int id);

int main()
{
    /*string inputData;

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
    // Sunucu soketini olu�turma
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket: ");
        exit(-1);
    }

    // Sunucu adresi ayarlar�
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(10000);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&server.sin_zero, 0);

    // Sunucu soketine ba�lama
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1)
    {
        perror("bind error: ");
        exit(-1);
    }

    
    if (listen(server_socket, 8) == -1)
    {
        perror("listen error: ");
        exit(-1);
    }

    struct sockaddr_in client;
    int client_socket;
    unsigned int len = sizeof(sockaddr_in);

    // Sunucu ba�latma ve istemci ba�lant�lar�n� kabul etme
    cout << colors[num_col - 1] << "\n\t  ====== WELCOME TO THE LOBBY ======   " << endl
         << def_col;

    while (1)
    {
        // Yeni bir istemci ba�lant�s�n� kabul etme
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client, &len)) == -1)
        {
            perror("accept error: ");
            exit(-1);
        }

        // Her istemci i�in bir ID olu�turarak thread olu�turma ve listeye ekleme
        seed++;
        thread t(handle_client, client_socket, seed);
        lock_guard<mutex> guard(clients_mtx);
        clients.push_back({seed, "Anonymous", client_socket, move(t)});
    }

    // T�m thread'leri birle�tirme ve sunucu soketini kapatma
    for (auto &client : clients)
    {
        if (client.th.joinable())
            client.th.join();
    }

    close(server_socket);
    return 0;
}


string color(int code)
{
    return colors[code % num_col];
}

// �stemci ismini ayarlayan fonksiyon
void set_name(int id, const char name[])
{
    for (auto &client : clients)
    {
        if (client.id == id)
        {
            client.name = name;
        }
    }
}

// Thread g�venli�i sa�layan konsol ��kt�s� yazd�ran fonksiyon
void shared_print(const string &str, bool endLine)
{
    lock_guard<mutex> guard(mtx_cout);
    cout << str;
    if (endLine)
        cout << endl;
}

// Mesaj� t�m istemcilere ileten fonksiyonlar
int broadcast_message(const string &message, int sender_id)
{
    char temp[max_lenght];
    strcpy(temp, message.c_str());
    for (auto &client : clients)
    {
        if (client.id != sender_id)
        {
            send(client.socket, temp, sizeof(temp), 0);
        }
    }
    return 0;
}

int broadcast_message(int num, int sender_id)
{
    for (auto &client : clients)
    {
        if (client.id != sender_id)
        {
            send(client.socket, &num, sizeof(num), 0);
        }
    }
    return 0;
}

// �stemci ba�lant�s�n� sonland�ran fonksiyon
void end_connection(int id)
{
    auto it = remove_if(clients.begin(), clients.end(),
                        [id](const Terminal &client) { return client.id == id; });

    if (it != clients.end())
    {
        it->th.join();
        lock_guard<mutex> guard(clients_mtx);
        clients.erase(it, clients.end());
        close(it->socket);
    }
}

// �stemciye gelen mesajlar� i�leyen fonksiyon
void handle_client(int client_socket, int id)
{
    char name[max_lenght], str[max_lenght];
    int bytes_received = recv(client_socket, name, sizeof(name), 0);
    if (bytes_received <= 0)
    {
        cerr << "Error receiving client name." << endl;
        return;
    }

    // �stemci ismini ayarlama
    set_name(id, name);

    // Ho� geldin mesaj�n� yay�nlama
    string welcome_message = name + string(" has joined");
    broadcast_message("#NULL", id);
    broadcast_message(id, id);
    broadcast_message(welcome_message, id);
    shared_print(color(id) + welcome_message + def_col);

    // �stemci mesajlar�n� al�p yay�nlama d�ng�s�
    while (true)
    {
        bytes_received = recv(client_socket, str, sizeof(str), 0);
        if (bytes_received <= 0)
            return;

        // �stemci " #exit" g�nderdi�inde ba�lant�y� sonland�rma
        if (strcmp(str, "#exit") == 0)
        {
            string message = name + string(" has left");
            broadcast_message("#NULL", id);
            broadcast_message(id, id);
            broadcast_message(message, id);
            shared_print(color(id) + message + def_col);
            end_connection(id);
            return;
        }

        // �stemciden gelen mesaj� t�m istemcilere ileterek konsola yazd�rma
        broadcast_message(name, id);
        broadcast_message(id, id);
        broadcast_message(str, id);
        shared_print(color(id) + name + " : " + def_col + str);
    }
}
