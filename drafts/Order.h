#ifndef ORDER_H
#define ORDER_H

#include "Limit.h" // Include for the forward declaration

class Order {
public:
    Order(int id, int side, int quantity, int limit);

    void setNextOrder(Order* order);
    void setPrevOrder(Order* order);
    void setParentLimit(Limit* limit);

private:
    int id;
    int side;
    int quantity;
    int limit;
    Order* nextOrder;
    Order* prevOrder;
    Limit* parentLimit;
};

#endif // ORDER_H