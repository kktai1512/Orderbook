#include "Limit.h"
#include "Order.h"

Limit::Limit(int price) : price(price), firstOrder(nullptr) {}

void Limit::addOrder(Order* order) {
    if (!firstOrder) {
        firstOrder = order;
    } else {
        order->setNextOrder(firstOrder);
        firstOrder->setPrevOrder(order);
        firstOrder = order;
    }
    order->setParentLimit(this);
}

