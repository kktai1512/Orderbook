#include <iostream>
#include <vector>
#include <deque>
#include <sstream>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// note that at any price points -> there can only be either buy or sell, cant be both together -> if they are both together-> one of them will be matched and exhausted ->leaving only one, so there's no way to have both together.
struct Order {
    Order(int side, int size, int price, const std::string& trader, int order_id)
        : side(side), size(size), price(price), trader(trader), order_id(order_id) {}

    int side;
    int size;
    int price;
    std::string trader;
    int order_id;
};

class OrderBook {
public:
    std::vector<std::string> execution_messages;
    OrderBook(const std::string& name, int max_price = 10000, int start_order_id = 0)
        : name(name), order_id(start_order_id), max_price(max_price), ask_min(max_price + 1), bid_max(0) {
        price_points.resize(max_price);
    }

    void execute(const std::string& buyer, const std::string& seller, int price, int size) {
        std::string message = "EXECUTE: " + buyer + " BUY " + seller + " SELL " + std::to_string(size) + " @ " + std::to_string(price);
    execution_messages.push_back(message);
        execution_messages.push_back(message);
    }

    int limit_order(int side, int size, int price, const std::string& trader) {
        std::lock_guard<std::mutex> lock(bookMutex); // Lock the mutex
        if (side == 0) {
            //  while the order price is greater than the minimum ask price, means there are sellers
            // we search from minimum ask, then get entry at that level, if we have entries - we check quantity - we try to match with the entries from the start
            while (price >= ask_min) {
                auto& entries = price_points[ask_min];
                while (!entries.empty()) {
                    auto& entry = entries.front();
                    // current order is more than entry, deduct size from current, and pop from before
                    if (entry.size < size) {
                        execute(trader, entry.trader, price, entry.size);
                        size -= entry.size;
                        entries.pop_front();
                    } else {
                      // current order size is less or equal than prev entry, deduct entry size from entry or pop from entry
                        execute(trader, entry.trader, price, size);
                        if (entry.size > size) {
                            entry.size -= size;
                        } else {
                            entries.pop_front();
                        }
                        // actually not used -> we don't append anything to our orders
                        order_id++;
                        return order_id;
                    }
                }
                ask_min++;
            }

            // we will only store the order if we exhaust all matchable entries. store the remaining unmatched ones.
            order_id++;
            price_points[price].push_back(Order(side, size, price, trader, order_id));
            if (bid_max < price) {
                bid_max = price;
            }
            return order_id;


        } else { // Sell order
            while (price <= bid_max) {
                auto& entries = price_points[bid_max];
                while (!entries.empty()) {
                    auto& entry = entries.front();
                    if (entry.size < size) {
                        execute(entry.trader, trader, price, entry.size);
                        size -= entry.size;
                        entries.pop_front();
                    } else {
                        execute(entry.trader, trader, price, size);
                        if (entry.size > size) {
                            entry.size -= size;
                        } else {
                            entries.pop_front();
                        }
                        order_id++;
                        return order_id;
                    }
                }
                bid_max--;
            }
            order_id++;
            price_points[price].push_back(Order(side, size, price, trader, order_id));
            if (ask_min > price) {
                ask_min = price;
            }
            return order_id;
        }
    }

    std::string render() {
        std::stringstream output;
        output << std::string(110, '-') << std::endl;
        output << "Buyers" << std::string(50, ' ') << " | " << "Sellers" << std::string(50, ' ') << std::endl;
        output << std::string(110, '-') << std::endl;
        for (int price = max_price - 1; price > 0; --price) {
            auto& level = price_points[price];
            if (!level.empty()) {
                std::string left_price, left_orders, right_price, right_orders;
                if (price >= ask_min) {
                    right_price = std::to_string(price);
                    right_orders = _render_level(level);
                } else {
                    left_price = std::to_string(price);
                    left_orders = _render_level(level);
                }
                output << left_orders << std::string(43 - left_orders.size(), ' ') << " | " << left_price << std::string(10 - left_price.size(), ' ');
                output << " | " << right_price << std::string(10 - right_price.size(), ' ') << " | " << right_orders << std::endl;
                output << std::string(110, '-') << std::endl;
            }
        }
        return output.str();
    }

private:
    std::vector<std::deque<Order> > price_points;
    std::string name;
    int order_id;
    int max_price;
    int ask_min;
    int bid_max;
    std::mutex bookMutex; // Mutex to protect shared data
   

    std::string _render_level(const std::deque<Order>& level, int maxlen = 40) {
        std::string ret;
        for (const auto& order : level) {
            ret += "(ID: " + std::to_string(order.order_id) + ")" + "-" + order.trader +  '@' + std::to_string(order.size) + ", ";
        }
        if (ret.size() > maxlen) {
            ret = std::to_string(level.size()) + " orders (total size " + std::to_string(_calculate_total_size(level)) + ")";
            if (ret.size() > maxlen) {
                ret = ret.substr(0, maxlen);
            }
        }
        return ret;
    }

    int _calculate_total_size(const std::deque<Order>& level) {
        int total_size = 0;
        for (const auto& order : level) {
            total_size += order.size;
        }
        return total_size;
    }
};


void handleClient(int client_socket, OrderBook &ob) {
    // Receive client requests and process orders
    while (true) {
        char buffer[1024] = {0};
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            std::cout << "Client disconnected." << std::endl;
            break;
        }

        // Parse received data and extract order information
        // Example format: "0 3 200 Kim"
        int side, size, price;
        std::string trader;
        std::stringstream ss(buffer);
        ss >> side >> size >> price >> trader;

        // Process the received order
        ob.limit_order(side, size, price, trader);


        // Send the updated order book to the client
        std::string orderBookData = ob.render();
        std::string dataToSend = orderBookData;

        if (!ob.execution_messages.empty()) {
            std::string exec_message = ob.execution_messages.back();
            dataToSend += "\n" + exec_message;
            ob.execution_messages.pop_back(); // Remove the sent message from the vector
        }

        send(client_socket, dataToSend.c_str(), dataToSend.size(), 0);

    }

    close(client_socket);
}

int main() {
    OrderBook ob("AAPL");

    // Create a socket for the server
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080);

    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(server_socket, 3);

    while (true) {
        int client_socket;
        struct sockaddr_in client_address;
        socklen_t addrlen = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &addrlen);

        if (client_socket < 0) {
            std::cerr << "Failed to accept client connection." << std::endl;
            continue;
        }

        std::thread client_thread(handleClient, client_socket, std::ref(ob));
        client_thread.detach();
    }

    close(server_socket);
    return 0;
}