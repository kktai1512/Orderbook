#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

int main() {
    // create a socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    // specify the server address and port
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);
    
    // connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Connection to server failed." << std::endl;
        return 1;
    }


    
     while (true) {
        // Get order input from the user
        std::cout << "Enter order (side size price trader): ";
        std::string order_input;
        std::getline(std::cin, order_input);

        if (order_input == "exit") {
            break;
        }

        send(client_socket, order_input.c_str(), order_input.size(), 0);

        char buffer[1024] = {0};
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            std::cerr << "Server disconnected." << std::endl;
            break;
        }

        std::cout << "Received updated order book:" << std::endl;
        std::cout << buffer << std::endl;
    }

    close(client_socket);
    return 0;
}

