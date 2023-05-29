#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BOARD_SIZE 15

using namespace std;

// Clase del cliente
class Client
{
private:
    int clientSocket;
    struct sockaddr_in serverAddress;

    char clientVisibleBoard[BOARD_SIZE][BOARD_SIZE]; // Tablero visible para el cliente (disparos)
    char clientBoard[BOARD_SIZE][BOARD_SIZE];        // Tablero del cliente (embarcaciones)

public:
    void run(const char *ipAddress, int port)
    {
        initializeClient(ipAddress, port);
        receiveBoardFromServer();
        

        while (true)
        {
            displayBoards();
            int currentPlayer;
            receiveTurnFromServer(currentPlayer);
            
            if (currentPlayer == 0)
            {
                // Turno del servidor
                cout << "Es el turno del servidor." << endl;
                sleep(2);
                int x = rand() % BOARD_SIZE;
                int y = rand() % BOARD_SIZE;
                sendShotToServer(x, y);
                receiveShotResultFromServer(x,y);
            }
            else
            {
                // Turno del cliente
                cout << "Es tu turno. Ingresa las coordenadas del disparo (x y): ";
                int x, y;
                cin >> x >> y;
                sendShotToServer(x, y);
                receiveShotResultFromServer(x,y);
            }
        }

        // Cerrar conexiÃ³n con el servidor
        close(clientSocket);
    }

private:
    void initializeClient(const char *ipAddress, int port)
    {
        // Crear socket
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket < 0)
        {
            cerr << "Error al crear el socket." << endl;
            exit(1);
        }

        // Configurar la direcciÃ³n del servidor
        memset((char *)&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        if (inet_pton(AF_INET, ipAddress, &(serverAddress.sin_addr)) <= 0)
        {
            cerr << "Direccion IP invalida o no soportada." << endl;
            exit(1);
        }

        // Conectar al servidor
        if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
        {
            cerr << "Error al conectar con el servidor." << endl;
            exit(1);
        }
    }

    void receiveBoardFromServer()
    {
        // Recibir tamaÃ±o del tablero del servidor
        int boardSize;
        recv(clientSocket, &boardSize, sizeof(int), 0);

        // Recibir el tablero del cliente
        recv(clientSocket, clientBoard, sizeof(clientBoard), 0);
        // Recibir el tablero visible del cliente
        recv(clientSocket, clientVisibleBoard, sizeof(clientVisibleBoard), 0);
    }

    void displayBoards()
    {
        // Mostrar tablero de disparos
        cout << "Tablero de disparos:" << endl;
        cout << "  ";
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            cout << i << "  ";
        }
        cout << endl;
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            cout << i << "  ";
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                cout << clientVisibleBoard[i][j] << " ";
            }
            cout << endl;
        }

        // Mostrar tablero de embarcaciones del cliente
        cout << "Tablero de tus embarcaciones:" << endl;
        cout << "  ";
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            cout << i << "  ";
        }
        cout << endl;
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            cout << i << "  "; 
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                cout << clientBoard[i][j] << " ";
            }
            cout << endl;
        }
    }

    void receiveTurnFromServer(int& currentPlayer) {
        // Recibir turno actual del servidor
        recv(clientSocket, &currentPlayer, sizeof(short),0);
    }

    void sendShotToServer(int x, int y)
    {
        // Enviar el disparo al servidor
        send(clientSocket, &x, sizeof(int), 0);
        send(clientSocket, &y, sizeof(int), 0);
    }

    void receiveShotResultFromServer(int x,int y )
    {
        // Recibir resultado del disparo
        char result;
        recv(clientSocket, &result, sizeof(char), 0);
        clientVisibleBoard[x][y] = result;

        // Mostrar el resultado del disparo en el cliente
        if (result == 'M')
        {
            cout << "Agua." << endl;
        }
        else if (result == 'H')
        {
            cout << "Golpe." << endl;
        }
        else if (result == 'X')
        {
            cout << "Hundido." << endl;
        }
    }
};

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cerr << "Uso: " << argv[0] << " <direccion IP> <puerto>" << endl;
        return 1;
    }

    const char *ipAddress = argv[1];
    int port = atoi(argv[2]);

    Client client;
    client.run(ipAddress, port);

    return 0;
}