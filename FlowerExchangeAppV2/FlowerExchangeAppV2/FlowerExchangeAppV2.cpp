#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <chrono>
#include <ctime>


// Utility function to trim whitespace
std::string trim(const std::string& str) {
    auto start = str.begin();
    while (start != str.end() && std::isspace(*start)) {
        ++start;
    }

    auto end = str.end();
    do {
        --end;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

// Order class
class Order {
public:
    std::string orderId;
    std::string clientOrderId;
    std::string instrument;
    int side;
    int quantity;
    double price;
    int sequenceNumber; // Sequence number to track order insertion

    Order(const std::string& orderId, const std::string& clientOrderId, const std::string& instrument,
        int side, int quantity, double price, int sequenceNumber)
        : orderId(orderId), clientOrderId(clientOrderId), instrument(instrument),
        side(side), quantity(quantity), price(price), sequenceNumber(sequenceNumber) {}
};

// OrderBook class
class OrderBook {
private:
    std::string instrument;
    std::vector<Order> buyOrders;
    std::vector<Order> sellOrders;

public:
    OrderBook() = default;

    OrderBook(const std::string& instrumentName) : instrument(instrumentName) {}

    void addOrder(const Order& order) {
        if (order.side == 1) { // Buy side
            buyOrders.push_back(order);
            std::sort(buyOrders.begin(), buyOrders.end(), [](const Order& a, const Order& b) {
                return (a.price < b.price) || (a.price == b.price && a.sequenceNumber < b.sequenceNumber);
                });
        }
        else { // Sell side
            sellOrders.push_back(order);
            std::sort(sellOrders.begin(), sellOrders.end(), [](const Order& a, const Order& b) {
                return (a.price > b.price) || (a.price == b.price && a.sequenceNumber < b.sequenceNumber);
                });
        }
    }

    std::vector<Order>& getBuyOrders() { return buyOrders; }
    std::vector<Order>& getSellOrders() { return sellOrders; }
};

// ExecutionReport class
class ExecutionReport {
public:
    static void generateReport(const std::string& orderId, const std::string& clientOrderId, const std::string& instrument,
        int side, const std::string& executionStatus, int quantity, double price) {
        std::ofstream file("Execution_Rep.csv", std::ios::app);
        if (file.is_open()) {
            file << orderId << "," << clientOrderId << "," << instrument << "," << side << ","
                << executionStatus << "," << quantity << "," << price << "\n";
            file.close();
        }
        else {
            std::cerr << "Error: Unable to open Execution_Rep.csv for writing." << std::endl;
        }
    }
};

// Exchange class
class Exchange {
private:
    std::unordered_map<std::string, OrderBook> orderBooks;
    int orderIdCounter = 1;       // To generate unique Order IDs (ord1, ord2, ...)
    int sequenceCounter = 1;      // Sequence counter for orders
    std::unordered_set<std::string> validInstruments = { "Rose", "Orchid", "Lavender", "Tulip", "Lotus" };

public:
    Exchange() {
        orderBooks.emplace("Rose", OrderBook("Rose"));
        orderBooks.emplace("Lavender", OrderBook("Lavender"));
        orderBooks.emplace("Lotus", OrderBook("Lotus"));
        orderBooks.emplace("Tulip", OrderBook("Tulip"));
        orderBooks.emplace("Lily", OrderBook("Orchid"));
    }

    void processCSV(const std::string& filePath) {
        std::ifstream file(filePath);
        std::string line;

        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filePath << std::endl;
            return;
        }

        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string clientOrderId, instrument, sideStr, quantityStr, priceStr;
            int side, quantity;
            double price;

            // Extract values based on commas
            std::getline(ss, clientOrderId, ',');
            std::getline(ss, instrument, ',');
            std::getline(ss, sideStr, ',');
            std::getline(ss, quantityStr, ',');
            std::getline(ss, priceStr, ',');

            // Trim the instrument name
            instrument = trim(instrument);

            // Check for empty instrument
            if (instrument.empty()) {
                std::cout << "Empty instrument for order: " << clientOrderId << ". Order rejected." << std::endl;
                ExecutionReport::generateReport("ord" + std::to_string(orderIdCounter++), clientOrderId, instrument, 0, "Rejected", 0, 0.0);
                continue; // Skip to the next line
            }

            // Convert extracted strings to proper types
            try {
                side = std::stoi(sideStr);
                quantity = std::stoi(quantityStr);
                price = std::stod(priceStr);
            }
            catch (...) {
                std::cout << "Invalid order format for order: " << clientOrderId << ". Order rejected." << std::endl;
                ExecutionReport::generateReport("ord" + std::to_string(orderIdCounter++), clientOrderId, instrument, 0, "Rejected", 0, 0.0);
                continue;
            }

            // Validate side
            if (side != 1 && side != 2) {
                std::cout << "Invalid side (" << side << ") for order: " << clientOrderId << ". Order rejected." << std::endl;
                ExecutionReport::generateReport("ord" + std::to_string(orderIdCounter++), clientOrderId, instrument, side, "Rejected", quantity, price);
                continue;
            }

            // Validate quantity
            if (quantity < 10 || quantity > 1000 || quantity % 10 != 0) {
                std::cout << "Invalid quantity (" << quantity << ") for order: " << clientOrderId << ". Order rejected." << std::endl;
                ExecutionReport::generateReport("ord" + std::to_string(orderIdCounter++), clientOrderId, instrument, side, "Rejected", quantity, price);
                continue;
            }

            if (price <= 0.0) {
                std::cout << "Invalid price (" << price << ") for order: " << clientOrderId << ". Order rejected." << std::endl;
                ExecutionReport::generateReport("ord" + std::to_string(orderIdCounter++), clientOrderId, instrument, side, "Rejected", quantity, price);
                continue;
            }

            std::string orderId = "ord" + std::to_string(orderIdCounter++);

            // Validate instrument against the list of valid instruments
            if (validInstruments.find(instrument) == validInstruments.end()) {
                std::cout << "Invalid instrument: " << instrument << " for order: " << clientOrderId << ". Order rejected." << std::endl;
                ExecutionReport::generateReport(orderId, clientOrderId, instrument, side, "Rejected", quantity, price);
                continue;
            }

            // Log valid order
            std::cout << "Valid instrument: " << instrument << " for order: " << clientOrderId << std::endl;

            // Create a new order with a sequence number
            Order order(orderId, clientOrderId, instrument, side, quantity, price, sequenceCounter++);

            // Check for immediate match before adding as a "New" order
            bool matched = checkAndExecuteTrades(orderBooks[instrument], order, instrument);

            // If no immediate match is found, add as a new order
            if (!matched) {
                orderBooks[instrument].addOrder(order);
                ExecutionReport::generateReport(orderId, clientOrderId, instrument, side, "New", quantity, price);
            }
        }

        file.close();
    }





    bool checkAndExecuteTrades(OrderBook& orderBook, Order& newOrder, const std::string& instrument) {
        auto& buyOrders = orderBook.getBuyOrders();
        auto& sellOrders = orderBook.getSellOrders();
        bool tradeExecuted = false;

        // Check if the new order can match with existing orders in the order book
        if (newOrder.side == 1 && !sellOrders.empty() && newOrder.price >= sellOrders.back().price) {
            tradeExecuted = true;
        }
        else if (newOrder.side == 2 && !buyOrders.empty() && newOrder.price <= buyOrders.back().price) {
            tradeExecuted = true;
        }

        if (tradeExecuted) {
            // Add the new order to the order book first to ensure it's available for trade execution
            orderBook.addOrder(newOrder);
            executeTrades(orderBook, instrument);
        }

        return tradeExecuted;
    }

    void executeTrades(OrderBook& orderBook, const std::string& instrument) {
        auto& buyOrders = orderBook.getBuyOrders();
        auto& sellOrders = orderBook.getSellOrders();

        while (!buyOrders.empty() && !sellOrders.empty() &&
            buyOrders.back().price >= sellOrders.back().price) {

            Order& buyOrder = buyOrders.back();
            Order& sellOrder = sellOrders.back();

            int tradeQuantity = std::min(buyOrder.quantity, sellOrder.quantity);

            // Compare the sequence numbers and select the price of the older order
            double tradePrice = (buyOrder.sequenceNumber < sellOrder.sequenceNumber) ? buyOrder.price : sellOrder.price;

            // Generate execution reports
            ExecutionReport::generateReport(buyOrder.orderId, buyOrder.clientOrderId, instrument, 1,
                (buyOrder.quantity == tradeQuantity) ? "Fill" : "Pfill",
                tradeQuantity, tradePrice);

            ExecutionReport::generateReport(sellOrder.orderId, sellOrder.clientOrderId, instrument, 2,
                (sellOrder.quantity == tradeQuantity) ? "Fill" : "Pfill",
                tradeQuantity, tradePrice);

            // Update quantities after the trade
            buyOrder.quantity -= tradeQuantity;
            sellOrder.quantity -= tradeQuantity;

            // Remove orders that are fully filled (quantity = 0)
            if (buyOrder.quantity == 0) {
                buyOrders.pop_back();  // Fully filled order, remove from buy side
            }

            if (sellOrder.quantity == 0) {
                sellOrders.pop_back();  // Fully filled order, remove from sell side
            }
        }
    }
};

// Main function
int main() {
    Exchange exchange;
    std::string inputFilePath = "orders.csv"; // Replace with the actual path to your input CSV file
    exchange.processCSV(inputFilePath);

    std::cout << "Order processing completed. Execution report generated in Execution_Rep.csv" << std::endl;

    return 0;
}
