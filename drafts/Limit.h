#ifndef LIMIT_H
#define LIMIT_H

class Order; // Forward declaration

class Limit {
public:
    Limit(int price);
    void addOrder(Order* order);
    // Other methods for order removal, execution, etc.

private:
    int price;
    Order* firstOrder;
};

#endif // LIMIT_H