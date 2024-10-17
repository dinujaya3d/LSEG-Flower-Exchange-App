#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <ctime>

// Order class
class Order {
public:
    std::string orderId;
    std::string clientOrderId;
    std::string instrument;
    std::string side;
    int quantity;
    double price;
    std::time_t timestamp;

    Order(const std::string& orderId, const std::string& clientOrderId, const std::string& instrument,
        const std::string& side, int quantity, double price, std::time_t timestamp)
        : orderId(orderId), clientOrderId(clientOrderId), instrument(instrument),
        side(side), quantity(quantity), price(price), timestamp(timestamp) {}
};

// OrderBook class
class OrderBook {
private:
    std::string instrument;
    std::vector<Order> buyOrders;
    std::vector<Order> sellOrders;

public:
    // Default constructor
    OrderBook() = default;

    // Constructor with instrument parameter
    OrderBook(const std::string& instrumentName) : instrument(instrumentName) {}

    void addOrder(const Order& order) {
        if (order.side == "buy") {
            buyOrders.push_back(order);
            std::sort(buyOrders.begin(), buyOrders.end(), [](const Order& a, const Order& b) {
                return (a.price > b.price) || (a.price == b.price && a.timestamp < b.timestamp);
                });
        }
        else {
            sellOrders.push_back(order);
            std::sort(sellOrders.begin(), sellOrders.end(), [](const Order& a, const Order& b) {
                return (a.price < b.price) || (a.price == b.price && a.timestamp < b.timestamp);
                });
        }
    }
};

// ExecutionReport class
class ExecutionReport {
public:
    static void generateReport(const std::string& orderId, const std::string& clientOrderId, const std::string& instrument,
        const std::string& side, const std::string& executionStatus, int quantity, double price) {
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
    int orderIdCounter = 1; // To generate unique Order IDs (ord1, ord2, ...)

public:
    Exchange() {
        // Explicitly create each OrderBook object with the instrument name
        orderBooks.emplace("Rose", OrderBook("Rose"));
        orderBooks.emplace("Lavender", OrderBook("Lavender"));
        orderBooks.emplace("Lotus", OrderBook("Lotus"));
        orderBooks.emplace("Tulip", OrderBook("Tulip"));
        orderBooks.emplace("Orchid", OrderBook("Orchid"));
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
            std::string clientOrderId, instrument, sideStr;
            int side, quantity;
            double price;
            std::time_t timestamp = std::time(nullptr);

            std::getline(ss, clientOrderId, ',');
            std::getline(ss, instrument, ',');
            std::getline(ss, sideStr, ',');
            side = std::stoi(sideStr);
            ss >> quantity >> price;

            std::string sideType = (side == 1) ? "buy" : "sell";
            std::string orderId = "ord" + std::to_string(orderIdCounter++);

            Order order(orderId, clientOrderId, instrument, sideType, quantity, price, timestamp);
            orderBooks[instrument].addOrder(order);

            // Generate an execution report with status "New" for newly added orders
            ExecutionReport::generateReport(orderId, clientOrderId, instrument, sideType, "New", quantity, price);
        }

        file.close();
    }
};

// Main function
int main() {
    Exchange exchange;
    std::string inputFilePath = "Orders.csv"; // Replace with the actual path to your input CSV file
    exchange.processCSV(inputFilePath);

    std::cout << "Order processing completed. Execution report generated in Execution_Rep.csv" << std::endl;

    return 0;
}
