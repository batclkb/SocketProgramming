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
/*const int ROWS = 3; // Örnek: 3 satýr
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
struct Terminal
{
    int id;
    string name;
    int socket;
    thread th;
};

// Aktif istemci terminal listesi ve renkler için deðiþkenler
vector<Terminal> clients;
string def_col = "\033[0m"; // Varsayýlan rengi sýfýrla
string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};
int seed = 0; 
mutex mtx_cout, clients_mtx; // Konsol çýktýsý ve istemci listesi için mutexler


string color(int code);
void set_name(int id, const char name[]);

// Konsola çýktý yazarken thread güvenliði saðlayan fonksiyon
void shared_print(const string &str, bool endLine = true);

// Mesajý tüm istemcilere ileten fonksiyonlar
int broadcast_message(const string &message, int sender_id);
int broadcast_message(int num, int sender_id);

// Baðlantýyý sonlandýran fonksiyonlar
void end_connection(int id);
void handle_client(int client_socket, int id);

int main()
{
    /*string inputData;

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
    // Sunucu soketini oluþturma
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket: ");
        exit(-1);
    }

    // Sunucu adresi ayarlarý
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(10000);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&server.sin_zero, 0);

    // Sunucu soketine baðlama
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

    // Sunucu baþlatma ve istemci baðlantýlarýný kabul etme
    cout << colors[num_col - 1] << "\n\t  ====== WELCOME TO THE LOBBY ======   " << endl
         << def_col;

    while (1)
    {
        // Yeni bir istemci baðlantýsýný kabul etme
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client, &len)) == -1)
        {
            perror("accept error: ");
            exit(-1);
        }

        // Her istemci için bir ID oluþturarak thread oluþturma ve listeye ekleme
        seed++;
        thread t(handle_client, client_socket, seed);
        lock_guard<mutex> guard(clients_mtx);
        clients.push_back({seed, "Anonymous", client_socket, move(t)});
    }

    // Tüm thread'leri birleþtirme ve sunucu soketini kapatma
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

// Ýstemci ismini ayarlayan fonksiyon
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

// Thread güvenliði saðlayan konsol çýktýsý yazdýran fonksiyon
void shared_print(const string &str, bool endLine)
{
    lock_guard<mutex> guard(mtx_cout);
    cout << str;
    if (endLine)
        cout << endl;
}

// Mesajý tüm istemcilere ileten fonksiyonlar
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

// Ýstemci baðlantýsýný sonlandýran fonksiyon
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

// Ýstemciye gelen mesajlarý iþleyen fonksiyon
void handle_client(int client_socket, int id)
{
    char name[max_lenght], str[max_lenght];
    int bytes_received = recv(client_socket, name, sizeof(name), 0);
    if (bytes_received <= 0)
    {
        cerr << "Error receiving client name." << endl;
        return;
    }

    // Ýstemci ismini ayarlama
    set_name(id, name);

    // Hoþ geldin mesajýný yayýnlama
    string welcome_message = name + string(" has joined");
    broadcast_message("#NULL", id);
    broadcast_message(id, id);
    broadcast_message(welcome_message, id);
    shared_print(color(id) + welcome_message + def_col);

    // Ýstemci mesajlarýný alýp yayýnlama döngüsü
    while (true)
    {
        bytes_received = recv(client_socket, str, sizeof(str), 0);
        if (bytes_received <= 0)
            return;

        // Ýstemci " #exit" gönderdiðinde baðlantýyý sonlandýrma
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

        // Ýstemciden gelen mesajý tüm istemcilere ileterek konsola yazdýrma
        broadcast_message(name, id);
        broadcast_message(id, id);
        broadcast_message(str, id);
        shared_print(color(id) + name + " : " + def_col + str);
    }
}
