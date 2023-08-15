#include <unordered_map>
#include <map>
class Order; // forward declaration
class Limit {
    // maintains a doubly linked list of orders
    public:
        Order* head;
        Order* tail;

        void addOrder(Order* order){
            if (!head) { // if we don't have any orders yet
                head = order;
                tail = order;
            }
            else {
                tail->setNextOrder(order);
                tail = tail->getNextOrder();
            }
        }
};

class Order {
    private:
        int id;
        int side; // 0 for buy, 1 for sell
        int price;
        int quantity;
        Order* nextOrder{nullptr};
        Order* prevOrder{nullptr};

    public:
        Order(int id, int side, int price, int quantity): 
        id{id}, side{side}, price{price}, quantity{quantity} {}

        int getQuantity() {
            return quantity;
        }

        int setQuantity(int quantity) {
            this->quantity = quantity;
        }

        void setNextOrder(Order* order) {
            nextOrder = order;
       }

        void setPrevOrder(Order* order ) {
            prevOrder = order;
        }

        Order* getNextOrder() {
            return this -> nextOrder;
        }

        Order* getPrevOrder() {
            return this -> prevOrder;
        }

        // todo add change price
};


struct TreeNode {
    int 
}
struct LimitTree {
    std::unordered_map<int, Limit> priceMap;
    std::map

};
class OrderBook {
    private:
        LimitTree bids; // buy side
        LimitTree asks; // sell side
        //some map for easy cancellation

    public:
        OrderBook(): bids{}, asks{} {}


        // add a new buy order to the book
        void addBuyOrder(Order* buyOrder)
}