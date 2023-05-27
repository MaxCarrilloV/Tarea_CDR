#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>

#define BOARD_SIZE 15
#define PORTA_AVIONES 5
#define BUQUE 4
#define SUBMARINO 3
#define LANCHA 1

using namespace std;

// Estructura para almacenar la información de un barco
struct Ship {
    char symbol;
    int size;
    int hits;
};

// Clase del servidor
class Server {
private:
    int serverSocket;
    struct sockaddr_in serverAddress;
    vector<int> clientSockets;

    char board[BOARD_SIZE][BOARD_SIZE];  // Tablero del servidor
    char clientBoard[BOARD_SIZE][BOARD_SIZE];  // Tablero del cliente

    Ship ships[8];  // Array de barcos

public:
    void run(int port) {
        initializeServer(port);
        

        while (true) {
            waitForConnections();
        }
    }

private:
    void initializeServer(int port) {
        // Crear socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            cerr << "Error al crear el socket." << endl;
            exit(1);
        }

        // Configurar la dirección del servidor
        memset((char*)&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(port);

        // Vincular el socket a la dirección del servidor
        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
            cerr << "Error al vincular el socket al puerto." << endl;
            exit(1);
        }

        // Escuchar conexiones entrantes
        listen(serverSocket, 5);
    }

    void initializeGame() {
        // Inicializar tableros
        memset(board, ' ', sizeof(board));
        memset(clientBoard, ' ', sizeof(clientBoard));

        // Inicializar barcos
        ships[0] = {'P', PORTA_AVIONES, 0};
        ships[1] = {'B', BUQUE, 0};
        ships[2] = {'B', BUQUE, 0};
        ships[3] = {'S', SUBMARINO, 0};
        ships[4] = {'S', SUBMARINO, 0};
        ships[5] = {'L', LANCHA, 0};
        ships[6] = {'L', LANCHA, 0};
        ships[7] = {'L', LANCHA, 0};

        // Colocar barcos en el tablero de manera aleatoria
        srand(time(0));
        for (int i = 0; i < 8; i++) {
            bool shipPlaced = false;
            while (!shipPlaced) {
                int x = rand() % BOARD_SIZE;
                int y = rand() % BOARD_SIZE;
                int direction = rand() % 2;  // 0: horizontal, 1: vertical

                if (canPlaceShip(x, y, ships[i].size, direction)) {
                    placeShip(x, y, ships[i].size, direction, ships[i].symbol);
                    shipPlaced = true;
                }
            }
        }
    }

    bool canPlaceShip(int x, int y, int size, int direction) {
        if (direction == 0) {  // Horizontal
            if (x + size > BOARD_SIZE) {
                return false;
            }
            for (int i = x; i < x + size; i++) {
                if (board[i][y] != ' ') {
                    return false;
                }
            }
        } else {  // Vertical
            if (y + size > BOARD_SIZE) {
                return false;
            }
            for (int i = y; i < y + size; i++) {
                if (board[x][i] != ' ') {
                    return false;
                }
            }
        }
        return true;
    }

    void placeShip(int x, int y, int size, int direction, char symbol) {
        if (direction == 0) {  // Horizontal
            for (int i = x; i < x + size; i++) {
                board[i][y] = symbol;
            }
        } else {  // Vertical
            for (int i = y; i < y + size; i++) {
                board[x][i] = symbol;
            }
        }
    }

    void waitForConnections() {
        // Esperar conexiones de los clientes
        cout << "Esperando conexiones..." << endl;

        struct sockaddr_in clientAddress;
        socklen_t clientLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLength);
        if (clientSocket < 0) {
            cerr << "Error al aceptar la conexión." << endl;
            exit(1);
        }

        // Agregar socket del cliente a la lista
        clientSockets.push_back(clientSocket);

        // Crear un hilo para manejar al nuevo cliente
        thread t(&Server::playGame, this, clientSocket);
        t.detach();

        cout << "Cliente conectado." << endl;
        initializeGame();
    }

    void playGame(int clientSocket) {
        int currentPlayer = rand() % 2;  // 0: servidor, 1: cliente

        // Enviar tablero del servidor al cliente
        sendBoardToClient(clientSocket);
        sendBoardPlayerToClient(clientSocket);

        // Ciclo del juego
        while (!isGameOver()) {
            // Enviar turno actual al cliente
            sendTurnToClient(clientSocket, currentPlayer);

            // Recibir disparo del cliente
            int x, y;
            receiveShotFromClient(clientSocket, x, y);

            // Verificar el resultado del disparo
            char result = checkShot(x, y);

            // Enviar el resultado del disparo al cliente
            sendShotResultToClient(clientSocket, result);

            // Cambiar de turno
            currentPlayer = (currentPlayer + 1) % 2;
        }

        // Verificar si el juego ha terminado
        if (isGameOver()) {
            // Enviar mensaje de fin del juego al cliente
            sendGameOverToClient(clientSocket);

            // Mostrar mensaje de fin del juego en el servidor
            cout << "¡Juego terminado! Cliente ha ganado." << endl;
        }

        // Cerrar conexión con el cliente
        close(clientSocket);
    }

    void sendBoardToClient(int clientSocket) {
        // Enviar tamaño del tablero al cliente
        int boardSize = BOARD_SIZE;
        send(clientSocket, &boardSize, sizeof(int), 0);

        // Enviar tablero del servidor al cliente
        send(clientSocket, board, sizeof(board), 0);
    }

    void sendBoardPlayerToClient(int clientSocket) {
        // Enviar tamaño del tablero al cliente
        int boardSize = BOARD_SIZE;
        send(clientSocket, &boardSize, sizeof(int), 0);

        // Enviar tablero del servidor al cliente
        send(clientSocket, clientBoard, sizeof(clientBoard), 0);
    }

    void sendTurnToClient(int clientSocket, int& currentPlayer) {
        // Enviar turno actual al cliente
        send(clientSocket, &currentPlayer, sizeof(int), 0);
    }

    void receiveShotFromClient(int clientSocket, int& x, int& y) {
        // Recibir disparo del cliente
        recv(clientSocket, &x, sizeof(int), 0);
        recv(clientSocket, &y, sizeof(int), 0);
    }

    char checkShot(int x, int y) {
        // Verificar el resultado del disparo
        char result = 'M';  // Marcador de "agua" (miss)

        if (board[x][y] != ' ') {
            // El disparo ha dado en un barco
            result = 'H';  // Marcador de "golpe" (hit)

            // Actualizar el tablero del servidor
            for (int i = 0; i < 8; i++) {
                if (ships[i].symbol == board[x][y]) {
                    ships[i].hits++;
                    if (ships[i].hits == ships[i].size) {
                        // El barco ha sido hundido
                        result = 'X';  // Marcador de "hundido" (sunk)
                    }
                }
            }

            // Actualizar el tablero del cliente
            clientBoard[x][y] = result;
        } else {
            // Actualizar el tablero del cliente
            clientBoard[x][y] = result;
        }

        return result;
    }

    void sendShotResultToClient(int clientSocket, char result) {
        // Enviar el resultado del disparo al cliente
        send(clientSocket, &result, sizeof(char), 0);
    }

    bool isGameOver() {
        // Verificar si todos los barcos han sido hundidos
        for (int i = 0; i < 8; i++) {
            if (ships[i].hits < ships[i].size) {
                return false;
            }
        }
        return true;
    }

    void sendGameOverToClient(int clientSocket) {
        // Enviar mensaje de fin del juego al cliente
        char gameOver = 'G';
        send(clientSocket, &gameOver, sizeof(char), 0);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <puerto>" << endl;
        return 1;
    }

    int port = atoi(argv[1]);

    Server server;
    server.run(port);

    return 0;
}




